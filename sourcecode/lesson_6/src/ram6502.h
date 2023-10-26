#pragma once
#include "stdint.h"

#define MAX_DATA_LEN 65536

class SystemRam 
{
public:
  void poke(uint16_t addr , uint8_t value);
  uint8_t peek(uint16_t addr);

private:
  uint8_t vmemory[MAX_DATA_LEN];
};

inline void SystemRam::poke(uint16_t addr , uint8_t value) {
  vmemory[addr] = value;
}

inline uint8_t SystemRam::peek(uint16_t addr) {
  return vmemory[addr];
}

