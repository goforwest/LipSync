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
//   7. Registration refusal (pool exhausted, or the 4.5.7-refused
//      0-interval RUN_FOREVER livelock) returns -1, counts, and leaves the
//      pool consistent.
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
//  10. (4.5.7) The pool's clock fields compile at the DEVICE width (32-bit
//      millis), and intervals survive the 49.7-day wrap.
//  11. (4.5.7) Callbacks may mutate the pool MID-RUN: a self-rearming
//      RUN_ONCE (the usbCheckConnection pattern) re-registers even on a
//      FULL pool; a callback may delete another timer; a nested run() on
//      the same pool cannot double-fire a RUNONLY slot.
//  12. (4.5.7) numRuns SATURATES at 65535 instead of wrapping, so a
//      maxNumRuns of 65535 actually terminates.
//  13. (4.5.7) A pool constructed at millis()==0 (the real boot region)
//      behaves: no immediate fires, refusal prints handle t=0.
//
// Build & run (matches the app repo's dev/firmware harness discipline):
//   c++ -std=c++17 -g -fsanitize=address,undefined -I../LipSync_Firmware \
//       -o lstimer_host_tests lstimer_host_tests.cpp && ./lstimer_host_tests
//
// This file: GPL v3.0 or later, like the firmware it tests.

#include <cstdio>
#include <cstring>
#include <cstdint>

// ---------------------------------------------------------------------------
// Arduino shims — LSTimer.h only needs a clock, a Serial sink and `boolean`.
//
// CLOCK FIDELITY (4.5.7 review): the nRF52840's millis() is a 32-bit
// unsigned long; a host `unsigned long` is 64 bits, which used to silently
// widen every stored time field and made the 32-bit wrap class structurally
// untestable here. LSTIMER_MILLIS_T pins the stored fields to uint32_t
// BEFORE including the real header, so this suite exercises the same
// arithmetic the device runs — sizeof(timer_t) is asserted to match.
// ---------------------------------------------------------------------------
#define LSTIMER_MILLIS_T uint32_t
static uint32_t g_fakeMillis = 100;  // nonzero for the legacy tests; the boot region gets its own test at 0
uint32_t millis() { return g_fakeMillis; }
static void advanceMs(uint32_t ms) { g_fakeMillis += ms; }

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

static_assert(sizeof(LSTIMER_MILLIS_T) == 4,
              "host clock fields must match the device's 32-bit millis");

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
                  __FILE__, __LINE__, #a, #b, _va, _vb);                     \
    }                                                                        \
  } while (0)

