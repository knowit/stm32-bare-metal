# stm32-bare-metal
A repo which contains the material for a bare metal programming workshop on stm32 microcontrollers!

# STM32 Bare Metal Programmering med STM32CubeIDE

## Om kurset
Dette kurset er ment som en introduksjon til STM32 ARM baserte microcontrollere og hvordan man kan gjøre utvikling på disse ved å bruke STM32CubeIDE miljøet. Dette er et mer proffesjonelt miljø enn for eksempel Arduino og man har tilgang til full hardware debugging med tracing og breakpoints.

## Utstyr
For å delta trenger du en datamaskin med en USB-A-kontakt. Kurset har blitt testet på Windows/macOS, men skal i teorien fungere på alle operativsystemer.

Som en del av kurset, så vil du få låne et [utviklingskort](https://www.st.com/en/evaluation-tools/nucleo-g0b1re.html), en micro USB kabel, tre ledninger med hunkjønnkontakter i hver ende og en piezo basert høytalermodul.

Utviklingskortet er fra ST Electronics og heter Nucleo-64 STM32G0B1 og er basert på en STM32G0B1 microcontroller. Denne består av en Arm Cortex M0+ kjerne med 512KB Flash(program minne) og 128KB SRAM(arbeidsminne). Mere informasjon om microcontrolleren kan du finne [her](https://www.st.com/en/microcontrollers-microprocessors/stm32g0b1vc.html).

Mer informasjon om utviklingskortet kan du finne [her](https://www.st.com/resource/en/user_manual/dm00452640-stm32-nucleo-64-boards-with-stm32g07xrb-mcus-stmicroelectronics.pdf).

## Forkunnskaper
Fokuset for kurset er å få opp et miljø for utvikling og debugging. Eksemplene som vi vil gå igjennom begynner med en enkel blinky applikasjon og påfølgende moduler vil øke i kompleksitet slik at man til slutt sitter igjen med en applikasjon som kan spille av rtttl sanger, dvs enkle sanger med et format som ble mye brukt på gamle Nokia mobil telefoner.  

Det forventes at du kan grunnleggende C programming og en overflatisk forståelse av hva en microcontroller er.

## Innhold

### Setup
1. Installasjon av STM32CubeIDE
Gå til [ST's webside](https://www.st.com/en/development-tools/stm32cubeide.html#get-software) og last ned installasjonsfilen for IDE.  Velg installasjons fil basert på ditt OS miljø. Du må mest sannsynligvis lage en profil for å få tilgang til installasjonsfilen.

2. Installasjon av en serie terminal klient
[CoolTerm](http://freeware.the-meiers.org/) er en enkel klient som er tilgjengelig for de fleste platformer og støtter bruk av makroer.  Last ned og installer.

3. Plugg inn USB ledningene til utviklingskortet og til en ledig port på din PC.

Du er nå klar til å begynne første leksjon!

### Leksjoner
- [1. IDE Primer](./doc/1_ide_primer.md)
- [2. Blinky](./doc/2_blinky.md)
- [3. Blinky RTOS](./doc/3_blinky_rtos.md)
- [4. RTTTL Player](./doc/4_rtttl_player.md)
