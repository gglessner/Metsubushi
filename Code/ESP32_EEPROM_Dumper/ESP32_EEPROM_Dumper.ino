#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include "i2c_addrs.h"
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32); 

#define LOGO_WIDTH    128
#define LOGO_HEIGHT   32
#define VERSION "1.4"

long eepromSize = 65535;
String filename;
String directory = "/";
File myFile; // File object for SD card
byte i2c_addr = 0x50; // Default I2C address

unsigned char PROGMEM logo_bmp[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xfc, 0xf8, 0xf8, 0xf1, 0x87, 0x9f, 0x9f, 0x0f, 0xfc, 0x38, 0xe2, 0xf3, 0xe1, 0xc7, 0xc7, 0x0f, 
	0xfc, 0xfc, 0xf8, 0xf3, 0x87, 0x9f, 0xdf, 0x0f, 0xfc, 0x39, 0xe2, 0xf3, 0xf3, 0xe7, 0xc7, 0x1f, 
	0xfc, 0xfd, 0xfb, 0xf7, 0xdf, 0xb9, 0xff, 0x3f, 0xfc, 0x3b, 0xee, 0xff, 0xf7, 0xff, 0xdf, 0x3f, 
	0xfc, 0xfd, 0xfb, 0xf7, 0xdf, 0xb9, 0xff, 0x3f, 0xfc, 0x3b, 0xee, 0xff, 0xf7, 0xff, 0xdf, 0x3f, 
	0x0c, 0x1c, 0x18, 0x37, 0xde, 0xb9, 0x61, 0x38, 0x8c, 0x3b, 0x6e, 0x0c, 0x37, 0xfc, 0xc0, 0x38, 
	0x1c, 0x3c, 0x38, 0x77, 0xde, 0xb9, 0xe3, 0x38, 0x9c, 0x3b, 0xee, 0x1c, 0x77, 0xfc, 0xc1, 0x39, 
	0xfc, 0xfc, 0xf8, 0xf3, 0xc7, 0xb9, 0xe3, 0x38, 0x9c, 0x3b, 0xee, 0x1c, 0xf7, 0xe7, 0xc7, 0x1f, 
	0xfc, 0xfc, 0xf8, 0xf1, 0xc7, 0xb9, 0xe3, 0x38, 0x9c, 0x3b, 0xee, 0x1c, 0xf7, 0xe7, 0xc7, 0x1f, 
	0x1c, 0x3c, 0x38, 0x70, 0xde, 0xb9, 0xe3, 0x38, 0x9c, 0x3b, 0xee, 0x1c, 0x77, 0xe0, 0xc1, 0x39, 
	0x1c, 0x3d, 0x38, 0x70, 0xde, 0xb9, 0xe3, 0x38, 0x9c, 0x3b, 0xee, 0x1c, 0x77, 0xe0, 0xc1, 0x39, 
	0x1c, 0x3d, 0x3b, 0x70, 0xde, 0xb9, 0xe3, 0x38, 0x9c, 0x1b, 0xee, 0x1c, 0x77, 0xe0, 0xd9, 0x39, 
	0xf8, 0xf9, 0x3b, 0x60, 0xd8, 0x1f, 0xc3, 0x30, 0xfc, 0xf3, 0xe3, 0x10, 0x64, 0xc0, 0x9f, 0x21, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void displayMenu() {
  Serial.println("\nMENU:\n");
  Serial.printf("1) Scan the I2C Bus for Devices\r\n");
  Serial.printf("2) Change I2C Address\r\n");
  Serial.printf("3) Change EEPROM size [%d]\r\n", eepromSize);
  Serial.printf("4) List files on SD card\r\n");
  Serial.printf("5) HEX Dump File\r\n");
  Serial.printf("6) Change Directory\r\n");
  Serial.printf("7) Copy File on SD card\r\n");
  Serial.printf("8) Delete a file on SD card\r\n");
  Serial.printf("9) Dump EEPROM to Serial\r\n");
  Serial.printf("10) Dump EEPROM to SD card\r\n");
  Serial.printf("11) Write EEPROM from SD card\r\n");
  Serial.printf("12) Compare File to EEPROM\r\n");
  Serial.printf("13) Patch File on SD\r\n");

  Serial.printf("\n[I2C: 0x%0.2x - DIR: %s] Selection your option: ", i2c_addr, directory);
  Serial.flush();
}

void lookupAndPrint(unsigned char byteToMatch) {
    Serial.printf("\tPossible device:\r\n");
    // Iterate through the lookup table
    for (int i = 0; i < sizeof(lookupTable) / sizeof(lookupTable[0]); i++) {
        // Check if the byte value matches
        if (lookupTable[i].byteValue == byteToMatch) {
            // Print the corresponding string
            Serial.printf("\t\t- %s\n", lookupTable[i].stringValue);
        }
    }
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

int getEepromSize() {
  int x;
  for(x=0; x< 1048576; x++) {
    if(testEeprom(x) != 1)
      break;
  } 
  Serial.printf("Size: %d\n", x);
  return(x);
}

int testEeprom(int i2c_addr) {
  Wire.requestFrom(i2c_addr,1);
  if(Wire.available()) {
    if(Wire.read()) {
      return 1;
    } else {
      return 0;
    }
  }
}

void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data ) {
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));      //writes the MSB
  Wire.write((int)(eeaddress & 0xFF));    //writes the LSB
  Wire.write(data);
  //Serial.printf("Write: %0.2d\n", data);
  Wire.endTransmission();
}

