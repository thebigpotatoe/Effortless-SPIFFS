#pragma once

#ifdef __cplusplus

// Check for arduino env here
// Arduino Libraries
#include <Print.h>

#include "FS.h"

// Architecture Specific Libraries
#if defined(ESP8266)
#include <LittleFS.h>
#define EFFORTLESS_SPIFFS_TYPE LittleFS
#elif defined(ESP32)
#include "SPIFFS.h"
#define EFFORTLESS_SPIFFS_TYPE SPIFFS
#else
#error Effortless SPIFFS does not work on the selected architecture
#endif

// Standard c++ libraries
#include <string>

#ifndef Effortless_SPIFFS_h
#define Effortless_SPIFFS_h

#define EFFORTLESS_SPIFFS_VERSION_MAJOR 2
#define EFFORTLESS_SPIFFS_VERSION_MINOR 1
#define EFFORTLESS_SPIFFS_VERSION_PATCH 2

// Effortless SPIFFS Constants
#ifndef Effortless_SPIFFS_CHAR_SIZE
#define Effortless_SPIFFS_CHAR_SIZE 1024
#endif

#ifndef Effortless_SPIFFS_PRECISION
#define Effortless_SPIFFS_PRECISION 15
#endif

// Effortless SPIFFS Debug Macros
#define ESPIFFS_DEBUG(x) \
  if (printer) printer->print(x)
#define ESPIFFS_DEBUGLN(x) \
  if (printer) printer->println(x)

// Effortless SPIFFS internal namespace
namespace Effortless_SPIFFS_Internal {
  template <bool B, class T = void>
  struct enable_if {};

  template <class T>
  struct enable_if<true, T> { typedef T type; };

  template <typename A, typename B>
  struct is_same {
    static const bool value = false;
  };
  template <typename A>
  struct is_same<A, A> {
    static const bool value = true;
  };
}  // namespace Effortless_SPIFFS_Internal

// Main Effortless SPIFFS Class
class eSPIFFS {
 public:  // constructors
  eSPIFFS(Print* _debug = nullptr) : printer(_debug) {}
  ~eSPIFFS() {}

