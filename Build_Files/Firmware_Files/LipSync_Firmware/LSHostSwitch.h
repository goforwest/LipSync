#ifndef LSHOST_SWITCH_H
#define LSHOST_SWITCH_H

// Pure decision + row-model layer for multi-host Bluetooth switching
// (upstream #16), split out so the host suite can pin the semantics —
// the same pattern as LSPressureClassify.h. No Arduino types; every
// string is a plain NUL-terminated char array.
//
// STORAGE MODEL (the design's central decision, docs/design/16-multi-host-ble.md):
// the host table IS the Bluefruit bond directory — one file per bonded peer
// under /adafruit/bond_prph/<12-hex-MAC> (vendored lib bonding.h), holding the
// keys AND the peer name captured at pair time. This layer therefore owns NO
// persistent state: rows are enumerated fresh (MAC-sorted for a deterministic
// order), the active host is runtime truth, and no settings file or budget is
// touched. Forgetting a row renumbers later rows (MAC order is stable, the
// count shrinks) — every caller must re-enumerate after a forget and the app
// re-reads; a cross-check pins that contract.
//
// CONTRACT (consumed by the LSAPI.ino endpoint shell and the LSScreen.h pages):
//  - Decisions are pure: callers gather the live flags, this layer returns
//    ALLOW or ONE refusal reason. Precedence is fixed (see switchDecision) so
//    a refusal's user-facing note is deterministic; tests pin the order.
//  - Forgetting the ACTIVE host is legal (design: disconnect first, then
//    remove the bond) — only range/calibration/uninitialized refuse a forget.
//    A forget of a non-active host never touches the radio.
//  - Wire names are SANITIZED (',' ':' '/' and non-printables -> '_') and
//    capped, so the comma/colon-delimited response grammar can never be
//    broken by a host's chosen Bluetooth name ("iPhone, John's" etc.).
//  - Fail-safe direction: an unknown/absent flag reads as "refuse" on the
//    switch path (a caller that forgets to set bleInitialized=1 gets a
//    refusal, never a radio call on a dead stack).

#define LSHOST_MAC_CHARS 12       /* 6 bytes as 12 hex chars — the bond filename stem */
#define LSHOST_NAME_WIRE_MAX 24   /* sanitized wire-name cap (chars, excl. NUL) */
#define LSHOST_OLED_NAME_MAX 17   /* OLED label name portion; + " (XX)" suffix */
#define LSHOST_MENU_MAX_ROWS 4    /* Hub page shows at most 4 host rows */
#define LSHOST_ENUM_CAP 8         /* wire/list cap: up to 8 bonds enumerated (the   */
                                  /* MAC-lowest 8 when more exist — except the      */
                                  /* ACTIVE host is always included (4.9.9 round 3, */
                                  /* lsHostCapEvictIndex); Hub shows 4, bench B8    */
                                  /* exercises 5; Forget is the valve)              */

/* Refusal reasons — the numeric values are internal; shells map them to
 * FAIL,3 + a named note (serial) or a note page (menu). */
#define LSHOST_ALLOW 0
#define LSHOST_REFUSE_NOT_INITIALIZED 1
#define LSHOST_REFUSE_CALIBRATION 2
#define LSHOST_REFUSE_LATCH 3
#define LSHOST_REFUSE_FROZEN 4
#define LSHOST_REFUSE_RANGE 5
#define LSHOST_REFUSE_ALREADY_ACTIVE 6
#define LSHOST_REFUSE_RADIO 7   /* the SoftDevice refused the disconnect/forget call */

typedef struct {
  char mac[LSHOST_MAC_CHARS + 1]; /* 12 lowercase-free hex chars, NUL-terminated */
  char name[LSHOST_NAME_WIRE_MAX + 1];
  int lastMacByte;                /* 0..255 — the OLED "(XX)" discriminator */
  int active;                     /* 1 = this host is the connected one */
} lsHostRow;

