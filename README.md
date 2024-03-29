# stm32-bare-metal
A repo which contains the material for a bare metal programming workshop on stm32 microcontrollers!

# STM32 Bare Metal Programmering med STM32CubeIDE

## Om kurset
Dette kurset er ment som en introduksjon til STM32 ARM baserte microcontrollere og hvordan man kan gjøre utvikling på disse ved å bruke STM32CubeIDE miljøet. Dette er et mer profesjonelt miljø enn for eksempel Arduino og man har tilgang til full hardware debugging med tracing og breakpoints. En av de fine fordelene med dette miljøet er at den har et grafisk konfigurasjons grensesnitt som gjør det enkelt å sette opp hardware ressursene visuelt og så lage all nødvendig skjelett kode som kan brukes direkte i kode prosjektet ditt.

## Utstyr
For å delta trenger du en datamaskin med en USB-A-kontakt. Kurset har blitt testet på Windows/macOS, men skal i teorien fungere på alle operativsystemer.

Som en del av kurset, så vil du få låne et [utviklingskort](https://www.st.com/en/evaluation-tools/nucleo-g0b1re.html) og et eget designet ekspansjonskort med knapper, led og en piezo basert høytalermodul. Noen kort har også en 3.5mm hodetelefonkontakt som kan drives av en PWM utgang fra mikrokontrolleren.  Denne funksjonaliteten utforskes ikke som en del av denne workshoppen i utgangspunktet, men er en bonus leksjon som er inkludert hvis du ønsker å teste ut denne funksjonaliteten.

Utviklingskortet er fra ST Electronics og heter Nucleo-64 STM32G0B1 og er basert på en STM32G0B1 microcontroller. Denne består av en Arm Cortex M0+ kjerne med 512KB Flash(program minne) og 128KB SRAM(arbeidsminne). Mere informasjon om microcontrolleren kan du finne [her](https://www.st.com/en/microcontrollers-microprocessors/stm32g0b1vc.html).

Mer informasjon om utviklingskortet kan du finne [her](https://www.st.com/resource/en/user_manual/dm00452640-stm32-nucleo-64-boards-with-stm32g07xrb-mcus-stmicroelectronics.pdf).

## Forkunnskaper
Fokuset for kurset er å få opp et miljø for utvikling og debugging. Eksemplene som vi vil gå igjennom begynner med en enkel blinky applikasjon og påfølgende moduler vil øke i kompleksitet slik at man til slutt sitter igjen med en applikasjon som kan spille av rtttl sanger, dvs enkle sanger med et format som ble mye brukt på gamle Nokia mobil telefoner.  

Det forventes at du kan grunnleggende C programming og en overflatisk forståelse av hva en microcontroller er.

## Innhold

### Setup
1. Installasjon av STM32CubeIDE
Gå til [ST's webside](https://www.st.com/en/development-tools/stm32cubeide.html#get-software) og last ned installasjonsfilen for IDE.  Velg installasjonsfil basert på ditt OS miljø. Du må mest sannsynligvis lage en profil for å få tilgang til installasjonsfilen. Per nå, så finnes det ikke en native STM32CubeMX installasjon for nye Macs med Apple silikon. Så her må man bruke en x86 versjon med Rosetta.

Se [her](https://community.st.com/s/article/how-to-install-stm32cubeide-on-mac-with-m1-core) for litt mere informasjon om hvordan dette kan gjøres.

2. Installasjon av en serie terminal klient
[CoolTerm](http://freeware.the-meiers.org/) er en enkel klient som er tilgjengelig for de fleste platformer og støtter bruk av makroer.  Last ned og installer.

3. Plugg inn USB ledningene til utviklingskortet og til en ledig port på din PC.

Du er nå klar til å begynne første leksjon!

### Leksjoner
- [1. IDE Primer](./doc/1_ide_primer.md)
- [2. Blinky](./doc/2_blinky.md)
- [3. Blinky RTOS](./doc/3_blinky_rtos.md)
- [4. RTTTL Player](./doc/4_rtttl_player.md)
- [5. RTTTL Player UART](./doc/5_rtttl_player_uart.md)
- [6. Bonus: PlatformIO og SID spiller](./doc/6_platformio_audio.md)