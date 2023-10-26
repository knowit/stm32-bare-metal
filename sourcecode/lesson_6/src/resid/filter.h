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
#include "filter6581.h"
#include "filter8580.h"

#define FILTER_SENSITIVITY  16

class Filter
{
public:
  Filter();

  void enable_filter(bool enable);
  void set_chip_model(int model);

  void clock(int voice1, int voice2, int voice3);

  void clock(int delta_t,
  	     int voice1, int voice2, int voice3);
  void reset();

  // Write registers.
  void writeFC_LO(uint8_t);
  void writeFC_HI(uint8_t);
  void writeRES_FILT(uint8_t);
  void writeMODE_VOL(uint8_t);

  // // SID audio output (16 bits).
  int output();

protected:
  void set_w0();
  void set_Q();

  // // Filter enabled.
  bool enabled;

  // Filter cutoff frequency.
  uint16_t fc;

  // Filter resonance.
  uint8_t res;

  // Selects which inputs to route through filter.
  uint8_t filt;

  // // Switch voice 3 off.
  uint8_t voice3off;

  // // Highpass, bandpass, and lowpass filter modes.
  uint8_t hp_bp_lp;

  // // Output master volume. 4 bit
  uint8_t vol;

  // // State of filter.
  int Vhp; // highpass
  int Vbp; // bandpass
  int Vlp; // lowpass
  int Vnf; // not filtered

  // // Cutoff frequency, resonance.
  int w0, w0_ceil_1, w0_ceil_dt;
  int _1024_div_Q;

  int sid_model;
  uint16_t* filter_lut;

};

// ----------------------------------------------------------------------------
// SID clocking - 1 cycle.
// ----------------------------------------------------------------------------
inline void Filter::clock(int voice1,
		   int voice2,
		   int voice3)
{
  // Scale each voice down from 20 to 13 bits.
  voice1 >>= 7;
  voice2 >>= 7;

  // NB! Voice 3 is not silenced by voice3off if it is routed through
  // the filter.
  if (voice3off && !(filt & 0x04)) {
    voice3 = 0;
  }
  else {
    voice3 >>= 7;
  }

  // This is handy for testing.
  if (!enabled) {
    Vnf = voice1 + voice2 + voice3;
    Vhp = Vbp = Vlp = 0;
    return;
  }

  // Route voices into or around filter.
  // The code below is expanded to a switch for faster execution.
  // (filt1 ? Vi : Vnf) += voice1;
  // (filt2 ? Vi : Vnf) += voice2;
  // (filt3 ? Vi : Vnf) += voice3;

  int Vi;

  switch (filt) {
  default:
  case 0x0:
    Vi = 0;
    Vnf = voice1 + voice2 + voice3;
    break;
  case 0x1:
    Vi = voice1;
    Vnf = voice2 + voice3;
    break;
  case 0x2:
    Vi = voice2;
    Vnf = voice1 + voice3;
    break;
  case 0x3:
    Vi = voice1 + voice2;
    Vnf = voice3;
    break;
  case 0x4:
    Vi = voice3;
    Vnf = voice1 + voice2;
    break;
  case 0x5:
    Vi = voice1 + voice3;
    Vnf = voice2;
    break;
  case 0x6:
    Vi = voice2 + voice3;
    Vnf = voice1;
    break;
  case 0x7:
    Vi = voice1 + voice2 + voice3;
    Vnf = 0;
    break;
  case 0x8:
    Vi = 0;
    Vnf = voice1 + voice2 + voice3;
    break;
  case 0x9:
    Vi = voice1;
    Vnf = voice2 + voice3;
    break;
  case 0xa:
    Vi = voice2;
    Vnf = voice1 + voice3;
    break;
  case 0xb:
    Vi = voice1 + voice2;
    Vnf = voice3;
    break;
  case 0xc:
    Vi = voice3;
    Vnf = voice1 + voice2;
    break;
  case 0xd:
    Vi = voice1 + voice3;
    Vnf = voice2;
    break;
  case 0xe:
    Vi = voice2 + voice3;
    Vnf = voice1;
    break;
  case 0xf:
    Vi = voice1 + voice2 + voice3;
    Vnf = 0;
    break;
  }
    
  // delta_t = 1 is converted to seconds given a 1MHz clock by dividing
  // with 1 000 000.

  // Calculate filter outputs.
  // Vhp = Vbp/Q - Vlp - Vi;
  // dVbp = -w0*Vhp*dt;
  // dVlp = -w0*Vbp*dt;

  int dVbp = (w0_ceil_1*Vhp >> 20);
  int dVlp = (w0_ceil_1*Vbp >> 20);
  Vbp -= dVbp;
  Vlp -= dVlp;
  Vhp = (Vbp*_1024_div_Q >> 10) - Vlp - Vi;
}

