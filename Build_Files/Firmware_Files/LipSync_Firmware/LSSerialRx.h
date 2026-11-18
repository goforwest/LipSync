#ifndef LSSERIALRX_H
#define LSSERIALRX_H

#include <stddef.h>

// Pure decision layer for the serial RX framing (serialSettings), split out
// so the host suite can pin the semantics — the same pattern as
// LSI2CScan.h/LSHostSwitch.h. Deliberately compiles without Arduino headers
// (the message BUFFER stays the caller's String; this layer only decides
// what one incoming byte does and tracks the swallow state), so the .ino
// pins its own constant to the one below.
//
// THE DEFECT THIS ENCODES (4.9.9 review round 3): the overflow path answered
// FAIL,1 once and cleared the buffer, but the REST of the host's frame kept
// accumulating and was processed as a NEW message when the newline (or the
// 50 ms idle gap) finally landed — one host frame produced two responses
// (the code's own comment promised "discard and answer once"), and a flood
// whose tail spelled "SETTINGS" re-entered settings mode immediately after
// the overflow had deliberately forced it off (the "a garbage flood ends the
// mode" recovery contract, defeated by the flood itself).

// Longest legal command is 11 bytes; past this it is a flood. The .ino's
// SERIAL_RX_MAX_LEN must equal this (static_assert there).
#define LS_RX_MAX_LEN 32

typedef enum {
  LS_RX_NONE = 0,       // swallow/blank-line: no buffer change, no dispatch
  LS_RX_ACCUMULATE = 1, // append the byte to the message buffer
  LS_RX_DISPATCH = 2,   // the frame is complete: process the buffer (then clear it)
  LS_RX_OVERFLOW = 3    // the frame is over-long: answer once, clear the buffer, swallow the rest
} LsRxAction;

/* One incoming byte's framing decision.
 *
 *   c            the byte (any value; '\n' and '\r' are the delimiters)
 *   bufLen       the CURRENT accumulated length (the caller's buffer's length)
 *   discarding   1 = the rest of an already-answered over-long frame is being swallowed
 *   outBufLen    the length after this byte (ACCUMULATE: bufLen+1; DISPATCH/OVERFLOW: 0;
 *                NONE: unchanged) — may equal bufLen when no change
 *   outDiscarding the swallow flag after this byte (a delimiter while discarding ends the
 *                frame; an OVERFLOW starts it)
 *
 * Rules, in order:
 *   - delimiter (\n or \r) while discarding: the drowned frame ends here —
 *     clear the flag, no dispatch (the overflow already answered);
 *   - delimiter with an empty buffer: blank line (e.g. the \n of a \r\n
 *     pair) — no dispatch;
 *   - delimiter with a non-empty buffer: DISPATCH;
 *   - any other byte while discarding: swallowed (NONE);
 *   - any other byte at bufLen >= LS_RX_MAX_LEN: OVERFLOW (the caller
 *     answers FAIL,1 with the first LS_RX_MAX_LEN bytes and clears the
 *     buffer — the 33rd byte triggers, exactly the old shape);
 *   - otherwise ACCUMULATE.
 */
static inline LsRxAction lsrxFeedByte(int c, int bufLen, int discarding,
                                      int* outBufLen, int* outDiscarding) {
  if (outBufLen == (int*)0 || outDiscarding == (int*)0) return LS_RX_NONE;
  *outBufLen = bufLen;
  *outDiscarding = discarding;
  if (c == '\n' || c == '\r') {
    if (discarding) {
      *outDiscarding = 0;  // the drowned frame ends with its delimiter
      return LS_RX_NONE;
    }
    if (bufLen == 0) return LS_RX_NONE;  // blank line
    *outBufLen = 0;
    return LS_RX_DISPATCH;
  }
  if (discarding) return LS_RX_NONE;  // swallow the drowned frame's tail
  if (bufLen >= LS_RX_MAX_LEN) {
    *outBufLen = 0;
    *outDiscarding = 1;  // answer once; swallow until the frame ends
    return LS_RX_OVERFLOW;
  }
  *outBufLen = bufLen + 1;
  return LS_RX_ACCUMULATE;
}

/* The legacy idle-gap rule for a DISCARDED frame: the flood stopped and the
 * stream has been idle for gapMs — the frame is over (there is no delimiter
 * coming), so the swallow flag may clear and the next bytes are a NEW
 * message, not the drowned one's tail. A non-empty buffer never happens
 * while discarding (the layer never accumulates then); the bufLen argument
 * keeps the caller's invariant explicit. */
static inline int lsrxIdleEndsDiscard(int discarding, int bufLen,
                                      unsigned long msSinceLastByte, unsigned long gapMs) {
  return discarding && bufLen == 0 && (msSinceLastByte >= gapMs);
}

#endif  // LSSERIALRX_H
