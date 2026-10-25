
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
