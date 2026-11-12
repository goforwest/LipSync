#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <type_traits>

#include "LSCircularBuffer.h"
#include "LSHostSwitch.h"
#include "LSPressureClassify.h"
#include "LSPressureEngageFilter.h"
#include "LSPressureOffsetFilter.h"
#include "LSSerialTxQueue.h"

static int checks = 0;
static int failures = 0;

#define CHECK(expr) do { \
  checks++; \
  if (!(expr)) { \
    failures++; \
    std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
  } \
} while (0)

struct Sample {
  int x;
  int y;
};

static void test_uninitialized_and_zero_sized_buffer_is_safe() {
  LSCircularBuffer<Sample> buffer;
  CHECK(buffer.getLength() == 0);
  CHECK(buffer.getElement(0).x == 0);
  CHECK(buffer.getLastElement().y == 0);
  buffer.pushElement({1, 2});
  buffer.updateLastElement({3, 4});
  CHECK(buffer.getLength() == 0);

  buffer.begin(0);
  buffer.pushElement({5, 6});
  CHECK(buffer.getLength() == 0);
}

static void test_order_clamping_clear_and_rebegin() {
  LSCircularBuffer<Sample> buffer;
  buffer.begin(3);
  buffer.pushElement({1, 10});
  buffer.pushElement({2, 20});
  buffer.pushElement({3, 30});
  buffer.pushElement({4, 40});
  CHECK(buffer.getLength() == 3);
  CHECK(buffer.getElement(0).x == 4);
  CHECK(buffer.getElement(1).x == 3);
  CHECK(buffer.getElement(2).x == 2);
  CHECK(buffer.getElement(999).x == 2);  // Out-of-range depth clamps to oldest live sample

  buffer.clear();
  CHECK(buffer.getLength() == 0);
  CHECK(buffer.getLastElement().x == 0);
  buffer.updateLastElement({9, 90});
  CHECK(buffer.getLength() == 0);
  buffer.pushElement({5, 50});
  CHECK(buffer.getLength() == 1);
  CHECK(buffer.getLastElement().y == 50);

  // begin() may be called again without leaking or retaining the old shape.
  buffer.begin(2);
  CHECK(buffer.getLength() == 0);
  buffer.pushElement({7, 70});
  CHECK(buffer.getLastElement().x == 7);
}

static void test_pressure_engagement_rejects_single_sample_spikes() {
  constexpr int NONE = 0;
  constexpr int SIP = 1;
  constexpr int PUFF = 2;
  LSPressureEngageFilter filter(2);

  CHECK(filter.qualify(PUFF, false) == NONE); // One positive sample is inert
  CHECK(filter.qualify(NONE, false) == NONE); // Returning idle clears it
  CHECK(filter.qualify(SIP, false) == NONE);  // One negative sample is inert
  CHECK(filter.qualify(NONE, false) == NONE);

  CHECK(filter.qualify(PUFF, false) == NONE);
  CHECK(filter.qualify(PUFF, false) == PUFF); // Matching second sample engages
  CHECK(filter.qualify(SIP, false) == NONE);
  CHECK(filter.qualify(PUFF, false) == NONE); // Opposite direction restarts qualification
  CHECK(filter.qualify(PUFF, false) == PUFF);

  // Once a hold is active, continuation and release remain immediate; the
  // filter adds no release latency and does not alter hysteresis semantics.
  CHECK(filter.qualify(PUFF, true) == PUFF);
  CHECK(filter.qualify(NONE, true) == NONE);

  LSPressureEngageFilter immediate(1);
  CHECK(immediate.qualify(SIP, false) == SIP);
}