// ----------------------------------------------------------------------------
// SID clocking - delta_t cycles.
// ----------------------------------------------------------------------------
inline void Filter::clock(int delta_t,
		   int voice1,
		   int voice2,
		   int voice3)
{
  // Scale each voice down from 20 to 13 bits.
  voice1 >>= 7;
  voice2 >>= 7;

  // NB! Voice 3 is not silenced by voice3off if it is routed through
  // the filter.
  if (voice3off && !(filt & 0x04)) {
    voice3 = 0;
  }
  else {
    voice3 >>= 7;
  }

  // Enable filter on/off.
  // This is not really part of SID, but is useful for testing.
  // On slow CPUs it may be necessary to bypass the filter to lower the CPU
  // load.
  if (!enabled) {
    Vnf = voice1 + voice2 + voice3;
    Vhp = Vbp = Vlp = 0;
    return;
  }

  // Route voices into or around filter.
  // The code below is expanded to a switch for faster execution.
  // (filt1 ? Vi : Vnf) += voice1;
  // (filt2 ? Vi : Vnf) += voice2;
  // (filt3 ? Vi : Vnf) += voice3;

  int Vi;

  switch (filt) {
  default:
  case 0x0:
    Vi = 0;
    Vnf = voice1 + voice2 + voice3;
    break;
  case 0x1:
    Vi = voice1;
    Vnf = voice2 + voice3;
    break;
  case 0x2:
    Vi = voice2;
    Vnf = voice1 + voice3;
    break;
  case 0x3:
    Vi = voice1 + voice2;
    Vnf = voice3;
    break;
  case 0x4:
    Vi = voice3;
    Vnf = voice1 + voice2;
    break;
  case 0x5:
    Vi = voice1 + voice3;
    Vnf = voice2;
    break;
  case 0x6:
    Vi = voice2 + voice3;
    Vnf = voice1;
    break;
  case 0x7:
    Vi = voice1 + voice2 + voice3;
    Vnf = 0;
    break;
  case 0x8:
    Vi = 0;
    Vnf = voice1 + voice2 + voice3;
    break;
  case 0x9:
    Vi = voice1;
    Vnf = voice2 + voice3;
    break;
  case 0xa:
    Vi = voice2;
    Vnf = voice1 + voice3;
    break;
  case 0xb:
    Vi = voice1 + voice2;
    Vnf = voice3;
    break;
  case 0xc:
    Vi = voice3;
    Vnf = voice1 + voice2;
    break;
  case 0xd:
    Vi = voice1 + voice3;
    Vnf = voice2;
    break;
  case 0xe:
    Vi = voice2 + voice3;
    Vnf = voice1;
    break;
  case 0xf:
    Vi = voice1 + voice2 + voice3;
    Vnf = 0;
    break;
  }

  // Maximum delta cycles for the filter to work satisfactorily under current
  // cutoff frequency and resonance constraints is approximately 8.
  int delta_t_flt = FILTER_SENSITIVITY;

  while (delta_t) {
    if (delta_t < delta_t_flt) {
      delta_t_flt = delta_t;
    }

    // delta_t is converted to seconds given a 1MHz clock by dividing
    // with 1 000 000. This is done in two operations to avoid integer
    // multiplication overflow.

    // Calculate filter outputs.
    // Vhp = Vbp/Q - Vlp - Vi;
    // dVbp = -w0*Vhp*dt;
    // dVlp = -w0*Vbp*dt;
    static int w0_delta_t = w0_ceil_dt*delta_t_flt >> 6;

    static int dVbp = (w0_delta_t*Vhp >> 14);
    static int dVlp = (w0_delta_t*Vbp >> 14);
    Vbp -= dVbp;
    Vlp -= dVlp;
    Vhp = (Vbp*_1024_div_Q >> 10) - Vlp - Vi;

    delta_t -= delta_t_flt;
  }
}

inline int Filter::output()
{
  // This is handy for testing.
  if (!enabled) {
    return Vnf*vol;
  }

  // Mix highpass, bandpass, and lowpass outputs. The sum is not
  // weighted, this can be confirmed by sampling sound output for
  // e.g. bandpass, lowpass, and bandpass+lowpass from a SID chip.

  // The code below is expanded to a switch for faster execution.
  // if (hp) Vf += Vhp;
  // if (bp) Vf += Vbp;
  // if (lp) Vf += Vlp;

  int Vf;

  switch (hp_bp_lp) {
  default:
  case 0x0:
    Vf = 0;
    break;
  case 0x1:
    Vf = Vlp;
    break;
  case 0x2:
    Vf = Vbp;
    break;
  case 0x3:
    Vf = Vlp + Vbp;
    break;
  case 0x4:
    Vf = Vhp;
    break;
  case 0x5:
    Vf = Vlp + Vhp;
    break;
  case 0x6:
    Vf = Vbp + Vhp;
    break;
  case 0x7:
    Vf = Vlp + Vbp + Vhp;
    break;
  }

  // Sum non-filtered and filtered output.
  // Multiply the sum with volume.
  return (Vnf + Vf)*(vol);
}
