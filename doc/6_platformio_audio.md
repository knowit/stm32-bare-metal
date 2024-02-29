# Bonus leksjon: PlatformIO bruk og en SID player

## Oversikt
STM32CubeMX er et veldig nyttig og kraftig utviklingsverktøy som gir deg full kontroll over hardware ressursene og muligheter til å debugge kode på et veldig lavt nivå. Læringskurven kan være litt bratt for noen og i Arduino miljøet så begynte noen mennesker å lage Arduino bibliotek som skulle forenkle lærekurven og gjøre tilgangen til hardware ressursen enklere i bruk, gitt visse forutsetninger.  Etter hvert så tok ST Electronics over dette arbeidet og har også jobbet med å få det inn i PlatformIO miljøet.

PlatformIO er en plug-in til Visual Studio Code og har et supert og enkelt grensesnitt til å lage boiler plate prosjekter basert på valgt mikrokontroller og utviklingskort. Den har per i dag støtte for veldige mange utviklerkort.

Hvordan velger man mellom å bruke STM32CubeMX og PlatformIO?  Her er det nok mange personlige preferanser som spiller inn, og hvis du liker eller trenger full kontroll av hardware ressurser, minne og hastighet, så er nok STM32CubeMX veien å gå. Hardware konfiguratoren i STM32CubeMX er super når man trenger å tweake hardware oppsettet mye.

Hvis du liker å jobbe med Arduino miljøet og trenger å lage noe kjapt, så er helt klart PlatformIO veien å gå.  Nå skal det sies at PlatformIO miljøet har tilgang til et veldig stort community med Arduino kode biblioteker, noe som kan gi en et veldig godt utgangspunkt for et mikrokontroller prosjekt.  Kvaliteten kan variere noe, men i det store og hele så er det et supert tilbud og det har et veldig aktivt miljø.

