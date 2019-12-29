#include <Effortless_SPIFFS.h>

void setup() {
  // Start Serial
  Serial.begin(115200);
  Serial.println();

  // Small delay for startup
  delay(1000);

  // Create a eSPIFFS class
  // eSPIFFS fileSystem;
  eSPIFFS fileSystem(&Serial);  // Optional - allow the methods to print debug

  /* A Word of Warning!
		The ESP SPIFFS class and consequently this library 
		only suports a file name size of 31 characters, it 
		is up to you to manage this. You may get away with 
		having more characters, but this could lead to 
		undefined behaviour
	*/

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
  double newFloat;
  if (writeToFlash) {
    newFloat = (float)random(1, 1000) / (float)random(1000, 65563);
    fileSystem.saveToFile("/Float.txt", newFloat);
  } else {
    fileSystem.openFromFile("/Float.txt", newFloat);
  }
  Serial.print("Float value is: ");
  Serial.println(newFloat, 15);

  // Signed Values
  signed long newSigned;
  if (writeToFlash) {
    newSigned = random(-100000, 100000);
    fileSystem.saveToFile("/Signed.txt", newSigned);
  } else {
    fileSystem.openFromFile("/Signed.txt", newSigned);
  }
  Serial.print("Signed value is: ");
  Serial.println(newSigned);

  // Unsigned Values
  unsigned long newUnsigned;
  if (writeToFlash) {
    newUnsigned = random(0, 100000);
    fileSystem.saveToFile("/Unsigned.txt", newUnsigned);
  } else {
    fileSystem.openFromFile("/Unsigned.txt", newUnsigned);
  }
  Serial.print("Unsigned value is: ");
  Serial.println(newUnsigned);

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

  // Reboot to see what values are stored
  if (writeToFlash) {
    Serial.println("Rebooting...");
    ESP.restart();
  }
}

void loop() {}
