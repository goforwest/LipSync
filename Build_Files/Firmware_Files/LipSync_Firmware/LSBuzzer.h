/* 
* File: LSBuzzer.h
* Firmware: LipSync
* Developed by: MakersMakingChange
* Version: v4.1 (28 March 2025)
  License: GPL v3.0 or later

  Copyright (C) 2024 - 2025 Neil Squire Society
  This program is free software: you can redistribute it and/or modify it under the terms of
  the GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.
  You should have received a copy of the GNU General Public License along with this program.
  If not, see <http://www.gnu.org/licenses/>
*/

// Header definition
#ifndef _LSBUZZER_H
#define _LSBUZZER_H

#include "LSApiDeclarations.h"  // getSoundMode (roadmap item 5: no auto-prototype dependency)

// Notes
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

class LSBuzzer {
  public: 
    LSBuzzer();
    void begin();
    void update();
    void clear();
    void playStartupSound();
    void playReadySound();
    void playErrorSound();
    void playShutdownSound();
    void enable();
    void disable();
    void setSoundModeLevel(int inputSoundMode);
    void calibCornerTone();
    void calibCenterTone();
    void modeEngageTone(bool scrollMode);

  private:
    boolean _buzzerOn = true; // Sound feedback is on by default
    int _soundModeLevel; // Levels that correspond to volume

    // Non-blocking note sequencing: multi-tone sounds queue their follow-up
    // tones and buzzer.update() (called from the main loop) fires each when
    // its time comes. The old delay()s between tones froze the whole main
    // loop — serial API, joystick, pressure — for the length of the sound.
    typedef struct {
      unsigned int frequency;      // Tone frequency in Hz
      unsigned int duration;       // Tone duration in ms
      unsigned long fireAtMs;      // millis() timestamp when the note fires
    } noteStep_t;
    noteStep_t _noteQueue[3];
    uint8_t _noteQueueLen = 0;
    void queueNote(unsigned long delayMs, unsigned int frequency, unsigned int duration);
};

//*********************************//
// Function   : LSBuzzer 
// 
// Description: Construct LSBuzzer
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
LSBuzzer::LSBuzzer() {
}

//*********************************//
// Function   : begin function 
// 
// Description: Initialize LSBuzzer with default settings 
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::begin(){
  pinMode(CONF_BUZZER_PIN, OUTPUT);
  // Read sound mode level from memory
  _soundModeLevel = getSoundMode(false, false);
}

//*********************************//
// Function   : update
// 
// Description: Updates buzzer (currently not implemented)
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::update(){
  // Fire queued notes whose time has come (called from the main loop)
  for (uint8_t i = 0; i < _noteQueueLen; ) {
    if ((long)(millis() - _noteQueue[i].fireAtMs) >= 0) {
      tone(CONF_BUZZER_PIN, _noteQueue[i].frequency, _noteQueue[i].duration);
      for (uint8_t j = i; j < _noteQueueLen - 1; j++) {
        _noteQueue[j] = _noteQueue[j + 1];
      }
      _noteQueueLen--;
    } else {
      i++;
    }
  }
}

//*********************************//
// Function   : queueNote
//
// Description: Queue a follow-up tone to fire delayMs from now.
//
// Arguments :  delayMs : unsigned long : delay from now in milliseconds
//              frequency : unsigned int : tone frequency in Hz
//              duration : unsigned int : tone duration in ms
//
// Return     : void
//*********************************//
void LSBuzzer::queueNote(unsigned long delayMs, unsigned int frequency, unsigned int duration) {
  if (_noteQueueLen >= sizeof(_noteQueue) / sizeof(_noteQueue[0])) return;  // Queue full — drop the note
  _noteQueue[_noteQueueLen] = { frequency, duration, millis() + delayMs };
  _noteQueueLen++;
}

//*********************************//
// Function   : clear function 
// 
// Description: Clears buzzer (currently no implemented) 
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::clear(){
  // No current implementation  
}

//*********************************//
// Function   : enable
// 
// Description: Enable sounds feedback
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::enable(){
  _buzzerOn = true;
}

//*********************************//
// Function   : disable
// 
// Description: Disable sound feedback
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::disable(){
  _buzzerOn = false;
}

//*********************************//
// Function   : setSoundModeLevel
// 
// Description: Set sound level
// 
// Arguments :  inputSoundMode : int : desired sound level
// 
// Return     : void
//*********************************//
void LSBuzzer::setSoundModeLevel(int inputSoundMode){
  _soundModeLevel = inputSoundMode;
}

//*********************************//
// Function   : playStartupSound 
// 
// Description: Sound to play to indicate LipSync startup.
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::playStartupSound(){ 
  if (_buzzerOn && (_soundModeLevel != CONF_SOUND_MODE_OFF)){
    tone(CONF_BUZZER_PIN, NOTE_F5, 200);
  }
}

