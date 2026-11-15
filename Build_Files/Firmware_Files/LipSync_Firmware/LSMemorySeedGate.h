#ifndef LSMEMORY_SEED_GATE_H
#define LSMEMORY_SEED_GATE_H

// Pure decision layer for LSMemory::initialize()'s boot-time (re)seed gate,
// split out so the host suite can pin the semantics — the same pattern as
// LSHostSwitch.h. No Arduino types: the caller resolves the two facts from
// the filesystem and this header answers one question — may the seed write
// proceed?
//
// The two failure directions the gate must survive:
//   - 4.5.7/4.9.1, an EXISTING file: Adafruit_LittleFS FILE_O_WRITE does NOT
//     truncate — it opens RDWR|CREAT and seeks to END — so writing the seed
//     over a surviving corrupt file APPENDS the default after the garbage
//     ([garbage][default]: still unparseable), and every boot appended
//     another copy. The corrupt file must therefore be removed first, and a
//     FAILED remove (a corrupt directory entry — the damage class the reseed
//     exists to heal — or a nearly-full flash) refuses the seed.
//   - 4.9.7 (review B2), an ABSENT file: Adafruit_LittleFS::remove() returns
//     (err == LFS_ERR_OK), i.e. FALSE for LFS_ERR_NOENT, so the 4.9.1 shape —
//     remove() unconditionally, refuse on false — refused the seed on EVERY
//     boot for a device with no file yet: the <=4.6.6 upgrade path for
//     /settings2.dat (the file simply does not exist until this seed writes
//     it), and any first boot for either file. The compiled default never
//     landed, every HS/HP/HA/HB SET answered FAIL,3 forever (the API writers
//     gate on read truth), the hard sip/puff band was unconfigurable on every
//     upgraded device, and the "power cut mid-reseed heals next boot" promise
//     was false — the healed shape IS an absent file, and a remove of an
//     absent file can never succeed. An absent file must seed, not refuse.

static inline int lsmemSeedMayWrite(int fileExists, int removeSucceeded) {
  return !fileExists || removeSucceeded;
}

#endif  // LSMEMORY_SEED_GATE_H