byte readEEPROM(int deviceaddress, unsigned int eeaddress ) {
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));      //writes the MSB
  Wire.write((int)(eeaddress & 0xFF));    //writes the LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()) 
    rdata = Wire.read();
  return rdata;
  }

void dumpEeprom() {

    Wire.beginTransmission(i2c_addr);
    Wire.write((int)(0 >> 8));      //writes the MSB
    Wire.write((int)(0 & 0xFF));    //writes the LSB
    Wire.endTransmission();

    for (unsigned int x = 0; x < eepromSize; x += 16) {
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

  Serial.printf("\r\nDumping EEPROM...\r\n");

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
  File file = SD.open(filepath); // Open the file on SD card

  if (!file) {
    Serial.println("Error opening file.");
    return;
  }

  Serial.printf("Writing file contents to EEPROM [%0.2X]...\n", i2c_addr);

  unsigned int addr = 0;
  int lastError = 0;

  Serial.printf("%0.6X) ", addr);
  while (file.available()) {
    byte buffer[1];
    int bytesRead = file.read(buffer, 1);

    Serial.printf("%0.2X ", buffer[0]);
    Serial.flush();
    if(((addr + 1) % 16 == 0) && (addr + 1 < eepromSize))
      Serial.printf("\n%0.6X) ", addr + 1);
  
    Wire.beginTransmission(i2c_addr);
    Wire.write(addr >> 8); // High byte of address
    Wire.write(addr & 0xFF); // Low byte of address
    Wire.write(buffer[0]); // Write data to EEPROM
    Wire.endTransmission();

    delay(5); // Delay to allow EEPROM to process the write operation
    addr++;
  }
  file.close(); // Close the file
  Serial.println("\n\nData written to EEPROM successfully.");
}

void copyFileOnSD() {
  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.printf("\r\nEnter the filename to copy: ");
  Serial.flush();
  while (Serial.available() <= 0) {
    // wait for user input
  }
  String originalFilename = directory + Serial.readString();
  originalFilename.trim();
  Serial.printf("%s\r\n", originalFilename);

  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.printf("\r\nEnter the new filename: ");
  Serial.flush();
  while (Serial.available() <= 0) {
    // wait for user input
  }
  String newFilename = directory + Serial.readString();
  newFilename.trim();
  Serial.printf("%s\r\n", newFilename);

  File originalFile = SD.open(originalFilename);
  File newFile = SD.open(newFilename, FILE_WRITE);

  if (originalFile && newFile) {
    while (originalFile.available()) {
      newFile.write(originalFile.read());
    }

    originalFile.close();
    newFile.close();

    Serial.println("File copied successfully.");
  } else {
    Serial.println("Error opening files. Make sure the filenames are correct.");
  }
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
            lookupAndPrint(address);
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

    Serial.printf("%0.6X) ", file.position() - 16);

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

void compareFileAndEeprom() {
  String filepath = directory + filename;
  myFile = SD.open(filepath); // Open the file on SD card

  int addr = 0;
  bool complete_match = true;

  Serial.printf("\r\nComparing file against EEPROM contents...\r\n");
  while (myFile.available()) {
    byte fileBuffer[16];
    byte eepromBuffer[16];

    // Read 16 bytes from file
    for (int i = 0; i < 16; i++) {
      if (myFile.available()) {
        fileBuffer[i] = myFile.read();
      }
    }

    // Read 16 bytes from EEPROM
    Wire.requestFrom(i2c_addr, 16);
    for (int i = 0; i < 16; i++) {
      eepromBuffer[i] = Wire.read();
    }

    bool match = true;

    // Compare the 16 bytes
    for (int i = 0; i < 16; i++) {
      if (fileBuffer[i] != eepromBuffer[i]) {
        match = false;
        break;
      }
    }

    if (!match) {
      complete_match = false;
      Serial.printf("Mismatch at %0.6X!\r\n",addr);

      // Print the 16 bytes in hexadecimal
      Serial.printf("  File: %0.6X) ", addr);
      for (int i = 0; i < 16; i++) {
        if (fileBuffer[i] < 16) {
          Serial.print("0");
        }
        Serial.print(fileBuffer[i], HEX);
        Serial.print(" ");
      }

      // Print the 16 bytes in ASCII
      for (int i = 0; i < 16; i++) {
        if (isprint(fileBuffer[i])) {
          Serial.write(fileBuffer[i]);
        } else {
          Serial.write('.');
        }
      }
      Serial.println();

      // Repeat for EEPROM
      Serial.printf("EEPROM: %0.6X) ", addr);
      for (int i = 0; i < 16; i++) {
        if (eepromBuffer[i] < 16) {
          Serial.print("0");
        }
        Serial.print(eepromBuffer[i], HEX);
        Serial.print(" ");
      }

      for (int i = 0; i < 16; i++) {
        if (isprint(eepromBuffer[i])) {
          Serial.write(eepromBuffer[i]);
        } else {
          Serial.write('.');
        }
      }
      Serial.println();
    }

    addr += 16;
  }
  if(complete_match) {
    Serial.printf("File completely matches EEPROM contents.\r\n");
  }

  myFile.close(); // Close the file
}

void patchFileOnSD() {
  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.printf("Enter the filename to patch: \r\n");
  Serial.flush();
  while (Serial.available() == 0) {
    // Wait for user input
  }
  String patchFilename = directory + Serial.readStringUntil('\n');

  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.printf("Enter the memory address in hexadecimal (e.g., 0x1000): \r\n");
  Serial.flush();
  while (Serial.available() == 0) {
    // Wait for user input
  }
  String addressStr = Serial.readStringUntil('\n');
  int address = strtol(addressStr.c_str(), NULL, 16);

  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.printf("Patch location: %0.6Xn\r\n\r\n", address);
  Serial.printf("Enter 'hex' or 'ascii' to specify the input format: \t\n");
  Serial.flush();
  while (Serial.available() == 0) {
    // Wait for user input
  }
  String inputFormat = Serial.readString();
  inputFormat.trim();

  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.printf("Enter the bytes to patch (in hex separated by spaces or ASCII string): \r\n");
  Serial.flush();
  while (Serial.available() == 0) {
    // Wait for user input
  }
  String patchData = Serial.readString();
  patchData.trim();

  File file = SD.open(patchFilename, "r+");

  if (file) {
    if (inputFormat == "hex") {
      // Parse hex bytes and write to file
      while (patchData.length() > 0) {
        String hexByte = patchData.substring(0, 2);
        patchData = patchData.substring(3); // Move to next byte

        byte byteValue = strtol(hexByte.c_str(), NULL, 16);
        file.seek(address);
        file.write(byteValue);
        address++;
      }
    } else if (inputFormat == "ascii") {
      // Write ASCII string to file

      for (int i = 0; i < patchData.length(); i++) {
        file.seek(address);
        file.write(patchData[i]);
        address++; 
      }
    } else {
      Serial.println("Invalid input format. Please enter 'hex' or 'ascii'.");
    }

    file.close();
    Serial.println("File patched successfully.");
  } else {
    Serial.println("Error opening file. Make sure the filename is correct.");
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
  display.init();
  display.drawXbm(0, 0, LOGO_WIDTH, LOGO_HEIGHT, logo_bmp);
  display.display();
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
    case 7: // Copy File on SD
      copyFileOnSD();
      break;
    case 8: // Delete a File
      promptForFilename();
      deleteFile();
      break;
    case 9: // Dump EEPROM to Serial
      dumpEeprom();
      break;
    case 10: // Dump EEPROM to SD
      promptForFilename();
      dumpEepromToSD();
      break;
    case 11: // Write EEPROM from SD
      promptForFilename();
      writeEepromFromSD();
      break;
    case 12: // Compare File and EEPROM
      promptForFilename();
      compareFileAndEeprom();
      break;
    case 13: // Patch File on SD
      patchFileOnSD();
      break;
    case 14: // Get EEPROM size
      getEepromSize();
      break;
    default:
      // Handle invalid option
      Serial.printf("Invalid option!\r\n");
      break;
  }
}
