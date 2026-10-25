
#include "ipParser.h"

_Static_assert(sizeof(ipv4Header_t) == 20,
               "IPv4 header must be at least 20 bytes");

static uint8_t ipVersion(ipv4Header_t *header) {
  return (header->version_IHL >> 4) & 0x0F;
}

static uint8_t ipIHL(ipv4Header_t *header) {
  return header->version_IHL & 0x0F;
}

static uint8_t ipDontFrag(ipv4Header_t *header) {
  // returns 1 if frag is prohibitited, 0 if allowed
  return (ntohs(header->flagsFragment) >> 14) & 0x1;
}

static uint8_t ipMoreFrag(ipv4Header_t *header) {
  // returns 1 if more framgents follow, 0 if last
  return (ntohs(header->flagsFragment) >> 13) & 0x1;
}

static uint16_t ipFragOffset(ipv4Header_t *header) {
  return ntohs(header->flagsFragment) & 0x1FFF;
}

static const char *protocolName(uint8_t protocol) {
  switch (protocol) {
  case 1:
    return "ICMP";
  case 6:
    return "TCP";
  case 17:
    return "UDP";
  default:
    return "other";
  }
}

static int parseIP(uint8_t *buffer, size_t len, ipv4Header_t *out) {
  if (len < 20) {
    printf("packet must be at least 20 bytes(%zu)\n", len);
    return -1;
  }

  memcpy(out, buffer, sizeof(*out));
  uint8_t version = ipVersion(out);
  if (version != 4) {
    printf("not an IPv4 packet (version=%u)\n",
           version); // saving version to the stack is better than making two
                     // calls to the ipV func(?)
  }

  if (ipIHL(out) < 5 || (size_t)ipIHL(out) * 4 > len) {
    printf("invalid IHL = %u\n", ipIHL(out)); // trying both options
    return -1;
  }

  return 0;
}

static void printHeader(ipv4Header_t *header) {
  char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &header->srcAddr, src, sizeof(src));
  inet_ntop(AF_INET, &header->dstAddrs, dst, sizeof(dst));

  printf("Version        : %u\n", ipVersion(header));
  printf("Header length  : %u words (%u bytes)\n", ipIHL(header),
         ipIHL(header) * 4);
  printf("Total Length   : %u bytes\n", ntohs(header->totaLength));
  printf("Identification : 0x%04x\n", ntohs(header->identification));
  printf("Flags          : DF = %u, MF = %u, fragment offset = %u\n",
         ipDontFrag(header), ipMoreFrag(header), ipFragOffset(header));
  printf("TTL            : %u\n", header->ttl);
  printf("Protocol       : %u (%s)\n", header->protocol,
         protocolName(header->protocol));
  printf("Checksum       : 0x%04x\n", ntohs(header->checksum));
  printf("Source IP      : %s\n", src);
  printf("Destination IP : %s\n", dst);
}

static void simulateTtl(uint8_t ttl) {
  printf("\nTTL sim");
  int hop = 0;
  while (ttl > 0) {
    ttl--;
    hop++;
    printf("hop %2d: TTL now %3u%s\n", hop, ttl,
           ttl == 0 ? "\nTTl expired, packet dropped" : "");
  }
}

int main() {
  uint8_t packet[] = {
      0x45, 0x00, 0x00, 0x3c, // ver/IHL, TOS, total length = 60
      0x1c, 0x46, 0x40, 0x00, // ID = 0x1c46, flags DF, frag offset 0
      0x40, 0x06, 0xb1, 0xe6, // TTL = 64, protocol = 6 (TCP), checksum
      0xc0, 0xa8, 0x01, 0x0a, // src 192.168.1.10
      0x5d, 0xb8, 0xd8, 0x22  // dst 93.184.216.34
  }; // hardcoded packet for testing

  ipv4Header_t header;
  if (parseIP(packet, sizeof(packet), &header) != 0) {
    return 1;
  }

  printHeader(&header);

  // simulateTtl(header.ttl);

  return 0;
}
