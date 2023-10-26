#pragma once

#include <Arduino.h>
// #include <stdbool.h>
// #include <stdint.h>
// #include <stddef.h>


extern void write6502(uint16_t address, uint8_t value);
extern uint8_t read6502(uint16_t address); 

void reset6502();
void exec6502();
