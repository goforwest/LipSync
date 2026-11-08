/* 
* File: LSMemory.h
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
#ifndef _LSMEMORY_H
#define _LSMEMORY_H

// (4.9.0 round-2 review) the shared instance, declared here so headers
// included AFTER LSMemory.h but BEFORE the .ino's definition (LSScreen.h's
// inline bodies read isLastReadOk for the hard-threshold heal gate) can see
// it. The definition lives in LipSync_Firmware.ino.
class LSMemory;
extern LSMemory mem;

#include <ArduinoJson.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include "LSApiDeclarations.h"

using namespace Adafruit_LittleFS_Namespace;

#define BUFFER_SIZE  512

JsonDocument doc;

// Initialize FileSystem
File file(InternalFS);

// Crash-safe settings persistence (4.5.7 review): writes go to a temp file
// and are then RENAMED over the target — LittleFS's lfs_rename is an atomic
// metadata commit, so a power cut at ANY instant leaves either the complete
// old file or the complete new one. The old remove-then-write pattern lost
// the whole file (every calibration, every caregiver restriction) to a
// reset landing in the window, and doFactoryReset's ~22 sequential writes
// multiplied that window 22-fold.
#define CONF_SETTINGS_TMP_FILE "/settings.tmp"

class LSMemory {
  public:
    LSMemory();
    void begin();
    void initialize(String fileString, String jsonString);
    void clear(String fileString);
    void format();
    long size(String fileString);
    bool writeAll(String fileString, String jsonString);
    JsonObject readObject(String fileString);
    int readInt(String fileString, String key);
    float readFloat(String fileString, String key);
    String readString(String fileString, String key);
    bool readPoint(String fileString, String key, pointFloatType &out);  // Reads a [x,y] point; returns false (out untouched) on any read/parse failure — never an implicit {0,0}
    bool writeObject(String fileString, String key, JsonObject obj);
    bool writeInt(String fileString, String key, int value);
    bool writeFloat(String fileString, String key, float value);
    bool writeString(String fileString, String key, String value);
    bool writePoint(String fileString, String key, pointFloatType value);
    bool isLastReadOk() { return _lastReadOk; }
  private:
    bool _lastReadOk = true;  // The most recent readObject deserialized a real document (4.5.7 review)

    // Read the just-written temp file back and byte-compare it against the
    // source before the atomic rename. Adafruit_LittleFS's File::close()
    // returns void and swallows lfs_file_close errors, so the written-byte
    // count alone cannot rule out a close-time truncation — without this
    // check a flash error could rename a truncated temp over the settings
    // file while writeAll/writeObject reported success. Length + byte
    // compare (no second JSON parse — it would spend a second document pool
    // on the loop-task stack for a failure class a byte compare already
    // catches).
    static bool verifyTempWrite(const char* jsonChar, size_t jsonLength) {
      File verifyFile(CONF_SETTINGS_TMP_FILE, FILE_O_READ, InternalFS);
      if (!verifyFile) return false;
      bool ok = true;
      for (size_t i = 0; ok && i < jsonLength; i++) {
        const int c = verifyFile.read();
        ok = (c >= 0) && ((char)c == jsonChar[i]);
      }
      if (ok) ok = (verifyFile.read() < 0);  // No trailing extra bytes either
      verifyFile.close();
      return ok;
    }

    // 4.7.0 / upstream #58: per-file size budget. The main settings file's
    // budget is the 4.1.0 downgrade cliff; /settings2.dat carries the
    // new-format keys under its own (generous) budget. Every size check in
    // this class routes through this so a second file can never be held to
    // the main file's cliff (or vice versa).
    static uint32_t budgetFor(String fileString) {
      return (fileString == CONF_SETTINGS2_FILE) ? (uint32_t)CONF_SETTINGS2_FILE_BUDGET
                                                 : (uint32_t)CONF_SETTINGS_FILE_BUDGET;
    }
};


//*********************************//
// Function   : LSMemory 
// 
// Description: Construct LSMemory
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
LSMemory::LSMemory() {

}

