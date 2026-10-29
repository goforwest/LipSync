/* 
* File: LSTimer.h
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
  
* Based on: SimpleTimer.h (https://github.com/jfturcot/SimpleTimer)
*/
 
// Header definition
#ifndef _LSTIMER_H
#define _LSTIMER_H


template<typename T>
class LSTimer {
  private:
    // Pool size (bench 2026-10-18, pool hygiene): 10 was tight — pollTimer
    // alone registers 9 at boot (joystick/pressure/input/bluetooth/debug/
    // scroll/screen/usb/watchdog), leaving ONE free slot for every future
    // feature, and rapid health-check-driven scheduling already exhausted it
    // on the bench ("ERROR: No free timers" in session logs). 16 keeps every
    // existing pool at ≥7 free slots for a cost of ~264 bytes per pool
    // (timer_t is 44 bytes on the nRF52840) against 237 KB of RAM.
    const static int MAX_TIMERS = 16;                                             // Maximum number of timers
    const static int RUN_FOREVER = 0;                                             // Flag used for timer that should run continously                 
    const static int RUN_ONCE = 1;                                                // Flag used for timer that should only run once
    const static int DEFCALL_DONTRUN = 0;                                         // Don't call the callback function
    const static int DEFCALL_RUNONLY = 1;                                         // Call the callback function but don't delete the timer
    const static int DEFCALL_RUNANDDEL = 2;                                       // Call the callback function and delete the timer
    const static int MAX_INT = 65535;                                             // Maximum integer used to prevent overrun
    // The time type of the stored clock fields. On the nRF52840 (ILP32)
    // `unsigned long` is 32 bits and millis() wraps at 49.7 days — the wrap
    // arithmetic in run()/elapsedTime() is written for exactly that width.
    // The host test suite overrides this to uint32_t BEFORE including this
    // header so a 64-bit host `unsigned long` cannot silently widen the
    // fields (the wrap class would be structurally untestable there); on
    // the device the default is the Arduino millis() type.
    #ifndef LSTIMER_MILLIS_T
    #define LSTIMER_MILLIS_T unsigned long
    #endif
    typedef LSTIMER_MILLIS_T ls_millis_t;
    typedef void (*timerCallbackPtr)(void);                                       // Define a function pointer type for a callback function with no parameters.
    typedef void (*timerCallbackParamPtr)(T *);                                   // Define a function pointer type for a callback function with parameters.
    typedef struct {                                                              // Define timer structure
      ls_millis_t previousTime;                                                    // Last time the timer was triggered
      void* callback;                                                             // Pointer to the callback function
      T* param;                                                                   // Pointer to the function parameter
      boolean hasParam;                                                           // Check if callback takes a parameter
      ls_millis_t intervalTime;                                                   // Time between triggers
      unsigned maxNumRuns;                                                        // Number of runs to be executed
      unsigned numRuns;                                                           // Number of executed runs
      boolean enabled;                                                            // Check if timer is enabled
      ls_millis_t startDelayTime;                                                  // Initial start delay
      boolean startDelayEnabled;                                                   // Check if offset is enabled
      unsigned toBeCalled;                                                         // Deferred function call
    } timer_t;
    
    // Stopwatch slots (startTimer) hold callback == NULL, which is the SAME
    // marker findFirstFreeSlot() uses to mean "free" — without this bitmap a
    // stopwatch slot is handed to the next registration, silently
    // overwriting the stopwatch (elapsedTime then reports the new timer's
    // schedule) while numTimers double-counts one slot. See startTimer().
    boolean stopwatch[MAX_TIMERS];
    
    int numTimers; 
    // Refused/invalid operation counters (bench 2026-10-18, pool hygiene):
    // setupTimer/startTimer print a rate-limited ERROR on refusal (a timer
    // that never registered must be impossible to miss), but every other
    // path stays SILENT in production — the old unconditional prints
    // ("ERROR: No timers are in use.") landed mid-response and corrupted
    // serial lines during the LK remote-walk bench session. The counters
    // keep the failures observable (debugger, future diagnostic endpoint)
    // without ever printing inside a response.
    int refusedRegistrations = 0;   // setInterval/setTimeout/setTimer/startTimer calls refused (pool full / null callback)
    int invalidOperations = 0;      // Calls with an out-of-range timer id (previously printed mid-response or wrote out of bounds)
    boolean refusalPrinted = false;         // First refusal prints immediately (millis() may still be 0 at boot)
    unsigned long lastRefusalPrintMs = 0;  // Rate-limit anchor for the refusal print

