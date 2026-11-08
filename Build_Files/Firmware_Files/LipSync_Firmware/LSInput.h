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
    int *_inputPinArray = nullptr;
    int _inputNumber = 0;
    int inputState[3] = {0, 0, 0};
    int inputAllState = 0;
    inputStateStruct inputCurrState = {0, 0, 0};
    inputStateStruct inputPrevState = {0, 0, 0};
    LSTimer <void> inputStateTimer;
    int inputStateTimerId = -1;
    // 4.8.7 review — the staged stroke-ending confirmation: a stroke-ending
    // change on a held input is not classified as RELEASED until one extra
    // poll confirms it. A one-poll dropout (jack wiggle/oxide on a held
    // external switch — this device's primary input path) used to classify
    // as a full RELEASED with partial elapsed, firing the short-window
    // action early, re-arming a spurious fresh hold, and permanently
    // consuming the long-window action the user earned. While staged, the
    // buffer still reports the live hold; the timer is NOT restarted, so a
    // closed dropout continues the hold with its original stopwatch.
    bool _releaseStaged = false;
    inputStateStruct _stagedState = {0, 0, 0};

};

LSInput::LSInput(int* inputPinArray, int inputNumber)
{
  inputBuffer.begin(INPUT_BUFF_SIZE);
  // The encoded input state and fixed inputState array are three bits wide.
  // Bound the count before allocating or copying so an invalid constructor
  // argument cannot become a huge allocation or an out-of-bounds walk.
  _inputNumber = constrain(inputNumber, 0, 3);
  if (_inputNumber > 0 && inputPinArray != nullptr) {
    _inputPinArray = new int[_inputNumber]();
  }
  if (_inputPinArray == nullptr) {
    _inputNumber = 0;
  }
  
  // Copy pin identities only. These objects have static storage duration, so
  // their constructors run before Arduino's runtime/variant initialization;
  // touching GPIO with pinMode() here is platform-order dependent. begin()
  // owns all hardware access once setup() is running.
  for (int i = 0; i < _inputNumber; i++){
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
  for (int i = 0; i < _inputNumber; i++) {
    pinMode(_inputPinArray[i], INPUT_PULLUP);
  }
  clear();  
}

void LSInput::clear() {

  inputStateStruct initialInputState = {INPUT_MAIN_STATE_NONE, INPUT_SEC_STATE_WAITING, 0};

  inputBuffer.clear();
  inputBuffer.pushElement(initialInputState);

  _releaseStaged = false;  // a clear() (mode switch, calibration, re-begin) discards any staged confirmation
  _stagedState = {0, 0, 0};

  if (inputStateTimerId >= 0) inputStateTimer.deleteTimer(inputStateTimerId);
  inputStateTimerId = inputStateTimer.startTimer();
}


void LSInput::update() {
  inputStateTimer.run();

  inputAllState = 0;

  // for each input pin, do a digital read and create a single number to represent the collective states
  for (int i = 0; i < _inputNumber; i++){
    inputState[i] = !digitalRead(_inputPinArray[i]);
    inputAllState += (1 << i) * inputState[i];  // create single integer to represent combination of button presses (integer shift, not pow())
  }
  
  inputPrevState = inputBuffer.getLastElement();
  const unsigned long currentElapsed = inputStateTimer.elapsedTime(inputStateTimerId);
  

  // 4.8.7 review — confirm a staged stroke-ending change against the LIVE
  // pin state before classifying it. Runs before the normal branches: while
  // a confirmation is pending, the buffer still holds the live hold element.
  if (_releaseStaged) {
    _releaseStaged = false;
    if (inputAllState == _stagedState.mainState) {
      // The dropped input came back within one poll — a transient dropout,
      // not a release. Continue the hold: refresh the held element's elapsed
      // (exactly what the same-state branch below would do) and leave the
      // ORIGINAL timer running, so the hold's true duration survives the
      // glitch. Nothing is pushed; no restart.
      inputCurrState = {inputAllState, inputPrevState.secondaryState,
                        inputStateTimer.elapsedTime(inputStateTimerId)};
      inputBuffer.updateLastElement(inputCurrState);
      return;
    }
    // Confirmed: the held combination really changed. Emit the staged
    // RELEASED with the elapsed frozen at staging time (the stroke's true
    // end), then restart the timer — the same shape the immediate
    // classification produced, one poll later. The next poll's normal
    // branches handle the re-arm of any still-held input or the idle anchor.
    inputCurrState = _stagedState;
    inputBuffer.pushElement(inputCurrState);
    inputStateTimer.restartTimer(inputStateTimerId);
    return;
  }

  // inputAllState = inputState[0] + 2 * inputState[1] + 4 * inputState[2];
 
  // prev: none, waiting  current : none 
  // prev: press x,started  current : press x 
  if((inputAllState == inputPrevState.mainState // current button states are the same as previous
      && inputPrevState.secondaryState != INPUT_SEC_STATE_RELEASED)) { // and last state was not a release
    
    inputCurrState = {inputAllState, inputPrevState.secondaryState, currentElapsed};

    inputBuffer.updateLastElement(inputCurrState);  // add state to buffer

  } // prev: press x,started  current : press y and elapsed time < reaction time in ms   (120 ms debounce?)
  else if(inputAllState != INPUT_MAIN_STATE_NONE                      // Something is currently pressed
          && inputPrevState.secondaryState == INPUT_SEC_STATE_STARTED // AND previous state was input start
          && currentElapsed < INPUT_REACTION_TIME){                  // AND live time elapsed is less than debounce threshold

    inputCurrState = {inputAllState, inputPrevState.secondaryState, currentElapsed};

    inputBuffer.updateLastElement(inputCurrState);  // add state to buffer
  } 

  // (current button states are different than previous OR last state was released) OR (current button states are different AND current button state is all off)
  else { 
      // prev: none,waiting  current : press x 
      if(inputPrevState.secondaryState == INPUT_SEC_STATE_WAITING) {
        
        inputCurrState = {inputAllState, INPUT_SEC_STATE_STARTED, 0};
        
      } // prev: press x,started  current : none  OR prev: press x,started  current : press y and elapsed time >= 150
      else if(inputPrevState.secondaryState == INPUT_SEC_STATE_STARTED
           && currentElapsed >= INPUT_REACTION_TIME) {
        
        // Classify against the stopwatch NOW, not the previous poll's cached
        // elapsed value. At a 20 ms poll the old check made a configured
        // 20 ms minimum effectively ~40 ms and discarded legitimate short
        // Hub-button presses (upstream #39).
        // 4.8.7 review: this change may be a REAL stroke end — or a one-poll
        // dropout on a still-held input (jack wiggle/oxide), which the old
        // shape classified as a full RELEASED with partial elapsed: the
        // short-window row fired mid-hold, the hold re-armed as a fresh
        // STARTED, and the long-window row the user was still earning was
        // consumed (a 2.0 s dropout inside a 3-8 s START_MENU hold fired
        // DRAG instead, and the true release then matched no row). Stage the
        // candidate release for one extra poll (see the confirmation block
        // at the top of update()); the timer keeps running either way.
        // Return WITHOUT the shared push+restart below: while staged, the
        // buffer keeps reporting the live hold and the stopwatch is intact.
        _releaseStaged = true;
        _stagedState = {inputPrevState.mainState, INPUT_SEC_STATE_RELEASED, currentElapsed};
        return;
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
      // discard it. This branch is only reached with inputAllState == NONE
      // (an "another input already down" shape is merged by the
      // within-reaction branch above) — the WAITING entry is a fresh idle
      // anchor, and a genuinely held input re-arms its STARTED on the next
      // tick's branch 1/2 when the still-down input re-merges.
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
