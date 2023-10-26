/**
 * @file SidPlayer.h
 * @author Gunnar Larsen
 * @brief Some glue logic between the underlying sid/6502 logic and the uC used
 * @version 0.1
 * @date 2023-09-04
 *
 *
 */
#include <stdio.h>
#include "SidPlayer.h"
#include "rom.h"


void SidPlayer::setDefaultConfig(SidPlayerConfig *config) {
  config->sid_model = 6581;
  config->magic_number = MHZ_PER_SEC;
  config->period = 4;
  config->delta_t = MHZ_PER_SEC;
  config->sample_rate = 1000000/config->delta_t;
  config->default_duration = 30;  // seconds
  config->use_durations = false;
}


bool SidPlayer::checkCompatibility()
{
  unsigned int i, datalen;
  unsigned char *filedata;
  filedata = config->song_data;
  datalen = config->song_data_size;

  uint16_t sid_file_magicid = filedata[0]; // PEEK (0 +  0x0380) ;
  uint16_t sid_file_version = filedata[5]; // PEEK (0x05 +  0x0380);

  info.offset = filedata[7];
  info.loadaddr = (filedata[8] * 256) + (filedata[9]);

  if (info.loadaddr == 0)
    info.loadaddr = ((filedata[0x7d] * 256) + (filedata[0x7c]));


  for (int i = 0; i < 32; i++)
  {
    info.timermode[31 - i] = (filedata[0x12 + (i >> 3)] & (byte)pow(2, 7 - i % 8)) ? 1 : 0;
    printf(" %1d", info.timermode[31 - i]);
  }
  printf("\n");

  info.irq_type_tunes = (((filedata[0x15]))
    | ((filedata[0x14]) << 8)
    | ((filedata[0x13]) << 16)
    | ((filedata[0x12]) << 24)
    );

  info.playaddr = filedata[0xc] * 256 + filedata[0xd];
  info.initaddr = filedata[0xa] + filedata[0xb] ? filedata[0xa] * 256 + filedata[0xb] : info.loadaddr;
  info.total_tunes = filedata[0xf];
  info.preferred_sid_model = (filedata[0x77] & 0x30) >= 0x20 ? 8580 : 6581;
  info.video_type = (filedata[0x77] >> 2) & 0x30;
  info.default_tune = filedata[0x11];
  info.current_tune = info.default_tune;

  int strend = 1;
  for (i = 0; i < 32; i++)
  {
    if (strend != 0)
    {
      strend = info.title[i] = filedata[0x16 + i];
    }
    else
    {
      strend = info.title[i] = 0;
    }
  }

  strend = 1;
  for (i = 0; i < 32; i++)
  {
    if (strend != 0)
    {
      strend = info.author[i] = filedata[0x36 + i];
    }
    else
    {
      strend = info.author[i] = 0;
    }
  }

  strend = 1;
  for (i = 0; i < 32; i++)
  {
    if (strend != 0)
    {
      strend = info.sid_info[i] = filedata[0x56 + i];
    }
    else
    {
      strend = info.sid_info[i] = 0;
    }
  }

  uint8_t ComputeMUSplayer = filedata[0x77] & 0x01; // bit0 - if set, not playable
  uint8_t C64Compatible = (filedata[0x77] >> 1) & 0x01; // bit1 = 0, C64 compatible, playable

  setTuneSpeed(); // set SID_speed

  playable = true;

  if (
      (sid_file_magicid != 0x50)
      | (sid_file_version < 2)
      | (info.playaddr == 0)
      | (ComputeMUSplayer)
      | (C64Compatible)
      )
    {
      playable = false;      
    }

  dumpDebugInfo();
  return playable;
}

void SidPlayer::setTuneSpeed () { // set tune speed best on IRQ_TYPE_PER_TUNE and VIDEO_TYPE
  uint8_t subtune = info.current_tune-1;

  if (vram->peek(0xDC05) || info.timermode[subtune])
  {
    if (vram->peek(0xDC05) == 0)
    {
      vram->poke(0xDC04,0x24);  // C64 startup defaults
      vram->poke(0xDC05, 0x40);
    }    
    info.sid_speed = (vram->peek(0xDC05) << 8) + vram->peek(0xDC04);
  }
  else
    info.sid_speed = (info.video_type == 0x02) ? 16667 : 20000;
}

void SidPlayer::dumpConfigInfo()
{
  printf("Magic      : %d\n", config->magic_number);
  printf("Period     : %d\n", config->period);
  printf("Multiplier : %d\n", config->delta_t);
  printf("Samplerate : %d\n", config->sample_rate);
  printf("SID model  : %d\n", config->sid_model);
}

void SidPlayer::load(SidPlayerConfig *cfg)
{
  unsigned int i, datalen;
  unsigned char *filedata;

  config = cfg;
  filedata = config->song_data;
  datalen = config->song_data_size;

  for (int header = 0; header < 0x7e; header++) {
    vram->poke(header + 0x0380, filedata[header]);
  }

  // playable = Compatibility_check(); // set sid's globals (from RAM), true if sid is playble
  checkCompatibility(); // set sid's globals (from RAM), true if sid is playble

  for ( int i = 0; i < 0x80; i++) {
    vram->poke(i + 0x0300, MyROM[i]); //
  }

  vram->poke(0x0304, info.current_tune - 1);
  vram->poke(0x0307, (info.initaddr >> 8) & 0xff);
  vram->poke(0x0306, info.initaddr & 0xff);
  vram->poke(0x0310, (info.playaddr >> 8) & 0xff);
  vram->poke(0x030f, info.playaddr & 0xff);

  for (uint16_t i = 0; i < datalen - 0x7e; i++) { // data start at $7e offset
      vram->poke(info.loadaddr + i, filedata[i + 0x7e]);    // load .sid directly to RAM in it's exact address . // TODO: Size check
  }

  playDefault();
  
}