    int findFirstFreeSlot();                                                      // Find the first available slot
    int setupTimer(unsigned long interval, unsigned long startDelay, boolean on, boolean hasParam, unsigned n, void* f, T* p);  
    
  public:
    LSTimer();                                                                    // Constructor
    void run();                                                                   // Check whether timers should be triggered and call callbacks. Must be called inside loop()
    int setInterval(unsigned long interval, unsigned long startDelay, timerCallbackPtr f);   // Call function f every interval milliseconds, start in startDelay milliseconds
    int setInterval(unsigned long interval, unsigned long startDelay, timerCallbackParamPtr f, T* p);
    int setTimeout(unsigned long startDelay, timerCallbackPtr f);                 // Call function f once after interval milliseconds
    int setTimeout(unsigned long startDelay, timerCallbackParamPtr f, T* p);      // Call function f once after interval milliseconds with parameters p
    // Method-callback setTimeout (see the trampoline below): the method is a
    // TEMPLATE PARAMETER, not a runtime argument — each (class, method) pair
    // gets its own generated trampoline function, so two pending method
    // timers on the same class can never overwrite each other's
    // continuation. The old (interval, &Class::method, obj) overload stored
    // the method pointer in ONE static per class, so a second pending
    // method timer silently redirected the first (LSScreen's safe-mode/error
    // pages schedule exactly such pairs on a degraded boot).
    template <typename ObjType, void (ObjType::*Method)()> int setTimeout(unsigned long interval, ObjType* obj);
    int setTimer(unsigned long interval, unsigned long startDelay, unsigned n, timerCallbackPtr f); // Call function f every interval milliseconds for n times, start in startDelay milliseconds
    int setTimer(unsigned long interval, unsigned long startDelay, unsigned n, timerCallbackParamPtr f, T* p);
    int startTimer();                                                             // Start a timer 
    unsigned long elapsedTime(int timerId);                                       // Time elapsed for specificed timer
    int deleteTimer(int timerId);                                                 // Destroy the specified timer
    void restartTimer(int timerId);                                               // Restart the specified timer
    boolean isEnabled(int timerId);                                               // Returns true if the specified timer is enabled
    void enable(int timerId);                                                     // Enables the specified timer
    void disable(int timerId);                                                    // Disables the specified timer
    void toggle(int timerId);                                                     // Toggle(Enables/Disables) the specified timer
    int getNumTimers();                                                           // Returns the number of used timers
    int getNumAvailableTimers() { return MAX_TIMERS - numTimers; };               // Returns the number of available timers
    int getNumRuns(int timerId);                                                  // Returns the number of executed runs
    int getRefusedRegistrations() { return refusedRegistrations; };              // Diagnostics: registration refusals since construction
    int getInvalidOperations() { return invalidOperations; };                     // Diagnostics: invalid-timer-id operations since construction
    timer_t timer[MAX_TIMERS];                                                    // Array of timer structures                                                 
                                                              
};


//*********************************//
// Function   : LSTimer 
// 
// Description: Constructor
//
// Arguments :  void
// 
// Return     : void
//*********************************//
template<typename T>
LSTimer<T>::LSTimer() { // TODO 2025-Feb-07 Consider adding overridden function with size argument for better memory manager
    unsigned long current_millis = millis();

   for (int i = 0; i < MAX_TIMERS; i++) {
        memset(&timer[i], 0, sizeof (timer_t)); //  Initialize each timer
        timer[i].previousTime = current_millis;  //  Set the start time for each timer
        stopwatch[i] = false;                   //  No stopwatch slots in use yet
    }

    numTimers = 0;
    refusedRegistrations = 0;
    invalidOperations = 0;
    refusalPrinted = false;
    lastRefusalPrintMs = 0;
}

