#pragma once

#ifdef __cplusplus

#if defined(ESP8266)

#include <ArduinoJson.h>
#include <string>
#include "FS.h"

#ifndef Effortless_SPIFFS_CHAR_SIZE
#define Effortless_SPIFFS_CHAR_SIZE 1024
#endif

#ifndef Effortless_SPIFFS_PRECISION
#define Effortless_SPIFFS_PRECISION 15
#endif

#ifndef Effortless_SPIFFS_h
#define Effortless_SPIFFS_h

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

class eSPIFFS {
 public:  // constructors
  eSPIFFS() : serial(nullptr) { checkFlashConfig(); }
  eSPIFFS(HardwareSerial* _serial) : serial(_serial) { checkFlashConfig(); }
  ~eSPIFFS() {}

 public:  // spiffs access methods
  virtual inline bool checkFlashConfig() {
    if (!flashSizeCorrect) {
      // Get actual flash size and size set in IDE
      uint32_t realSize = ESP.getFlashChipRealSize();
      uint32_t ideSize = ESP.getFlashChipSize();

      // Compare the two sizes
      if (realSize >= ideSize) {
        // Get info about the SPIFFS
        if (SPIFFS.begin()) {
          FSInfo fs_info;
          SPIFFS.info(fs_info);
          if (fs_info.totalBytes != 0) {
            // Change the boolean to true if the config is ok
            flashSizeCorrect = true;
          } else {
            println("[checkFlashConfig] - SPIFFS size was set to 0, please select a SPIFFS size from the \"tools->flash size:\" menu");
          }
        } else {
          println("[checkFlashConfig] - SPIFFS size was set to 0, please select a SPIFFS size from the \"tools->flash size:\" menu");
        }
      } else {
        // Tell the user the flash is incorrect if it is not
        println("[checkFlashConfig] - Flash chip set to the incorrect size, correct size is; " + String(realSize));
      }
    }

    // Return the boolean
    return flashSizeCorrect;
  }
  virtual inline int getFileSize(const char* _filename) {
    // Check if the flash config is set correctly
    if (checkFlashConfig()) {
      // Check if the spiffs starts correctly
      if (SPIFFS.begin()) {
        // Check if the file exists
        if (SPIFFS.exists(_filename)) {
          // Open the dir and check if it is a file
          File currentFile = SPIFFS.open(_filename, "r");
          if (currentFile) {
            // Return the file size
            return currentFile.size();
          } else {
            print("[getFileSize] - File did not open correctly: ");
            println(_filename);
          }
        } else {
          print("[getFileSize] - File does not exist: ");
          println(_filename);
        }
      } else {
        println("[getFileSize] - Failed to start file system");
      }
    }
    return 0;
  }
  virtual bool openFile(const char* _filename, char* _output, size_t _len = 0) {
    // Check if the flash config is set correctly
    if (checkFlashConfig()) {  // 5us
      // Check if the spiffs starts correctly
      if (SPIFFS.begin()) {  // 5us
        // Check if the file exists
        if (SPIFFS.exists(_filename)) {  // 49us
          // Open it in read mode and check if its ok
          File currentFile = SPIFFS.open(_filename, "r");  // 115us
          if (currentFile) {
            // Read the desired number of bytes from the array to the output buffer
            size_t numBytesToRead = (_len > 0 && _len <= currentFile.size()) ? _len : currentFile.size();
            if (currentFile.readBytes(_output, numBytesToRead)) {  // readBytes - 300us, readBytesUntil - 465us - goes up with larger strings
              return true;
            } else {
              print("[openFile] - Failed to read any bytes from file: ");
              println(_filename);
            }
          } else {
            print("[openFile] - Failed to open file");
            println(_filename);
          }
        } else {
          print("[openFile] - File does not exist: ");
          println(_filename);
        }
      } else {
        println("[openFile] - Failed to start SPIFFS");
      }
    }
    return false;
  }
  virtual bool saveFile(const char* _filename, const char* _input) {  // Total time is about 6000us for small strings
    // Check if the flash config is set correctly
    if (checkFlashConfig()) {  // 5us
      // Check if the spiffs starts correctly
      if (SPIFFS.begin()) {  // 10us
        // Open the file in write mode and check if open
        File currentFile = SPIFFS.open(_filename, "w");
        if (currentFile) {
          // Print the input string to the file
          if (currentFile.print(_input)) {
            currentFile.close();
            return true;
          } else {
            print("[saveFile] - Failed to write any bytes to file: ");
            println(_filename);
          }
        } else {
          print("[saveFile] - Failed to open file for writing");
          println(_filename);
        }
      } else {
        println("[saveFile] - Failed to start SPIFFS");
      }
    }
    return false;
  }

 public:  // open value templates
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, bool>::value, bool>::type
  openFromFile(const char* _filename, T& _output) {
    size_t fileSize = getFileSize(_filename);
    char fileContents[fileSize + 1];
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
    char fileContents[fileSize + 1];
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
    char fileContents[fileSize + 1];
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
    char fileContents[fileSize + 1];
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
      println("[openFromFile<char*>] - Internal static char buffer to small for file contents, set Effortless_SPIFFS_CHAR_SIZE larger if required (default 1024)");
    }

    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, String>::value ||
                                                     Effortless_SPIFFS_Internal::is_same<T, std::string>::value,
                                                 bool>::type
  openFromFile(const char* _filename, T& _output) {
    size_t fileSize = getFileSize(_filename);
    char fileContents[fileSize + 1];
    memset(fileContents, 0x00, fileSize + 1);

    if (openFile(_filename, fileContents, fileSize)) {
      _output = fileContents;
      return true;
    }
    return false;
  }
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, DynamicJsonDocument>::value, bool>::type
  openFromFile(const char* _filename, T& _output) {
    size_t fileSize = getFileSize(_filename);
    char fileContents[fileSize + 1];
    memset(fileContents, 0x00, fileSize + 1);

    if (openFile(_filename, fileContents, fileSize)) {
      DynamicJsonDocument currentjsonDocument(_output.capacity());
      DeserializationError jsonError = deserializeJson(currentjsonDocument, fileContents);
      if (!jsonError) {
        _output = currentjsonDocument;
        return true;
      } else {
        print("[openFromFile<DynamicJsonDocument>] - Failed to parse JSON: ");
        print(jsonError.c_str());
        print(" for file ");
        println(_filename);
      }
    }
    return false;
  }

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
    if (sprintf(inputString, "%i", _input)) {
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
    if (sprintf(inputString, "%i", _input)) {
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
  template <class T>
  typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, DynamicJsonDocument>::value, bool>::type
  saveToFile(const char* _filename, T& _input) {
    String bufferString;
    if (serializeJson(_input, bufferString)) {
      if (saveFile(_filename, bufferString.c_str())) {
        return true;
      }
    } else {
      print("[saveToFile<DynamicJsonDocument>] - Failed to serialize JSON for file ");
      println(_filename);
    }
    return false;
  }

 private:  // serial methods
  void print(const char* _msg) {
    // Check if the pointer has been set and serial has begun
    if (serial && *serial) serial->print(_msg);
  }
  void print(String _msg) {
    print(_msg.c_str());
  }
  void println(const char* _msg) {
    // Check if the pointer has been set and serial has begun
    if (serial && *serial) serial->println(_msg);
  }
  void println(String _msg) {
    println(_msg.c_str());
  }

 private:  // storage
  bool flashSizeCorrect = false;
  HardwareSerial* serial = nullptr;
};

#endif

#else
#error Effortless_SPIFFS currently works on the ESP8266 only
#endif

#else
#error Effortless_SPIFFS requires a C++ compiler
#endif