typedef struct {
  int bleInitialized;      /* 1 = the BLE init task reported full success */
  int calibrationRunning;  /* 1 = full calibration or center flow in flight */
  int latchHeld;           /* 1 = a drag/scroll hold is engaged (host button down) */
  int menuInputFrozen;     /* 1 = a scheduled transition/dwell owns the menu */
  int rowCount;            /* enumerated bonds */
  int activeRow;           /* 1-based row of the connected host, 0 = none */
} lsHostSwitchState;

/* Replace ',' ':' '/' and any byte outside printable 7-bit ASCII with '_',
 * truncate to cap-1 chars, always NUL-terminate. in may be NULL/empty. */
static inline void lsHostSanitizeName(const char* in, char* out, int cap) {
  int o = 0;
  if (cap <= 0) return;
  if (in != (const char*)0) {
    for (; in[o] != '\0' && o < cap - 1; o++) {
      char c = in[o];
      if (c == ',' || c == ':' || c == '/' || c < 0x20 || c > 0x7E) c = '_';
      out[o] = c;
    }
  }
  out[o] = '\0';
}

/* Insertion sort by mac (ascending byte-wise) — deterministic row order.
 * n may be 0. Rows beyond n are untouched. */
static inline void lsHostSortRows(lsHostRow* rows, int n) {
  for (int i = 1; i < n; i++) {
    lsHostRow key = rows[i];
    int j = i - 1;
    while (j >= 0) {
      int less = 0;
      for (int k = 0; k <= LSHOST_MAC_CHARS; k++) {
        if (rows[j].mac[k] != key.mac[k]) {
          less = ((unsigned char)rows[j].mac[k] < (unsigned char)key.mac[k]);
          break;
        }
        if (rows[j].mac[k] == '\0') { less = 1; break; } /* shorter prefix sorts first */
      }
      if (less) break;  /* rows[j] < key: key's slot is j+1 — stop shifting */
      rows[j + 1] = rows[j];
      j--;
    }
    rows[j + 1] = key;
  }
}

/* Rows the 4-slot Hub page cannot show. */
static inline int lsHostHiddenCount(int rowCount) {
  return (rowCount > LSHOST_MENU_MAX_ROWS) ? (rowCount - LSHOST_MENU_MAX_ROWS) : 0;
}

/* Strict byte-wise mac12 ordering ("shorter prefix sorts first", the
 * lsHostSortRows convention) without string.h — the same NUL-terminated
 * fixed-width key the sort uses. */
static inline int lsHostMacLess(const char* a, const char* b) {
  for (int k = 0; k <= LSHOST_MAC_CHARS; k++) {
    if (a[k] != b[k]) return ((unsigned char)a[k] < (unsigned char)b[k]);
    if (a[k] == '\0') return 0;  // equal keys
  }
  return 0;  // both ran the full length: equal
}

/* 4.9.9 review round 3: the enumeration cap's eviction decision, extracted
 * from LSBLE.h's lsHostEnumerate so it is host-testable (the LSHostSwitch.h
 * pattern). The caller consults it once per bond seen while the capped set
 * is FULL (rows holds the current set; cap == its length).
 *
 * Returns the slot index to overwrite, or -1 to keep the current set.
 *   - The ACTIVE row is never evicted, and an active newcomer ALWAYS takes
 *     a slot: with more bonds than the cap, a connected host whose MAC
 *     sorts high used to fall outside the enumerated set — BT,5 then
 *     reported "no active host" while a host WAS connected, the app showed
 *     it as connected-but-unpaired, and the already-active switch refusal
 *     could never fire for it. Only one row can be active, so a non-active
 *     slot always exists for the newcomer to displace.
 *   - A non-active newcomer yields to nothing unless it is MAC-lower than
 *     the MAC-highest NON-active slot (the documented MAC-lowest set,
 *     unchanged for every non-active case). */
