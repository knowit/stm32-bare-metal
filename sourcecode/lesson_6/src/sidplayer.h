/**
 * @file SidPlayer.h
 * @author Gunnar Larsen
 * @brief A simple abstraction to underlying sid/6502 logic
 * @version 0.1
 * @date 2023-02-23
 * 
 * 
 */
#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "resid.h"
#include "fake6502.h"
#include "ram6502.h"

#define MHZ_PER_SEC (F_CPU / 1000000)

typedef struct{
    uint32_t sample_rate = 15000;
    int sid_model = 6581;
    unsigned char *song_data;
    int song_data_size;
    uint16_t *tune_durations;
    uint32_t default_duration;
    bool use_durations;

    uint8_t delta_t = 66;    // 985248(PAL clk)/samplerate = dt
    uint8_t magic_number = MHZ_PER_SEC;
    uint8_t period = 4;    
} SidPlayerConfig;

typedef struct{
    char title[32];
    char author[32];
    char sid_info[32];

    uint16_t loadaddr;
    uint16_t initaddr;
    uint16_t playaddr;
    uint16_t offset;
    byte timermode[32];

    uint8_t total_tunes;
    uint8_t default_tune;
    uint8_t current_tune;

    int preferred_sid_model;
    uint8_t video_type;
    uint32_t sid_speed;
    uint32_t irq_type_tunes;

} SidSongInfo;

class SidPlayer {

    public:
        SidPlayer(SystemRam *ram, SID *sid) {vram = ram; vsid = sid;};

        void setDefaultConfig(SidPlayerConfig *cfg);
        void load(SidPlayerConfig *config);
        void play();
        void playDefault();
        void playNext();
        void playTune(uint8_t tune);

        void speedTest();

        void clock();
        int output();
        void end();

        void dumpDebugInfo();
        void dumpConfigInfo();

        void setTuneSpeed();

        bool isPlaying() {return playing;}
        bool isPlayable() {return playable;}

        SidSongInfo info;
        SID *vsid;
        SystemRam *vram;

        volatile bool jsr1003 = false;

    protected:
        SidPlayerConfig *config;
        bool playing = false;
        bool playable = false;

    private:
        const uint8_t instructionsPerIrq = 2; 
        uint32_t play_counter; // uS counter
        uint32_t end_counter = 1000000 * 60; // play new tune every x seconds  (this is the number in uS) (maximum 32bit number is around 71minutes)
        uint32_t vic_irq = 0;   // in us, keeps track of time between vic irq events
        volatile bool vic_irq_request = false;

        bool checkCompatibility();
        uint32_t speedTestCpu(uint32_t num_ops);
};

inline void SidPlayer::clock()
{
  static int cc;
  static uint32_t vic_irq = 0;

  play_counter += config->delta_t;
  if (play_counter >= end_counter) {
    playing = false;
    return;
  }

  vsid->clock(config->delta_t);

  for (cc = 0; cc < instructionsPerIrq; cc++)
    if (!jsr1003) exec6502();

  // check if it is time for a vic irq, normally set to 1/50hz for PAL
  vic_irq += config->delta_t;
  if (vic_irq < info.sid_speed)
    return;

  vic_irq = 0;
  jsr1003 = false;

}

inline int SidPlayer::output()
{
  static int32_t _Volume;
  static uint32_t _main_volume;
  _Volume = vsid->output();

  if (_Volume < 0) _Volume = 0;
  if (_Volume > 0xfffff) _Volume = 0xfffff;

  _main_volume = (_Volume) ;
  _main_volume *= config->magic_number;
  _main_volume >>= 12;
  _main_volume *= config->period;
  _main_volume >>= 8;
  return (_main_volume + 1);
}

extern SidPlayer player;
extern void write6502(uint16_t address, uint8_t value);
extern uint8_t read6502(uint16_t address); 