//*********************************//
// Function   : run 
// 
// Description: This function checks to see which timers should be run and runs them
//
// Arguments :  void
// 
// Return     : void
//*********************************//
template<typename T>
void LSTimer<T>::run() {
    int i;  // Timer index
    ls_millis_t current_millis; //  Current time
    ls_millis_t delay_millis;  //

    // Get the current time
    current_millis = millis();

    // Determine which timers should be called
    for (i = 0; i < MAX_TIMERS; i++) {

        timer[i].toBeCalled = DEFCALL_DONTRUN;  // Default case is not to run

        if (timer[i].callback != NULL) {

            if ((timer[i].numRuns == 0) && timer[i].startDelayEnabled) {  // If offset is enabled, use offset as delay for first run
              delay_millis = timer[i].startDelayTime;
            } else {
              delay_millis = timer[i].intervalTime;
            }

            // Check if it's time to process timer
            if ((current_millis - timer[i].previousTime) >= delay_millis) {

                // Update the time the timer was triggered
                timer[i].previousTime += delay_millis;  //  TODO shouldn't this be the actual triggered time, e.g., current_millis (vs, idealized trigger time when current-prev = delay) (should also be moved to where function is actually called?)

                // Check if the timer callback has to be executed
                if (timer[i].enabled) {

                    //  If a timer is triggered, increment the number of runs.
                    //  SATURATE at MAX_INT (4.5.7 review): the old wrap
                    //  (65534 -> 1) could never reach a maxNumRuns of
                    //  65535, silently turning such a timer into
                    //  RUN_FOREVER. Saturating preserves the >= comparison
                    //  for every maxNumRuns value.
                    if (timer[i].numRuns < MAX_INT) {
                      timer[i].numRuns++;  //  Increment the number of times
                    }

                    //  Always executed RUN_FOREVER timers
                    if (timer[i].maxNumRuns == RUN_FOREVER) { //  RUN_FOREVER == 0
                        timer[i].toBeCalled = DEFCALL_RUNONLY;
                    }
                    // Other timers
                    else if (timer[i].numRuns < timer[i].maxNumRuns) {
                        timer[i].toBeCalled = DEFCALL_RUNONLY;
                    }
                    // Delete timer after the last run
                    else if (timer[i].numRuns >= timer[i].maxNumRuns) {
                        timer[i].toBeCalled = DEFCALL_RUNANDDEL;
                    }
                }
            }
        }
    }

    //  Trigger the timers that needs to be run
    //  (4.5.7 review):
    //  - RUNONLY: the toBeCalled mark is cleared BEFORE the callback runs,
    //    so a nested run() on the SAME pool cannot double-fire the slot in
    //    the same millisecond (the host suite pins it).
    //  - RUNANDDEL: INVOKE first, then delete — the ORIGINAL, bench-validated
    //    order. An earlier 4.5.7 draft deleted before invoking so a
    //    self-rearming callback could reuse its own slot on a FULL pool,
    //    but deleteTimer() memsets the slot and the final-run callback then
    //    read getNumRuns() == 0 instead of N — which silently killed every
    //    corner persistence and CA,n ack in the calibration flows (they key
    //    their last-reading branch on getNumRuns(...) == N). The full-pool
    //    self-rearm case is unreachable in this firmware (each pool runs
    //    far fewer live slots than MAX_TIMERS; the usbConnectTimer pool
    //    holds exactly one), so the original order stands and the limitation
    //    is documented instead: a self-rearm from inside its own final
    //    callback on a FULL pool is refused (counted) and the chain ends
    //    after its current fire.
    for (i = 0; i < MAX_TIMERS; i++) {

      if (timer[i].toBeCalled != DEFCALL_DONTRUN){    // Check if timer should be run, if not equal to DONTRUN (either RUNONLY or RUNANDDEL)
          if (timer[i].toBeCalled == DEFCALL_RUNONLY){
            timer[i].toBeCalled = DEFCALL_DONTRUN;  // Cleared BEFORE the invoke: a nested run() cannot double-fire this slot
          }
          if (timer[i].hasParam) {
            (*(timerCallbackParamPtr)timer[i].callback)(timer[i].param);
          } else {
            (*(timerCallbackPtr)timer[i].callback)();
          }

          if (timer[i].toBeCalled == DEFCALL_RUNANDDEL){  // Check if timer should be deleted
            deleteTimer(i);
          }
      }

    }
}