//*********************************//
// Function   : begin 
// 
// Description: Begins internal file structure.
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSMemory::begin() {
  InternalFS.begin();
}


//*********************************//
// Function   : initialize 
// 
// Description: Initializes memory file.
// 
// Arguments :  fileString : String : the name of the file
//              jsonString : String : the default json string with list of key and value pairs 
// 
// Return     : void
//*********************************//
void LSMemory::initialize(String fileString, String jsonString) {
  const char* fileName = fileString.c_str();

  // A settings file that exists must also PARSE (4.5.7 review): a power cut
  // or partial write in the old remove-then-write window, or flash
  // corruption, could leave a truncated or empty file — and the old code
  // kept such a file forever (every read failed, every write serialized
  // the failed document and re-emptied the file, and the device silently
  // ran on garbage while initialize() reported success because open()
  // succeeded). A file that does not deserialize is re-seeded with the
  // compiled default, so one bad write heals at the next boot instead of
  // bricking every future one.
  bool fileHealthy = false;
  bool fileExistedOverBudget = false;  // 4.6.6: a parseable file over the budget is destroyed — name it (see below)
  uint32_t fileOverBudgetSize = 0;
  if (file.open(fileName, FILE_O_READ))
  {
    const uint32_t fileSize = file.size();
    uint32_t readlen = 0;
    char buffer[BUFFER_SIZE] = { 0 };
    int readResult = file.read(buffer, sizeof(buffer));
    if (readResult > 0) readlen = (uint32_t)readResult;
    if (readlen >= sizeof(buffer)) readlen = sizeof(buffer) - 1;  // 4.5.7 review: the one-byte overflow at exactly 512 bytes
    buffer[readlen] = 0;
    file.close();
    delay(5);
    // A file that exists must deserialize to a real OBJECT (4.5.7 review,
    // hardened): deserializeJson succeeding is not enough — the literal
    // "null" (what a corrupt-read write could have stored, see writeObject)
    // parses cleanly yet carries no settings, so doc.isNull() must also be
    // false. Otherwise a stray "null" would be treated as healthy and never
    // re-seeded.
    doc.clear();
    DeserializationError err = deserializeJson(doc, String(buffer));
    fileHealthy = (fileSize > 0) &&
                  (fileSize <= budgetFor(fileString)) &&
                  (fileSize < sizeof(buffer)) &&
                  (readlen == fileSize) &&
                  !err && !doc.isNull() && doc.is<JsonObject>();
    if (!fileHealthy && fileSize > budgetFor(fileString)) {
      fileExistedOverBudget = true;
      fileOverBudgetSize = fileSize;
    }
  }
  if (fileHealthy) {
    return;  // Nothing to do — the stored settings parse
  }
  // 4.6.6: a file whose ONLY failure is the size budget (parseable, 481-511
  // bytes — e.g. written by a build without the write-side guard) used to be
  // destroyed and re-seeded in total silence: calibration corners and the
  // caregiver restrictions (SD/MA/LK/CF) vanished with no wire trace at all.
  // The destroy itself stays (4.1.0 would truncate the tail off exactly such
  // a file on its next write — the cliff this budget exists for), but the
  // loss is now named on the wire.
  if (fileExistedOverBudget) {
    Serial.print("ERROR: settings file (");
    Serial.print((int)fileOverBudgetSize);
    Serial.print(" bytes) is parseable but exceeds the ");
    // Per-file budget, not the main file's constant: /settings2.dat's 256-byte
    // budget is NOT the 4.1.0 downgrade cliff, and a support tech reading this
    // line for a destroyed settings2 must be told the right number.
    Serial.print(budgetFor(fileString));
    Serial.println("-byte budget — replacing it with the compiled defaults");
  }

  // Seed (or re-seed) the compiled default. It must fit the budget by
  // construction — if it ever does not, a new settings key grew the file
  // past the 4.1.0 downgrade cliff and the host-side size test should
  // have caught it first. Refuse the write rather than seeding every boot
  // with a file the downgrade path would truncate. Per-file budget: the
  // main file's cliff, /settings2.dat's own budget.
  if (jsonString.length() > budgetFor(fileString)) {
    Serial.print("ERROR: compiled default settings (");
    Serial.print((int)jsonString.length());
    Serial.print(" bytes) exceed the ");
    Serial.print(budgetFor(fileString));
    Serial.println("-byte budget — refusing to initialize settings file");
    return;
  }
  // Remove the old file before writing the seed (4.5.7 review — the
  // append-mode trap): Adafruit_LittleFS FILE_O_WRITE does NOT truncate —
  // it opens RDWR|CREAT and seeks to END — so writing the seed over an
  // existing corrupt file APPENDED the default after the garbage
  // ([garbage][default]: still unparseable), and every boot appended
  // another copy, growing the file forever and never healing. A fresh seed
  // write here is boot-time-only, so remove-then-write is sufficient
  // crash-safety: a power cut mid-reseed leaves no/partial file, which the
  // NEXT boot's re-seed heals.
  InternalFS.remove(fileName);
  delay(1);
  if( file.open(fileName, FILE_O_WRITE) )
  {
    const char* jsonChar = jsonString.c_str();
    const size_t jsonLength = strlen(jsonChar);
    const size_t written = file.write(jsonChar, jsonLength);
    delay(1);
    file.close();
    delay(1);
    if (written != jsonLength) {
      InternalFS.remove(fileName);
    }
  }
}


