#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>

#define VERSION "1.0"

long eepromSize = 65535;
String filename;
String directory = "/";
File myFile; // File object for SD card
byte i2c_addr = 0x50; // Default I2C address

void displayMenu() {
  Serial.println("\nMENU:\n");
  Serial.printf("1) Scan the I2C Bus for Devices\r\n");
  Serial.printf("2) Change I2C Address\r\n");
  Serial.printf("3) Change EEPROM size [%d]\r\n", eepromSize);
  Serial.printf("4) List files on SD card\r\n");
  Serial.printf("5) HEX Dump File\r\n");
  Serial.printf("6) Change Directory\r\n");
  Serial.printf("7) Delete a file on SD card\r\n");
  Serial.printf("8) Dump EEPROM to Serial\r\n");
  Serial.printf("9) Dump EEPROM to SD card\r\n");
  Serial.printf("10) Write EEPROM from SD card\r\n");

  Serial.printf("\n[I2C: 0x%0.2x - DIR: %s] Selection your option: ", i2c_addr, directory);
  Serial.flush();
}

void changeEepromSize() {

  int my_eepromSize = 0;

  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.println("Enter your EEPROM byte size:");
  while (Serial.available() <= 0) {
    // wait for user input
  }
  while(my_eepromSize == 0) {
    my_eepromSize = Serial.parseInt();
  }
  eepromSize = my_eepromSize;
}

void listFiles() {
  File root = SD.open(directory);

  if (!root) {
    Serial.printf("Failed to open directory \"%s\"\n", directory);
    return;
  }

  Serial.printf("\r\n\r\nListing files for %s:\r\n", directory);
  Serial.printf("-------------------------------------------\r\n");
  Serial.printf("%-20s %-12s %-8s\r\n", "Name", "Size (bytes)", "Type");

  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      // No more files
      break;
    }
    
    // Print file details
    Serial.printf("%-20s %-12lu %-8s\r\n", entry.name(), entry.size(), (entry.isDirectory() ? "DIR" : "FILE"));
    entry.close();
  }

  Serial.printf("-------------------------------------------\r\n");
  root.close();
}

void deleteFile() {
  String filepath = directory + filename;
  if (SD.remove(filepath)) {
    Serial.println("File deleted successfully.");
  } else {
    Serial.println("Failed to delete the file.");
  }
}

void dumpEeprom() {

    for (int x = 0; x < eepromSize; x += 16) {
    byte buffer[16];
    Wire.requestFrom(i2c_addr, 16);
    int index = 0;
    while (Wire.available() && index < 16) {
      buffer[index] = Wire.read();
      index++;
    }

    Serial.printf("%0.6X) ", x);

    // Print the 16 bytes in hexadecimal
    for (int i = 0; i < 16; i++) {
      if (buffer[i] < 16) {
        Serial.print("0");
      }
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }

    // Print the 16 bytes in ASCII
    Serial.print(" ");
    for (int i = 0; i < 16; i++) {
      if (isprint(buffer[i])) {
        Serial.write(buffer[i]);
      } else {
        Serial.write('.');
      }
    }
    Serial.println();
  }
}

void dumpEepromToSD() {
  String filepath = directory + filename;
  myFile = SD.open(filepath, FILE_WRITE); // Open the file on SD card

  Serial.printf("Dumping EEPROM...\r\n");

  for (int x = 0; x < eepromSize; x += 16) {
    byte buffer[16];
    Wire.requestFrom(i2c_addr, 16);
    int index = 0;
    while (Wire.available() && index < 16) {
      buffer[index] = Wire.read();
      index++;
    }

    myFile.write(buffer, 16); // Write the binary data directly   
    if (x % 255 == 0 && x != 0) { // Print "." for every 16K iteration of x
      Serial.printf(".");
      Serial.flush();
    }
  }
  Serial.printf("\r\nDone.\r\n");

  myFile.close(); // Close the file
}

void writeEepromFromSD() {
  String filepath = directory + filename;
  myFile = SD.open(filepath); // Open the file on SD card

  int addr = 0;
  while (myFile.available()) {
    byte data = myFile.parseInt();
    Wire.beginTransmission(i2c_addr);
    Wire.write(addr);
    Wire.write(data);
    Wire.endTransmission();
    addr++;
  }

  myFile.close(); // Close the file
}

void scanI2CDevices() {
    byte error, address;
    int deviceCount = 0;

    Serial.printf("\r\nScanning for I2C devices...\r\n\r\n");

    for(address = 1; address < 127; address++ ) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            Serial.println();
            deviceCount++;
        }
        delay(10); // Some devices need a small delay
    }

    if (deviceCount == 0) {
        Serial.printf("\r\nNo I2C devices found.\r\n");
    } else {
        Serial.printf("\r\nScan complete.\r\n");
    }
}