static void test_pressure_offset_only_accepts_stable_neutral_samples() {
  using Filter = LSPressureOffsetFilter;
  Filter filter(5, 10, 0.25f);

  // A normal periodic refresh may make a small stable correction.
  filter.begin(2.50f, true, 0.75f);
  CHECK(filter.addSample(true, 2.60f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.58f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.62f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.59f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.61f) == Filter::COMPLETE);
  CHECK(!filter.isRunning());
  CHECK(filter.resultOffset() > 2.59f && filter.resultOffset() < 2.61f);

  // A sip/puff during that refresh must abort immediately and leave the
  // caller's established offset untouched.
  filter.begin(2.50f, true, 0.75f);
  CHECK(filter.addSample(true, 5.50f) == Filter::ACTIVITY_REJECTED);
  CHECK(!filter.isRunning());

  // Even values inside the broad neutral band must form one stable run.
  filter.begin(2.50f, true, 0.75f);
  CHECK(filter.addSample(true, 2.30f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.70f) == Filter::ACTIVITY_REJECTED);

  // Failed I2C reads cannot bridge two groups into a five-sample average.
  filter.begin(2.50f, true, 0.75f);
  CHECK(filter.addSample(true, 2.50f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.50f) == Filter::COLLECTING);
  CHECK(filter.addSample(false, 0.0f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.50f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.50f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.50f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.50f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 2.50f) == Filter::COMPLETE);

  // First boot has no reference, so changing samples reset the candidate
  // instead of being blended. If a bounded attempt window expires, sensing
  // stays inert and collection retries rather than accepting a partial mean.
  Filter initial(5, 5, 0.25f);
  initial.begin(0.0f, false, 0.75f);
  CHECK(initial.addSample(true, 1.0f) == Filter::COLLECTING);
  CHECK(initial.addSample(true, 2.0f) == Filter::COLLECTING);
  CHECK(initial.addSample(true, 3.0f) == Filter::COLLECTING);
  CHECK(initial.addSample(true, 4.0f) == Filter::COLLECTING);
  CHECK(initial.addSample(true, 5.0f) == Filter::RETRYING_INITIAL);
  CHECK(initial.isRunning());
  for (int i = 0; i < 4; ++i) {
    CHECK(initial.addSample(true, 2.5f) == Filter::COLLECTING);
  }
  CHECK(initial.addSample(true, 2.5f) == Filter::COMPLETE);
}

// 4.8.1: the boot-baseline magnitude bound. A no-baseline (initial) run whose
// stable value exceeds the initial magnitude limit is a held gesture (or a
// shift too large to call neutral) — it must be rejected and retried, never
// averaged into the boot baseline (a baseline that absorbed a held sip/puff
// inverts the apparent direction after release: the phantom hold/click class).
// Established-baseline refreshes are NOT magnitude-bounded — their neutral
// band already guards them.
static void test_pressure_offset_boot_baseline_rejects_held_gestures() {
  using Filter = LSPressureOffsetFilter;
  Filter filter(5, 5, 0.25f, 3.0f);

  // A stable-but-beyond-limit run is refused sample by sample: the candidate
  // resets and the window keeps retrying (sensing stays inert).
  filter.begin(0.0f, false, 0.75f);
  CHECK(filter.addSample(true, 4.0f) == Filter::COLLECTING);   // beyond limit -> run reset
  CHECK(filter.addSample(true, 4.0f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 4.0f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 4.0f) == Filter::COLLECTING);
  CHECK(filter.addSample(true, 4.0f) == Filter::RETRYING_INITIAL);
  CHECK(filter.isRunning());  // still collecting — release re-opens collection
  // The user releases: a stable within-limit run now completes.
  for (int i = 0; i < 4; ++i) {
    CHECK(filter.addSample(true, 0.5f) == Filter::COLLECTING);
  }
  CHECK(filter.addSample(true, 0.5f) == Filter::COMPLETE);
  CHECK(!filter.isRunning());
  CHECK(filter.resultOffset() > 0.49f && filter.resultOffset() < 0.51f);

  // The bound applies only to the initial path: an established baseline
  // refresh still accepts a stable correction within its neutral band even
  // when that band sits beyond the initial limit.
  Filter refresh(5, 10, 0.25f, 3.0f);
  refresh.begin(2.50f, true, 0.75f);
  CHECK(refresh.addSample(true, 2.60f) == Filter::COLLECTING);
  CHECK(refresh.addSample(true, 2.58f) == Filter::COLLECTING);
  CHECK(refresh.addSample(true, 2.62f) == Filter::COLLECTING);
  CHECK(refresh.addSample(true, 2.59f) == Filter::COLLECTING);
  CHECK(refresh.addSample(true, 2.61f) == Filter::COMPLETE);

  // A limit of 0 disables the bound (the legacy shape the older tests pin).
  Filter unbounded(5, 5, 0.25f, 0.0f);
  unbounded.begin(0.0f, false, 0.75f);
  for (int i = 0; i < 4; ++i) {
    CHECK(unbounded.addSample(true, 6.0f) == Filter::COLLECTING);
  }
  CHECK(unbounded.addSample(true, 6.0f) == Filter::COMPLETE);
  CHECK(unbounded.resultOffset() > 5.99f && unbounded.resultOffset() < 6.01f);
}

static void drain_queue(LSSerialTxQueue<32>& queue, char* output, size_t& length, size_t chunkLimit) {
  while (queue.size() > 0) {
    const size_t chunk = queue.nextChunkSize(chunkLimit);
    const char* source = queue.readPointer();
    for (size_t i = 0; i < chunk; ++i) output[length++] = source[i];
    queue.consume(chunk);
  }
}