//*********************************//
// Function   : clear 
// 
// Description: Deletes the memory file
// 
// Arguments :  fileString : String : the name of the file
// 
// Return     : void
//*********************************//
void LSMemory::clear(String fileString) {
  const char* fileName = fileString.c_str();
  file.close();
  InternalFS.remove(fileName);
  delay(1);  
}

//***FORMAT ALL FILES FUNCTION***//
//*********************************//
// Function   : format 
// 
// Description: Formats all files in the internal file system
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSMemory::format(){
  file.close();
  InternalFS.format();
  delay(1);
}


//*********************************//
// Function   : size
//
// Description: Reports the file's exact on-flash size in bytes (-1 when the
//              file does not exist or the filesystem cannot answer). This is
//              the truth the settings-size budget is measured against —
//              readAll()'s 512-byte buffer view would clamp at the very
//              cliff (4.1.0's reader) the budget exists to stay away from.
//
// Arguments :  fileString : String : the name of the file
//
// Return     : long : the file size in bytes, -1 when absent
//*********************************//
long LSMemory::size(String fileString) {
  const char* fileName = fileString.c_str();
  if (!file.open(fileName, FILE_O_READ)) {
    file.close();
    return -1;
  }
  long fileSize = (long)file.size();
  file.close();
  delay(1);
  return fileSize;
}

//*********************************//
// Function   : readAll — REMOVED (4.8.1 review)
//
// Dead code: no caller anywhere in the firmware (only this definition and
// its declaration existed). It duplicated readObject's budget/short-read/
// clamp logic that had to be kept in sync for nothing — the next caller
// would have silently inherited divergent read semantics.
//
//*********************************//