static inline int lsHostCapEvictIndex(const lsHostRow* rows, int cap,
                                      const char* newMac12, int newIsActive) {
  if (rows == (const lsHostRow*)0 || cap < 1) return -1;
  int worst = -1;
  for (int i = 0; i < cap; i++) {
    if (rows[i].active) continue;  // the connected host is not evictable
    if (worst < 0 || lsHostMacLess(rows[worst].mac, rows[i].mac)) worst = i;
  }
  if (newIsActive) return worst;
  if (worst < 0) return -1;  // defensive: an all-active set is unreachable
  return lsHostMacLess(newMac12, rows[worst].mac) ? worst : -1;
}

/* OLED label: sanitized name truncated to LSHOST_OLED_NAME_MAX chars, then
 * " (XX)" with XX = the last MAC byte in two hex digits — the discriminator
 * for two identically-named hosts. cap must be >= 7 to hold any suffix. */
static inline void lsHostOledLabel(const lsHostRow* row, char* out, int cap) {
  int o = 0;
  if (cap <= 0 || row == (const lsHostRow*)0) return;
  // (5.0.1 round-2 audit) The name portion yields room for the " (XX)"
  // suffix (cap - 6): the old free-for-name loop meant a caller with a
  // tight cap and a full-length name (LSHOST_OLED_NAME_MAX = 17) failed the
  // suffix's fit check and got a BARE name — the (XX) discriminator, the
  // only thing that distinguishes two same-named hosts, silently dropped
  // for exactly the longest (most common real-phone) name class. cap - 6
  // changes nothing for the wide callers (cap 24: min(17, 24-6) is still
  // the 17-name cap, suffix 17+5=22 <= 23 fits as before) and gives tight
  // callers a guaranteed whole label: name <= cap-6, suffix always fits.
  for (; row->name[o] != '\0' && o < LSHOST_OLED_NAME_MAX && o < cap - 6; o++) {
    out[o] = row->name[o];
  }
  static const char hex[] = "0123456789ABCDEF";
  if (o + 5 <= cap - 1) {
    out[o++] = ' ';
    out[o++] = '(';
    out[o++] = hex[(row->lastMacByte >> 4) & 0xF];
    out[o++] = hex[row->lastMacByte & 0xF];
    out[o++] = ')';
  }
  out[o] = '\0';
}

/* Switch precondition. Precedence is FIXED (uninitialized first, then
 * calibration, latch, frozen menu, range, already-active) — the refusal note
 * the user sees is deterministic and test-pinned. targetRow is 1-based. */
static inline int lsHostSwitchDecision(const lsHostSwitchState* st, int targetRow) {
  if (st == (const lsHostSwitchState*)0) return LSHOST_REFUSE_NOT_INITIALIZED;
  if (!st->bleInitialized) return LSHOST_REFUSE_NOT_INITIALIZED;
  if (st->calibrationRunning) return LSHOST_REFUSE_CALIBRATION;
  if (st->latchHeld) return LSHOST_REFUSE_LATCH;
  if (st->menuInputFrozen) return LSHOST_REFUSE_FROZEN;
  if (targetRow < 1 || targetRow > st->rowCount) return LSHOST_REFUSE_RANGE;
  if (st->activeRow == targetRow) return LSHOST_REFUSE_ALREADY_ACTIVE;
  return LSHOST_ALLOW;
}

/* Forget precondition. Deliberately does NOT check latch/frozen (a forget of
 * an inactive host touches no radio state) and allows forgetting the ACTIVE
 * host — the caller disconnects first (design Q8 item 4 / Q4 asymmetry). */
static inline int lsHostForgetDecision(const lsHostSwitchState* st, int targetRow) {
  if (st == (const lsHostSwitchState*)0) return LSHOST_REFUSE_NOT_INITIALIZED;
  if (!st->bleInitialized) return LSHOST_REFUSE_NOT_INITIALIZED;
  if (st->calibrationRunning) return LSHOST_REFUSE_CALIBRATION;
  if (targetRow < 1 || targetRow > st->rowCount) return LSHOST_REFUSE_RANGE;
  return LSHOST_ALLOW;
}

