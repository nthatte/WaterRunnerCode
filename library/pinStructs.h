#ifndef PIN_STRUCTS_H
#define PIN_STRUCTS_H

#include <stdint.h>

struct HCTL2032SCPinList
{
    uint8_t SEL1;
    uint8_t SEL2;
    uint8_t EN1;
    uint8_t EN2;
    uint8_t OE;
    uint8_t RST;
    uint8_t XY;
    volatile uint8_t *PINREG;
    volatile uint8_t *DDRREG;
};
#endif
