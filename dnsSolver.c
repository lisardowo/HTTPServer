
#include "dnsSolver.h"

static int getQname(const uint8_t *buff, size_t len, size_t off, char *out,
                    size_t outSize) {
  size_t pos = off, outposition = 0;

  for (;;) {
    if (pos >= len)
      return -1;
    uint8_t labelLen = buff[pos];

    if (labelLen & COMPRESSED_MASK)
      return -1; // Avoiding compression support rn

    if (labelLen == 0) {
      pos++;
      break;
    }

    if (pos + 1 + labelLen > len)
      return -1;

    if (outposition != 0) {
      if (outposition + 1 >= outSize)
        return -1;
      out[outposition++] = '.';
    }

    if (outposition + labelLen >= outSize)
      return -1;
    memcpy(out + outposition, buff + pos + 1, labelLen);
    outposition += labelLen;
    pos += 1 + labelLen;
  }
  out[outposition] = '\0';
  return (int)(pos - off);
}

static int equalDnsName(const char *a, const char *b) {
  // DNS is case insensitive
  while (*a && *b) {
    char charA = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
    char charB = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
    if (charA != charB)
      return 0;
    a++;
    b++;
  }
  return *a == '\0' && *b == '\0';
}

static void putU16(uint8_t *packet, uint16_t value) {
  // BIG endian packet composer
  packet[0] = (uint8_t)(value >> 8);   // takes first 8 bits
  packet[1] = (uint8_t)(value & 0xFF); // cleans and leaves only last 8 bits
}

static void putU32(uint8_t *packet, uint32_t value) {
  packet[0] = (uint8_t)(value >> 24); // same as function above but 4 32 bits
  packet[1] = (uint8_t)(value >> 16);
  packet[2] = (uint8_t)(value >> 8);
  packet[3] = (uint8_t)(value & 0xFF);
}

static int buildResponse(const uint8_t *request, size_t qlen, uint8_t *response,
                         size_t responseSize) {
  if (qlen < sizeof(dnsHeader_t))
    return -1;

  dnsHeader_t header;
  memcpy(&header, request, sizeof(header));

  uint16_t id = ntohs(header.id);
  uint16_t flags = ntohs(header.flags);
  uint16_t questionCount = ntohs(header.questionCount);

  if (FLAG_OPCODE(flags) != 0 || (flags & FLAG_QR) || questionCount != 1)
    return -1; // Only handling requests w 1 question

  char qname[256];
  int nameBytes = getQname(request, qlen, sizeof(header), qname, sizeof(qname));
  if (nameBytes < 0)
    return -1;

  size_t qtypeOff = sizeof(header) + (size_t)nameBytes;

  if (qtypeOff + 4 > qlen)
    return -1;
  uint16_t qType = (uint16_t)((request[qtypeOff] << 8) | request[qtypeOff + 1]);
  uint16_t qClass =
      (uint16_t)((request[qtypeOff + 2] << 8) | request[qtypeOff + 3]);
  size_t questionEnd = qtypeOff + 4;

  int match = equalDnsName(qname, WEBSITE_NAME) && qType == TYPE_A &&
              qClass == CLASS_IN;
  int answer = match;

  size_t outLen = questionEnd;
  if (outLen > responseSize)
    return -1;
  memcpy(response, request, questionEnd);

  uint16_t responseFlags = FLAG_QR | FLAG_AA | (flags & FLAG_RD) | FLAG_RA;
  responseFlags |= answer ? RCODE_NOERR : Rcode_NXDOMAIN;

  putU16(response + 0, id);
  putU16(response + 2, responseFlags);
  putU16(response + 4, 1);
  putU16(response + 6, answer ? 1 : 0);
  putU16(response + 8, 0);
  putU16(response + 10, 0);

  if (answer) {
    if (outLen + 16 > responseSize)
      return -1;
    uint8_t *a = response + outLen;

    putU16(a + 0, 0xC00C);
    putU16(a + 2, TYPE_A);
    putU16(a + 4, CLASS_IN); // TODO could colapse this to a func to save space
    putU32(a + 6, ANSWER_TTl);
    putU16(a + 10, 4);
    memcpy(a + 12, WEBSITE_ADDR, 4);
    outLen += 16;
  }
  return (int)outLen;
}
static void printRequestSummary(const uint8_t *request, size_t qlen,
                                const char *who) {
  dnsHeader_t header;
  memcpy(&header, request, sizeof(header));
  char qname[256];
  int n = getQname(request, qlen, sizeof(header), qname, sizeof(qname));
  if (n < 0) {
    printf("%s: malformed request\n", who);
    return;
  }
  uint16_t qtype = (uint16_t)((request[sizeof(header) + n] << 8) |
                              request[sizeof(header) + n + 1]);
  printf("%s: id=0x%04x query %s type=%u\n", who, ntohs(header.id), qname,
         qtype);
}

int main() {
  int port = 5353;
  setvbuf(stdout, NULL, _IOLBF, 0);

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    return 1;
  }

  int enable = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons((uint16_t)port);

  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(sock);
    return 1;
  }

  printf("DNS up on port %d (example web: %s)\n", port, WEBSITE_NAME);

  uint8_t request[MAX_PACKET], response[MAX_PACKET];

  for (;;) {
    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    ssize_t n = recvfrom(sock, request, sizeof(request), 0,
                         (struct sockaddr *)&client, &clientLen);

    if (n < 0) {
      perror("recvfrom");
      continue;
    }
    char who[64];

    snprintf(who, sizeof(who), "%s:%d", inet_ntoa(client.sin_addr),
             ntohs(client.sin_port));
    printRequestSummary(request, (size_t)n, who);

    int responseLen =
        buildResponse(request, (size_t)n, response, sizeof(response));
    if (responseLen < 0) {
      printf("\nunsuported/malformed\n");
      continue;
    }
    sendto(sock, response, (size_t)responseLen, 0, (struct sockaddr *)&client,
           clientLen);
    printf("  -> replied %d bytes, rcode=%u\n", responseLen,
           FLAG_RCODE(ntohs(*(uint16_t *)(response + 2))));
  }

  close(sock);
  return 0;
}