I denne leksjonen skal du lære hvordan du installerer PlatformIO og bruker denne til å lage et prosjekt for utviklingskortet med ekspansjonskortet som kan spille [SID](https://www.c64-wiki.com/wiki/SID) sanger. SID var navnet på lyd chippen som ble utviklet for hjemmedatamaskinen Commodore 64 på tidlig 80 tallet. Dette prosjektet er ganske avansert men gir et godt innblikk i hvordan man kan ta i bruk ressursene i en mikrokontroller og gjøre ganske avanserte greier.

Hoved mekanismen for spilleren blir at man tar i bruk 2 tellere, den ene blir satt opp til å generere en interrupt for å styre timingen av lyd og prosessor simuleringen, den andre telleren blir brukt til å generere et firkantpuls signal med en høy frekvens.  Ved å variere duty cycle på firkantpuls signalet, i kombinasjon med å ha et lite lavpass filter på utgangen av pinnen, så kan spenninger genereres.  Utgangsnivået ut fra en pinne er normalt sett enten 0V eller 3.3V.  Ved å legge til en motstand og en kondensator på utgangspinnen, så kan man lage alle spenninger i mellom disse ytterpunktene ved å variere som nevnt duty cycle.  For eksempel, så vil en duty cycle på 50%, lage en utgangsspenning på 1.15V(halvparten av 3.3V). Lyd signaler er vanligvis et signal mellom -1V til +1V, og i dette prosjektet bruker man et triks ved å putte en kondensator i serie med signalet for å lage et pluss/minus signal.  Dette fordi kondensatorer leder kun endringer av spenninger, så man kan lage +/- signaler.

For å spille en SID sang, så man også kunne emulere lydchippen med sine 3 'stemmer' og også mikroprosessoren 6502.  Disse software komponentene har heldigvis noen andre brukt mye tid på å lage, noe som gjør det ganske greit for oss å ta disse i bruk her. Vi kommer til å bruke en forenklet utgave av reSID(en model av en SID chip laget av Dag Lem) og Fake6502(en model av en 6502 mikroprosessoren av Mike Chambers).

Hvis du vil lære mer om tingene nevnt her, så er det en liste med mulige utgangspunkt nedenunder.

- [Commodore 64](https://www.c64-wiki.com/wiki/C64)
- [6502 data sheet](http://archive.6502.org/datasheets/mos_6500_mpu_nov_1985.pdf)
- [SID chip data sheet](https://ccrma.stanford.edu/~chet/projects/tech_content/6581.pdf)
- [PWM based DA](https://www.allaboutcircuits.com/technical-articles/turn-your-pwm-into-a-dac/)

Gøy! La oss begynne.

## Hardware Setup
Ta utgangspunkt i oppsettet fra leksjon 4, men påse at du har et ekspansjonskort med en 3.5mm hodetelefon utgang.  For å høre lyden, så må du enten koble til høretelefoner eller en liten høytaler med AUX inngang.  Det er en liten variabel motstand(blå komponent med en liten skrue) montert på kortet som kan brukes til å regulere volumet.  Denne er nok satt til et ukjent nivå, så vær litt forsiktig ved første gangs bruk! Dvs IKKE ha høretelefonene i øret ved første gangs bruk og sett volumet på høytaleren til et lavt nivå.

## Software Setup
- Installer Visual Studio Code for ditt miljø. [Link](https://code.visualstudio.com/)
- Start Visual Studio Code, velg "Extensions" menyen på venstre side, søk etter 'PlatformIO'. Installer denne.

## Lage Et Nytt Prosjekt
- Velg ```Create a New Project``` på start siden til PlatformIO i Visual Studio Code.
- En ```Project Wizard``` vil dukke opp, skriv inn ```stm32-sid-player``` som prosjekt navn og velg vårt utviklingskort ```ST Nucleo G0B1RE```. Framework skal være ```Arduino```. Klikk på ```Finish```.
- PlatformIO vil laste ned nødvendige komponenter og lage et enkelt prosjekt.
- Når prosessen er ferdig, legg merke til hvordan et PlatformIO prosjekt er organisert.  ```main.cpp``` er applikasjonsfilen og er ekvivalenten til ```main.ino``` filen i Arduino IDE. I denne filen finner man to funksjoner, ```setup``` og ```loop```. Setup kjøres kun ved boot up og loop funksjonen kjøres kontinuerlig etter setup funksjonen.
- Velg ```main.cpp``` filen og kompiler prosjektet ved å klikke på ```Checkmark``` ikonet i den lyseblå taskbaren på bunnen av vinduet. Hvis ok, så er du klar til å gå videre!

- Kopier over alle kilde filene inklusive mappene som finnes i linken under over til ```src``` mappen i prosjektet ditt:
[kilde kode filer](/sourcecode/lesson_6/src/)

- Kopier over alle include filer som finnes i linken under over til ```include``` mappen i prosjektet ditt:
[include filer](/sourcecode/lesson_6/include/)

- Dobbel klikk på ```main.cpp``` filen og se at det matcher det som er vist under.

```cpp
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
```


- Dobbel klikk på ```platform.ini``` filen i roten av prosjektmappa og oppdater den som under:

```
[env:nucleo_g0b1re]
platform = ststm32
board = nucleo_g0b1re
framework = arduino
build_flags = -O3
build_unflags = -Os
```

- Velg ```main.cpp``` filen og kompiler prosjektet ved å klikke på ```Checkmark``` ikonet i den lyseblå taskbaren på bunnen av vinduet.
- Det er mulig kompilatoren gir en warning om noe, men det er kun en warning og hvis alt er riktig ellers, så fungerer prosjektet.
- Hvis kompileringen går ok, klikk på ```Pil til høyre``` ikonet ved siden av kompiler ikonet. Dette vil laste opp binær filen til mikrokontrolleren.
- Trykk gjerne på ```Stikk kontakt``` ikonet, dette vil åpne opp den serielle monitoren og gjør at man kan følge med på når mikrokontrolleren kjører koden.
- Hvis alt er satt opp riktig, koble til noen høretelefoner til 3.5mm audio utgangen for å høre noen fete Commodore 64 musikk stykker! Juster potmeter skruen for å få ønsket volum nivå. Knappen ```BTN1``` kan brukes til å velge neste sang i playlisten. Til referanse, så er ```BTN2``` er koblet til ```reset```.
- Det er mye å lære i dette prosjektet, både om STM32 mikroprosessoren og også om hvordan SID/6502 chippene emuleres.  I ```main.cpp``` så kan du også finne en forenklet kretskjema over hvordan lyden lages via en pinne drevet av PWM og med et RC nettverk.
- Ha det gøy med å utforske videre på egenhånd! Always stay curious. :)