// Drive the pool like loop() does: one run() per simulated millisecond.
static void pump(LSTimer<void>& pool, uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
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

// (4.5.7 review) A 0-interval RUN_FOREVER registration is refused at the
// source: delay 0 is always elapsed and previousTime += 0 never advances
// the anchor, so run() would fire the callback on EVERY pass — a main-loop
// livelock that still feeds the watchdog.
static void test_zero_interval_run_forever_is_refused() {
  resetCounters();
  LSTimer<void> pool;
  int refusalsBefore = pool.getRefusedRegistrations();
  CHECK_EQ(pool.setInterval(0, 0, watchdogFeedCb), -1);  // the livelock: refused
  CHECK_EQ(pool.getRefusedRegistrations(), refusalsBefore + 1);
  CHECK_EQ(pool.getNumTimers(), 0);
  pump(pool, 10000);
  CHECK_EQ(s_watchdogFeeds, 0);                     // nothing ever fires
  // setTimeout(0) stays legal ("fire immediately", bounded)…
  CHECK(pool.setTimeout(0, onceCb) >= 0);
  pump(pool, 1);
  CHECK_EQ(s_onceFires, 1);
  // …and a bounded setTimer(0, n) too: n fires in n passes, then the
  // (n+1)-th pass's RUNANDDEL cleans it up.
  CHECK(pool.setTimer(0, 0, 3, joystickTickCb) >= 0);
  pump(pool, 4);
  CHECK_EQ(s_joystickTicks, 3);                     // fired n times, then self-deleted
  CHECK_EQ(pool.getNumTimers(), 0);
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

// THE HONEST SPLASH-CLEAR (4.5.7 review): the 4.5.5 discipline-half test
// simulated the id reset OUTSIDE the callback and never executed it; this
// recorder models the REAL fixed pattern — LSScreen::finishSplashClear
// resets the id from INSIDE the firing callback.
static int s_honestClearFires = 0;
static int* s_honestIdSlot = nullptr;  // points at the caller's id variable
static void honestClearCb() {
  s_honestClearFires++;
  if (s_honestIdSlot) { *s_honestIdSlot = -1; }  // the actual discipline, executed
}

static void test_stale_id_after_fire_can_kill_recycled_slot_reset_avoids_it() {
  resetCounters();
  s_clearFires = 0;
  s_transitionFires = 0;
  s_honestClearFires = 0;
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

  // DISCIPLINE HALF (now actually exercised): the continuation resets the
  // id from inside the callback — exactly LSScreen::finishSplashClear.
  int honestId = pool.setTimeout(100, honestClearCb);
  s_honestIdSlot = &honestId;
  pump(pool, 100);
  CHECK_EQ(s_honestClearFires, 1);
  CHECK_EQ(honestId, -1);             // the discipline ran: the id no longer names a slot
  int liveId2 = pool.setTimeout(500, transitionCb);
  CHECK(liveId2 == staleId);          // recycled again
  pool.deleteTimer(honestId);         // the cancel through the reset id…
  pump(pool, 500);
  CHECK_EQ(s_transitionFires, 1);     // …was the harmless -1 no-op; the live registration survived
  CHECK_EQ(s_clearFires, 1);
  s_honestIdSlot = nullptr;
}

// (4.5.7 review) THE 32-BIT WRAP: the device's millis() wraps at 49.7 days.
// With the clock fields pinned to uint32_t, a pool seeded before the wrap
// must keep firing across it — the unsigned-subtraction idiom survives, and
// the `previousTime += interval` anchor wraps consistently.
static void test_intervals_survive_the_32bit_wrap() {
  resetCounters();
  LSTimer<void> pool;
  g_fakeMillis = 0xFFFFFF00UL;  // 256 ms before the wrap
  int id = pool.setInterval(100, 0, watchdogFeedCb);
  CHECK(id >= 0);
  pump(pool, 300);               // cross the wrap
  CHECK_EQ(s_watchdogFeeds, 3);  // fired at +100, +200, +300 across 2^32
  pump(pool, 100);
  CHECK_EQ(s_watchdogFeeds, 4);
  // And a stopwatch straddling the wrap reads a sane elapsed time.
  int watch = pool.startTimer();
  pump(pool, 50);
  CHECK(pool.elapsedTime(watch) <= 51 && pool.elapsedTime(watch) >= 50);
  g_fakeMillis = 100;  // restore the shared clock for the remaining tests
}

// (4.5.7 review) CALLBACKS THAT MUTATE THE POOL MID-RUN — the historically
// bug-prone surface, previously untested. NOTE on the RUNANDDEL order: it
// is invoke-THEN-delete (the original, bench-validated order) — an earlier
// 4.5.7 draft deleted first, which zeroed numRuns inside the final callback
// and silently killed the calibration flows' getNumRuns(...)==N persistence
// gates (test_final_callback_sees_full_numruns pins this). The tradeoff,
// documented in LSTimer.h: a self-rearm from inside its own FINAL callback
// on a FULL pool is refused (unreachable in this firmware — see below).
static int s_rearmFires = 0;
static LSTimer<void>* s_rearmPool = nullptr;
static void rearmCb() {
  s_rearmFires++;
  s_rearmPool->setTimeout(100, rearmCb);  // re-register from inside the callback
}

static void test_self_rearming_callback_survives_a_full_pool() {
  resetCounters();
  LSTimer<void> pool;
  s_rearmFires = 0;
  s_rearmPool = &pool;
  // Leave the chain one FREE slot (14 inert intervals + the chain): with the
  // invoke-then-delete RUNANDDEL order the firing slot is still occupied
  // during its own callback, so a self-rearm needs some other free slot —
  // which is exactly the firmware's real shape (the usbConnectTimer pool
  // holds one timer of sixteen). The genuinely-full-pool refusal case is a
  // documented LSTimer limitation (see the RUNANDDEL comment in LSTimer.h):
  // unreachable here, pinned as refusal semantics in
  // test_registration_refusal_is_loud_and_consistent.
  int fillIds[14];
  int n = 0;
  while (pool.getNumAvailableTimers() > 2) {
    int id = pool.setInterval(100000, 0, watchdogFeedCb);
    CHECK(id >= 0);
    fillIds[n++] = id;
  }
  CHECK_EQ(pool.getNumAvailableTimers(), 2);
  CHECK(pool.setTimeout(100, rearmCb) >= 0);       // the chain takes a slot
  CHECK_EQ(pool.getNumAvailableTimers(), 1);       // one free slot remains
  pump(pool, 350);
  CHECK_EQ(s_rearmFires, 3);  // the self-rearming chain survives
  CHECK(pool.getNumTimers() >= 15);  // accounting stayed consistent (chain + inerts; exact count varies with slot reuse)
  s_rearmPool = nullptr;
}

// (4.5.7 review — CRITICAL regression pin) The final RUNANDDEL callback
// MUST still see numRuns == N: the calibration flows key their corner
// persistence and CA,n acks on getNumRuns(...) == CONF_JOY_CALIB_READING_NUMBER
// inside the last reading callback. An earlier 4.5.7 draft's
// delete-before-invoke memset the slot first and every such gate read 0 —
// corners were never persisted, no ack ever reached the wire.
static int s_cornerCbFires = 0;
static int s_persistBranchFires = 0;
static int s_finalNumRunsSeen = -1;
static LSTimer<void>* s_cornerPool = nullptr;
static int s_cornerId = -1;
static void cornerReadingCb(void* p) {
  (void)p;
  s_cornerCbFires++;
  int runs = s_cornerPool->getNumRuns(s_cornerId);
  if (runs == 10) {           // the firmware's CONF_JOY_CALIB_READING_NUMBER gate
    s_persistBranchFires++;   // "persist the corner + ack"
  }
  if (s_cornerCbFires == 10) s_finalNumRunsSeen = runs;
}

static void test_final_callback_sees_full_numruns() {
  resetCounters();
  LSTimer<void> pool;
  s_cornerPool = &pool;
  s_cornerCbFires = 0;
  s_persistBranchFires = 0;
  s_finalNumRunsSeen = -1;
  // The exact corner shape: CONF_JOY_CALIB_READING_DELAY(200) interval,
  // currentReadingStart delay, 10 readings.
  s_cornerId = pool.setTimer(200, 2400, 10, cornerReadingCb, (void*)0L);
  CHECK(s_cornerId >= 0);
  pump(pool, 2400 + 10 * 200 + 50);
  CHECK_EQ(s_cornerCbFires, 10);
  CHECK_EQ(s_finalNumRunsSeen, 10);      // the Nth callback saw numRuns == N (delete-first draft: 0)
  CHECK_EQ(s_persistBranchFires, 1);     // the persistence/ack gate fired EXACTLY once
  CHECK_EQ(pool.getNumTimers(), 0);      // self-deleted after
  s_cornerPool = nullptr;
}

//   - a callback deleting ANOTHER timer mid-run: the later slot's pending
//     mark is wiped by the memset, so it must not fire this pass.
static LSTimer<void>* s_delPool = nullptr;
static int s_victimFires = 0;
static int s_killerFires = 0;
static int s_victimIdStorage = -1;
static void victimCb() { s_victimFires++; }
static void killerCb() {
  s_killerFires++;
  if (s_victimIdStorage >= 0) s_delPool->deleteTimer(s_victimIdStorage);
}

static void test_callback_may_delete_another_timer_mid_run() {
  resetCounters();
  LSTimer<void> pool;
  s_delPool = &pool;
  s_victimFires = 0;
  s_killerFires = 0;
  s_victimIdStorage = pool.setTimeout(100, victimCb);  // slot 0: due FIRST
  int killerId = pool.setTimeout(100, killerCb);        // slot 1: due the same pass
  CHECK(s_victimIdStorage >= 0 && killerId >= 0);
  pool.restartTimer(s_victimIdStorage);  // re-arm the victim so it is due in the same pass as the killer... (both at 100)
  pump(pool, 100);
  CHECK_EQ(s_victimFires, 1);   // the victim fired (its pass-1 mark was set before the killer ran)
  CHECK_EQ(s_killerFires, 1);   // the killer fired and deleted…
  // …whatever now sits in the victim's slot (it self-freed after firing, so
  // the delete hit a free slot — harmless). The pool must stay consistent:
  CHECK_EQ(pool.getNumTimers(), 0);
  pump(pool, 5000);
  CHECK_EQ(s_victimFires, 1);   // and nothing zombie-fires later
  CHECK_EQ(s_killerFires, 1);
  s_delPool = nullptr;
}

//   - a NESTED run() on the same pool cannot double-fire a due RUNONLY
//     timer in the same millisecond (the 4.5.7 pass-2 clears the mark
//     before invoking).
static LSTimer<void>* s_nestedPool = nullptr;
static int s_nestedFires = 0;
static void nestedTickCb() {
  s_nestedFires++;
  s_nestedPool->run();  // hostile: re-enter the pool from inside its own callback
}

static void test_nested_run_cannot_double_fire() {
  resetCounters();
  LSTimer<void> pool;
  s_nestedPool = &pool;
  s_nestedFires = 0;
  int id = pool.setInterval(100, 0, nestedTickCb);
  CHECK(id >= 0);
  pump(pool, 100);
  // Exactly ONE fire per due millisecond — the nested run() saw the cleared
  // mark (old code: the still-set toBeCalled made the nested pass fire, and
  // the outer pass fired AGAIN).
  CHECK_EQ(s_nestedFires, 1);
  pump(pool, 100);
  CHECK_EQ(s_nestedFires, 2);
  s_nestedPool = nullptr;
}

// (4.5.7 review) numRuns saturates at MAX_INT instead of wrapping to 1, so
// a maxNumRuns of 65535 terminates (the old wrap silently made it
// RUN_FOREVER).
static void test_numruns_saturation_terminates_max_count() {
  resetCounters();
  LSTimer<void> pool;
  s_joystickTicks = 0;
  int id = pool.setTimer(1, 0, 65535, joystickTickCb);
  CHECK(id >= 0);
  pump(pool, 65540);               // well past MAX_INT fires
  CHECK_EQ(s_joystickTicks, 65535);  // fired EXACTLY maxNumRuns times…
  CHECK_EQ(pool.getNumTimers(), 0);   // …then self-deleted (old wrap: never)
}

// (4.5.7 review) THE REAL BOOT REGION: the nRF52 core serves millis()==0
// during static init, so pool constructors and first registrations run at
// t=0 — the region where the 4.1.0 immediate-fire bug lived. The old shim
// started at 100 with a comment claiming millis() is never 0 at first
// run(), which is false on the device.
static void test_pool_constructed_at_millis_zero() {
  resetCounters();
  g_fakeMillis = 0;
  LSTimer<void> pool;               // constructor seeds previousTime = millis() = 0
  int id = pool.setInterval(1000, 0, watchdogFeedCb);
  CHECK(id >= 0);
  pump(pool, 999);
  CHECK_EQ(s_watchdogFeeds, 0);     // no immediate fire at t=0
  pump(pool, 1);
  CHECK_EQ(s_watchdogFeeds, 1);
  int watch = pool.startTimer();    // a stopwatch at t~1000
  pump(pool, 100);
  CHECK_EQ(pool.elapsedTime(watch), 100);
  g_fakeMillis = 100;               // restore the shared clock
}

int main() {
  test_zero_delay_interval_never_fires_immediately();
  test_nonzero_start_delay_offsets_only_the_first_fire();
  test_disabled_anchor_advances_resume_within_one_interval();
  test_run_once_timeout_deletes_itself();
  test_invalid_ids_are_harmless_noops();
  test_stopwatch_slot_is_not_stolen();
  test_registration_refusal_is_loud_and_consistent();
  test_zero_interval_run_forever_is_refused();
  test_pending_method_timers_cannot_collide();
  test_parameterized_callbacks_receive_their_payload();
  test_stale_id_after_fire_can_kill_recycled_slot_reset_avoids_it();
  test_intervals_survive_the_32bit_wrap();
  test_self_rearming_callback_survives_a_full_pool();
  test_final_callback_sees_full_numruns();
  test_callback_may_delete_another_timer_mid_run();
  test_nested_run_cannot_double_fire();
  test_numruns_saturation_terminates_max_count();
  test_pool_constructed_at_millis_zero();

  std::printf("%d checks, %d failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
