#ifndef __ATOM_DATA_H_
#define __ATOM_DATA_H_

struct AtomDataFlags {
  uint8_t doseExceed: 1;
  uint8_t dosePowerExceed: 1;
  uint8_t countSpeedInc: 1;
  uint8_t spare: 1;
  uint8_t currentExceed: 1;
  uint8_t detectorOverload: 1;
  uint8_t chargerConnected: 1;
  uint8_t emergencyShutdown: 1;
};

struct AtomData {
    AtomDataFlags flags;
    float doseAccumulated;
    float dosePower;
    uint16_t impulses;
    uint8_t charge;
    int8_t temperature;
};

AtomData currentAtomData;

float getFloat(uint8_t* pData, uint8_t startIdx);
uint16_t getWord(uint8_t* pData, uint8_t startIdx);
AtomDataFlags getFlags(uint8_t* pData);
void parseAtomData(AtomData& atomData, uint8_t* pData);

#endif // __ATOM_DATA_H_
