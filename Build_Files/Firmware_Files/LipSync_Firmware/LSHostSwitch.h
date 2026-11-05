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
                                  /* MAC-lowest 8 when more exist; Hub shows 4,    */
                                  /* bench B8 exercises 5; Forget is the valve)    */

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

/* OLED label: sanitized name truncated to LSHOST_OLED_NAME_MAX chars, then
 * " (XX)" with XX = the last MAC byte in two hex digits — the discriminator
 * for two identically-named hosts. cap must be >= 7 to hold any suffix. */
static inline void lsHostOledLabel(const lsHostRow* row, char* out, int cap) {
  int o = 0;
  if (cap <= 0 || row == (const lsHostRow*)0) return;
  for (; row->name[o] != '\0' && o < LSHOST_OLED_NAME_MAX && o < cap - 1; o++) {
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

#endif  // LSHOST_SWITCH_H