static void test_serial_queue_never_truncates_an_inflight_line() {
  LSSerialTxQueue<32> queue;
  CHECK(queue.enqueueLine("MANUAL,0:IA", 11));
  CHECK(queue.enqueueLine("DEBUG,1", 7));

  // Simulate a USB FIFO accepting only a prefix of the first line.
  char wire[96] = {};
  size_t wireLength = 0;
  size_t chunk = queue.nextChunkSize(4);
  CHECK(chunk == 4);
  for (size_t i = 0; i < chunk; ++i) wire[wireLength++] = queue.readPointer()[i];
  queue.consume(chunk);
  CHECK(queue.lineInProgress());

  // A request arriving now drops stale DEBUG output, but must preserve the
  // MANUAL remainder through its newline before the command response.
  queue.prioritizeRequest();
  CHECK(queue.enqueueLine("SUCCESS,0:VN,0:4.6.5", 20));
  drain_queue(queue, wire, wireLength, 3);
  wire[wireLength] = '\0';
  CHECK(std::string(wire) == "MANUAL,0:IA\nSUCCESS,0:VN,0:4.6.5\n");

  // With no transmitted prefix, prioritization may discard every stale line.
  CHECK(queue.enqueueLine("DEBUG,2", 7));
  queue.prioritizeRequest();
  CHECK(queue.size() == 0);

  // Overflow during an in-flight line follows the same rule: preserve that
  // line, discard whole lines behind it, then admit the newest response.
  CHECK(queue.enqueueLine("1234567890", 10));
  CHECK(queue.enqueueLine("stale-one", 9));
  CHECK(queue.enqueueLine("old", 3));
  chunk = queue.nextChunkSize(5);
  queue.consume(chunk);
  CHECK(queue.lineInProgress());
  CHECK(queue.enqueueLine("newest-response", 15));
  wireLength = 5;
  for (size_t i = 0; i < 5; ++i) wire[i] = "12345"[i];
  drain_queue(queue, wire, wireLength, 32);
  wire[wireLength] = '\0';
  CHECK(std::string(wire) == "1234567890\nnewest-response\n");
}

