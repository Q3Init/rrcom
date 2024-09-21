#ifndef CRC_H
#define CRC_H

#include "Platform_Types.h"

class CRC
{
public:
    CRC();

    uint16 apu_CRC16(uint8 *puchMsg, uint16 usDataLen);
};

#endif // CRC_H
