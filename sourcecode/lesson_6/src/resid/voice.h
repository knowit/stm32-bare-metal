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

#include "wave.h"
#include "envelope.h"

class Voice
{
public:
  Voice();

  // void set_chip_model(chip_model model);
  void set_sync_source(Voice*);
  void reset();

  void writeCONTROL_REG(uint8_t);

  // Amplitude modulated waveform output.
  // Range [-2048*255, 2047*255].
  int output();

  // EnvelopeGenerator envelope;
  // WaveformGenerator wave;
protected:
  WaveformGenerator wave;
  EnvelopeGenerator envelope;

  // Waveform D/A zero level.
  // int wave_zero;

  // // Multiplying D/A DC offset.
  // int voice_DC;

  friend class SID;
};

inline int Voice::output()
{
  // Multiply oscillator output with envelope output.
  return (wave.output()*envelope.output());
}