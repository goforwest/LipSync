// Host-side regression tests for the firmware's cooperative timer pool
// (Build_Files/Firmware_Files/LipSync_Firmware/LSTimer.h).
//
// This directory is OUTSIDE the sketch folder: the Arduino builder compiles
// LipSync_Firmware/ and Libraries/ only, so this harness never enters the
// firmware build graph. It compiles the REAL LSTimer.h (the same template
// the firmware instantiates) on the host with small Arduino shims, so every
// pool-hygiene invariant the 4.5.1 line depends on is regression-tested
// off-hardware:
//
//   1. A 0-delay interval NEVER fires immediately (the boot-tick bug class:
//      the watchdog feed fired at t=0 once, then appeared dead).
//   2. A nonzero startDelay offsets only the first fire (documented offset
//      behavior), then settles into the interval cadence.
//   3. A disabled timer's anchor keeps advancing: enable resumes within one
//      interval instead of burst-firing the whole missed span at once.
//   4. A RUN_ONCE timeout deletes itself; deleteTimer on the fired slot is
//      harmless and does not underflow numTimers.
//   5. Negative and out-of-range ids on EVERY public accessor are harmless
//      no-ops (the old deleteTimer(-1)/restartTimer(-1) wrote out of bounds
//      before the array — the leading suspect class for the safe-mode
//      interval death).
//   6. A stopwatch slot (startTimer) is never stolen by a later registration
//      (callback==NULL was the pool's own free marker).
//   7. Registration refusal (pool exhausted) returns -1, counts, and leaves
//      the pool consistent.
//   8. Pending METHOD timers cannot collide: two distinct
//      setTimeout<Obj, &Obj::m1/m2> scheduled together both fire their OWN
//      method (the old single-static-per-class bug ran whichever fired
//      first's method for both).
//   9. A fired RUN_ONCE timeout's id names a FREED slot that the pool hands
//      to the NEXT registration — deleting through a stale id kills the new
//      occupant (the LSScreen::_screenStateTimerId hazard the 4.5.5 audit
//      follow-up closed by resetting the id in every continuation). The
//      discipline, not the pool, is what a caller must hold: this test pins
//      both halves so neither silently regresses.
//
// Build & run (matches the app repo's dev/firmware harness discipline):
//   c++ -std=c++17 -g -fsanitize=address,undefined -I../LipSync_Firmware \
//       -o lstimer_host_tests lstimer_host_tests.cpp && ./lstimer_host_tests
//
// This file: GPL v3.0 or later, like the firmware it tests.

#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
// Arduino shims — LSTimer.h only needs a clock, a Serial sink and `boolean`.
// ---------------------------------------------------------------------------
static unsigned long g_fakeMillis = 100;  // like real millis(): never 0 at first run()
unsigned long millis() { return g_fakeMillis; }
static void advanceMs(unsigned long ms) { g_fakeMillis += ms; }

#define boolean bool
class SerialShim {
  public:
    void print(const char* s) { (void)s; }
    void print(int v) { (void)v; }
    void print(unsigned int v) { (void)v; }
    void println(const char* s) { (void)s; }
    void println(int v) { (void)v; }
};
static SerialShim Serial;

#include "LSTimer.h"

// ---------------------------------------------------------------------------
// Tiny test framework
// ---------------------------------------------------------------------------
static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond) do {                                                     \
    g_checks++;                                                              \
    if (!(cond)) {                                                           \
      g_failures++;                                                          \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);            \
    }                                                                        \
  } while (0)

#define CHECK_EQ(a, b) do {                                                  \
    g_checks++;                                                              \
    long long _va = (long long)(a), _vb = (long long)(b);                     \
    if (_va != _vb) {                                                        \
      g_failures++;                                                          \
      std::printf("FAIL %s:%d: %s == %s (got %lld, want %lld)\n",             \
                  __FILE__, __LINE__, #a, #b, _va, _vb);                      \
    }                                                                        \
  } while (0)