//*********************************//
// Function   : findFirstFreeSlot 
// 
// Description: This function finds the first available slot
//
// Arguments :  void
// 
// Return     : void
//*********************************//
template<typename T>
int LSTimer<T>::findFirstFreeSlot() {
    // All slots are used
    if (numTimers >= MAX_TIMERS) {
        return -1;
    }

    // return the first slot with no callback (i.e. free)
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timer[i].callback == NULL && !stopwatch[i]) {
            return i;
        }
    }

    // No available slots
    return -1;
}


//*********************************//
// Function   : setupTimer 
// 
// Description: Set up timer
//
// Arguments :  unsigned long : interval : interval timey (ms)
//           :  unsigned long : startDelay : start delay time (ms)
//           :  boolean : on : whether start delay is enabled 
//           :  boolean : h : Whether callback has parameters
//           :  unsigned : n : Maximum number of runs 
//           :  void* : f : callback Function
//           :  T* : p : Callback function parameters
// 
// Return     : int : Timer index
//*********************************//
template<typename T>
int LSTimer<T>::setupTimer(unsigned long interval, unsigned long startDelay, boolean on, boolean h, unsigned n, void* f, T* p) {
    int freeTimerIndex;

    // A zero-interval RUN_FOREVER timer is a main-loop livelock (4.5.7
    // review): delay 0 is always elapsed and previousTime += 0 never
    // advances the anchor, so run() would fire the callback on EVERY pass,
    // thousands of times per second, while still feeding the watchdog.
    // setTimeout(0)/setTimer(0, n) stay legal (bounded, "fire immediately").
    if (interval == 0 && n == RUN_FOREVER) {
        refusedRegistrations++;
        if (!refusalPrinted || (millis() - lastRefusalPrintMs) > 30000UL) {
          refusalPrinted = true;
          lastRefusalPrintMs = millis();
          Serial.print("ERROR: LSTimer refused a 0-interval RUN_FOREVER registration (");
          Serial.print(refusedRegistrations);
          Serial.println(" total) -- it would livelock run(); NOT registered");
        }
        return -1;
    }

    freeTimerIndex = findFirstFreeSlot();
    if (freeTimerIndex < 0 || f == NULL) {
        // Registration refused — count it, and make it impossible to miss
        // (bench 2026-10-18): every caller used to ignore the returned -1,
        // so a "registered" timer silently never ran — the pool-exhaustion
        // prints in bench logs were the only trace, and nothing tied them
        // to which feature died. Print at most once per 30 s per pool (a
        // wedged caller can retry every loop pass; the first line must not
        // become a firehose that itself corrupts serial output), and keep a
        // counter a diagnostic endpoint/debugger can read.
        refusedRegistrations++;
        if (!refusalPrinted || (millis() - lastRefusalPrintMs) > 30000UL) {
          refusalPrinted = true;
          lastRefusalPrintMs = millis();
          Serial.print("ERROR: LSTimer registration refused (");
            Serial.print(refusedRegistrations);
            Serial.print(" total, ");
            Serial.print(getNumAvailableTimers());
            Serial.println(" slots free) -- the timer/callback was NOT registered");
        }
        return -1;
    }

    timer[freeTimerIndex].intervalTime = interval;
    timer[freeTimerIndex].callback = f;
    timer[freeTimerIndex].param = p;
    timer[freeTimerIndex].hasParam = h;
    timer[freeTimerIndex].maxNumRuns = n;
    timer[freeTimerIndex].numRuns = 0;
    timer[freeTimerIndex].enabled = true;
    timer[freeTimerIndex].startDelayTime = startDelay;
    timer[freeTimerIndex].startDelayEnabled = on;
    timer[freeTimerIndex].previousTime = millis();

    numTimers++;

    return freeTimerIndex;
}


