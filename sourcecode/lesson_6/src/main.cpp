#include <Arduino.h>
#include "main.h"

#define AUDIO_OUT       PA8                 // can't be changed, this is just reminder 
#define USE_SERIAL                          // for debugging info on Serial (usually USB Serial), uncomment if it's needed
#define SERIAL_SPEED    9600                // Speed of serial connection
#define BUTTON_1        PA10                // can be any pin
#define USE_FILTERS                         // uncomment for testing, irq is  faster in calculations (so delta_t will be smaller, and samplerate will be higher)
#define USE_DEFAULT_SID                         
// #define USE_6581_SID                         
//#define USE_8580_SID                         


struct SidTitle {
  uint8_t *data;
  size_t size;
  uint16_t *durations;
};

const SidTitle DemoSongs[] = {
    // {(uint8_t *)Wizball_sid, Wizball_sid_len, (uint16_t *)Wizball_tune_durations_sec},
    {(uint8_t *)Comic_Bakery_sid, Comic_Bakery_sid_len, (uint16_t *)Comic_Bakery_tune_durations},
    {(uint8_t *)Rambo_First_Blood_Part_II_sid, Rambo_First_Blood_Part_II_sid_len, (uint16_t *)Rambo_First_Blood_Part_II_tune_durations_sec},
    // {(uint8_t *)International_Karate_sid, International_Karate_sid_len, (uint16_t *)International_Karate_tune_delay_times_sec},
    // {(uint8_t *)Commando_sid, Commando_sid_len, (uint16_t *)Commando_tune_durations_sec},
    // {(uint8_t *)Yie_Ar_Kung_Fu_sid, Yie_Ar_Kung_Fu_sid_len, (uint16_t *)Yie_Ar_Kung_Fu_tune_durations_sec},
    // {(uint8_t *)Mikie_sid, Mikie_sid_len, (uint16_t *)Mikie_tune_durations_sec},
    // {(uint8_t *)Ocean_Loader_2_sid, Ocean_Loader_2_sid_len, (uint16_t *)Ocean_Loader_2_tune_durations_sec},
};

const int DemoSongsCount = 2;
int currentSong = 0;

// Globals
SystemRam vram;
SID vsid;
SidPlayerConfig vsid_cfg;
SidPlayer player(&vram, &vsid);

HardwareTimer *PWM = new HardwareTimer(TIM1);       // timer used to generate PWM signal
HardwareTimer *IRQtimer = new HardwareTimer(TIM2);  // timer used to drive interrupt handler for emulation

inline void updateOutput () {
  TIM1->CCR1 =  player.output();
}

// The irq handler will be called at at the rate of (1000000 / sample_rate)
void irq_handler(void) { //
  updateOutput();
  player.clock();
}

inline void setupTimers()
{
  PWM->pause();
  IRQtimer->pause();
  PWM->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA8);
  PWM->setPrescaleFactor(1);
  PWM->setOverflow(vsid_cfg.period * vsid_cfg.magic_number, TICK_FORMAT);
  PWM->resume();

  IRQtimer->setMode(2, TIMER_OUTPUT_COMPARE);
  IRQtimer->setOverflow(vsid_cfg.delta_t, MICROSEC_FORMAT);
  IRQtimer->attachInterrupt(irq_handler);
  IRQtimer->resume();
}

void startPlaying()
{
  player.speedTest();
  player.dumpConfigInfo();
  setupTimers();
}

void stopPlaying()
{
  IRQtimer->pause();
  PWM->pause();
  player.end();
}

void selectSong(int idx)
{
  if ((idx<0) || (idx>=DemoSongsCount))
    idx = 0;

  stopPlaying();
  printf("Playing tune %d\n", idx);
  SidTitle title = DemoSongs[idx];
  vsid_cfg.song_data = title.data;
  vsid_cfg.song_data_size = title.size;
  vsid_cfg.tune_durations = title.durations;
  vsid_cfg.use_durations = true;
  player.load(&vsid_cfg);

#ifdef USE_DEFAULT_SID  
  vsid_cfg.sid_model = player.info.preferred_sid_model; // uses song's preferred sid model
#endif

#ifdef USE_6581_SID  
  vsid_cfg.sid_model = 6581; // 6581 filter
#endif

#ifdef USE_8580_SID  
  vsid_cfg.sid_model = 8580; // 8580 filter
#endif

#ifndef USE_FILTERS   
  player.vsid->enable_filter(false);
#endif
  startPlaying();
}

void nextSong() {  
  currentSong = (++currentSong) % DemoSongsCount;
  selectSong(currentSong);
}

void playNext()
{
  uint8_t pivot = (player.info.default_tune-1) % player.info.total_tunes;
  pivot = (pivot==0) ? player.info.total_tunes : pivot; 

  if ((player.info.current_tune == pivot) && !(DemoSongsCount == 1))
  {
    nextSong();
    return;
  }

  player.playNext();
}

bool checkButton()
{
  const uint32_t buttonCheckInterval = 500;
  static uint32_t previous_time;
  static int previous_button_state = 0;
  static int current_button_state = 0;
  static bool buttonPressed;

  buttonPressed = false;
  if (millis() - previous_time < buttonCheckInterval)
    return buttonPressed;

  current_button_state = !digitalRead(BUTTON_1);  // active low

  if (previous_button_state == 0 && current_button_state == 1)
  {
    buttonPressed = true;
    previous_time = millis();
    printf("Button pressed\n");
  }
  previous_button_state = current_button_state;
  return buttonPressed;
}


void setup() {
  Serial.begin(SERIAL_SPEED);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(PA8, OUTPUT);

  player.setDefaultConfig(&vsid_cfg);
  selectSong(0);
}

void loop() {
  if(!player.isPlaying() || checkButton())
    playNext();
}

// SCHEMATICS:
//
//
//    .-----------------.
//    |                 |
//    | STM32FxxxXXxx   |
//    .------------|----.
//     |G         P|
//     |N         A|
//     |D         8--R1----|------C2---------|
//     |                   |                 --
//     |                   C                 || P1
//     |                   1                 ||<--------- AUDIO OUT
//     |                   |                 --
//     .-------------------|------------------|---------- GND
//                        GND
//    R1 = 100-500 Ohm
//    C1 = 100 nF
//    C2 = 10 uF
//    P1 = 10KOhm potentiometer
//