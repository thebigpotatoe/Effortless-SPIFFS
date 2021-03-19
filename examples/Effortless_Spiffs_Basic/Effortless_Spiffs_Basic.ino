/*
Copyright (c) 2019 thebigpotatoe

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
*/

// #define USE_SERIAL_DEBUG_FOR_eSPIFFS  // Uncomment to use Serial for debug in eSIFFS library
// #include <ArduinoJson.h>  // Uncomment this to include ArduinoJson. Make sure to have it installed first
#include <Effortless_SPIFFS.h>

/* A Word of Warning!
		The ESP SPIFFS class and consequently this library
		only suports a file name size of 31 characters, it
		is up to you to manage this. You may get away with
		having more characters, but this could lead to
		undefined behaviour
	*/

void setup() {
  // Start Serial
  Serial.begin(115200);
  Serial.println();

  // Small delay for startup
  delay(1000);

  // Create a eSPIFFS class
  #ifndef USE_SERIAL_DEBUG_FOR_eSPIFFS
    // Create fileSystem
    eSPIFFS fileSystem;

    // Check Flash Size - Always try to incorrperate a check when not debugging to know if you have set the SPIFFS correctly
    if (!fileSystem.checkFlashConfig()) {
      Serial.println("Flash size was not correct! Please check your SPIFFS config and try again");
      delay(100000);
      ESP.restart();
    }
  #else
    // Create fileSystem with debug output
    eSPIFFS fileSystem(&Serial);  // Optional - allow the methods to print debug
  #endif

  // This will change value through reboots
  bool writeToFlash = false;                                   // default value will be overriden by opening file
  fileSystem.openFromFile("/writeToFlash.txt", writeToFlash);  // This will open the value of writeToFlash
  writeToFlash = !writeToFlash;                                // Flip value
  fileSystem.saveToFile("/writeToFlash.txt", writeToFlash);    // This will save the value of writeToFlash

  if (writeToFlash) {
    Serial.println();
    Serial.println("** Setting a random value to each variable and saving it **");
  } else {
    Serial.println();
    Serial.println("**  Opening last saved state of variables **");
  }

  // Booleans
  bool newBool;
  if (writeToFlash) {
    newBool = random(0, 1);
    fileSystem.saveToFile("/Boolean.txt", newBool);
  } else {
    fileSystem.openFromFile("/Boolean.txt", newBool);
  }
  Serial.print("Boolean value is: ");
  Serial.println(newBool);

  // floats and doubles
  double newFloat = 0.0;
  if (writeToFlash) {
    newFloat = (float)random(1, 1000) / (float)random(1000, 65563);
    fileSystem.saveToFile("/Float.txt", newFloat);
  } else {
    fileSystem.openFromFile("/Float.txt", newFloat);
  }
  Serial.print("Float value is: ");
  Serial.println(newFloat, 15);

  // Signed Int Values
  signed int newSignedInt = 0;
  if (writeToFlash) {
    newSignedInt = random(-100000, 100000);
    fileSystem.saveToFile("/SignedInt.txt", newSignedInt);
  } else {
    fileSystem.openFromFile("/SignedInt.txt", newSignedInt);
  }
  Serial.print("Signed Int value is: ");
  Serial.println(newSignedInt);

  // Signed Long Values
  signed long newSignedLong = 0;
  if (writeToFlash) {
    newSignedLong = random(-100000, 100000);
    fileSystem.saveToFile("/SignedLong.txt", newSignedLong);
  } else {
    fileSystem.openFromFile("/SignedLong.txt", newSignedLong);
  }
  Serial.print("Signed Long value is: ");
  Serial.println(newSignedLong);

  // Unsigned Int Values
  unsigned long newUnsignedInt = 0;
  if (writeToFlash) {
    newUnsignedInt = random(0, 100000);
    fileSystem.saveToFile("/UnsignedInt.txt", newUnsignedInt);
  } else {
    fileSystem.openFromFile("/UnsignedInt.txt", newUnsignedInt);
  }
  Serial.print("UnsignedInt value is: ");
  Serial.println(newUnsignedInt);

  // Unsigned Long Values
  unsigned long newUnsignedLong = 0;
  if (writeToFlash) {
    newUnsignedLong = random(0, 100000);
    fileSystem.saveToFile("/UnsignedLong.txt", newUnsignedLong);
  } else {
    fileSystem.openFromFile("/UnsignedLong.txt", newUnsignedLong);
  }
  Serial.print("Unsigned Long value is: ");
  Serial.println(newUnsignedLong);

  // Char buffers
  const char* newCharBuffer;
  if (writeToFlash) {
    newCharBuffer = "Hello World";
    fileSystem.saveToFile("/CharBuffer.txt", newCharBuffer);
  } else {
    fileSystem.openFromFile("/CharBuffer.txt", newCharBuffer);
  }
  Serial.print("Char Buffer is: ");
  Serial.println(newCharBuffer);

  // Strings
  String newString;
  if (writeToFlash) {
    newString = "Hello World";
    fileSystem.saveToFile("/String.txt", newString);
  } else {
    fileSystem.openFromFile("/String.txt", newString);
  }
  Serial.print("String is: ");
  Serial.println(newString);

  // std::strings
  std::string newStdString;
  if (writeToFlash) {
    newStdString = "Hello World";
    fileSystem.saveToFile("/stdString.txt", newStdString);
  } else {
    fileSystem.openFromFile("/stdString.txt", newStdString);
  }
  Serial.print("std::string is: ");
  Serial.println(newStdString.c_str());

  // Arduino JSON documents - currently only dynamic documents
#if defined ARDUINOJSON_VERSION_MAJOR && ARDUINOJSON_VERSION_MAJOR == 6
  DynamicJsonDocument jsonDocument(1024);
  if (writeToFlash) {
    jsonDocument["Value"] = random(0, 100000);
    fileSystem.saveToFile("/json.txt", jsonDocument);
  } else {
    fileSystem.openFromFile("/json.txt", jsonDocument);
  }
  Serial.print("JSON Document is: ");
  serializeJson(jsonDocument, Serial);
  Serial.println();

  JsonObject jobject = jsonDocument.to<JsonObject>();
  if (writeToFlash) {
    jobject["Value"] = random(0, 100000);
    fileSystem.saveToFile("/jsonObject.txt", jobject);
  } else {
    fileSystem.openFromFile("/jsonObject.txt", jsonDocument);
  }
  Serial.print("JSON Object is: ");
  serializeJson(jobject, Serial);
  Serial.println();

  JsonArray jarray = jsonDocument.to<JsonArray>();
  if (writeToFlash) {
    jarray[0] = random(0, 100000);
    fileSystem.saveToFile("/jsonArray.txt", jarray);
  } else {
    fileSystem.openFromFile("/jsonArray.txt", jsonDocument);
  }
  Serial.print("JSON Array is: ");
  serializeJson(jarray, Serial);
  Serial.println();
#endif

  // Reboot to see what values are stored
  if (writeToFlash) {
    Serial.println("Rebooting...");
    ESP.restart();
  }
}

void loop() {}
