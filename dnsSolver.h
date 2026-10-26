#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// uhhh useful macros

#define COMPRESSED_MASK 0xC0

// harcoded values both address andd associated name for testing purposes

static const char *WEBSITE_NAME = "koolDomain.cum";
static const uint8_t WEBSITE_ADDR[] = {93, 184, 216, 34};

#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdio.h>

#include <netinet/in.h>
#include <stdint.h>
#include <string.h>

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PACKET 512
#define TYPE_A 1
#define CLASS_IN 1
#define RCODE_NOERR 0
#define Rcode_NXDOMAIN 3
#define ANSWER_TTl 300
#define FLAG_QR 0x8000
#define FLAG_AA 0x0400
#define FLAG_RD 0x0100
#define FLAG_RA 0x0080
#define FLAG_OPCODE(f) (((f) >> 11) & 0x0F)
#define FLAG_RCODE(f) ((f) & 0x0F)

typedef struct {

  uint16_t id;
  uint16_t flags;
  uint16_t questionCount;
  uint16_t answerCount;
  uint16_t authorityCount;
  uint16_t additionalCount;

} __attribute__((packed)) dnsHeader_t;

_Static_assert(sizeof(dnsHeader_t) == 12, "DNS header shall be 12 bytes");
