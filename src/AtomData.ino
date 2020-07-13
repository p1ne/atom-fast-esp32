#include "AtomData.h"

float getFloat(uint8_t* pData, uint8_t startIdx) {
  union {
    float val;
    uint8_t b[4];
  } float_bytes_u;

  for (uint8_t i=0;i<4;i++) {
    float_bytes_u.b[i] = pData[i+startIdx];
  }

  return float_bytes_u.val;
}

uint16_t getWord(uint8_t* pData, uint8_t startIdx) {
  union {
    uint16_t val;
    uint8_t b[2];
  } uint16_bytes_u;

  uint16_bytes_u.b[0] = pData[startIdx];
  uint16_bytes_u.b[1] = pData[startIdx+1];

  return uint16_bytes_u.val;
}

AtomDataFlags getFlags(uint8_t* pData) {
  union {
    AtomDataFlags val;
    uint8_t b;
  } flags_bytes_u;

  flags_bytes_u.b = pData[0];

  return flags_bytes_u.val;
}

void parseAtomData(AtomData& atomData, uint8_t* pData) {
  atomData.flags = getFlags(pData);
  atomData.doseAccumulated = getFloat(pData, 1);
  atomData.dosePower = getFloat(pData, 5);
  atomData.impulses = getWord(pData, 9);
  atomData.charge = pData[11];
  atomData.temperature = pData[12];
}
