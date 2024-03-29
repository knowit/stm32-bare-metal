#pragma once
#include <stdint.h>

const int MyROM_size = 256;
const uint8_t MyROM[256] = {
  //  code and opcodes generated in Commodore 128's emulator, in Monitor (old habits die hard)

  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xA9, 0x00,         //  LDA#00
  0x20, 0x00, 0x08,   //  JSR $1000
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0x20, 0x03, 0x10,   //  JSR $1003 ($030c)
  0x18,               // CLC
  0x90, 0xF8,         // BCC 030C
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP
  0xEA,               //  NOP


  0x4C, 0x0E, 0xFF,   //  JMP $ff0e

  0xEA, 0xA2, 0x00, 0xA0, 0x00, 0xC8, 0xC0, 0xFF, 0xD0,
  0xFB, 0xE8, 0xE0, 0xFF, 0xD0, 0xF4, 0xEA, 0xEA, 0xEA, 0xA2, 0x00, 0xA0, 0x00, 0xC8, 0xC0, 0xFF,
  0xD0, 0xFB, 0xE8, 0xE0, 0xFF, 0xD0, 0xF4, 0xEA, 0xEA, 0xEA, 0xA2, 0x00, 0xA0, 0x00, 0xC8, 0xC0,
  0xFF, 0xD0, 0xFB, 0xE8, 0xE0, 0xFF, 0xD0, 0xF4, 0xEA, 0xEA, 0xEA, 0xA2, 0x00, 0xA0, 0x00, 0xC8,
  0xC0, 0xFF, 0xD0, 0xFB, 0xE8, 0xE0, 0xFF, 0xD0, 0xF4, 0xEA, 0xEA, 0xEA, 0xA2, 0x00, 0xA0, 0x00,
  0xC8, 0xC0, 0xFF, 0xD0, 0xFB, 0xE8, 0xE0, 0xFF, 0xD0, 0xF4, 0xEA, 0xEA, 0xEA, 0xA2, 0x00, 0xA0,
  0x00, 0xC8, 0xC0, 0xFF, 0xD0, 0xFB, 0xE8, 0xE0, 0xFF, 0xD0, 0xF4, 0xEA, 0xEA, 0xEA, 0xA2, 0x00,
  0xA0, 0x00, 0xC8, 0xC0, 0xFF, 0xD0, 0xFB, 0xE8, 0xE0, 0xFF, 0xD0, 0xF4, 0xEA, 0xEA, 0xEA, 0x4C,
  0x0E, 0xFF, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
  0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
  0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
  0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
  0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, // Filler
  0xEA, 0xEA, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
  0x00, 0x03,   // NMI Vector
  0x00, 0x03,   // Reset Vector (This will jump to program at 0x0300 )
  0x00, 0x03    // IRQ/BRK Vector
};
