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

#include "filter.h"

// ----------------------------------------------------------------------------
// Constructor.
// ----------------------------------------------------------------------------
Filter::Filter()
{
  fc = 0;

  res = 0;

  filt = 0;

  voice3off = 0;

  hp_bp_lp = 0;

  vol = 0;

  // // State of filter
  Vhp = 0;
  Vbp = 0;
  Vlp = 0;
  Vnf = 0;

  sid_model = 6581;

  enable_filter(true);
  set_chip_model(sid_model);
}


// ----------------------------------------------------------------------------
// Enable filter
// ----------------------------------------------------------------------------
void Filter::enable_filter(bool enable)
{
  enabled = enable;
}


// ----------------------------------------------------------------------------
// Set chip model.
// ----------------------------------------------------------------------------
void Filter::set_chip_model(int model)
{
  if (model == 6581) {
    filter_lut = (uint16_t*)filter_6581;
  }
  else {
    filter_lut = (uint16_t*)filter_8580;
  }

  set_w0();
  set_Q();
}

// ----------------------------------------------------------------------------
// SID reset.
// ----------------------------------------------------------------------------
void Filter::reset()
{
  fc = 0;

  res = 0;

  filt = 0;

  voice3off = 0;

  hp_bp_lp = 0;

  vol = 0;

  // State of filter.
  Vhp = 0;
  Vbp = 0;
  Vlp = 0;
  Vnf = 0;

  set_w0();
  set_Q();
}

// ----------------------------------------------------------------------------
// Register functions
// ----------------------------------------------------------------------------
void Filter::writeFC_LO(uint8_t fc_lo)
{
  fc = (fc & 0x7f8) | (fc_lo & 0x007);
  set_w0();
}

void Filter::writeFC_HI(uint8_t fc_hi)
{
  fc = ((fc_hi << 3) & 0x7f8) | (fc & 0x007);
  set_w0();
}

void Filter::writeRES_FILT(uint8_t res_filt)
{
  res = (res_filt >> 4) & 0x0f;
  set_Q();

  filt = res_filt & 0x0f;
}

void Filter::writeMODE_VOL(uint8_t mode_vol)
{
  voice3off = mode_vol & 0x80;

  hp_bp_lp = (mode_vol >> 4) & 0x07;

  vol = mode_vol & 0x0f;
}

// Set filter cutoff frequency.
void Filter::set_w0()
{
  const double pi = 3.1415926535897932385;
  //static int32_t w0_constant_part = 2.0 * pi * FILTER_FREQUENCY * 1.048576 / 2048;

  // 8580 seems to have an almost linear relationship between fc and f0?
  // f0 = 6.1878 * fc + 122.88
  // Will test out this instead of Resid's spline and lut method, which is
  // too much for a small microcontroller. This equation gives an R2 of 0.9975
  // with the data set provided in the original ReSID code.

  // // Multiply with 1.048576 to facilitate division by 1 000 000 by right-
  // // shifting 20 times (2 ^ 20 = 1048576).
  // w0 = static_cast<int>(2*pi*f0[fc]*1.048576); // resid...
  // TODO: 11 bit look up table?
  // w0 = static_cast<int>(2*pi*(6.1878*fc+122.88)*1.048576);
  // w0 = static_cast<int>(2*pi*filter_6581[fc]*1.048576); // resid...
  w0 = static_cast<int>(2*pi*filter_lut[fc]*1.048576); // resid...
  
  // // Limit f0 to 16kHz to keep 1 cycle filter stable.
  const int w0_max_1 = static_cast<int>(2*pi*16000*1.048576);
  w0_ceil_1 = w0 <= w0_max_1 ? w0 : w0_max_1;

  // // Limit f0 to 4kHz to keep delta_t cycle filter stable.
  const int w0_max_dt = static_cast<int>(2*pi*4000*1.048576);
  w0_ceil_dt = w0 <= w0_max_dt ? w0 : w0_max_dt;
}

// Set filter resonance.
void Filter::set_Q()
{
  // Q is controlled linearly by res. Q has approximate range [0.707, 1.7].
  // As resonance is increased, the filter must be clocked more often to keep
  // stable.

  // The coefficient 1024 is dispensed of later by right-shifting 10 times
  // (2 ^ 10 = 1024).
  _1024_div_Q = static_cast<int>(1024.0/(0.707 + 1.0*res/0x0f));
}