//*********************************//
// Function   : setInterval 
// 
// Description: This function sets up a timer to run forever every interval ms after a startDelay
//
// Arguments :  unsigned long : interval : Time between timer runs (ms)
//           :  unsigned long : startDelay : Time to delay between start of timer and first run
//           :  timerCallbackPtr : f : Function called when timer runs
// 
// Return     : int : Timer index
//*********************************//
template<typename T>
int LSTimer<T>::setInterval(unsigned long interval, unsigned long startDelay, timerCallbackPtr f) {
    // startDelayEnabled only when a nonzero startDelay was actually given
    // (bench 2026-10-18, semantic fix): the old code passed `true`
    // unconditionally, and run() treats (numRuns == 0 && startDelayEnabled)
    // as "use startDelayTime as the first delay" — with startDelay == 0
    // that meant EVERY interval timer fired once IMMEDIATELY at the first
    // run() pass (delay 0 is always elapsed), then again a full interval
    // later. Nine boot registrations fired their first tick at t≈0: the
    // watchdog feed ran once at registration, the joystick/pressure polls
    // read before their sensors had settled, and — worse — a timer DISABLED
    // before its first tick never advanced previousTime (delay 0 adds 0),
    // freezing its schedule anchor at boot. A nonzero startDelay keeps the
    // documented offset behavior (first fire at startDelay, then every
    // interval); zero now means a clean interval-only cadence.
    return setupTimer(interval, startDelay, (startDelay > 0), false, RUN_FOREVER, (void *)f, NULL);
}


//*********************************//
// Function   : setInterval 
// 
// Description: This function sets up a timer to run forever every interval ms after a startDelay with callback parameters
//
// Arguments :  unsigned long : interval : Time between timer runs (ms)
//           :  unsigned long : startDelay : Time to delay between start of timer and first run
//           :  timerCallbackPtr : f : Function called when timer runs
//           :  T* : p : Function parameters
// 
// Return     : int : Timer index
//*********************************//
template<typename T>
int LSTimer<T>::setInterval(unsigned long interval, unsigned long startDelay, timerCallbackParamPtr f, T* p) {
  // See the plain-callback overload above for the startDelayEnabled fix.
  return setupTimer(interval, // delay
                    startDelay, // startDelay
                    (startDelay > 0), // startDelay enabled only with a real offset
                    true, // has parameter
                    RUN_FOREVER, //  Keep running
                    (void *)f, //  Callback function
                    p); // Callback function parameter
}


//*********************************//
// Function   : setTimeout 
// 
// Description: This function sets up a timer to run forever every interval ms after a startDelay
//
// Arguments :  unsigned long : interval : Time between timer runs (ms)
//           :  timerCallbackPtr : f : Function called when timer runs
// 
// Return     : int : Timer index
//*********************************//
template<typename T>
int LSTimer<T>::setTimeout(unsigned long interval, timerCallbackPtr f) {
    return setupTimer(interval, 0, false, false, RUN_ONCE, (void *)f, NULL);
}


//*********************************//
// Function   : setTimeout 
// 
// Description: This function sets up a timer to run forever every interval ms after a startDelay
//
// Arguments :  unsigned long : interval : Time between timer runs (ms)
//           :  timerCallbackPtr : f : Function called when timer runs
//           :  T* : p : Function parameters
// 
// Return     : int : Timer index
//*********************************//
template<typename T>
int LSTimer<T>::setTimeout(unsigned long interval, timerCallbackParamPtr f, T* p) {
  return setupTimer(interval, 0, false, true, RUN_ONCE, (void *)f, p);
}

