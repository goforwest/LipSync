/* 
* File: LSInput.h
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
#ifndef _LSINPUT_H
#define _LSINPUT_H

#define INPUT_BUFF_SIZE 5

#define INPUT_SEC_STATE_WAITING 0             // OFF->OFF (and ON->ON?)
#define INPUT_SEC_STATE_STARTED 1             // OFF->ON
#define INPUT_SEC_STATE_RELEASED 2            // ON ->OFF

#define INPUT_REACTION_TIME 20

#define INPUT_ACTION_TIMEOUT 60000

class LSInput {
  public:
    LSInput(int* inputPinArray, int inputNumber);
    ~LSInput();
    void begin();                                    
    void clear();  
    void update();    
    inputStateStruct getInputState();
  
  private: 
    LSCircularBuffer <inputStateStruct> inputBuffer;
    int *_inputPinArray;
    int _inputNumber;
    int inputState[3];
    int inputAllState;
    inputStateStruct inputCurrState = {0, 0, 0};
    inputStateStruct inputPrevState = {0, 0, 0};
    LSTimer <void> inputStateTimer;
    int inputStateTimerId;

};

LSInput::LSInput(int* inputPinArray, int inputNumber)
{
  inputBuffer.begin(INPUT_BUFF_SIZE);
  _inputPinArray = new int[inputNumber]();

  _inputNumber = inputNumber;
  
  // Set the input mode for each of the pins to an INPUT_PULLUP
  for (int i = 0; i < inputNumber; i++){
    pinMode(inputPinArray[i], INPUT_PULLUP);
    _inputPinArray[i] = inputPinArray[i];
  }

}

LSInput::~LSInput()
{
  if (_inputPinArray) {
    delete[] _inputPinArray;
    _inputPinArray = nullptr;
  }
}

void LSInput::begin() {
  clear();  
}

void LSInput::clear() {

  inputStateStruct initialInputState = {INPUT_MAIN_STATE_NONE, INPUT_SEC_STATE_WAITING, 0};

  inputBuffer.pushElement(initialInputState);

  inputStateTimerId = inputStateTimer.startTimer();
}


void LSInput::update() {
  inputStateTimer.run();

  inputAllState = 0;

  // for each input pin, do a digital read and create a single number to represent the collective states
  for (int i = 0; i < _inputNumber && i < 3; i++){
    inputState[i] = !digitalRead(_inputPinArray[i]);
    inputAllState += (1 << i) * inputState[i];  // create single integer to represent combination of button presses (integer shift, not pow())
  }
  
  inputPrevState = inputBuffer.getLastElement();
  

  // inputAllState = inputState[0] + 2 * inputState[1] + 4 * inputState[2];
 
  // prev: none, waiting  current : none 
  // prev: press x,started  current : press x 
  if((inputAllState == inputPrevState.mainState // current button states are the same as previous
      && inputPrevState.secondaryState != INPUT_SEC_STATE_RELEASED)) { // and last state was not a release
    
    inputCurrState = {inputAllState, inputPrevState.secondaryState, inputStateTimer.elapsedTime(inputStateTimerId)};

    inputBuffer.updateLastElement(inputCurrState);  // add state to buffer

  } // prev: press x,started  current : press y and elapsed time < reaction time in ms   (120 ms debounce?)
  else if(inputAllState != INPUT_MAIN_STATE_NONE                      // Something is currently pressed
          && inputPrevState.secondaryState == INPUT_SEC_STATE_STARTED // AND previous state was input start
          && inputPrevState.elapsedTime < INPUT_REACTION_TIME){       // AND time elapsed is less than (debounce?) threshold

    inputCurrState = {inputAllState, inputPrevState.secondaryState, inputStateTimer.elapsedTime(inputStateTimerId)};

    inputBuffer.updateLastElement(inputCurrState);  // add state to buffer
  } 

  // (current button states are different than previous OR last state was released) OR (current button states are different AND current button state is all off)
  else { 
      // prev: none,waiting  current : press x 
      if(inputPrevState.secondaryState == INPUT_SEC_STATE_WAITING) {
        
        inputCurrState = {inputAllState, INPUT_SEC_STATE_STARTED, 0};
        
      } // prev: press x,started  current : none  OR prev: press x,started  current : press y and elapsed time >= 150
      else if(inputPrevState.secondaryState == INPUT_SEC_STATE_STARTED 
           && inputPrevState.elapsedTime >= INPUT_REACTION_TIME) {
        
        inputCurrState = {inputPrevState.mainState, INPUT_SEC_STATE_RELEASED, inputPrevState.elapsedTime};
      } // prev: press x,started  current : none or press y, elapsed < reaction time
      // (4.5.7 review — the fall-through that became a latch): a press
      // visible for less than one reaction window used to match NO branch
      // here, so the STALE inputCurrState was re-pushed with the timer
      // restarted every tick — the machine latched {x, STARTED, ~0}
      // forever (quick taps silently eaten until a >=2-tick press), the
      // LM=3 LEDs showed a held input that wasn't, and while latched on
      // combo x a real press of y took the >=-reaction branch and merged
      // into inputAllState = x|y — a glitch on jack 1 plus a real press
      // of jack 3 could fire the S13 combo row's START_MENU on release.
      // A sub-reaction press is a GLITCH by this machine's own definition:
      // discard it. If another input is already down, the WAITING entry
      // re-arms its fresh STARTED on the next tick.
      else if(inputPrevState.secondaryState == INPUT_SEC_STATE_STARTED) {

        inputCurrState = {inputAllState, INPUT_SEC_STATE_WAITING, 0};
      } // prev: press x,released  current : none
      else if(inputPrevState.secondaryState == INPUT_SEC_STATE_RELEASED && inputAllState == INPUT_MAIN_STATE_NONE) {
        
        inputCurrState = {inputAllState, INPUT_SEC_STATE_WAITING, 0};
        
      } // prev: press x,released  current : press y 
      else if(inputPrevState.secondaryState == INPUT_SEC_STATE_RELEASED 
              && inputAllState != INPUT_MAIN_STATE_NONE){

        inputCurrState = {inputAllState, INPUT_SEC_STATE_STARTED, 0};
      }     
       
      inputBuffer.pushElement(inputCurrState); // Push the new state   

      
      inputStateTimer.restartTimer(inputStateTimerId);  // Reset and start the timer
  }
  
  // No action in 1 minute : reset timer
  if(inputPrevState.secondaryState == INPUT_SEC_STATE_WAITING
    && inputStateTimer.elapsedTime(inputStateTimerId) > INPUT_ACTION_TIMEOUT) {
      // Reset and start the timer         
      inputStateTimer.restartTimer(inputStateTimerId);
  }
  
}

inputStateStruct LSInput::getInputState() {
  inputStateStruct inputCurrState = inputBuffer.getLastElement();  // Get the state
  return inputCurrState;
}

#endif 
