#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipParser.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef struct {
  uint8_t version_IHL;
  uint8_t tos;
  uint16_t totaLength;
  uint16_t identification;
  uint16_t flagsFragment;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t checksum;
  uint32_t srcAddr;
  uint32_t dstAddrs;
} __attribute__((packed)) ipv4Header_t;