// Per-method trampoline for the method-callback setTimeout overload: the
// method pointer is a TEMPLATE PARAMETER, so the compiler generates one
// distinct function per (class, method) pair with the method baked in.
//
// This replaces the old TimerCallbackHelper, which stored the method
// pointer in ONE static member per class — two pending method timers on the
// same class shared it, so the second registration silently redirected the
// first's continuation (LSScreen's degraded-boot pages schedule exactly
// such pairs: safeModePage → &LSScreen::safeModeMenu while an error page's
// → &LSScreen::errorPageCable is still pending, and whichever fired first
// ran the OTHER's method). With the method as a template parameter there is
// no shared state to collide; each pending timer carries its own generated
// trampoline as the callback and the object pointer as the param.
template <typename ObjType, void (ObjType::*Method)()>
static void lstimerMethodTrampoline(void* context) {
    (static_cast<ObjType*>(context)->*Method)();
}


//*********************************//
// Function   : setTimeout 
// 
// Description: This function sets up a timer to run once after interval ms, calling a class method on obj.
//
// Arguments :  unsigned long : interval : Time before the callback runs (ms)
//              ObjType : obj's class; Method : the member function to run
//              obj : ObjType* : the object the method runs on
// 
// Return     : int : Timer index
//*********************************//
template <typename T>
template <typename ObjType, void (ObjType::*Method)()>
int LSTimer<T>::setTimeout(unsigned long interval, ObjType* obj) {
    return setTimeout(interval, &lstimerMethodTrampoline<ObjType, Method>, obj);
}


//*********************************//
// Function   : setTimer 
// 
// Description: This function sets up a timer to call the specified function every interval (ms), up to n times.
//
// Arguments :  unsigned long : interval : Time between function calls (ms)
//             unsigned long : startDelay : Delay before the first call (ms)
//             unsigned n     : n         : Number of times to call the function
//             timerCallbackPtr : f         : Function to be called repeatedly
// 
// Return     : int : Timer index (identifier for the timer)
//*********************************//
template<typename T>
int LSTimer<T>::setTimer(unsigned long interval, unsigned long startDelay, unsigned n, timerCallbackPtr f) {
  // See setInterval for the startDelayEnabled fix: 0 means interval-only.
  return setupTimer(interval, startDelay, (startDelay > 0), false, n, (void *)f, NULL);
}


//*********************************//
// Function   : setTimer 
// 
// Description: This function sets up a timer to call the specified function every interval (ms), up to n times with function parameters p.
//
// Arguments :  unsigned long : interval : Time between function calls (ms)
//             unsigned long : startDelay : Delay before the first call (ms)
//             unsigned n     : n         : Number of times to call the function
//             timerCallbackPtr : f         : Function to be called repeatedly
//             p
// 
// Return     : int : Timer index (identifier for the timer)
//*********************************//
template<typename T>
int LSTimer<T>::setTimer(unsigned long interval, unsigned long startDelay, unsigned n, timerCallbackParamPtr f, T* p) {
  // See setInterval for the startDelayEnabled fix: 0 means interval-only.
  return setupTimer(interval, startDelay, (startDelay > 0), true, n, (void *)f, p);
}


//*********************************//
// Function   : startTimer 
// 
// Description: This function sets up a timer without any delay, startDelay, or callback to be used like a stopwatch.
//
// Arguments :  void
// 
// Return     : int : freeTimerIndex : Index of first free timer
//*********************************//
template<typename T>
int LSTimer<T>::startTimer() {
    int freeTimerIndex;

    freeTimerIndex = findFirstFreeSlot();
    if (freeTimerIndex < 0) {
        // Same refusal contract as setupTimer (bench 2026-10-18): the old
        // code returned -1 SILENTLY, and the callers (LSInput/LSPressure
        // state machines) stored it as their stopwatch id — every later
        // elapsedTime()/restartTimer() on that id then read/wrote OUT OF
        // BOUNDS (timer[-1]) instead of failing visibly. Count + the
        // rate-limited print make the refusal observable; the negative-id
        // guards below make a stored -1 harmless.
        refusedRegistrations++;
        if (!refusalPrinted || (millis() - lastRefusalPrintMs) > 30000UL) {
          refusalPrinted = true;
          lastRefusalPrintMs = millis();
          Serial.print("ERROR: LSTimer stopwatch registration refused (");
            Serial.print(refusedRegistrations);
            Serial.print(" total, ");
            Serial.print(getNumAvailableTimers());
            Serial.println(" slots free) -- the stopwatch was NOT started");
        }
        return -1;
    }

    timer[freeTimerIndex].intervalTime = 0;
    timer[freeTimerIndex].callback = 0;
    timer[freeTimerIndex].param = 0;
    timer[freeTimerIndex].hasParam = 0;
    timer[freeTimerIndex].maxNumRuns = 1;
    timer[freeTimerIndex].enabled = true;
    timer[freeTimerIndex].startDelayTime = 0;
    timer[freeTimerIndex].startDelayEnabled = false;
    timer[freeTimerIndex].previousTime = millis();

    // Mark the slot as a live stopwatch: its callback stays NULL (the free
    // marker), so the bitmap is what keeps findFirstFreeSlot() from handing
    // this slot to the next registration — the old slot-stealing bug
    // silently repurposed a stopwatch into somebody's interval timer while
    // double-counting numTimers.
    stopwatch[freeTimerIndex] = true;

    numTimers++;

    return freeTimerIndex;
}