//*********************************//
// Function   : playReadySound 
// 
// Description: Sound to play to indicate that LipSync is ready to use.
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::playReadySound(){ 
  if (_buzzerOn && (_soundModeLevel != CONF_SOUND_MODE_OFF)){
    tone(CONF_BUZZER_PIN, NOTE_F5, 500);
    queueNote(500, NOTE_C6, 250);  // Was delay(500) between the tones — the loop now runs through the pause
  }
}

//*********************************//
// Function   : playErrorSound 
// 
// Description: Sound to play to indicate LipSync error.
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::playErrorSound(){
  // THE FREEZE ROOT CAUSE FIX (bench 2026-10-23, cold-boot freeze hunt): the
  // queued follow-up note was the ONLY tone() call that ever fired from
  // loop() context — every other sound plays during setup, before the LED
  // feedback machinery is live. The Seeed core's tone() claims ownership of
  // hardware PWM2, the LED brightness writes analogWrite() onto the same
  // shared PWM instances, and the hardware-error path is the ONE place both
  // run interleaved (this sound + the ledError blink): the +500 ms queued
  // note fired from the loop straight into the blink window, and the main
  // loop froze there on 12/12 boots whose joystick sensor was dead at
  // power-on (the watchdog closed every one; the bench even heard the
  // second note silently swallowed on some boots — tone() returning
  // without playing when the ownership takeover failed). Two fixes at the
  // source: (1) the sound is now fully blocking and plays BOTH notes
  // before the caller starts the LED error feedback — the error path runs
  // once at boot on a degraded device ahead of the error pages, so a
  // bounded ~1 s block (the pre-4.2.0 norm, watchdog budget 30 s) buys a
  // loop context that never touches tone(); (2) the sound-mode gate every
  // other tone has (this one beeped even with sound OFF — a long-standing
  // gap). The queue machinery stays for any future user; nothing queues
  // today, so buzzer.update() is a no-op and the loop never calls tone().
  if (!_buzzerOn || (_soundModeLevel == CONF_SOUND_MODE_OFF)) return;
    tone(CONF_BUZZER_PIN, NOTE_G4, 500);
    delay(500);
    tone(CONF_BUZZER_PIN, NOTE_C4, 500);
    delay(500);
    // Release the PWM peripheral explicitly (Tone.cpp: noTone -> stopPlayback
    // with release) — the LED feedback's analogWrite allocates the same
    // instance right after this, and an ownership left behind here is how
    // the peripheral ended up in the conflicted state that froze the loop.
    noTone(CONF_BUZZER_PIN);
}

//*********************************//
// Function   : playShutdownSound 
// 
// Description: Sound to play to indicate LipSync shutdown.
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::playShutdownSound(){
  // Deliberately still blocking: this plays inside softwareReset()'s
  // pre-restart sequence, and the reset that follows would cut off any
  // queued notes. The block is bounded (~1 s) and the device is about to
  // reboot anyway.
  if (_buzzerOn && (_soundModeLevel != CONF_SOUND_MODE_OFF)) {
    tone(CONF_BUZZER_PIN, NOTE_C6, 500);
    delay(250);
    tone(CONF_BUZZER_PIN, NOTE_G5, 500);       
    delay(250);
    tone(CONF_BUZZER_PIN, NOTE_C5, 300);
    delay(500);  
  } 
}

//*********************************//
// Function   : calibCornerTone 
// 
// Description: Sound to play to indicate corner during full calibration. 
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::calibCornerTone(){
  if (_buzzerOn && (_soundModeLevel != CONF_SOUND_MODE_OFF)){
    tone(CONF_BUZZER_PIN, NOTE_A4, 300);
  }
}

//*********************************//
// Function   : calibCenterTone 
// 
// Description: Sound to play to indicate center during full calibration. 
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSBuzzer::calibCenterTone(){
  if (_buzzerOn && (_soundModeLevel != CONF_SOUND_MODE_OFF)){
    tone(CONF_BUZZER_PIN, NOTE_A6, 500);
  }
}

//*********************************//
// Function   : modeEngageTone
//
// Description: One short cue when a hold mode engages (roadmap item 16,
//              upstream #61): scroll mode chirps higher than drag mode so
//              a user who cannot see the LEDs can tell which one took.
//              Gated to the "All" sound tier only — "Basic" is the minimal
//              sound set users already know, and a new cue does not belong
//              in it. A single immediate note: nothing queued, nothing
//              blocking.
//
// Arguments :  scrollMode : bool : true = scroll mode engaged, false = drag
//
// Return     : void
//*********************************//
void LSBuzzer::modeEngageTone(bool scrollMode){
  if (!_buzzerOn || (_soundModeLevel != CONF_SOUND_MODE_ALL)) {
    return;
  }
  tone(CONF_BUZZER_PIN, scrollMode ? NOTE_E6 : NOTE_C5, 60);
}

#endif