/* Row-identity verification (4.8.1): rows are MAC-sorted and RENUMBER whenever
 * the bond set changes, so a range check alone cannot make a stale index
 * "only ever refuse" — a concurrent forget renumbers rows in-range and the
 * executor would hit the WRONG host. Callers that captured a row's identity
 * when they rendered it (the Hub pages) pass it here; a mismatch means the
 * world moved under the index and the action must refuse (the serial path
 * keeps the documented index contract and passes NULL).
 *
 * Semantics: a NULL or EMPTY expected mac means "no identity captured — no
 * check" (the serial path). Any non-empty expected that does not match
 * character-for-character (12 uppercase hex + NUL) is a MISMATCH — including
 * a too-short capture and a lowercase form (the on-disk convention is
 * bonding.cpp's %02X uppercase). A NULL row with a real expected identity is
 * a mismatch too. */
static inline int lsHostRowIdentityMatches(const lsHostRow* row, const char* expectedMac12) {
  if (expectedMac12 == (const char*)0 || expectedMac12[0] == '\0') return 1;  // no identity captured — no check
  if (row == (const lsHostRow*)0) return 0;
  for (int i = 0; i < LSHOST_MAC_CHARS; i++) {
    if (expectedMac12[i] == '\0') return 0;  // captured identity too short
    if (row->mac[i] != expectedMac12[i]) return 0;
  }
  return row->mac[LSHOST_MAC_CHARS] == '\0' && expectedMac12[LSHOST_MAC_CHARS] == '\0';
}

/* RPA admission for the switch window's bouncer (4.9.0): an iOS-style privacy
 * target reconnects with a rotating resolvable private address whose bytes
 * never equal the bond's stored identity — a bytes-only match bounced the
 * legitimate target for the whole window. Two pure pieces (host-tested):
 *
 * - lsHostAddrIsRpa: the two most significant address bits are 01 (Core 5.3
 *   Vol 6 Part B 1.3.2.2). Nordic byte order: addr[5] carries them.
 *
 * - lsHostRpaHashMatches: the byte-order contract of the ah() resolution
 *   (Core 5.3 Vol 3 Part H 2.2.2, Zephyr/Nordic rpa.c ah()): the RPA is
 *   prand = addr[3..5] (addr[5] carries the type bits) and hash = addr[0..2];
 *   hash equals the LE-AES ciphertext's least significant 24 bits, which —
 *   when the AES core is big-endian (sd_ecb_block_encrypt) — are the BE
 *   ciphertext's bytes [15], [14], [13]. The firmware computes cipherBe with
 *   the SD's ECB and passes it here; the host suite pins the byte order with
 *   a fixed vector (IRK 000102...0f, prand 334455 -> hash ab1f54,
 *   cipherBe d8343d6d3fc4ae4554475cc121541fab). */
static inline int lsHostAddrIsRpa(const unsigned char addr[6]) {
  return (addr[5] & 0xC0) == 0x40;
}