//*********************************//
// Function   : elapsedTime 
// 
// Description: Returns the time elapsed since the timer has been started
//
// Arguments :  int : timerId : Index of timer to determine how much time has elapsed
// 
// Return     : unsigned long : Time elasped since last run
//*********************************//
template<typename T>
unsigned long LSTimer<T>::elapsedTime(int timerId) {
    // Lower-bound guard (bench 2026-10-18): a -1 from a refused registration
    // flows straight here from callers that never checked it, and timer[-1]
    // reads whatever sits before the array — garbage elapsed times in the
    // input state machines. Count it, return a harmless 0.
    if (timerId < 0 || timerId >= MAX_TIMERS) {
        invalidOperations++;
        return 0;
    }
    if (timer[timerId].callback == NULL && !stopwatch[timerId]) {
        invalidOperations++;
        return 0;
    }

    ls_millis_t current_millis, diff_millis;

    // Get current time
    current_millis = millis();
    diff_millis = current_millis - timer[timerId].previousTime;
    return diff_millis;
}


//*********************************//
// Function   : deleteTimer 
// 
// Description: This function deletes the timer corresponding to the timer index
//              WITHOUT printing: the old unconditional ERROR prints (invalid
//              id; "No timers are in use.") landed MID-RESPONSE during the
//              LK remote-walk bench session and corrupted serial lines —
//              deleteTimer runs inside run()'s callback pass and inside the
//              serial API, where any print interleaves with a response.
//              Refusals are counted (invalidOperations) instead.
// 
// Arguments :  int : timerId : Index of timer to delete
// 
// Return     : int : 0 when deleted (or the slot was already empty), -1 on
//                     an out-of-range id (counted, silent)
//*********************************//
template<typename T>
int LSTimer<T>::deleteTimer(int timerId) {
    // Both bounds — the old code only checked the upper end, so a -1 from a
    // refused registration (or any negative id) memset 44 bytes BEFORE the
    // timer array: memory corruption of whatever global precedes the pool,
    // and numTimers zeroed/negative (timer[-1].toBeCalled overlaps it on
    // the nRF52840 layout). Silent, counted, harmless.
    if (timerId < 0 || timerId >= MAX_TIMERS) {
        invalidOperations++;
        return -1;
    }

    // Don't decrease the number of timers if the slot is already empty
    if (timer[timerId].callback != NULL || stopwatch[timerId]) {
        memset(&timer[timerId], 0, sizeof (timer_t));
        timer[timerId].previousTime = millis();
        stopwatch[timerId] = false;

        // update number of timers
        numTimers--;
    }
    return 0;
}


