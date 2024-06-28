#include <Wire.h>

long eepromSize = 65535;
String filename;

void displayMenu() {
  Serial.println("\nMENU:\n");
  Serial.printf("1) Change EEPROM size [%d]\n", eepromSize);
  Serial.println("2) Dump EEPROM to Serial");
  Serial.println("3) Dump EEPROM to SD card");
  Serial.println("4) Write EEPROM from SD card");
  Serial.printf("\nSelection your option:\n\n");
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

void dumpEeprom() {

    for (int x = 0; x < eepromSize; x += 16) {
    byte buffer[16];
    Wire.requestFrom(0x50, 16);
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

void promptForFilename() {
  while (Serial.available() > 0) {
    Serial.read();
  }
  Serial.println("Enter the filename:\n");
  while (Serial.available() <= 0) {
    // wait for user input
  }
  filename = Serial.readString();
  filename.trim();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  delay(2000);  // Give the serial port a couple of seconds before writing to it
  Serial.print("\n███████╗███████╗██████╗ ██████╗  ██████╗ ███╗   ███╗\n██╔════╝██╔════╝██╔══██╗██╔══██╗██╔═══██╗████╗ ████║\n█████╗  █████╗  ██████╔╝██████╔╝██║   ██║██╔████╔██║\n██╔══╝  ██╔══╝  ██╔═══╝ ██╔══██╗██║   ██║██║╚██╔╝██║\n███████╗███████╗██║     ██║  ██║╚██████╔╝██║ ╚═╝ ██║\n╚══════╝╚══════╝╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝\n\n██████╗ ██╗   ██╗███╗   ███╗██████╗ ███████╗██████╗\n██╔══██╗██║   ██║████╗ ████║██╔══██╗██╔════╝██╔══██╗\n██║  ██║██║   ██║██╔████╔██║██████╔╝█████╗  ██████╔╝\n██║  ██║██║   ██║██║╚██╔╝██║██╔═══╝ ██╔══╝  ██╔══██╗\n██████╔╝╚██████╔╝██║ ╚═╝ ██║██║     ███████╗██║  ██║\n╚═════╝  ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚══════╝╚═╝  ╚═╝");
  Serial.println();
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
  switch(option) {
    case 1:
      changeEepromSize();
      break;
    case 2:
      dumpEeprom();
      break;
    case 3:
      promptForFilename();
//      dumpEepromToSdCard(filename);
      break;
    case 4:
      promptForFilename();
//      writeEepromFromSdCard(filename);
      break;
    default:
      // Handle invalid option
      break;
  }
}