static inline int lsHostRpaHashMatches(const unsigned char peerAddr[6], const unsigned char cipherBe[16]) {
  return peerAddr[0] == cipherBe[15]
      && peerAddr[1] == cipherBe[14]
      && peerAddr[2] == cipherBe[13];
}

 /* Teardown-horizon decision for the bouncer's same-handle tracking (4.9.4,
  * contract corrected 4.9.5 by the final-diff-audit).
  *
  * A bounced non-target's handle lingers while the SoftDevice tears the link
  * down; while it does, the tick must skip quietly (4.8.16: one connection =
  * one bounce). The 4.9.0 escape hatch re-decides a handle that lingers too
  * long, so a REFUSED disconnect can never surrender the lockout forever.
  *
  * That horizon must be measured from the BOUNCE, not from the connect. The
  * original form keyed on the connect stamp under a 4x-grace (12 s) horizon,
  * which is wrong in two ways for a connection that was bounced LATE — an
  * undecidable RPA peer sits through the 3 s decision grace first, and any
  * slow/loaded peer can outlive the remainder. The re-decide then fires while
  * the ORIGINAL connection is still tearing down, re-counting and
  * re-disconnecting the same dying link: the bounce counter races toward its
  * cap (600), and the cap's only exit is the MISMATCH close — an honest
  * "switch NOT confirmed" report for a window the bouncer was still
  * legitimately holding.
  *
  * ANCHOR SELECTION (the shipped truth — the 4.9.4 notes mis-attributed the
  * fallback): g_switchBounceDispatchedMs is stamped at the decision point,
  * BEFORE the disconnect attempt whose rc is deliberately ignored — so a
  * REFUSED disconnect is ALSO dispatch-anchored (the stamp marks the attempt,
  * and each re-observe re-stamps at its own decision point). The
  * connect-stamp fallback below is therefore NOT the refused-case path; it
  * exists solely as a guard for the millis()==exactly-0 dispatch tick (the
  * ~49.7-day wrap instant — the boot-first-millisecond alternative is
  * impossible because windows open only from the serial/menu executor long
  * after setup). In every reachable state the dispatched stamp is nonzero
  * when the anchor is selected (counted==true is set only at the site that
  * co-stamps it).
  * elapsedMs is the caller's (millis() - horizonStart); wrap-safe because the
  * caller subtracts in unsigned arithmetic.
  */
static inline unsigned long lsHostBounceHorizonAnchor(unsigned long dispatchedMs, unsigned long connectMs) {
  return dispatchedMs ? dispatchedMs : connectMs;
}

static inline int lsHostBounceTeardownStillRunning(unsigned long elapsedMs, unsigned long horizonMs) {
  return elapsedMs < horizonMs;
}

/* Latch-crossing guard (4.9.5): must a held DRAG/SCROLL latch be released
 * because the CONNECTED HOST changed?
 *
 * The contract: a drag/scroll latch is a HOLD on the wire, so it must never
 * cross to a different host — the new host would receive a button/wheel state
 * the user never pressed on it, and the user's first input there would be
 * consumed releasing the stale hold instead of performing its action (the
 * 4.9.4 final-diff-audit's confirmed minor: the pre-4.9.5 guard keyed on
 * "disconnected", which the switch window suppresses, so a latch engaged
 * MID-WINDOW crossed into the admitted target).
 *
 * Two callers, two questions, one rule:
 *   - the DISCONNECTED trigger (never fires during a window) — the pre-4.9.4
 *     behavior, unchanged;
 *   - the HOST-KEY edge (fires in every ordering, including a window so short
 *     the 1 s poll never observes it open): release when the connected host's
 *     identity differs from the one the latch was armed under.
 *
 * Both keys are the STORED BOND IDENTITY (LSBLE.h's lsHostActiveMac ->
 * BLEConnection::getPeerAddr, which returns _bond_id_addr whenever bonded),
 * never the live peer address: a privacy host's rotating RPA would otherwise
 * make consecutive reads differ with no host change and spuriously drop a
 * legitimate hold. An ABSENT host keys as "" (the caller maps
 * haveActiveHost==false to the empty string) so none->host and host->none are
 * ordinary edges.
 *
 * latchHeld is the caller's (outputAction == DRAG || SCROLL). A latch-free
 * pass never needs a release, so the caller may skip the whole query; this
 * helper is pure and takes the already-resolved keys so it stays host-testable
 * (the LSHostSwitch.h pattern).
 */
static inline int lsHostLatchCrossedHost(const char* currentHostKey, const char* latchHostKey, int latchHeld) {
  if (!latchHeld) return 0;
  if (currentHostKey == (const char*)0 || latchHostKey == (const char*)0) return 0;
  // Compare up to and including the NUL, but STOP at the first NUL: the keys
  // are short strings ("" for an absent host), so a fixed-count loop would
  // read past a short literal's end. The 4.9.5 host suite caught exactly that
  // (ASan global-buffer-overflow reading 1 byte past ""). Differing lengths
  // still compare unequal — one side's NUL meets the other's character.
  for (int i = 0; i <= LSHOST_MAC_CHARS; i++) {
    if (currentHostKey[i] != latchHostKey[i]) return 1;
    if (currentHostKey[i] == '\0') return 0;  // both NUL here (the != test above passed)
  }
  return 0;  // both keys ran the full length with no NUL: equal
}

