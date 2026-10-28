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

#include <ArduinoJson.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

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
    String readAll(String fileString);
    long size(String fileString);
    bool writeAll(String fileString, String jsonString);
    JsonObject readObject(String fileString);
    int readInt(String fileString, String key);
    float readFloat(String fileString, String key);
    String readString(String fileString, String key);
    pointFloatType readPoint(String fileString, String key);
    bool writeObject(String fileString, String key, JsonObject obj);
    bool writeInt(String fileString, String key, int value);
    bool writeFloat(String fileString, String key, float value);
    bool writeString(String fileString, String key, String value);
    bool writePoint(String fileString, String key, pointFloatType value);
    bool isLastReadOk() { return _lastReadOk; }
  private:
    bool _lastReadOk = true;  // The most recent readObject deserialized a real document (4.5.7 review)
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
  if (file.open(fileName, FILE_O_READ))
  {
    uint32_t readlen;
    char buffer[BUFFER_SIZE] = { 0 };
    readlen = file.read(buffer, sizeof(buffer));
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
    DeserializationError err = deserializeJson(doc, String(buffer));
    fileHealthy = (readlen > 0) && !err && !doc.isNull() && doc.is<JsonObject>();
  }
  if (fileHealthy) {
    return;  // Nothing to do — the stored settings parse
  }

  // Seed (or re-seed) the compiled default. It must fit the budget by
  // construction — if it ever does not, a new settings key grew the file
  // past the 4.1.0 downgrade cliff and the host-side size test should
  // have caught it first. Refuse the write rather than seeding every boot
  // with a file the downgrade path would truncate.
  if (jsonString.length() > CONF_SETTINGS_FILE_BUDGET) {
    Serial.print("ERROR: compiled default settings (");
    Serial.print((int)jsonString.length());
    Serial.print(" bytes) exceed the ");
    Serial.print(CONF_SETTINGS_FILE_BUDGET);
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
    file.write(jsonChar, strlen(jsonChar));
    delay(1);
    file.close();
    delay(1);
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
  InternalFS.remove(fileName);
  file.close();
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
// Function   : readAll
//
// Description: Reads all Json data from the settings file and stores it in a buffer
//
// Arguments :  fileString : String : the name of the file
//
// Return     : buffer : String array : Output from file
//*********************************//
String LSMemory::readAll(String fileString){
  const char* fileName = fileString.c_str();

  // Check the open (4.5.7 review): File::read on an unopened handle passes
  // a NULL lfs_file_t into littlefs — a verified HardFault chain (vector-
  // table garbage passes the BADF check, the ctz walk hits an unmapped
  // block, the raw flash read dereferences it). A missing/unopenable file
  // reads as empty; the callers' defaults apply.
  if (!file.open(fileName, FILE_O_READ)) {
    return "";
  }
  uint32_t readlen;
  char buffer[BUFFER_SIZE] = { 0 };
  readlen = file.read(buffer, sizeof(buffer));
  if (readlen >= sizeof(buffer)) readlen = sizeof(buffer) - 1;  // 4.5.7 review: the one-byte overflow at exactly 512 bytes

  delay(1);
  buffer[readlen] = 0;
  file.close();
  delay(1);
  return buffer;
}


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

    if (strlen(jsonChar) > CONF_SETTINGS_FILE_BUDGET) {
      // Name the reason on serial: a refused settings write must never read
      // as a flash failure — it is a deliberate downgrade guard, and the
      // line tells the maker/support tech which knob moved.
      Serial.print("ERROR: settings file budget refused (");
      Serial.print((int)strlen(jsonChar));
      Serial.print(" > ");
      Serial.print(CONF_SETTINGS_FILE_BUDGET);
      Serial.println(" bytes) — firmware 4.1.0's 512-byte reader would truncate it");
      return false;
    }

    // Crash-safe replace (4.5.7 review): write the new content to a temp
    // file, close, then rename over the target — LittleFS's rename is an
    // atomic metadata commit, so a reset at ANY instant leaves either the
    // complete old file or the complete new one. The old remove-then-write
    // lost the entire settings file (calibration, caregiver restrictions)
    // to a power cut inside the window — and doFactoryReset's ~22
    // sequential writes multiplied that window 22-fold.
    InternalFS.remove(CONF_SETTINGS_TMP_FILE);
    delay(1);

    if (!file.open(CONF_SETTINGS_TMP_FILE, FILE_O_WRITE)) {
      return false;
    }
    delay(1);

    file.write(jsonChar, strlen(jsonChar));
    delay(1);
    file.close();

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

  uint32_t readLength;
  const char* fileName = fileString.c_str();
  char jsonBuffer[BUFFER_SIZE] = { 0 };
  // Check the open — see readAll() (4.5.7 review): an unopened read is a
  // NULL-handle HardFault, not an empty result. A missing/unopenable file
  // marks the read failed so writes refuse (writeObject's isNull guard)
  // and the next boot's re-seed heals.
  if (!file.open(fileName, FILE_O_READ)) {
    _lastReadOk = false;
    return doc.as<JsonObject>();
  }
  delay(1);
  readLength = file.read(jsonBuffer, sizeof(jsonBuffer));
  if (readLength >= sizeof(jsonBuffer)) readLength = sizeof(jsonBuffer) - 1;  // 4.5.7 review: the one-byte overflow at exactly 512 bytes
  delay(1);
  jsonBuffer[readLength] = 0;

  // Deserialize he JSON documetn
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
      Serial.print("ERROR: deserializeJson() failed (reported once per boot): ");
      Serial.println(deserializationError.f_str());
    }
  } else if (doc.isNull() || !doc.is<JsonObject>()) {
    static bool reportedInvalidType = false;
    if (!reportedInvalidType) {
      reportedInvalidType = true;
      Serial.println(F("ERROR: settings file is not a JSON object (reported once per boot)"));
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
// Return     : value : point : The returned value corresponding to the key.
pointFloatType LSMemory::readPoint(String fileString, String key){
  JsonObject obj = readObject(fileString);
  return {obj[key][0], obj[key][1]};
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
      Serial.print("ERROR: settings write refused for key ");
      Serial.print(key);
      Serial.println(" — the settings file did not parse (it will be re-seeded at the next boot)");
      return false;
    }

    if (strlen(jsonChar) > CONF_SETTINGS_FILE_BUDGET) {
      Serial.print("ERROR: settings file budget refused for key ");
      Serial.print(key);
      Serial.print(" (");
      Serial.print((int)strlen(jsonChar));
      Serial.print(" > ");
      Serial.print(CONF_SETTINGS_FILE_BUDGET);
      Serial.println(" bytes) — firmware 4.1.0's 512-byte reader would truncate it");
      return false;
    }

    // Crash-safe replace — see writeAll() (4.5.7 review).
    InternalFS.remove(CONF_SETTINGS_TMP_FILE);
    delay(1);

    if (!file.open(CONF_SETTINGS_TMP_FILE, FILE_O_WRITE)) {
      return false;
    }
    delay(1);

    file.write(jsonChar, strlen(jsonChar));
    delay(1);
    file.close();

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
  JsonObject obj = readObject(fileString);
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
  JsonArray point = obj[key].to<JsonArray>();
  point.add(value.x);
  point.add(value.y);
  return writeObject(fileString,key,obj);
}

#endif 