// Drive the pool like loop() does: one run() per simulated millisecond.
static void pump(LSTimer<void>& pool, unsigned long ms) {
  for (unsigned long i = 0; i < ms; i++) {
    advanceMs(1);
    pool.run();
  }
}

// ---------------------------------------------------------------------------
// Callback recorders
// ---------------------------------------------------------------------------
static int s_watchdogFeeds = 0;
static void watchdogFeedCb() { s_watchdogFeeds++; }
static int s_joystickTicks = 0;
static void joystickTickCb() { s_joystickTicks++; }
static int s_onceFires = 0;
static void onceCb() { s_onceFires++; }
static int s_argFires = 0;
static int s_argLast = -1;
static void argCb(void* p) { s_argFires++; s_argLast = (int)(long)p; }

struct ScreenPage {
  int fired = 0;
  void safeModeMenu() { fired = 100; }
  void errorPageCable() { fired = 200; }
};

// The recorder counters are shared across tests; each test starts from zero.
static void resetCounters() {
  s_watchdogFeeds = 0;
  s_joystickTicks = 0;
  s_onceFires = 0;
  s_argFires = 0;
  s_argLast = -1;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

static void test_zero_delay_interval_never_fires_immediately() {
  resetCounters();
  LSTimer<void> pool;
  int id = pool.setInterval(5000, 0, watchdogFeedCb);
  CHECK(id >= 0);
  // The old bug: startDelayEnabled=true unconditionally made a 0-delay
  // interval fire on the FIRST run() pass (delay 0 is always elapsed).
  pump(pool, 4999);
  CHECK_EQ(s_watchdogFeeds, 0);
  pump(pool, 1);
  CHECK_EQ(s_watchdogFeeds, 1);  // fires exactly at t+5000, not before
  pump(pool, 5000);
  CHECK_EQ(s_watchdogFeeds, 2);  // clean cadence afterwards
}

static void test_nonzero_start_delay_offsets_only_the_first_fire() {
  resetCounters();
  LSTimer<void> pool;
  int id = pool.setInterval(1000, 250, joystickTickCb);
  CHECK(id >= 0);
  pump(pool, 249);
  CHECK_EQ(s_joystickTicks, 0);
  pump(pool, 1);
  CHECK_EQ(s_joystickTicks, 1);      // first fire at startDelay
  pump(pool, 999);
  CHECK_EQ(s_joystickTicks, 1);      // full interval after the first fire
  pump(pool, 1);
  CHECK_EQ(s_joystickTicks, 2);
}

static void test_disabled_anchor_advances_resume_within_one_interval() {
  resetCounters();
  LSTimer<void> pool;
  int id = pool.setInterval(100, 0, watchdogFeedCb);
  pump(pool, 1000);
  CHECK_EQ(s_watchdogFeeds, 10);      // warm cadence
  s_watchdogFeeds = 0;
  pool.disable(id);
  pump(pool, 10000);                  // long disabled stretch
  CHECK_EQ(s_watchdogFeeds, 0);       // never fires while disabled
  pool.enable(id);
  pump(pool, 100);
  CHECK_EQ(s_watchdogFeeds, 1);       // resumes within ONE interval — no burst
  pump(pool, 100);
  CHECK_EQ(s_watchdogFeeds, 2);
}

static void test_run_once_timeout_deletes_itself() {
  resetCounters();
  LSTimer<void> pool;
  s_onceFires = 0;
  int id = pool.setTimeout(700, onceCb);
  CHECK(id >= 0);
  CHECK_EQ(pool.getNumTimers(), 1);
  pump(pool, 700);
  CHECK_EQ(s_onceFires, 1);
  CHECK_EQ(pool.getNumTimers(), 0);   // self-deleted; no double fire ever
  pump(pool, 2000);
  CHECK_EQ(s_onceFires, 1);
  CHECK_EQ(pool.deleteTimer(id), 0);  // deleting the already-freed slot: harmless
  CHECK_EQ(pool.getNumTimers(), 0);   // and no numTimers underflow
}

static void test_invalid_ids_are_harmless_noops() {
  resetCounters();
  LSTimer<void> pool;
  int live = pool.setInterval(1000, 0, watchdogFeedCb);
  CHECK(live >= 0);
  // The old defects: only the upper bound was checked, so -1 wrote 44 bytes
  // BEFORE the array (deleteTimer) or into an adjacent global (restartTimer).
  CHECK_EQ(pool.deleteTimer(-1), -1);
  pool.restartTimer(-1);
  CHECK_EQ(pool.elapsedTime(-1), 0);
  CHECK(!pool.isEnabled(-1));
  pool.enable(-1);
  pool.disable(-1);
  pool.toggle(-1);
  CHECK_EQ(pool.getNumRuns(-1), 0);
  CHECK_EQ(pool.deleteTimer(16), -1);   // MAX_TIMERS is 16: 0..15 valid
  pool.restartTimer(16);
  CHECK_EQ(pool.getNumRuns(16), 0);
  CHECK_EQ(pool.getInvalidOperations(), 11);  // every guard above counted once (8 lower-bound + 3 upper-bound)
  CHECK_EQ(pool.getNumTimers(), 1);         // the live timer survived it all
  pump(pool, 1000);
  CHECK_EQ(s_watchdogFeeds, 1);             // and still fires
}

static void test_stopwatch_slot_is_not_stolen() {
  resetCounters();
  LSTimer<void> pool;
  int watch = pool.startTimer();
  CHECK(watch >= 0);
  CHECK_EQ(pool.getNumTimers(), 1);
  pump(pool, 1000);
  CHECK_EQ(pool.elapsedTime(watch), 1000);  // the stopwatch measures real time
  int other = pool.setInterval(500, 0, joystickTickCb);  // must NOT take the slot
  CHECK(other != watch);
  CHECK_EQ(pool.getNumTimers(), 2);
  pump(pool, 500);
  CHECK_EQ(s_joystickTicks, 1);
  CHECK_EQ(pool.elapsedTime(watch), 1500);  // stopwatch untouched by the registration
  CHECK_EQ(pool.deleteTimer(watch), 0);
  CHECK_EQ(pool.getNumTimers(), 1);
}

static void test_registration_refusal_is_loud_and_consistent() {
  resetCounters();
  LSTimer<void> pool;
  int ids[32];
  for (int i = 0; i < 32; i++) ids[i] = pool.setInterval(1000, 0, watchdogFeedCb);
  for (int i = 0; i < 16; i++) CHECK(ids[i] >= 0);   // first 16 register
  for (int i = 16; i < 32; i++) CHECK_EQ(ids[i], -1);  // the rest refuse, counted
  CHECK_EQ(pool.getNumAvailableTimers(), 0);
  CHECK(pool.getRefusedRegistrations() >= 16);
  CHECK_EQ(pool.startTimer(), -1);                   // stopwatch refuses too
  CHECK_EQ(pool.getNumTimers(), 16);                // pool still consistent
  pool.deleteTimer(ids[0]);
  int again = pool.setInterval(1000, 0, watchdogFeedCb);
  CHECK(again >= 0);                                 // a freed slot registers again
}

static void test_pending_method_timers_cannot_collide() {
  resetCounters();
  LSTimer<void> pool;
  ScreenPage screen;
  // LSScreen's degraded-boot shape: an error-page continuation and a
  // safe-mode menu scheduled together. The old single-static-per-class
  // helper ran whichever fired first's method for BOTH.
  int t1 = pool.setTimeout<ScreenPage, &ScreenPage::errorPageCable>(1000, &screen);
  int t2 = pool.setTimeout<ScreenPage, &ScreenPage::safeModeMenu>(3000, &screen);
  CHECK(t1 >= 0);
  CHECK(t2 >= 0);
  CHECK_EQ(pool.getNumTimers(), 2);
  screen.fired = 0;
  pump(pool, 1000);
  CHECK_EQ(screen.fired, 200);       // the FIRST timer ran ITS OWN method
  CHECK_EQ(pool.getNumTimers(), 1);
  pump(pool, 2000);
  CHECK_EQ(screen.fired, 100);       // the SECOND timer ran ITS OWN method
  CHECK_EQ(pool.getNumTimers(), 0);
}

static void test_parameterized_callbacks_receive_their_payload() {
  resetCounters();
  LSTimer<void> pool;
  // The center/calibration flows carry (step | initiator-flag) payloads.
  int id = pool.setTimeout(100, argCb, (void*)0L);
  int id2 = pool.setTimeout(200, argCb, (void*)(long)(1 | 0x80));
  CHECK(id >= 0 && id2 >= 0);
  pump(pool, 100);
  CHECK_EQ(s_argLast, 0);
  pump(pool, 100);
  CHECK_EQ(s_argLast, (1 | 0x80));
}

// The stale-id hazard class (4.5.5 audit follow-up, the LSScreen fix):
// a fired RUN_ONCE timeout frees its slot and the pool recycles it, so an
// id that is not reset when its timer fires can later delete an unrelated
// LIVE registration. Models the exact screen pattern: a splash-clear fires
// (its continuation must clear the id), a scheduled transition takes the
// recycled slot, and an error-page cancel reads the id.
static int s_clearFires = 0;
static int s_transitionFires = 0;
static void clearFiredCb() { s_clearFires++; }                 // the "old" splash clear: forgot the id
static void transitionCb() { s_transitionFires++; }             // the live occupant of the recycled slot

static void test_stale_id_after_fire_can_kill_recycled_slot_reset_avoids_it() {
  resetCounters();
  s_clearFires = 0;
  s_transitionFires = 0;
  LSTimer<void> pool;

  // HAZARD HALF: the caller keeps the id after the timer fires.
  int staleId = pool.setTimeout(100, clearFiredCb);
  CHECK(staleId >= 0);
  pump(pool, 100);
  CHECK_EQ(s_clearFires, 1);          // fired; RUN_ONCE freed its slot
  int liveId = pool.setTimeout(500, transitionCb);
  CHECK(liveId == staleId);           // the pool recycles the lowest free slot — the trap is armed
  CHECK_EQ(pool.deleteTimer(staleId), 0);  // a cancel through the stale id…
  pump(pool, 500);
  CHECK_EQ(s_transitionFires, 0);     // …killed the LIVE registration (the defect, demonstrated)

  // DISCIPLINE HALF: the caller that resets the id at fire cannot hit it.
  int honestId = pool.setTimeout(100, clearFiredCb);  // the fixed pattern: the
  pump(pool, 100);                     // continuation resets the id (simulated
  int honestIdReset = -1;              // here because the reset lives in the
  (void)honestId;                      // application's callback, not the pool)
  int liveId2 = pool.setTimeout(500, transitionCb);
  CHECK(liveId2 == staleId);          // recycled again
  if (honestIdReset >= 0) {            // the fixed continuation leaves nothing to read
    pool.deleteTimer(honestIdReset);
  }
  pump(pool, 500);
  CHECK_EQ(s_transitionFires, 1);     // the live registration survived the cancel
  CHECK_EQ(s_clearFires, 2);
}

int main() {
  test_zero_delay_interval_never_fires_immediately();
  test_nonzero_start_delay_offsets_only_the_first_fire();
  test_disabled_anchor_advances_resume_within_one_interval();
  test_run_once_timeout_deletes_itself();
  test_invalid_ids_are_harmless_noops();
  test_stopwatch_slot_is_not_stolen();
  test_registration_refusal_is_loud_and_consistent();
  test_pending_method_timers_cannot_collide();
  test_parameterized_callbacks_receive_their_payload();
  test_stale_id_after_fire_can_kill_recycled_slot_reset_avoids_it();

  std::printf("%d checks, %d failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