 public:  // spiffs access methods
  virtual inline bool checkFlashConfig() {
#if defined(ESP8266)
    if (!flashSizeCorrect) {
      // Get actual flash size and size set in IDE
      uint32_t realSize = ESP.getFlashChipRealSize();
      uint32_t ideSize = ESP.getFlashChipSize();

      // Compare the two sizes
      if (realSize >= ideSize) {
        // Get info about the LittleFS
        if (EFFORTLESS_SPIFFS_TYPE.begin()) {
          FSInfo fs_info;
          EFFORTLESS_SPIFFS_TYPE.info(fs_info);
          if (fs_info.totalBytes != 0) {
            // Change the boolean to true if the config is ok
            flashSizeCorrect = true;
          } else {
            ESPIFFS_DEBUGLN("[checkFlashConfig] - LittleFS size was set to 0, please select a LittleFS size from the \"tools->flash size:\" menu");
          }
        } else {
          ESPIFFS_DEBUGLN("[checkFlashConfig] - LittleFS size was set to 0, please select a LittleFS size from the \"tools->flash size:\" menu");
        }
      } else {
        // Tell the user the flash is incorrect if it is not
        ESPIFFS_DEBUGLN("[checkFlashConfig] - Flash chip set to the incorrect size, correct size is; " + String(realSize));
      }
    }

#elif defined(ESP32)
    if (EFFORTLESS_SPIFFS_TYPE.begin()) {
      if (EFFORTLESS_SPIFFS_TYPE.totalBytes() > 0) {
        flashSizeCorrect = true;
      } else {
        ESPIFFS_DEBUGLN("[checkFlashConfig] - SPIFFS size was set to 0, please select a SPIFFS size from the \"tools->flash size:\" menu or partition a SPIFFS");
      }
    } else {
      ESPIFFS_DEBUGLN("[checkFlashConfig] - Failed to start SPIFFS");
    }
#endif

    // Return the boolean
    return flashSizeCorrect;
  }
  virtual inline int getFileSize(const char* _filename) {
    // Check if the flash config is set correctly
    if (checkFlashConfig()) {
      // Check if the spiffs starts correctly
      if (EFFORTLESS_SPIFFS_TYPE.begin()) {
        // Check if the file exists
        if (EFFORTLESS_SPIFFS_TYPE.exists(_filename)) {
          // Open the dir and check if it is a file
          File currentFile = EFFORTLESS_SPIFFS_TYPE.open(_filename, "r");
          if (currentFile) {
            // Return the file size
            return currentFile.size();
          } else {
            ESPIFFS_DEBUG("[getFileSize] - File did not open correctly: ");
            ESPIFFS_DEBUGLN(_filename);
          }
        } else {
          ESPIFFS_DEBUG("[getFileSize] - File does not exist: ");
          ESPIFFS_DEBUGLN(_filename);
        }
      } else {
        ESPIFFS_DEBUGLN("[getFileSize] - Failed to start file system");
      }
    }
    return 0;
  }
  virtual File getFile(const char* _filename, const char* _readWrite) {
    // Check if the flash config is set correctly
    if (checkFlashConfig()) {  // 5us
      // Check if the spiffs starts correctly
      if (EFFORTLESS_SPIFFS_TYPE.begin()) {  // 5us
        // Check if the file exists
        if (strcmp(_readWrite, "r") == 0 ? EFFORTLESS_SPIFFS_TYPE.exists(_filename) : true) {  // 49us
          // Open it in read mode and check if its ok
          File currentFile = EFFORTLESS_SPIFFS_TYPE.open(_filename, _readWrite);  // 115us
          if (currentFile) {
            return currentFile;
          } else {
            ESPIFFS_DEBUG("[openFile] - Failed to open file");
            ESPIFFS_DEBUGLN(_filename);
          }
        } else {
          ESPIFFS_DEBUG("[openFile] - File does not exist: ");
          ESPIFFS_DEBUGLN(_filename);
        }
      } else {
        ESPIFFS_DEBUGLN("[openFile] - Failed to start LittleFS");
      }
    }
    return File();
  }
  virtual bool openFile(const char* _filename, char* _output, size_t _len = 0) {
    // Open it in read mode and check if its ok
    File currentFile = getFile(_filename, "r");  // 115us
    if (currentFile) {
      // Read the desired number of bytes from the array to the output buffer
      size_t numBytesToRead = (_len > 0 && _len <= currentFile.size()) ? _len : currentFile.size();
      if (currentFile.readBytes(_output, numBytesToRead)) {  // readBytes - 300us, readBytesUntil - 465us - goes up with larger strings
        return true;
      } else {
        ESPIFFS_DEBUG("[openFile] - Failed to read any bytes from file: ");
        ESPIFFS_DEBUGLN(_filename);
      }
    }
    return false;
  }
  virtual bool saveFile(const char* _filename, const char* _input) {  // Total time is about 6000us for small strings
    // Open the file in write mode and check if open
    File currentFile = getFile(_filename, "w");
    if (currentFile) {
      // Print the input string to the file
      if (currentFile.print(_input)) {
        currentFile.close();
        return true;
      } else {
        ESPIFFS_DEBUG("[saveFile] - Failed to write any bytes to file: ");
        ESPIFFS_DEBUGLN(_filename);
      }
    }

    return false;
  }
  virtual bool appendFile(const char* _filename, const char* _input) {
    // Open the file in write mode and check if open
    File currentFile = getFile(_filename, "a");
    if (currentFile) {
      // Print the input string to the file
      if (currentFile.print(_input)) {
        currentFile.close();
        return true;
      } else {
        ESPIFFS_DEBUG("[saveFile] - Failed to append any bytes to file: ");
        ESPIFFS_DEBUGLN(_filename);
      }
    }
    return false;
  }

 public:
  void setDebugOutput(Print* _debug) {
    if (_debug) printer = _debug;
  }
  void clearDebugOutput() {
    printer = nullptr;
  }