//*********************************//
// Function   : writeAll
// 
// Description: Replaces the entire settings file with new values. Refuses
//              (returns false, file untouched) a write whose serialized form
//              would push the file past CONF_SETTINGS_FILE_BUDGET — the
//              downgrade guard (see LSConfig.h): firmware 4.1.0 reads this
//              file with a 512-byte buffer and truncates anything longer,
//              so a too-big file survives only until the next 4.1.0 boot.
//
// Arguments :  fileString : String : the name of the file
//              jsonString : String : a json string with list of key and value pairs 
// 
// Return     : bool : true when the file was written, false when the budget
//                     guard refused (or the file could not be opened)
//*********************************//
bool LSMemory::writeAll(String fileString, String jsonString){
    const char* fileName = fileString.c_str();
    const char* jsonChar = jsonString.c_str();

    const size_t jsonLength = strlen(jsonChar);
    if (jsonLength == 0) return false;
    if (jsonLength > budgetFor(fileString)) {
      // Name the reason on serial: a refused settings write must never read
      // as a flash failure — it is a deliberate downgrade guard, and the
      // line tells the maker/support tech which knob moved. The rationale is
      // per-file: only the main file's budget is the 4.1.0 downgrade cliff.
      String errorLine = "ERROR: settings file budget refused (";
      errorLine += String((int)strlen(jsonChar));
      errorLine += " > ";
      errorLine += String(budgetFor(fileString));
      errorLine += " bytes) — ";
      errorLine += (fileString == CONF_SETTINGS_FILE)
          ? "firmware 4.1.0's 512-byte reader would truncate it"
          : "the secondary settings file budget is a hard cap (not the 4.1.0 downgrade cliff)";
      serialTxEnqueueLine(errorLine);
      return false;
    }

    JsonDocument candidate;
    DeserializationError candidateError = deserializeJson(candidate, jsonString);
    if (candidateError || candidate.isNull() || !candidate.is<JsonObject>()) {
      return false;
    }

    // Crash-safe replace (4.5.7 review): write the new content to a temp
    // file, close, then rename over the target — LittleFS's rename is an
    // atomic metadata commit, so a reset at ANY instant leaves either the
    // complete old file or the complete new one. The old remove-then-write
    // lost the entire settings file (calibration, caregiver restrictions)
    // to a power cut inside the window — and doFactoryReset's ~22
    // sequential writes multiplied that window 22-fold.
    file.close();
    InternalFS.remove(CONF_SETTINGS_TMP_FILE);
    delay(1);

    if (!file.open(CONF_SETTINGS_TMP_FILE, FILE_O_WRITE)) {
      return false;
    }
    delay(1);

    const size_t written = file.write(jsonChar, jsonLength);
    delay(1);
    file.close();

    if (written != jsonLength) {
      InternalFS.remove(CONF_SETTINGS_TMP_FILE);
      return false;
    }

    // Read-back verification before the rename: a swallowed close-time error
    // must not rename a truncated temp over the settings while reporting
    // success (verifyTempWrite comment).
    if (!verifyTempWrite(jsonChar, jsonLength)) {
      InternalFS.remove(CONF_SETTINGS_TMP_FILE);
      return false;
    }

    if (!InternalFS.rename(CONF_SETTINGS_TMP_FILE, fileName)) {
      InternalFS.remove(CONF_SETTINGS_TMP_FILE);  // Leave no stale temp behind
      return false;
    }
    return true;
}