void SidPlayer::play() {
  vsid->reset();

  if(config->use_durations)
  {
    uint16_t hiByte, loByte;
    loByte = config->tune_durations[(info.current_tune-1)*2];
    hiByte = config->tune_durations[(info.current_tune-1)*2+1];

    // end_counter = config->tune_durations[info.current_tune-1] * 1000000;
    end_counter = ((hiByte << 8) | loByte) * 1000000;
    printf("tune %d, duration %d [sec]\n",info.current_tune,((hiByte << 8) | loByte));
  }
  else
    end_counter = 1000000 * config->default_duration;

  playing = true;
  play_counter = 0;
  setTuneSpeed();
  reset6502();
  vram->poke(0x0304, info.current_tune - 1); // player's address for init tune
}

void SidPlayer::playTune(uint8_t tune) {
  info.current_tune = tune;
  play();
}

void SidPlayer::playDefault() {
  info.current_tune = info.default_tune;
  play();
}

void SidPlayer::playNext() {
  info.current_tune = (info.current_tune == info.total_tunes) ? 1 : ++info.current_tune;
  play();
}

void SidPlayer::end()
{
  playing = false;
  vsid->reset();
  reset6502();
  vram->poke(0xDC04,0x0); // reset CIA settings to prep for a new tune...
  vram->poke(0xDC05,0x0);  
}

void SidPlayer::dumpDebugInfo() {
  printf("SID title        : %s\n", info.title);
  printf("SID author       : %s\n", info.author);
  printf("SID sid info     : %s\n", info.sid_info);
  printf("SID speed        : %d\n", info.sid_speed);  
  printf("SID loadaddress  : 0x%X\n", info.loadaddr);
  printf("SID playaddress  : 0x%X\n", info.playaddr);
  printf("SID initaddress  : 0x%X\n", info.initaddr);
  printf("SID offset       : 0x%X\n", info.offset);
  printf("SID pref model   : %d\n", info.preferred_sid_model);
  printf("SID # tunes      : %d\n", info.total_tunes);
  printf("SID default tune : %d\n", info.default_tune);
  printf("SID current tune : %d\n", info.current_tune);
}

void SidPlayer::speedTest() 
{
  uint32_t startTime, stopTime, timeSpent, cpu_uS, sid_uS, total_uS, best_multiplier, frame_time;
  uint8_t i = 1;

  const uint32_t iterations = 1000;              // number of instructions to test
  const uint32_t inst_per_irq = 3;

  vsid->set_chip_model(config->sid_model);

  cpu_uS = speedTestCpu(1000);

  for (i = 1; i < 249; i++) {
    startTime = micros();

    for (uint32_t total_instructions = 0; total_instructions < iterations; total_instructions++)
      vsid->clock(i);

    stopTime = micros();
    timeSpent = stopTime - startTime;
    sid_uS = timeSpent / iterations;
    
    total_uS = (inst_per_irq * cpu_uS) + sid_uS;
    frame_time = ((20000 / i) * (  total_uS  )) ;

    if (frame_time < 13000)  // worst case: 13000 for bluepill at 48MHz, O0 (smallest code) optimatization (best case is same as emulated uS)
      break;
  }
  config->delta_t = (i < 32) ? 32 : i;
  config->period = (i < 12) ? 4 : 4;
  config->sample_rate = (1000000/config->delta_t);
}

uint32_t SidPlayer::speedTestCpu(uint32_t num_ops)
{
  uint32_t start, stop, realTimeSpent;

  reset6502();
  start = micros();

  for (int i = 0; i < num_ops; i++) {
    exec6502(); // execute 1 instruction
  }// instrucion loop

  stop = micros();
  realTimeSpent = stop - start;
  return (realTimeSpent / num_ops);
}

uint8_t read6502(uint16_t address) {

  if (address == 0x030c) { // player's  sid-play subroutine's adress
    player.jsr1003 = true;  // sid loading routine, fake timer interrupt signal
  }

  if ((address >= 0xD400) && (address < 0xD420)) {
    return player.vsid->read(address - 0xD400);
  }

  if (address <= (0xffff))
    return player.vram->peek(address); // for  memory space that is covered by RAM

  return 0x60;
}


void write6502(uint16_t address, uint8_t value) {
  if ((address >= 0xD400) & (address < 0xD420)) // writing to sid register
    player.vsid->write(address - 0xD400, value);

  if (address <= (0xffff))
    player.vram->poke(address, value); // for  memory space that is covered by RAM

  // CIA timer check
  if (address == 0xdc05/* || address == 0xdc04*/) {
    if (value > 0) { // set song speed only when Hi value of CIA timer is greater then 0, and only on write to $DC05)
      player.setTuneSpeed();
    }
  }
}