//*********************************//
// Function   : restartTimer 
// 
// Description: Restart the specified timer
//
// Arguments :  int : timerId : Index of the timer to restart
// 
// Return     : void
//*********************************//
template<typename T>
void LSTimer<T>::restartTimer(int timerId) {
    // Lower-bound guard, silent + counted (see deleteTimer): restartTimer
    // with a refused-registration -1 used to WRITE out of bounds —
    // timer[-1].previousTime and timer[-1].numRuns are 40 bytes before the
    // array, i.e. straight into an adjacent global.
    if (timerId < 0 || timerId >= MAX_TIMERS) {
        invalidOperations++;
        return;
    }
    if (timer[timerId].callback == NULL && !stopwatch[timerId]) {
        invalidOperations++;
        return;
    }

    timer[timerId].previousTime = millis();
    timer[timerId].numRuns = 0;
}


//*********************************//
// Function   : isEnabled 
// 
// Description: Returns whether the specified timer is enabled
//
// Arguments :  int : timerId : Index of timer to check if enabled
// 
// Return     : boolean : true - timer enabled, false - timer disabled
//*********************************//
template<typename T>
boolean LSTimer<T>::isEnabled(int timerId) {
    // Lower-bound guard, silent + counted (see deleteTimer).
    if (timerId < 0 || timerId >= MAX_TIMERS) {
        invalidOperations++;
        return false;
    }
    if (timer[timerId].callback == NULL && !stopwatch[timerId]) {
        invalidOperations++;
        return false;
    }

    return timer[timerId].enabled;
}


//*********************************//
// Function   : enable 
// 
// Description: Enable the specified timer
//
// Arguments :  int : timerId : Index of the timer to enable
// 
// Return     : void
//*********************************//
template<typename T>
void LSTimer<T>::enable(int timerId) {
    // Lower-bound guard, silent + counted (see deleteTimer).
    if (timerId < 0 || timerId >= MAX_TIMERS) {
        invalidOperations++;
        return;
    }
    if (timer[timerId].callback == NULL && !stopwatch[timerId]) {
        invalidOperations++;
        return;
    }

    timer[timerId].enabled = true;
}


//*********************************//
// Function   : disable 
// 
// Description: Disables the specified timer
//
// Arguments :  int : timerId : Index of timer to disable
// 
// Return     : void
//*********************************//
template<typename T>
void LSTimer<T>::disable(int timerId) {
    // Lower-bound guard, silent + counted (see deleteTimer).
    if (timerId < 0 || timerId >= MAX_TIMERS) {
        invalidOperations++;
        return;
    }
    if (timer[timerId].callback == NULL && !stopwatch[timerId]) {
        invalidOperations++;
        return;
    }

    timer[timerId].enabled = false;
}


//*********************************//
// Function   : toggle 
// 
// Description: Toggles the specified timer between enabled and disabled.
//
// Arguments :  int : timerId : Index of timer to toggle
// 
// Return     : void
//*********************************//
template<typename T>
void LSTimer<T>::toggle(int timerId) {
    // Lower-bound guard, silent + counted (see deleteTimer).
    if (timerId < 0 || timerId >= MAX_TIMERS) {
        invalidOperations++;
        return;
    }
    if (timer[timerId].callback == NULL && !stopwatch[timerId]) {
        invalidOperations++;
        return;
    }

    timer[timerId].enabled = !timer[timerId].enabled;
}


//*********************************//
// Function   : getNumTimers 
// 
// Description: Return the number of timers.
//
// Arguments :  void
// 
// Return     : int : numTimers : Number of timers
//*********************************//
template<typename T>
int LSTimer<T>::getNumTimers() {
    return numTimers;
}


//*********************************//
// Function   : getNumRuns 
// 
// Description: Return the number of runs for the specified timer
//
// Arguments :  int : timerId : Index of timer to get number of runs
// 
// Return     : int : numRuns : The number of times the timer has been activated.
//*********************************//
template<typename T>
int LSTimer<T>::getNumRuns(int timerId) {
    // Bounds guard — the old code had NONE at all (not even the upper end),
    // so a refused registration's -1 read timer[-1].numRuns out of bounds.
    if (timerId < 0 || timerId >= MAX_TIMERS) {
        invalidOperations++;
        return 0;
    }
    if (timer[timerId].callback == NULL && !stopwatch[timerId]) {
        invalidOperations++;
        return 0;
    }

    return timer[timerId].numRuns;
}

#endif