//*********************************//
// Function   : readObject 
// 
// Description: Reads the settings file and returns it as a Json Object
// 
// Arguments :  fileString : String : the name of the file
// 
// Return     : obj : JsonObject : A Json Object of the file contents
//*********************************//
JsonObject LSMemory::readObject(String fileString){

  uint32_t readLength = 0;
  const char* fileName = fileString.c_str();
  char jsonBuffer[BUFFER_SIZE] = { 0 };
  // Check the open — see readAll() (4.5.7 review): an unopened read is a
  // NULL-handle HardFault, not an empty result. A missing/unopenable file
  // marks the read failed so writes refuse (writeObject's isNull guard)
  // and the next boot's re-seed heals.
  if (!file.open(fileName, FILE_O_READ)) {
    // Never expose the previous successful read through the shared document.
    // A stale object here let a failed read mutate and rewrite old settings.
    doc.clear();
    _lastReadOk = false;
    return doc.as<JsonObject>();
  }
  delay(1);
  const uint32_t fileSize = file.size();
  if (fileSize == 0 || fileSize > budgetFor(fileString) || fileSize >= sizeof(jsonBuffer)) {
    file.close();
    doc.clear();
    _lastReadOk = false;
    return doc.as<JsonObject>();
  }
  int readResult = file.read(jsonBuffer, sizeof(jsonBuffer));
  if (readResult > 0) readLength = (uint32_t)readResult;
  if (readLength >= sizeof(jsonBuffer)) readLength = sizeof(jsonBuffer) - 1;  // 4.5.7 review: the one-byte overflow at exactly 512 bytes
  delay(1);
  jsonBuffer[readLength] = 0;
  file.close();

  if (readLength != fileSize) {
    doc.clear();
    _lastReadOk = false;
    return doc.as<JsonObject>();
  }

  // Deserialize the JSON document
  doc.clear();
  DeserializationError deserializationError = deserializeJson(doc, String(jsonBuffer));

  // Test is parsing suceeds. The error prints ONCE PER BOOT (4.5.7
  // review): readObject runs inside every serial setter/getter, so the
  // old unconditional print landed mid-response on every exchange while
  // the file was corrupt — the exact wire-corruption class the 4.5.1
  // timer-print cleanup eliminated. The failed state is still observable
  // through isLastReadOk(), and initialize()'s re-seed heals the file at
  // the next boot.
  _lastReadOk = !deserializationError && !doc.isNull() && doc.is<JsonObject>();
  if (deserializationError) {
    static bool reportedDeserializeFailure = false;
    if (!reportedDeserializeFailure) {
      reportedDeserializeFailure = true;
      String errorLine = "ERROR: deserializeJson() failed (reported once per boot): ";
      errorLine += deserializationError.f_str();
      serialTxEnqueueLine(errorLine);
    }
  } else if (doc.isNull() || !doc.is<JsonObject>()) {
    static bool reportedInvalidType = false;
    if (!reportedInvalidType) {
      reportedInvalidType = true;
      static const char typeError[] = "ERROR: settings file is not a JSON object (reported once per boot)";
      serialTxEnqueueLine(typeError, sizeof(typeError) - 1);
    }
    _lastReadOk = false;
  }

  JsonObject obj = doc.as<JsonObject>();
  return obj;
}


//*********************************//
// Function   : readInt 
// 
// Description: Returns the value of the corresponding key within the settings file.
// 
// Arguments :  fileString : String : the name of the file
//              key : String : the key of the desired Json value
// 
// Return     : value : int : The returned value corresponding to the key.
//*********************************//
int LSMemory::readInt(String fileString, String key){
  int value = readObject(fileString)[key];
  return value;
}


//*********************************//
// Function   : readFloat 
// 
// Description: Returns the value of the corresponding key within the settings file.
// 
// Arguments :  fileString : String : the name of the file
//              key : String : the key of the desired Json value
// 
// Return     : value : float : The returned value corresponding to the key.
float LSMemory::readFloat(String fileString, String key){
  float value = readObject(fileString)[key];
  return value;
}


//*********************************//
// Function   : readString 
// 
// Description: Returns the value of the corresponding key within the settings file.
// 
// Arguments :  fileString : String : the name of the file
//              key : String : the key of the desired Json value
// 
// Return     : value : string : The returned value corresponding to the key.
String LSMemory::readString(String fileString, String key){
  String value = readObject(fileString)[key];
  return value;
}

//*********************************//
// Function   : readPoint 
// 
// Description: Returns the value of the corresponding key within the settings file.
// 
// Arguments :  fileString : String : the name of the file
//              key : String : the key of the desired Json value
// 
// Return     : bool : true when a real, finite [x,y] pair was read; false on
//              any failure (out untouched — the caller falls back to live
//              state). See the 4.6.6 note in the implementation.
//*********************************//
bool LSMemory::readPoint(String fileString, String key, pointFloatType &out){
  JsonObject obj = readObject(fileString);
  // 4.6.6: a failed read (open failure, size guard, parse failure) used to
  // silently convert to {0,0} — indexing a null JsonObject yields 0 — and
  // the one live consumer applied that verbatim through setInputMax +
  // setMinimumRadius + reprocessLastReading: a mid-session settings read
  // failure reset every calibration corner to the origin, on the wire AND in
  // the live cursor math, including inside abortJoystickCalibration's
  // reload-to-persisted path. Report validity instead; `out` is untouched on
  // failure so the caller can fall back to live state.
  if (!isLastReadOk()) return false;
  JsonVariant storedValue = obj[key];
  if (storedValue.isNull() || !storedValue.is<JsonArray>()) return false;
  JsonArray corner = storedValue.as<JsonArray>();
  if (corner.size() < 2) return false;
  const float x = corner[0].as<float>();
  const float y = corner[1].as<float>();
  // isfinite, not just isnan: a corrupt-but-parseable literal that overflows
  // float (e.g. 1e400) yields +/-inf, which passes an isnan check and flows
  // into setInputMax -> an infinite/degenerate radius (the full-throw cursor
  // class). A failed read must leave the caller's fallback in charge.
  if (!isfinite(x) || !isfinite(y)) return false;
  out = {x, y};
  return true;
}


