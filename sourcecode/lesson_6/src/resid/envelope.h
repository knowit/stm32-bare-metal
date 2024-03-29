//  ---------------------------------------------------------------------------
//  This file is part of reSID, a MOS6581 SID emulator engine.
//  Copyright (C) 2004  Dag Lem <resid@nimrod.no>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ---------------------------------------------------------------------------

#pragma once

#include "stdint.h"

class EnvelopeGenerator
{
public:
  EnvelopeGenerator();

  enum State { ATTACK, DECAY_SUSTAIN, RELEASE };

  void clock();
  void clock(int delta_t);
  void reset();

  void writeCONTROL_REG(uint8_t);
  void writeATTACK_DECAY(uint8_t);
  void writeSUSTAIN_RELEASE(uint8_t);
  uint8_t readENV();

  // 8-bit envelope output.
  uint8_t output();

protected:
  uint16_t rate_counter;
  uint16_t rate_period;
  uint8_t exponential_counter;
  uint8_t exponential_counter_period;
  uint8_t envelope_counter;
  bool hold_zero;

  uint8_t attack;
  uint8_t decay;
  uint8_t sustain;
  uint8_t release;

  uint8_t gate;

  State state;

  // Lookup table to convert from attack, decay, or release value to rate
  // counter period.
  static uint16_t rate_counter_period[];

  // The 16 selectable sustain levels.
  static uint8_t sustain_level[];

};


inline void EnvelopeGenerator::clock()
{
  // Check for ADSR delay bug.
  // If the rate counter comparison value is set below the current value of the
  // rate counter, the counter will continue counting up until it wraps around
  // to zero at 2^15 = 0x8000, and then count rate_period - 1 before the
  // envelope can finally be stepped.
  // This has been verified by sampling ENV3.
  //
  if (++rate_counter & 0x8000) {
    ++rate_counter &= 0x7fff;
  }

  if (rate_counter != rate_period) {
    return;
  }

  rate_counter = 0;

  // The first envelope step in the attack state also resets the exponential
  // counter. This has been verified by sampling ENV3.
  //
  if (state == ATTACK || ++exponential_counter == exponential_counter_period)
  {
    exponential_counter = 0;

    // Check whether the envelope counter is frozen at zero.
    if (hold_zero) {
      return;
    }

    switch (state) {
    case ATTACK:
      // The envelope counter can flip from 0xff to 0x00 by changing state to
      // release, then to attack. The envelope counter is then frozen at
      // zero; to unlock this situation the state must be changed to release,
      // then to attack. This has been verified by sampling ENV3.
      //
      ++envelope_counter &= 0xff;
      if (envelope_counter == 0xff) {
        state = DECAY_SUSTAIN;
        rate_period = rate_counter_period[decay];
      }
      break;
    case DECAY_SUSTAIN:
      if (envelope_counter != sustain_level[sustain]) {
        --envelope_counter;
      }
      break;
    case RELEASE:
      // The envelope counter can flip from 0x00 to 0xff by changing state to
      // attack, then to release. The envelope counter will then continue
      // counting down in the release state.
      // This has been verified by sampling ENV3.
      // NB! The operation below requires two's complement integer.
      //
      --envelope_counter &= 0xff;
      break;
    }

    // Check for change of exponential counter period.
    switch (envelope_counter) {
    case 0xff:
      exponential_counter_period = 1;
      break;
    case 0x5d:
      exponential_counter_period = 2;
      break;
    case 0x36:
      exponential_counter_period = 4;
      break;
    case 0x1a:
      exponential_counter_period = 8;
      break;
    case 0x0e:
      exponential_counter_period = 16;
      break;
    case 0x06:
      exponential_counter_period = 30;
      break;
    case 0x00:
      exponential_counter_period = 1;

      // When the envelope counter is changed to zero, it is frozen at zero.
      // This has been verified by sampling ENV3.
      hold_zero = true;
      break;
    }
  }
}


// ----------------------------------------------------------------------------
// SID clocking - delta_t cycles.
// ----------------------------------------------------------------------------
inline void EnvelopeGenerator::clock(int delta_t)
{
  // Check for ADSR delay bug.
  // If the rate counter comparison value is set below the current value of the
  // rate counter, the counter will continue counting up until it wraps around
  // to zero at 2^15 = 0x8000, and then count rate_period - 1 before the
  // envelope can finally be stepped.
  // This has been verified by sampling ENV3.
  //

  // NB! This requires two's complement integer.
  int rate_step = rate_period - rate_counter;
  if (rate_step <= 0) {
    rate_step += 0x7fff;
  }

  while (delta_t) {
    if (delta_t < rate_step) {
      rate_counter += delta_t;
      if (rate_counter & 0x8000) {
        ++rate_counter &= 0x7fff;
      }
      return;
    }

    rate_counter = 0;
    delta_t -= rate_step;

    // The first envelope step in the attack state also resets the exponential
    // counter. This has been verified by sampling ENV3.
    //
    if (state == ATTACK || ++exponential_counter == exponential_counter_period)
    {
      exponential_counter = 0;

      // Check whether the envelope counter is frozen at zero.
      if (hold_zero) {
        rate_step = rate_period;
        continue;
      }

      switch (state) {
      case ATTACK:
        // The envelope counter can flip from 0xff to 0x00 by changing state to
        // release, then to attack. The envelope counter is then frozen at
        // zero; to unlock this situation the state must be changed to release,
        // then to attack. This has been verified by sampling ENV3.
        //
        ++envelope_counter &= 0xff;
        if (envelope_counter == 0xff) {
          state = DECAY_SUSTAIN;
          rate_period = rate_counter_period[decay];
        }
        break;
      case DECAY_SUSTAIN:
        if (envelope_counter != sustain_level[sustain]) {
          --envelope_counter;
        }
        break;
      case RELEASE:
        // The envelope counter can flip from 0x00 to 0xff by changing state to
        // attack, then to release. The envelope counter will then continue
        // counting down in the release state.
        // This has been verified by sampling ENV3.
        // NB! The operation below requires two's complement integer.
        //
        --envelope_counter &= 0xff;
        break;
      }

      // Check for change of exponential counter period.
      switch (envelope_counter) {
      case 0xff:
        exponential_counter_period = 1;
        break;
      case 0x5d:
        exponential_counter_period = 2;
        break;
      case 0x36:
        exponential_counter_period = 4;
        break;
      case 0x1a:
        exponential_counter_period = 8;
        break;
      case 0x0e:
        exponential_counter_period = 16;
        break;
      case 0x06:
        exponential_counter_period = 30;
        break;
      case 0x00:
        exponential_counter_period = 1;

        // When the envelope counter is changed to zero, it is frozen at zero.
        // This has been verified by sampling ENV3.
        hold_zero = true;
        break;
      }
    }

    rate_step = rate_period;
  }
}

// ----------------------------------------------------------------------------
// Read the envelope generator output.
// ----------------------------------------------------------------------------
inline uint8_t EnvelopeGenerator::output()
{
  return envelope_counter;
}