void readFromFileAndHexDump() {
  String filepath = directory + filename;
  File file = SD.open(filepath); // Open the file on SD card

  if (!file) {
    Serial.println("Error opening file.");
    return;
  }

  Serial.println("Dumping file contents in hex...");

  while (file.available()) {
    byte buffer[16];
    int bytesRead = file.read(buffer, 16);

    Serial.printf("%0.6X) ", file.position());

    // Print the 16 bytes in hexadecimal
    for (int i = 0; i < bytesRead; i++) {
      if (buffer[i] < 16) {
        Serial.print("0");
      }
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }

    // Print the remaining spaces for alignment
    for (int i = bytesRead; i < 16; i++) {
      Serial.print("   "); // Three spaces for each byte not read
    }

    // Print the 16 bytes in ASCII
    Serial.print(" ");
    for (int i = 0; i < bytesRead; i++) {
      if (isprint(buffer[i])) {
        Serial.write(buffer[i]);
      } else {
        Serial.write('.');
      }
    }
    Serial.println();
  }

  Serial.println("File dump complete.");

  file.close(); // Close the file
}

void promptI2CAddress() {
  String i2c_string;

  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.printf("\r\nEnter the I2C address in hexadecimal format (e.g., 0x50): ");
  Serial.flush();
  while (Serial.available() <= 0) {
    // wait for user input
  }
  i2c_string = Serial.readString();
  i2c_string.trim();
  Serial.printf("%s\r\n", i2c_string);

    if (i2c_string.startsWith("0x") && i2c_string.length() == 4) {
        i2c_addr = (byte)strtol(i2c_string.substring(2).c_str(), NULL, 16); // Convert hexadecimal string to byte
        Serial.print("I2C address set to 0x");
        if (i2c_addr < 16) {
            Serial.print("0");
        }
        Serial.println(i2c_addr, HEX);
    } else {
        Serial.println("Invalid format. Please enter the address in the format 0xXX (e.g., 0x50).");
    }
}

void promptForFilename() {
  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.printf("\r\nEnter the filename: ");
  Serial.flush();
  while (Serial.available() <= 0) {
    // wait for user input
  }
  filename = Serial.readString();
  filename.trim();
  Serial.printf("%s\r\n", filename);
}

void promptForDirectory() {
  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.printf("\r\nEnter the directory: ");
  Serial.flush();
  while (Serial.available() <= 0) {
    // wait for user input
  }
  directory = Serial.readString();
  directory.trim();
  if (!directory.startsWith("/")) {
    directory = "/" + directory; // Add "/" at the beginning if it's missing
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  SPI.begin();

  if (!SD.begin()) {
    Serial.printf("Card Mount Failed\r\n");
    return;
  }

  delay(2000);  // Give the serial port a couple of seconds before writing to it
  Serial.print("\n\n\n\n\n\n\n\n\n\n\n\n███████╗███████╗██████╗ ██████╗  ██████╗ ███╗   ███╗\r\n██╔════╝██╔════╝██╔══██╗██╔══██╗██╔═══██╗████╗ ████║\n█████╗  █████╗  ██████╔╝██████╔╝██║   ██║██╔████╔██║\r\n██╔══╝  ██╔══╝  ██╔═══╝ ██╔══██╗██║   ██║██║╚██╔╝██║\r\n███████╗███████╗██║     ██║  ██║╚██████╔╝██║ ╚═╝ ██║\r\n╚══════╝╚══════╝╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝\r\n\r\n██████╗ ██╗   ██╗███╗   ███╗██████╗ ███████╗██████╗\r\n██╔══██╗██║   ██║████╗ ████║██╔══██╗██╔════╝██╔══██╗\r\n██║  ██║██║   ██║██╔████╔██║██████╔╝█████╗  ██████╔╝\r\n██║  ██║██║   ██║██║╚██╔╝██║██╔═══╝ ██╔══╝  ██╔══██╗\r\n██████╔╝╚██████╔╝██║ ╚═╝ ██║██║     ███████╗██║  ██║\r\n╚═════╝  ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚══════╝╚═╝  ╚═╝\r\n");
  Serial.printf("\t\t\t\t\t\tv%s\r\n", VERSION);
}

void loop() {
  int option = 0; // Get the user input for the menu option

  while (Serial.available() > 0) {
    Serial.read();
  }
  displayMenu();
  while (Serial.available() <= 0) {
    // wait for user input
  }
  option = Serial.parseInt();
  Serial.printf("%d\r\n", option);
  switch(option) {
    case 1: // Scan I2C Bus
      scanI2CDevices();
      break;
    case 2: // Change I2C Address
      promptI2CAddress();
      break;
    case 3: // Change EEPROM Size
      changeEepromSize();
      break;
    case 4: // List Files
      listFiles();
      break;
    case 5: // HEX Dump File
      promptForFilename();
      readFromFileAndHexDump();
      break;
    case 6: // Change Directory
      promptForDirectory();
      break;
    case 7: // Delete a File
      promptForFilename();
      deleteFile();
      break;
    case 8: // Dump EEPROM to Serial
      dumpEeprom();
      break;
    case 9: // Dump EEPROM to SD
      promptForFilename();
      dumpEepromToSD();
      break;
    case 10: // Write EEPROM from SD
      promptForFilename();
      writeEepromFromSD();
      break;
    default:
      // Handle invalid option
      Serial.printf("Invalid option!\r\n");
      break;
  }
}