//*********************************//
// Function   : writeObject 
// 
// Description: General function for writing an individual attribute-value pair to the settings file.
//              Refuses (returns false, file untouched) a write whose serialized
//              form would push the file past CONF_SETTINGS_FILE_BUDGET —
//              the 4.1.0 downgrade guard (see writeAll): the API layer
//              turns the refusal into the existing FAIL,3 response shape.
// 
// Arguments :  fileString : String : the name of the file
//              key : String : the key of the desired Json attribute
//              obj : JsonObject : the Json Object value
// 
// Return     : bool : true when written, false when the budget guard refused
//*********************************//
bool LSMemory::writeObject(String fileString, String key, JsonObject obj){
    const char* fileName = fileString.c_str();

    String jsonString;
    serializeJson(obj, jsonString);
    const char* jsonChar = jsonString.c_str();

    // Refuse a write whose source document did not deserialize (4.5.7
    // review, hardened): when the settings file is corrupt, readObject()
    // returns a NULL JsonObject, the setter's obj[key]=value is a silent
    // no-op, and serializeJson() of a null document produces the 4-byte
    // string "null" — NOT an empty string. The earlier strlen==0 guard
    // therefore let "null" through (and, being valid JSON, it parsed clean
    // next boot so initialize() never re-seeded — a permanent settings
    // loss). obj.isNull() catches the null-document case directly; the
    // "null"/empty string checks are belt-and-braces.
    if (obj.isNull() || jsonString == "null" || strlen(jsonChar) == 0) {
      String errorLine = "ERROR: settings write refused for key ";
      errorLine += key;
      errorLine += " — the settings file did not parse (it will be re-seeded at the next boot)";
      serialTxEnqueueLine(errorLine);
      return false;
    }

    if (strlen(jsonChar) > budgetFor(fileString)) {
      String errorLine = "ERROR: settings file budget refused for key ";
      errorLine += key;
      errorLine += " (";
      errorLine += String((int)strlen(jsonChar));
      errorLine += " > ";
      errorLine += String(budgetFor(fileString));
      errorLine += " bytes) — ";
      errorLine += (fileString == CONF_SETTINGS_FILE)
          ? "firmware 4.1.0's 512-byte reader would truncate it"
          : "the secondary settings file budget is a hard cap (not the 4.1.0 downgrade cliff)";
      serialTxEnqueueLine(errorLine);
      return false;
    }

    // Crash-safe replace — see writeAll() (4.5.7 review).
    file.close();
    InternalFS.remove(CONF_SETTINGS_TMP_FILE);
    delay(1);

    if (!file.open(CONF_SETTINGS_TMP_FILE, FILE_O_WRITE)) {
      return false;
    }
    delay(1);

    const size_t jsonLength = strlen(jsonChar);
    const size_t written = file.write(jsonChar, jsonLength);
    delay(1);
    file.close();

    if (written != jsonLength) {
      InternalFS.remove(CONF_SETTINGS_TMP_FILE);
      return false;
    }

    // Read-back verification before the rename: a swallowed close-time error
    // must not rename a truncated temp over the settings while reporting
    // success (verifyTempWrite comment).
    if (!verifyTempWrite(jsonChar, jsonLength)) {
      InternalFS.remove(CONF_SETTINGS_TMP_FILE);
      return false;
    }

    if (!InternalFS.rename(CONF_SETTINGS_TMP_FILE, fileName)) {
      InternalFS.remove(CONF_SETTINGS_TMP_FILE);  // Leave no stale temp behind
      return false;
    }
    return true;
}