 public:  // open value templates
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, bool>::value, bool>::type
  openFromFile(const char* _filename, T& _output) {
    size_t fileSize = getFileSize(_filename);
    char   fileContents[fileSize + 1];
    memset(fileContents, 0x00, fileSize + 1);

    if (openFile(_filename, fileContents, fileSize)) {
      char* ptr;
      _output = (strtol(fileContents, &ptr, 10));
      return true;
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, float>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, double>::value,
                                                 bool>::type
  openFromFile(const char* _filename, T& _output) {
    size_t fileSize = getFileSize(_filename);
    char   fileContents[fileSize + 1];
    memset(fileContents, 0x00, fileSize + 1);

    if (openFile(_filename, fileContents, fileSize)) {
      char* ptr;
      _output = strtod(fileContents, &ptr);
      return true;
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, signed char>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, signed int>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, signed short>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, signed long>::value,
                                                 bool>::type
  openFromFile(const char* _filename, T& _output) {
    size_t fileSize = getFileSize(_filename);
    char   fileContents[fileSize + 1];
    memset(fileContents, 0x00, fileSize + 1);

    if (openFile(_filename, fileContents, fileSize)) {
      char* ptr;
      _output = strtol(fileContents, &ptr, 10);  //80us
      return true;
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, unsigned char>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, unsigned int>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, unsigned short>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, unsigned long>::value,
                                                 bool>::type
  openFromFile(const char* _filename, T& _output) {
    size_t fileSize = getFileSize(_filename);
    char   fileContents[fileSize + 1];
    memset(fileContents, 0x00, fileSize + 1);

    if (openFile(_filename, fileContents, fileSize)) {
      char* ptr;
      _output = strtoul(fileContents, &ptr, 10);
      return true;
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, char*>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, const char*>::value,
                                                 bool>::type
  openFromFile(const char* _filename, T& _output) {
    size_t fileSize = getFileSize(_filename);
    if (Effortless_SPIFFS_CHAR_SIZE > fileSize) {
      static char fileContents[Effortless_SPIFFS_CHAR_SIZE];
      memset(fileContents, 0x00, Effortless_SPIFFS_CHAR_SIZE);

      if (openFile(_filename, fileContents, fileSize)) {
        _output = fileContents;
        return true;
      }
    } else {
      ESPIFFS_DEBUGLN("[openFromFile<char*>] - Internal static char buffer to small for file contents, set Effortless_SPIFFS_CHAR_SIZE larger if required (default 1024)");
    }

    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, String>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, std::string>::value,
                                                 bool>::type
  openFromFile(const char* _filename, T& _output) {
    size_t fileSize = getFileSize(_filename);
    char   fileContents[fileSize + 1];
    memset(fileContents, 0x00, fileSize + 1);

    if (openFile(_filename, fileContents, fileSize)) {
      _output = fileContents;
      return true;
    }
    return false;
  }
#if defined ARDUINOJSON_VERSION_MAJOR && ARDUINOJSON_VERSION_MAJOR == 6
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, DynamicJsonDocument>::value, bool>::type
  openFromFile(const char* _filename, T& _output) {
    File file = getFile(_filename, "r");
    if (file) {
      DeserializationError jsonError = deserializeJson(_output, file);
      if (!jsonError) {
        return true;
      } else {
        ESPIFFS_DEBUG("[openFromFile<DynamicJsonDocument>] - Failed to parse JSON: ");
        ESPIFFS_DEBUG(jsonError.c_str());
        ESPIFFS_DEBUG(" for file ");
        ESPIFFS_DEBUGLN(_filename);
      }
    }
    return false;
  }
#endif

 public:  // save value templates
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, bool>::value, bool>::type
  saveToFile(const char* _filename, T& _input) {
    char inputString[2];  // Bool string will be: "0" + \0
    if (sprintf(inputString, "%d", _input)) {
      if (saveFile(_filename, inputString)) {
        return true;
      }
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, float>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, double>::value,
                                                 bool>::type
  saveToFile(const char* _filename, T& _input) {
    char inputString[Effortless_SPIFFS_PRECISION + 7];  // precision + decimal() + notation(4) + sign(1) + null(1)
    if (sprintf(inputString, "%.*g", Effortless_SPIFFS_PRECISION, _input)) {
      if (saveFile(_filename, inputString)) {
        return true;
      }
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, signed char>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, signed int>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, signed short>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, signed long>::value,
                                                 bool>::type
  saveToFile(const char* _filename, T& _input) {
    char inputString[15];  // signed long string will be 11 digits + \0
    if (sprintf(inputString, "%li", (signed long)_input)) {
      if (saveFile(_filename, inputString)) {
        return true;
      }
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, unsigned char>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, unsigned int>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, unsigned short>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, unsigned long>::value,
                                                 bool>::type
  saveToFile(const char* _filename, T& _input) {
    char inputString[15];  // unsigned long string will be 10 digits + \0
    if (sprintf(inputString, "%lu", (unsigned long)_input)) {
      if (saveFile(_filename, inputString)) {
        return true;
      }
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, char*>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, const char*>::value,
                                                 bool>::type
  saveToFile(const char* _filename, T _input) {
    if (saveFile(_filename, _input)) {
      return true;
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, String>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, std::string>::value,
                                                 bool>::type
  saveToFile(const char* _filename, T& _input) {
    if (saveFile(_filename, _input.c_str())) {
      return true;
    }
    return false;
  }
#if defined ARDUINOJSON_VERSION_MAJOR && ARDUINOJSON_VERSION_MAJOR == 6
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, DynamicJsonDocument>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, JsonObject>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, JsonArray>::value,
                                                 bool>::type
  saveToFile(const char* _filename, T& _input) {
    File file = getFile(_filename, "w");
    if (file) {
      if (serializeJson(_input, file)) {
        file.close();
        return true;
      } else {
        ESPIFFS_DEBUG("[saveToFile<DynamicJsonDocument>] - Failed to serialize JSON for file ");
        ESPIFFS_DEBUGLN(_filename);
      }
    }
    return false;
  }
#endif

 public:  // save value templates
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, bool>::value, bool>::type
  appendToFile(const char* _filename, T& _input) {
    char inputString[2];  // Bool string will be: "0" + \0
    if (sprintf(inputString, "%d", _input)) {
      if (appendFile(_filename, inputString)) {
        return true;
      }
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, float>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, double>::value,
                                                 bool>::type
  appendToFile(const char* _filename, T& _input) {
    char inputString[Effortless_SPIFFS_PRECISION + 7];  // precision + decimal() + notation(4) + sign(1) + null(1)
    if (sprintf(inputString, "%.*g", Effortless_SPIFFS_PRECISION, _input)) {
      if (appendFile(_filename, inputString)) {
        return true;
      }
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, signed char>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, signed int>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, signed short>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, signed long>::value,
                                                 bool>::type
  appendToFile(const char* _filename, T& _input) {
    char inputString[15];  // signed long string will be 11 digits + \0
    if (sprintf(inputString, "%li", (signed long)_input)) {
      if (appendFile(_filename, inputString)) {
        return true;
      }
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, unsigned char>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, unsigned int>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, unsigned short>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, unsigned long>::value,
                                                 bool>::type
  appendToFile(const char* _filename, T& _input) {
    char inputString[15];  // unsigned long string will be 10 digits + \0
    if (sprintf(inputString, "%lu", (unsigned long)_input)) {
      if (appendFile(_filename, inputString)) {
        return true;
      }
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, char*>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, const char*>::value,
                                                 bool>::type
  appendToFile(const char* _filename, T _input) {
    if (appendFile(_filename, _input)) {
      return true;
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, String>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, std::string>::value,
                                                 bool>::type
  appendToFile(const char* _filename, T& _input) {
    if (saveFile(_filename, _input.c_str())) {
      return true;
    }
    return false;
  }
#if defined ARDUINOJSON_VERSION_MAJOR && ARDUINOJSON_VERSION_MAJOR == 6
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, DynamicJsonDocument>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, JsonObject>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, JsonArray>::value,
                                                 bool>::type
  appendToFile(const char* _filename, T& _input) {
    File file = getFile(_filename, "a");
    if (file) {
      if (serializeJson(_input, file)) {
        file.close();
        return true;
      } else {
        ESPIFFS_DEBUG("[saveToFile<DynamicJsonDocument>] - Failed to serialize JSON for file ");
        ESPIFFS_DEBUGLN(_filename);
      }
    }
    return false;
  }
#endif

 private:  // storage
  bool   flashSizeCorrect = false;
  Print* printer = nullptr;
};

#endif

#else
#error Effortless_SPIFFS requires a C++ compiler
#endif