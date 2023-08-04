/*
 * rtttl.c
 *
 *  Created on: 01.10.2023
 *      Author: Gunnar Larsen
 *
 *	TODO
 *  - code probably could use better error handling. :)
 */

#include <ctype.h>
#include <rtttl.h>

NOTES notes[] = { PAUSE, NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4,
		NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4, NOTE_C5, NOTE_CS5,
		NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5,
		NOTE_AS5, NOTE_B5, NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6,
		NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6, NOTE_C7, NOTE_CS7,
		NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7,
		NOTE_AS7, NOTE_B7 };

/* Private function prototypes -----------------------------------------------*/
void rtttl_set_frequency(st_rtttl_t *ds, uint32_t frequency);
void rtttl_play_tone(st_rtttl_t *ds, uint32_t frequency);


void rtttl_start(st_rtttl_t *ds) {
	int num;

	// load up some safe defaults
	ds->default_dur = 4;
	ds->default_oct = 6;
	ds->bpm = 63;
	ds->index = 0;

	// format: d=N,o=N,b=NNN:
	// find the start (skip name, etc)
	while (ds->tune_data[ds->index] != ':')
		ds->index++; // ignore name

	ds->index++; // skip ':'

	// get default duration
	if (ds->tune_data[ds->index] == 'd') {
		ds->index++;
		ds->index++; // skip "d="
		num = 0;
		while (isdigit(ds->tune_data[ds->index])) {
			num = (num * 10) + (ds->tune_data[ds->index++] - '0');
		}
		if (num > 0) {
			ds->default_dur = num;
		}
		ds->index++; // skip comma
	}
	// get default octave
	if (ds->tune_data[ds->index] == 'o') {
		ds->index++;
		ds->index++; // skip "o="
		num = ds->tune_data[ds->index++] - '0';
		if (num >= 3 && num <= 7) {
			ds->default_oct = num;
		}
		ds->index++; // skip comma
	}
	// get BPM
	if (ds->tune_data[ds->index] == 'b') {
		ds->index++;
		ds->index++; // skip "b="
		num = 0;
		while (isdigit(ds->tune_data[ds->index])) {
			num = (num * 10) + (ds->tune_data[ds->index++] - '0');
		}
		ds->bpm = num;
		ds->index++; // skip colon
	}
	// BPM usually expresses the number of quarter notes per minute
	// calculate  time for whole note (in milliseconds)
	ds->wholenote = (60 * 1000L / ds->bpm) << 2;
	ds->delay_cnt = HAL_GetTick();
	ds->playing = true;
	rtttl_play(ds);
}

void rtttl_play(st_rtttl_t *ds) {
	uint8_t note, scale;
	long duration;
	long frequency;
	int num = 0;

	if (!ds->playing)
		return;

	if ((HAL_GetTick() < ds->delay_cnt))
		// not time yet
		return;

	if (ds->tune_data[ds->index] == '\0') {
		// song is done
		rtttl_stop(ds);
		ds->playing = false;
		return;
	}
	// calculate note duration
	while (isdigit(ds->tune_data[ds->index]))
		num = (num * 10) + (ds->tune_data[ds->index++] - '0');

	duration = ds->wholenote / (num ? num : ds->default_dur);

	// get next note
	switch (ds->tune_data[ds->index]) {
	case 'c':
		note = 1;
		break;
	case 'd':
		note = 3;
		break;
	case 'e':
		note = 5;
		break;
	case 'f':
		note = 6;
		break;
	case 'g':
		note = 8;
		break;
	case 'a':
		note = 10;
		break;
	case 'b':
		note = 12;
		break;
	case 'p':
	default:
		note = 0;
	}
	ds->index++;

	// check for optional '#' sharp
	if (ds->tune_data[ds->index] == '#') {
		note++;
		ds->index++;
	}
	// check for optional '.' dotted note
	if (ds->tune_data[ds->index] == '.') {
		duration += (duration >> 1);
		ds->index++;
	}
	// get scale
	if (isdigit(ds->tune_data[ds->index])) {
		scale = ds->tune_data[ds->index] - '0';
		ds->index++;
	} else {
		scale = ds->default_oct;
	}
	scale += RTTTL_OCTAVE_OFFSET;

	if (ds->tune_data[ds->index] == ',') {
		ds->index++; // skip comma for next note
	}
	// now play the note
	frequency = note ? notes[(scale - 4) * 12 + note] : 0;
	rtttl_play_tone(ds, frequency);
	ds->delay_cnt = HAL_GetTick() + duration;
}

void rtttl_stop(st_rtttl_t *ds) {
	HAL_TIM_PWM_Stop(ds->tim_ref, ds->tim_channel);
	ds->playing = false;
}

void rtttl_set_frequency(st_rtttl_t *ds, uint32_t frequency) {
	TIM_HandleTypeDef *tmp;

	tmp = ds->tim_ref;

	(*tmp).Init.Prescaler = (SystemCoreClock / (1000000)) - 1;
	(*tmp).Init.Period = (1000000 / (frequency)) - 1;

	(*tmp).Init.CounterMode = TIM_COUNTERMODE_UP;
	(*tmp).Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	//(*tmp).Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	if (HAL_TIM_Base_Init(tmp) != HAL_OK) {
		//_Error_Handler(__FILE__, __LINE__);
	}

	__HAL_TIM_SET_COMPARE(ds->tim_ref, ds->tim_channel,
			((*tmp).Init.Period / 2L));
}

void rtttl_play_tone(st_rtttl_t *ds, uint32_t frequency) {
	rtttl_set_frequency(ds, frequency);
	HAL_TIM_PWM_Start(ds->tim_ref, ds->tim_channel);
}