// Upstream #58: the hard sip/puff band classifier. Numeric states mirror
// PRESS_SAP_MAIN_STATE_*; thresholds mirror the compiled defaults (regular
// 3.0 hPa, hard 10.0 hPa, release fraction 0.7).
static void test_hard_band_classification_semantics() {
  constexpr int NONE = LSPRESSURE_CLASSIFY_NONE;
  constexpr int SIP = LSPRESSURE_CLASSIFY_SIP;
  constexpr int PUFF = LSPRESSURE_CLASSIFY_PUFF;
  constexpr int HARD_SIP = LSPRESSURE_CLASSIFY_HARD_SIP;
  constexpr int HARD_PUFF = LSPRESSURE_CLASSIFY_HARD_PUFF;
  const int WAITING = 0;  // PRESS_SAP_SEC_STATE_WAITING's numeric value
  const int STARTED = 1;  // PRESS_SAP_SEC_STATE_STARTED's numeric value

  // Fresh entries: hard wins by magnitude, regular bands below the hard line.
  CHECK(lipsyncClassifySapMainState(15.0f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == HARD_PUFF);
  CHECK(lipsyncClassifySapMainState(-15.0f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == HARD_SIP);
  CHECK(lipsyncClassifySapMainState(5.0f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == PUFF);
  CHECK(lipsyncClassifySapMainState(-5.0f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == SIP);
  CHECK(lipsyncClassifySapMainState(0.0f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == NONE);

  // Boundary exactness: just OVER the hard threshold is hard (strict `>`,
  // mirroring the original inline chain's comparisons); AT it is regular, and
  // AT the regular threshold is nothing.
  CHECK(lipsyncClassifySapMainState(10.1f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == HARD_PUFF);
  CHECK(lipsyncClassifySapMainState(10.0f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == PUFF);
  CHECK(lipsyncClassifySapMainState(9.9f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == PUFF);
  CHECK(lipsyncClassifySapMainState(3.0f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == NONE);
  CHECK(lipsyncClassifySapMainState(-10.1f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == HARD_SIP);
  CHECK(lipsyncClassifySapMainState(-10.0f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == SIP);
  CHECK(lipsyncClassifySapMainState(-9.9f, 3.0f, 3.0f, 10.0f, 10.0f, NONE, WAITING, STARTED) == SIP);

  // Hard-sticky DOWN: a hold that STARTED hard stays hard while decaying
  // through the regular band (8 hPa is below the 10 hPa hard line but above
  // the 2.1 hPa release boundary) — decay must not convert it into a click.
  CHECK(lipsyncClassifySapMainState(8.0f, 3.0f, 3.0f, 10.0f, 10.0f, HARD_PUFF, STARTED, STARTED) == HARD_PUFF);
  CHECK(lipsyncClassifySapMainState(-8.0f, 3.0f, 3.0f, 10.0f, 10.0f, HARD_SIP, STARTED, STARTED) == HARD_SIP);
  // Even a deeper-than-started re-dive stays the held class.
  CHECK(lipsyncClassifySapMainState(20.0f, 3.0f, 3.0f, 10.0f, 10.0f, HARD_PUFF, STARTED, STARTED) == HARD_PUFF);

  // No mid-hold UPGRADE: a regular hold pushed into the hard band stays
  // regular (one gesture, one action — never click-then-hard).
  CHECK(lipsyncClassifySapMainState(20.0f, 3.0f, 3.0f, 10.0f, 10.0f, PUFF, STARTED, STARTED) == PUFF);
  CHECK(lipsyncClassifySapMainState(-20.0f, 3.0f, 3.0f, 10.0f, 10.0f, SIP, STARTED, STARTED) == SIP);

  // Release hysteresis (the 4.5.7 semantics, subsumed by the continuation
  // rules): held puff stays until the pressure falls through 0.7x the
  // regular threshold (2.1 hPa here), then releases. The sip side mirrors it.
  CHECK(lipsyncClassifySapMainState(2.2f, 3.0f, 3.0f, 10.0f, 10.0f, PUFF, STARTED, STARTED) == PUFF);
  CHECK(lipsyncClassifySapMainState(2.0f, 3.0f, 3.0f, 10.0f, 10.0f, PUFF, STARTED, STARTED) == NONE);
  CHECK(lipsyncClassifySapMainState(-2.2f, 3.0f, 3.0f, 10.0f, 10.0f, SIP, STARTED, STARTED) == SIP);
  CHECK(lipsyncClassifySapMainState(-2.0f, 3.0f, 3.0f, 10.0f, 10.0f, SIP, STARTED, STARTED) == NONE);
  // A hard hold releases on the same regular-boundary rule.
  CHECK(lipsyncClassifySapMainState(2.0f, 3.0f, 3.0f, 10.0f, 10.0f, HARD_PUFF, STARTED, STARTED) == NONE);
  CHECK(lipsyncClassifySapMainState(-2.0f, 3.0f, 3.0f, 10.0f, 10.0f, HARD_SIP, STARTED, STARTED) == NONE);

  // Fail-safe: inverted or equal hard/regular thresholds make the hard band
  // empty — hard states never classify, the regular band keeps working.
  CHECK(lipsyncClassifySapMainState(15.0f, 3.0f, 3.0f, 2.0f, 2.0f, NONE, WAITING, STARTED) == PUFF);
  CHECK(lipsyncClassifySapMainState(-15.0f, 3.0f, 3.0f, 2.0f, 2.0f, NONE, WAITING, STARTED) == SIP);
  CHECK(lipsyncClassifySapMainState(15.0f, 3.0f, 3.0f, 3.0f, 3.0f, NONE, WAITING, STARTED) == PUFF);

  // The engage filter integrates with hard states exactly as with regular
  // ones: two consecutive identical fresh classifications to start.
  LSPressureEngageFilter filter(2);
  CHECK(filter.qualify(HARD_PUFF, false) == NONE);
  CHECK(filter.qualify(HARD_PUFF, false) == HARD_PUFF);
  CHECK(filter.qualify(PUFF, false) == NONE);   // State change restarts qualification
  CHECK(filter.qualify(PUFF, false) == PUFF);   // (a decayed hard entry starts regular)
  CHECK(filter.qualify(HARD_SIP, false) == NONE);
  CHECK(filter.qualify(HARD_SIP, false) == HARD_SIP);
}

// ---- Upstream #16: LSHostSwitch (multi-host switching decisions/rows) ----

static lsHostSwitchState makeReadyState(int rowCount, int activeRow) {
  lsHostSwitchState st = {};
  st.bleInitialized = 1;
  st.rowCount = rowCount;
  st.activeRow = activeRow;
  return st;
}

static void test_host_name_sanitization() {
  char out[LSHOST_NAME_WIRE_MAX + 1];

  lsHostSanitizeName("Hamoudi's MacBook Pro", out, sizeof(out));
  CHECK(std::string(out) == "Hamoudi's MacBook Pro");  // printable ASCII passes through

  lsHostSanitizeName("iPhone, John's:2/3", out, sizeof(out));
  CHECK(std::string(out) == "iPhone_ John's_2_3");  // , : / each become '_'

  lsHostSanitizeName("tab\there\x01", out, sizeof(out));
  CHECK(std::string(out) == "tab_here_");  // control bytes become '_'

  // Exact-cap truncation: 24 chars kept, the rest dropped, still NUL-terminated.
  lsHostSanitizeName("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123", out, sizeof(out));
  CHECK(std::string(out) == "ABCDEFGHIJKLMNOPQRSTUVWX");
  CHECK(out[LSHOST_NAME_WIRE_MAX] == '\0');

  out[0] = 'X';
  lsHostSanitizeName((const char*)0, out, sizeof(out));
  CHECK(out[0] == '\0');  // NULL name sanitizes to empty
  lsHostSanitizeName("", out, sizeof(out));
  CHECK(out[0] == '\0');
}

static void test_host_row_sort_is_deterministic() {
  lsHostRow rows[4];
  const char* macs[4] = {"112233445566", "001122334455", "aabbccddeeff", "090a0b0c0d0e"};
  for (int i = 0; i < 4; i++) {
    std::snprintf(rows[i].mac, sizeof(rows[i].mac), "%s", macs[i]);
    std::snprintf(rows[i].name, sizeof(rows[i].name), "H%d", i);
    rows[i].lastMacByte = 0;
    rows[i].active = 0;
  }
  lsHostSortRows(rows, 4);
  CHECK(std::string(rows[0].mac) == "001122334455");
  CHECK(std::string(rows[1].mac) == "090a0b0c0d0e");
  CHECK(std::string(rows[2].mac) == "112233445566");
  CHECK(std::string(rows[3].mac) == "aabbccddeeff");

  // Renumber-after-forget stability: removing a middle row keeps the
  // relative order of the rest (the app's re-read contract, cross-checked).
  lsHostRow three[3];
  for (int i = 0, src = 0; i < 3; i++, src++) {
    if (src == 1) src++;  // skip "090a0b0c0d0e"
    three[i] = rows[src];
  }
  lsHostSortRows(three, 3);
  CHECK(std::string(three[1].mac) == "112233445566");  // was row 3, now row 2
}

static void test_host_oled_label_and_overflow() {
  lsHostRow row;
  std::snprintf(row.mac, sizeof(row.mac), "AABBCCDDEEFF");
  std::snprintf(row.name, sizeof(row.name), "Hamoudi's MacBook Pro");
  row.lastMacByte = 0xFF;
  row.active = 1;
  char label[32];
  lsHostOledLabel(&row, label, sizeof(label));
  CHECK(std::string(label) == "Hamoudi's MacBook (FF)");  // 17 name chars + suffix

  row.lastMacByte = 0x3F;
  lsHostOledLabel(&row, label, sizeof(label));
  CHECK(std::string(label) == "Hamoudi's MacBook (3F)");

  std::snprintf(row.name, sizeof(row.name), "iPhone");  // short name: full + suffix
  lsHostOledLabel(&row, label, sizeof(label));
  CHECK(std::string(label) == "iPhone (3F)");

  CHECK(lsHostHiddenCount(4) == 0);
  CHECK(lsHostHiddenCount(5) == 1);
  CHECK(lsHostHiddenCount(7) == 3);
  CHECK(lsHostHiddenCount(0) == 0);
}

static void test_host_switch_decision_precedence() {
  lsHostSwitchState st = makeReadyState(3, 1);

  // The allow path: ready device, valid non-active row.
  CHECK(lsHostSwitchDecision(&st, 2) == LSHOST_ALLOW);
  CHECK(lsHostSwitchDecision(&st, 3) == LSHOST_ALLOW);

  // Fixed precedence (deterministic refusal note): uninitialized wins over
  // everything, then calibration, latch, frozen menu, range, already-active.
  st.bleInitialized = 0;
  st.calibrationRunning = 1;
  st.latchHeld = 1;
  st.menuInputFrozen = 1;
  CHECK(lsHostSwitchDecision(&st, 2) == LSHOST_REFUSE_NOT_INITIALIZED);
  st.bleInitialized = 1;
  CHECK(lsHostSwitchDecision(&st, 2) == LSHOST_REFUSE_CALIBRATION);
  st.calibrationRunning = 0;
  CHECK(lsHostSwitchDecision(&st, 2) == LSHOST_REFUSE_LATCH);
  st.latchHeld = 0;
  CHECK(lsHostSwitchDecision(&st, 2) == LSHOST_REFUSE_FROZEN);
  st.menuInputFrozen = 0;
  CHECK(lsHostSwitchDecision(&st, 4) == LSHOST_REFUSE_RANGE);
  CHECK(lsHostSwitchDecision(&st, 0) == LSHOST_REFUSE_RANGE);
  CHECK(lsHostSwitchDecision(&st, 1) == LSHOST_REFUSE_ALREADY_ACTIVE);

  // NULL state refuses (fail-safe direction: no flag can enable a radio call).
  CHECK(lsHostSwitchDecision((const lsHostSwitchState*)0, 1) == LSHOST_REFUSE_NOT_INITIALIZED);
}

static void test_host_forget_decision_allows_active() {
  lsHostSwitchState st = makeReadyState(3, 2);

  CHECK(lsHostForgetDecision(&st, 1) == LSHOST_ALLOW);  // inactive host
  CHECK(lsHostForgetDecision(&st, 2) == LSHOST_ALLOW);  // ACTIVE host: legal, disconnect first
  CHECK(lsHostForgetDecision(&st, 0) == LSHOST_REFUSE_RANGE);
  CHECK(lsHostForgetDecision(&st, 4) == LSHOST_REFUSE_RANGE);

  st.calibrationRunning = 1;
  CHECK(lsHostForgetDecision(&st, 1) == LSHOST_REFUSE_CALIBRATION);
  st.calibrationRunning = 0;
  st.bleInitialized = 0;
  CHECK(lsHostForgetDecision(&st, 1) == LSHOST_REFUSE_NOT_INITIALIZED);

  // A latch held does NOT refuse a forget (no radio touch for inactive rows).
  st.bleInitialized = 1;
  st.latchHeld = 1;
  CHECK(lsHostForgetDecision(&st, 1) == LSHOST_ALLOW);
}

// 4.8.1: row-identity verification. Rows are MAC-sorted and RENUMBER when the
// bond set changes, so the menu paths capture a row's MAC at render and verify
// it at execution — a stale index must refuse, never hit the wrong host (for a
// forget that means erasing the wrong bond). NULL/empty expected means the
// serial index contract: no check.
static void test_host_rpa_admission_helpers() {
  // The switch-window bouncer's RPA admission (4.9.0): the pure byte test
  // and the ah() byte-order contract, pinned with a fixed vector computed
  // against the Zephyr/Nordic rpa.c reference (IRK 000102...0f, prand
  // 334455 -> hash ab1f54; cipherBe d8343d6d3fc4ae4554475cc121541fab).
  {
    // RPA types: the two MSBs are 01. Nordic byte order: addr[5] carries them.
    const unsigned char rpa[6] = {0xAB, 0x1F, 0x54, 0x33, 0x44, 0x55};
    CHECK(lsHostAddrIsRpa(rpa) == 1);
    const unsigned char pub[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    CHECK(lsHostAddrIsRpa(pub) == 0);                       // 00 = public
    const unsigned char stat[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xC0};
    CHECK(lsHostAddrIsRpa(stat) == 0);                      // 11 = static random
    const unsigned char res[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
    CHECK(lsHostAddrIsRpa(res) == 0);                       // 10 = reserved (RRA)
    const unsigned char rpa2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x7F};
    CHECK(lsHostAddrIsRpa(rpa2) == 1);                      // 01 across the byte
  }
  {
    // Hash extraction: the LE ciphertext's least significant 24 bits are the
    // BE ciphertext's bytes [15],[14],[13] (the byte order the firmware's
    // sd_ecb_block_encrypt path relies on).
    unsigned char cipherBe[16] = {0xd8, 0x34, 0x3d, 0x6d, 0x3f, 0xc4, 0xae, 0x45,
                                  0x54, 0x47, 0x5c, 0xc1, 0x21, 0x54, 0x1f, 0xab};
    const unsigned char target[6] = {0xAB, 0x1F, 0x54, 0x33, 0x44, 0x55};
    CHECK(lsHostRpaHashMatches(target, cipherBe) == 1);     // the vector's own RPA
    const unsigned char wrongHash[6] = {0xAB, 0x1F, 0x55, 0x33, 0x44, 0x55};
    CHECK(lsHostRpaHashMatches(wrongHash, cipherBe) == 0);  // one hash bit off -> refuse
    const unsigned char wrongPrand[6] = {0xAB, 0x1F, 0x54, 0x33, 0x44, 0x56};
    CHECK(lsHostRpaHashMatches(wrongPrand, cipherBe) == 1); // hash compare ignores prand bytes (resolution supplies them)
    // A permutation that would pass a naive cipherBe[0..2] compare must NOT
    // pass the real extraction (the byte-order regression guard).
    unsigned char naiveTrap[16];
    std::memcpy(naiveTrap, cipherBe, 16);
    std::memmove(naiveTrap, naiveTrap + 13, 3);  // put the hash at [0..2] reversed? no: [13..15] = 54 1f ab
    const unsigned char naiveAddr[6] = {0x54, 0x1F, 0xAB, 0x33, 0x44, 0x55};
    CHECK(lsHostRpaHashMatches(naiveAddr, naiveTrap) == 0); // naive BE[0..2] order is refused
  }
}

static void test_host_row_identity_matches() {
  lsHostRow row;
  std::strcpy(row.mac, "A1B2C3D4E5F6");
  std::strcpy(row.name, "Laptop");
  row.lastMacByte = 0xF6;
  row.active = 0;

  // Exact match (12 hex chars + NUL) verifies.
  CHECK(lsHostRowIdentityMatches(&row, "A1B2C3D4E5F6") == 1);

  // A renumbered table presents a DIFFERENT mac at the same index — mismatch.
  CHECK(lsHostRowIdentityMatches(&row, "A1B2C3D4E5FF") == 0);
  CHECK(lsHostRowIdentityMatches(&row, "B1B2C3D4E5F6") == 0);

  // No identity captured: NULL or empty means "no check" (serial path).
  CHECK(lsHostRowIdentityMatches(&row, (const char*)0) == 1);
  CHECK(lsHostRowIdentityMatches(&row, "") == 1);

  // A too-short capture is a mismatch, not a prefix match.
  CHECK(lsHostRowIdentityMatches(&row, "A1B2C3D4E5F") == 0);

  // A NULL row with a real expected identity is a mismatch.
  CHECK(lsHostRowIdentityMatches((const lsHostRow*)0, "A1B2C3D4E5F6") == 0);

  // Uppercase hex is the on-disk convention (bonding.cpp %02X): lowercase is
  // NOT the same identity.
  CHECK(lsHostRowIdentityMatches(&row, "a1b2c3d4e5f6") == 0);
}

// 4.9.4: the bouncer's teardown horizon is measured from the BOUNCE, not the
// connect. The bug this pins: a non-target bounced LATE (an undecidable RPA
// peer waits out the 3 s decision grace first) had its 12 s horizon largely
// consumed before the bounce even dispatched, so a slow teardown tripped the
// refused-disconnect re-decide and re-counted the same dying connection.
static void test_bounce_teardown_horizon_is_from_dispatch() {
  const unsigned long HORIZON = 3000UL * 4UL;  // LSHOST_BOUNCE_DECISION_GRACE_MS * 4

  // Freshly bounced: teardown is definitely still running.
  CHECK(lsHostBounceTeardownStillRunning(0, HORIZON) == 1);
  CHECK(lsHostBounceTeardownStillRunning(HORIZON - 1, HORIZON) == 1);

  // At and past the horizon the hatch opens — the refused-disconnect escape.
  CHECK(lsHostBounceTeardownStillRunning(HORIZON, HORIZON) == 0);
  CHECK(lsHostBounceTeardownStillRunning(HORIZON + 1, HORIZON) == 0);

  // The regression itself: a connection bounced at t=3 s (after the grace)
  // and re-checked 1 s later is 1 s into its HORIZON, not 4 s into it. Under
  // the old connect-keyed math this read as (3000+1000)-0 >= 12000? No —
  // but at 10 s after connect it did, one second after the bounce. Pin the
  // dispatch-relative answer for that exact shape: still tearing down.
  const unsigned long connectToBounceMs = 3000UL;
  const unsigned long sinceConnect = connectToBounceMs + 1000UL;  // 1 s after the bounce
  CHECK(lsHostBounceTeardownStillRunning(sinceConnect - connectToBounceMs, HORIZON) == 1);

  // And the same instant measured from CONNECT (the old, wrong anchor) is
  // well past a 4 s comparison — demonstrating the two anchors disagree,
  // which is the whole point of the fix.
  CHECK(lsHostBounceTeardownStillRunning(sinceConnect, 4000UL) == 0);
}

// 4.9.5 (final-diff-audit): pin the SHIPPED anchor selection, not just the
// comparison helper. The 4.9.4 notes claimed the connect-stamp fallback
// served "the refused-disconnect case the hatch is for" — it does not: the
// stamp is taken at the decision point BEFORE the disconnect attempt, so a
// refused disconnect is also dispatch-anchored, and the fallback is only the
// millis()==0 wrap-tick guard.
static void test_bounce_horizon_anchor_selection() {
  // Normal dispatch: the dispatched stamp wins, whatever the connect stamp is.
  CHECK(lsHostBounceHorizonAnchor(1000UL, 0UL) == 1000UL);
  CHECK(lsHostBounceHorizonAnchor(4000UL, 1000UL) == 4000UL);

  // Wrap-tick guard: a dispatch landing exactly on millis()==0 falls back to
  // the connect stamp (the pre-4.9.4 anchor for that one connection).
  CHECK(lsHostBounceHorizonAnchor(0UL, 3000UL) == 3000UL);

  // Both zero (never bounced, wrap-tick at connect): the anchor is 0 — the
  // caller's unsigned subtraction then measures from the wrap, which is the
  // honest reading of that degenerate state.
  CHECK(lsHostBounceHorizonAnchor(0UL, 0UL) == 0UL);
}

// 4.9.5: the latch-crossing guard. Pins the semantics the radio bench cannot
// reach cheaply (outputAction has no wire readout, so the field test would
// need a precisely timed physical gesture).
static void test_latch_crossing_guard() {
  // No latch held: never release, whatever the keys say (this is what keeps a
  // latch-free bounce storm from re-running releaseOutputAction).
  CHECK(lsHostLatchCrossedHost("A1B2C3D4E5F6", "", 0) == 0);
  CHECK(lsHostLatchCrossedHost("A1B2C3D4E5F6", "FFFFFFFFFFFF", 0) == 0);

  // Same host, latch held: keep it (the hold continues on its own host).
  CHECK(lsHostLatchCrossedHost("A1B2C3D4E5F6", "A1B2C3D4E5F6", 1) == 0);

  // Host changed with a latch held: release — the confirmed 4.9.5 minor.
  CHECK(lsHostLatchCrossedHost("A1B2C3D4E5F6", "FFFFFFFFFFFF", 1) == 1);

  // none -> host (the switch-admission edge): "" -> a real key releases.
  CHECK(lsHostLatchCrossedHost("A1B2C3D4E5F6", "", 1) == 1);
  // host -> none (disconnect): releases too (the pre-4.9.4 behavior kept).
  CHECK(lsHostLatchCrossedHost("", "A1B2C3D4E5F6", 1) == 1);
  // none -> none: nothing to do.
  CHECK(lsHostLatchCrossedHost("", "", 1) == 0);

  // Full-string equality, not a prefix match: a 12-char key must not equal a
  // differently-terminated 12-char key, and identical 12-char keys must match.
  CHECK(lsHostLatchCrossedHost("A1B2C3D4E5F6", "A1B2C3D4E5F", 1) == 1);   // shorter -> differs at NUL
  CHECK(lsHostLatchCrossedHost("A1B2C3D4E5F", "A1B2C3D4E5F6", 1) == 1);   // the other direction
  CHECK(lsHostLatchCrossedHost("A1B2C3D4E5F6", "A1B2C3D4E5F6", 1) == 0);  // equal

  // NULL keys are harmless (defensive: a caller with no host reference).
  CHECK(lsHostLatchCrossedHost((const char*)0, "A1B2C3D4E5F6", 1) == 0);
  CHECK(lsHostLatchCrossedHost("A1B2C3D4E5F6", (const char*)0, 1) == 0);

  // RPA-stability property the fix RELIES on: the key is the STORED BOND
  // identity, so a privacy host's rotating address must NOT produce an edge.
  // Simulated here as the same stored key across passes while the live peer
  // address would have rotated — the helper sees only the stored key.
  const char* storedIdentity = "9DE25364404E";  // a bond's stored identity
  CHECK(lsHostLatchCrossedHost(storedIdentity, storedIdentity, 1) == 0);
  CHECK(lsHostLatchCrossedHost(storedIdentity, storedIdentity, 1) == 0);
}

int main() {
  static_assert(!std::is_copy_constructible<LSCircularBuffer<int>>::value,
                "owning circular buffers must not be shallow-copyable");
  static_assert(!std::is_copy_assignable<LSCircularBuffer<int>>::value,
                "owning circular buffers must not be shallow-copy-assignable");

  test_uninitialized_and_zero_sized_buffer_is_safe();
  test_order_clamping_clear_and_rebegin();
  test_pressure_engagement_rejects_single_sample_spikes();
  test_pressure_offset_only_accepts_stable_neutral_samples();
  test_pressure_offset_boot_baseline_rejects_held_gestures();
  test_serial_queue_never_truncates_an_inflight_line();
  test_hard_band_classification_semantics();
  test_host_name_sanitization();
  test_host_row_sort_is_deterministic();
  test_host_oled_label_and_overflow();
  test_host_switch_decision_precedence();
  test_host_forget_decision_allows_active();
  test_host_row_identity_matches();
  test_host_rpa_admission_helpers();
  test_bounce_teardown_horizon_is_from_dispatch();
  test_bounce_horizon_anchor_selection();
  test_latch_crossing_guard();

  std::printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
