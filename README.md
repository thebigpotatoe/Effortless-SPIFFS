# Effortless-SPIFFS

[![GitHub release](https://img.shields.io/github/release/thebigpotatoe/Effortless-SPIFFS.svg)](https://github.com/thebigpotatoe/Effortless-SPIFFS/releases)
[![arduino-library-badge](https://www.ardu-badge.com/badge/Effortless-SPIFFS.svg?)](https://www.ardu-badge.com/Effortless-SPIFFS)
[![PlatformIO Build Status](https://github.com/thebigpotatoe/Effortless-SPIFFS/workflows/PlatformIO%20CI/badge.svg)](https://github.com/thebigpotatoe/Effortless-SPIFFS/actions?query=workflow%3A%22PlatformIO+CI%22)
[![Arduino Build Status](https://github.com/thebigpotatoe/Effortless-SPIFFS/workflows/Arduino/badge.svg)](https://github.com/thebigpotatoe/Effortless-SPIFFS/actions?query=workflow%3A%22PlatformIO+CI%22)
[![GitHub License](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/thebigpotatoe/Effortless-SPIFFS/blob/master/LICENSE)

A class designed to make reading and storing data on the __ESP8266__ and __ESP32__ effortless. This library aims to make access to SPIFFS much easier to allow users to get on with writing the important stuff rather than debugging why storage isn't working.

## :exclamation: IMPORTANT :exclamation:

When using in a project or exploring the example make sure to set your SPIFFS size correctly for your project/board. While the library will not fail when trying to open files from a non existent SPIFFS partition, failure to set the size will mean the library and hence project will not work as intended.

The library has a very handy method `checkFlashConfig()` to check this for you. Using this method at the start of your sketch with a meaningful `Serial.print()` statement will save you a lot of time wondering why things are not working in production.

For more info on setting the SPIFFS size see:

- [ESP8266 SPIFFS in Arduino](https://cdn.instructables.com/ORIG/FST/OP12/IZT6TEBJ/FSTOP12IZT6TEBJ.png?auto=webp&frame=1&width=1024&fit=bounds&md=f16daccb8697a6bd8728838feb18d9de)
- [ESP8266 SPIFFS in PlatformIO](https://docs.platformio.org/en/latest/platforms/espressif8266.html#flash-size)
- [ESP32 SPIFFS in Arduino](https://user-images.githubusercontent.com/26627719/44614648-d76f9980-a852-11e8-8383-d6ae74a2ff5b.png)
- [ESP32 SPIFFS in PlatformIO](https://docs.platformio.org/en/latest/platforms/espressif32.html#partition-tables)

## Quick Start

The example below is all you need to get going with opening and saving data from a variable. Be sure to also check out the example `Effortless_Spiffs_Basic.ino` for detailed understanding of how to use this library with all data types.

``` c++
#include <Effortless_SPIFFS.h>

eSPIFFS fileSystem;
float myVariable;

void setup(){
    Serial.begin(115200);
    Serial.println();
}

void loop() {
    // Open the storage file and save data to myVariable
    if (fileSystem.openFromFile("/Float.txt", myVariable)) {
        Serial.print("Successfully read file and parsed data: ");
        Serial.println(myVariable, 6);
    }

    // Set myVariable to a random value
    myVariable = (float)random(1, 1000) / (float)random(1000, 65563);
    Serial.print("myVariable is now: ");
    Serial.println(myVariable, 6);

    // Write the data back to the SPIFFS
    if (fileSystem.saveToFile("/Float.txt", myVariable)) {
        Serial.println("Successfully wrote data to file");
    }

    // Delay between loops
    delay(10000);
}
```

## Usage

### As an Object

This library can be used directly by creating an object using the eSIFFS class. When declaring the class there is an optional input for a reference to a Print object to help with debugging.

``` c++
// Definition
eSPIFFS();
eSPIFFS(&Print);

// Usage
eSPIFFS fileSystem;
eSPIFFS fileSystem(&Print);
```

### Extending a Class

Each method in the class definition of eSPIFFS is defined as virtual so that it can be overriden by a child class if desired. The definitions of these are described below.

``` c++
virtual inline bool checkFlashConfig()
virtual inline bool getFileSize(const char* _filename)
virtual bool openFile(const char* _filename, char* _output, size_t _len = 0)
virtual bool saveFile(const char* _filename, const char* _input)
```

There are two functions however which are not virtual; `openFromFile` and `saveToFile`. These are instead template functions which rely on an implementation of `std::enable_if` and `std::is_same`. These functionality provided by these can be extended to new types by using the same format template function as the eSPIFFS class.

``` c++
template <class T>
typename Effortless_SPIFFS_Internal::enable_if<Effortless_SPIFFS_Internal::is_same<T, bool>::value, __NewTypeHere__>::type
eSPIFFS::openFromFile(const char* _filename, NewType& _output)
```

## Helper Methods

### Check that SPIFFS size is correct

eSPIFFS checks if the SPIFFS system is correct in the Arduino IDE using `checkFlashConfig` each time `saveFile` and `openFile` are called. This is to ensure that SPIFFS can be read and written to before doing any useful work. This method uses Serial debug outputs to tell the user how to set up spiffs if passed a serial reference. This function is public and inline to optimise for frequent calls.

``` c++
// Definition
inline bool checkFlashConfig()

// Usage
eSPIFFS fileSystem();
bool spiffsSetCorrectly = fileSystem.checkFlashConfig();
```

#### Getting the file size

The eSPIFFS class will get the file size of any file given a path. Th method `getFileSize` is used internally and is extended to the public API to make finding the length of a file easy. If a file is not found or not able to be read, the value will be 0.

``` c++
// Definition
inline size_t getFileSize(const char* fileName)

// Usage
eSPIFFS fileSystem();
size_t fileSize = fileSystem.getFileSize("/Example.file");
```

## Saving data to files

The eSPIFFS API allows users to store data to the SPIFFS two possible methods; by passing a const char* or a variable reference of your choice.

### Saving data to file from a const char* C String

The first method `saveFile` is used to store data into SPIFFS is via a const char* C string. The function takes a filename and a const char* pointer to a string which is then printed to SPIFFS. The file will return true if the data was written and false if there was an issue.

``` c++
// Definition
bool saveFile(const char* _filename, const char* _input)

// Usage
eSPIFFS fileSystem();
const char* myString = "Hello World";
fileSystem.saveFile("Example.file", myString);
```

#### Saving data to file using a variable

The second method `saveToFile` is used to store data is to use a reference to a variable of the users choice. This function wraps `saveFile` by parsing the users variable to a C String and saving that. The arguments it takes are filename and a reference value to any standard type. It also supports String, std::string, and ArduinoJson DynamicJsonDocuments. This method will also return true on a successful write to SPIFFS and false if there is an error.

``` c++
// Definition
bool saveToFile(const char*, &bool);
bool saveToFile(const char*, &float);
bool saveToFile(const char*, &double);
bool saveToFile(const char*, &signed char);
bool saveToFile(const char*, &unsigned char);
bool saveToFile(const char*, &signed int);
bool saveToFile(const char*, &unsigned int);
bool saveToFile(const char*, &signed short);
bool saveToFile(const char*, &unsigned short);
bool saveToFile(const char*, &signed long);
bool saveToFile(const char*, &unsigned long);
bool saveToFile(const char*, &char*);
bool saveToFile(const char*, &const char*);
bool saveToFile(const char*, &String);
bool saveToFile(const char*, &std::string);
bool saveToFile(const char*, &ArduinoJson::DynamicJsonDocument);

// Usage
eSPIFFS fileSystem();
float myVariable = 324.890;
fileSystem.saveToFile("Example.file", myVariable);
```

## Opening data from files

The eSPIFFS API extends access to the SPIFFS of your ESP8266 in two ways; by modifying a C String or by storing a parsed value to a passed variable reference.

### Opening files as a string

The first method `openFile` accesses data stored in the SPIFFS is through a modified const char* value. This method takes a filename, a char pointer to an output C String buffer, and the size of the number of bytes to read from the file. The function will return true if it stored the data in the char pointer and false if it failed for any reason.

> It should be noted that if the number of bytes asked to read will be automatically limited to the size of the file. Using the getFileSize() method before this is useful to set a char array of the correct size.

``` c++
// Definition
bool openFile(const char* filename, char* outputString, size_t numBytesToRead = 0);

// Usage
eSPIFFS fileSystem();
size_t fileSize = getFileSize("/Example.file");
char* fileContents[fileSize + 1];  // Dont forget about the null terminator for C Strings
fileSystem.openFile("/Example.file", fileContents, fileSize);
Serial.print("Data stored in fileContents is: ")
Serial.println(fileContents);
```

### Opening Files with a Variable Reference

The second method `openFromFile` wraps `openFile` to return a parsed value to a variable of your choice. It takes a file name as well as a reference to your variable where it will store parsed data. The method returns true if a value was parsed and stored correctly and false if it failed at any point. If the function failed it will not modify the original data in the variable reference. This method supports all basic types plus String, std::string, and ArduinoJson DynamicJsonDocuments

``` c++
// Definitions
bool openFromFile(const char*, &bool);
bool openFromFile(const char*, &float);
bool openFromFile(const char*, &double);
bool openFromFile(const char*, &signed char);
bool openFromFile(const char*, &unsigned char);
bool openFromFile(const char*, &signed int);
bool openFromFile(const char*, &unsigned int);
bool openFromFile(const char*, &signed short);
bool openFromFile(const char*, &unsigned short);
bool openFromFile(const char*, &signed long);
bool openFromFile(const char*, &unsigned long);
bool openFromFile(const char*, &char*);
bool openFromFile(const char*, &const char*);
bool openFromFile(const char*, &String);
bool openFromFile(const char*, &std::string);
bool openFromFile(const char*, &ArduinoJson::DynamicJsonDocument);

// Usage
eSPIFFS fileSystem();
float myVariable;
fileSystem.openFromFile("/Example.file", myVariable);
Serial.print("Data stored in myVariable is: ")
Serial.println(myVariable, 6);
```

## Contributing and Feedback

This is my first Arduino library and while I have tried to optimise it there is most likely room for improvement. Any feedback in the form of issues or pull request are welcome.
