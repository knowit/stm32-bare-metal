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

#include "sid.h"

// ----------------------------------------------------------------------------
// Constructor.
// ----------------------------------------------------------------------------
SID::SID()
{
  // Initialize pointers.
  // sample = 0;
  // fir = 0;

  voice[0].set_sync_source(&voice[2]);
  voice[1].set_sync_source(&voice[0]);
  voice[2].set_sync_source(&voice[1]);

  // set_sampling_parameters(985248, SAMPLE_FAST, 44100);

  bus_value = 0;
  bus_value_ttl = 0;

  // ext_in = 0;
}


// ----------------------------------------------------------------------------
// Destructor.
// ----------------------------------------------------------------------------
SID::~SID()
{}

// ----------------------------------------------------------------------------
// Set chip model.
// ----------------------------------------------------------------------------
void SID::set_chip_model(int model)
{
  // for (int i = 0; i < 3; i++) {
  //   voice[i].set_chip_model(model);
  // }

  filter.set_chip_model(model);
  // extfilt.set_chip_model(model);
}

// ----------------------------------------------------------------------------
// SID reset.
// ----------------------------------------------------------------------------
void SID::reset()
{
  for (int i = 0; i < 3; i++) {
    voice[i].reset();
  }
  filter.reset();
  // extfilt.reset();

  bus_value = 0;
  bus_value_ttl = 0;
}



// ----------------------------------------------------------------------------
// Enable filter.
// ----------------------------------------------------------------------------
void SID::enable_filter(bool enable)
{
  filter.enable_filter(enable);
}

// ----------------------------------------------------------------------------
// SID clocking - 1 cycle.
// ----------------------------------------------------------------------------
void SID::clock()
{}

// ----------------------------------------------------------------------------
// SID clocking - delta_t cycles.
// ----------------------------------------------------------------------------
void SID::clock(int delta_t)
{
  int i;

  if (delta_t <= 0) {
    return;
  }

  // Age bus value
  bus_value_ttl -= delta_t;
  if (bus_value_ttl <= 0) {
    bus_value = 0;
    bus_value_ttl = 0;
  }

  // Clock amplitude modulators
  for (i = 0; i < 3; i++) {
    voice[i].envelope.clock(delta_t);
  }

  // Clock and synchronize oscillators.
  // Loop until we reach the current cycle.
  int delta_t_osc = delta_t;
  while (delta_t_osc) {
    int delta_t_min = delta_t_osc;

    // Find minimum number of cycles to an oscillator accumulator MSB toggle.
    // We have to clock on each MSB on / MSB off for hard sync to operate
    // correctly.
    for (i = 0; i < 3; i++) {
      WaveformGenerator& wave = voice[i].wave;

      // It is only necessary to clock on the MSB of an oscillator that is
      // a sync source and has freq != 0.
      if (!(wave.sync_dest->sync && wave.freq)) {
        continue;
      }

      uint16_t freq = wave.freq;
      uint32_t accumulator = wave.accumulator;

      // Clock on MSB off if MSB is on, clock on MSB on if MSB is off.
      uint32_t delta_accumulator =
        (accumulator & 0x800000 ? 0x1000000 : 0x800000) - accumulator;

      int delta_t_next = delta_accumulator / freq;
      if (delta_accumulator % freq) {
        ++delta_t_next;
      }

      if (delta_t_next < delta_t_min) {
        delta_t_min = delta_t_next;
      }
    }

    // Clock oscillators.
    for (i = 0; i < 3; i++) {
      voice[i].wave.clock(delta_t_min);
    }

    // Synchronize oscillators.
    for (i = 0; i < 3; i++) {
      voice[i].wave.synchronize();
    }

    delta_t_osc -= delta_t_min;
  }

  // Clock filter
  filter.clock(delta_t,
	       voice[0].output(), voice[1].output(), voice[2].output());

}

// ----------------------------------------------------------------------------
// Read sample from audio output.
// Both 16-bit and n-bit output is provided.
// ----------------------------------------------------------------------------
int SID::output()
{
  return filter.output();
}