//*********************************//
// Function   : writeInt 
// 
// Description: Writing an individual int value to the corresponding attribute-value pair given by the key to the settings file
// 
// Arguments :  fileString : String : the name of the file
//              key : String : the key of the desired Json attribute
//              value : int : The integer to write
// 
// Return     : bool : true when written, false when refused (see writeObject)
//*********************************//
bool LSMemory::writeInt(String fileString, String key, int value){
  // (4.9.0 review) read-truth: never commit a write built on a FAILED read.
  // ArduinoJson 7 clears the document then parses, so a TRUNCATED-but-
  // parseable file leaves the successfully-parsed members in place — the
  // document is non-null while _lastReadOk is false. The old shape merged
  // the new key into that partial document and wrote it back: the file then
  // parses clean, initialize()'s re-seed never fires, and the keys the
  // truncation destroyed silently became per-key-healed defaults instead of
  // a named refusal (the storeModePair house rule: refuse on !isLastReadOk).
  // The gate must sit AFTER readObject: _lastReadOk is refreshed by that
  // read, and a stale true from a read of the OTHER settings file must not
  // wave THIS file's failed read through (4.9.0 round-2 review).
  JsonObject obj = readObject(fileString);
  if (!isLastReadOk()) return false;
  obj[String(key)] = value;
  return writeObject(fileString,key,obj);
}

//*********************************//
// Function   : writeFloat 
// 
// Description: Writing an individual float value to the corresponding attribute-value pair given by the key to the settings file
// 
// Arguments :  fileString : String : the name of the file
//              key : String : the key of the desired Json attribute
//              value : float : The float value to write
// 
// Return     : bool : true when written, false when refused (see writeObject)
//*********************************//
bool LSMemory::writeFloat(String fileString, String key, float value){
  JsonObject obj = readObject(fileString);
  if (!isLastReadOk()) return false;  // read-truth — after the read, see writeInt
  obj[String(key)] = value;
  return writeObject(fileString,key,obj);
}

//*********************************//
// Function   : writeString 
// 
// Description: Writing an individual string value to the corresponding attribute-value pair given by the key to the settings file
// 
// Arguments :  fileString : String : the name of the file
//              key : String : the key of the desired Json attribute
//              value : string : The string to write to the settings file
// 
// Return     : bool : true when written, false when refused (see writeObject)
//*********************************//
bool LSMemory::writeString(String fileString, String key, String value){
  JsonObject obj = readObject(fileString);
  if (!isLastReadOk()) return false;  // read-truth — after the read, see writeInt
  obj[String(key)] = value;
  return writeObject(fileString,key,obj);
}

//*********************************//
// Function   : writePoint 
// 
// Description: Writing an individual point value to the corresponding attribute-value pair given by the key to the settings file
// 
// Arguments :  fileString : String : the name of the file
//              key : String : the key of the desired Json attribute
//              value : pointFloatType : The point to write to the settings file
// 
// Return     : bool : true when written, false when refused (see writeObject)
//*********************************//
bool LSMemory::writePoint(String fileString, String key, pointFloatType value){
  JsonObject obj = readObject(fileString);
  if (!isLastReadOk()) return false;  // read-truth — after the read, see writeInt
  // Replace, do not append (4.6.5 review): the seeded defaults already store
  // CA1–CA4 as two-element arrays, and JsonDocument::to<JsonArray>() returns
  // the EXISTING array untouched, so add() appended a third/fourth value on
  // every recalibration. readPoint() reads [0]/[1], so the newest calibration
  // was never seen, and the file grew by two numbers per calibration until
  // the budget guard refused writes (silently pinning the old values).
  JsonArray point = obj[key].to<JsonArray>();
  point.clear();
  point.add(value.x);
  point.add(value.y);
  return writeObject(fileString,key,obj);
}

#endif 