/* 4.9.7: the latch guard's PER-PASS update, extracted from btFeedbackLoop so
 * the refresh cadence itself is host-pinned — the 4.9.6 suite pinned only the
 * crossing comparison above and could not see the glue defect the review
 * found: the .ino refreshed the stored key on latch-HELD passes only, so
 * across every latch-free stretch the key retained the PREVIOUS episode's
 * host ("" after a boot with a bonded host, the old host's key after a BT,1
 * switch), and the user's FIRST drag/scroll on the new host was compared
 * against that stale key, hit the crossing edge, and was killed mid-hold —
 * the exact one-swallowed-input class the guard exists to prevent,
 * reintroduced by the guard itself.
 *
 * Semantics (the fix, and the contract going forward):
 *   - the stored key is refreshed to the current host key on EVERY pass,
 *     held or not, so between episodes the key simply tracks the live host;
 *   - the crossing decision is consulted ONLY on a pass where the hold was
 *     ALREADY live on the previous pass (latchWasHeld, 4.9.9 review round 3),
 *     and it compares the stored key BEFORE the refresh, so a hold engaged
 *     under host A releases on the first pass that sees host B;
 *   - a FIRST held pass is an ARM: no comparison at all — the refresh adopts
 *     the current key. The 4.9.7 every-pass refresh still compared a hold's
 *     first held pass against a key one episode behind whenever the host
 *     changed inside the 1 s observation gap (the switch window admits its
 *     target on the millisecond-granularity service, between two
 *     btFeedbackLoop passes), killing the first drag/scroll started within a
 *     second of the new host's connection. With the arm rule a hold that
 *     starts on a host is never compared against any earlier episode's key
 *     whenever the previous guard pass saw the latch-free state — the arm
 *     adopts the live key first. KNOWN CONSERVATIVE RESIDUAL (4.9.9 review
 *     round 4, behavior identical to every earlier shape): a hold RELEASED
 *     and RE-ENGAGED within the same observation gap as a host change still
 *     compares — the guard's last observation saw the previous hold live
 *     (latchWasHeld stale 1) and cannot distinguish the re-engage from a
 *     hold that truly crossed, so it releases. The kill is the conservative
 *     reading of an ambiguous sub-second window; the user's next gesture
 *     works. The documented none->host edge keeps its meaning (a hold armed
 *     while absent carries "" and releases when a host arrives — a later
 *     held pass compares "" against the host key).
 *
 * Copies currentHostKey into guardKey, NUL-terminated, bounded by
 * guardKeySize. NULL guardKey, zero size, or NULL currentHostKey are
 * defensive no-ops (same convention as lsHostLatchCrossedHost; the caller
 * maps an absent host to "" rather than NULL). Returns 1 when a held latch
 * must be released this pass. The caller keeps the previous pass's
 * latchHeld and passes it as latchWasHeld.
 */
static inline int lsHostLatchGuardPass(const char* currentHostKey, char* guardKey, unsigned guardKeySize, int latchHeld, int latchWasHeld) {
  const int crossed = (latchHeld && latchWasHeld) ? lsHostLatchCrossedHost(currentHostKey, guardKey, 1) : 0;
  if (guardKey != (char*)0 && guardKeySize > 0) {
    unsigned i = 0;
    if (currentHostKey != (const char*)0) {
      while (i + 1 < guardKeySize && currentHostKey[i] != '\0') {
        guardKey[i] = currentHostKey[i];
        i++;
      }
    }
    guardKey[i] = '\0';
  }
  return crossed;
}

#endif  // LSHOST_SWITCH_H
