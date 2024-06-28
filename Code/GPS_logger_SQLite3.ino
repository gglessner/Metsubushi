#define OLED
#define OLED_OFFSET 5

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <sqlite3.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include "WiFi.h"
#ifdef OLED
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
#endif

char sql[1024];
sqlite3 *db1;
sqlite3_stmt *res;
const char *tail;
int rc;
HardwareSerial SerialPort(2);
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
// GPS data...
double lat;
double lng;
int month;
int day;
int year;
int hour;
int minute;
int second;
int alt;
double acc;
char timestamp_string[] = "%04d-%02d-%02d %02d:%02d:%02d";
int bt_count = 0;
char my_timestamp[23] = {0};
int oled_mode = 0;

// The TinyGPSPlus object
TinyGPSPlus gps;

#ifdef OLED
SSD1306Wire display(0x3c, 21, 22, GEOMETRY_128_32);
#endif

void dumpInfo() {

  lat = gps.location.lat();
  lng = gps.location.lng();
  alt = gps.altitude.meters();
  month = gps.date.month();
  day = gps.date.day();
  year = gps.date.year();
  hour = gps.time.hour();
  minute = gps.time.minute();
  second = gps.time.second();
  acc = gps.hdop.hdop();

  return;
}

void trim(char *str) {
  int i;
  int begin = 0;
  int end = strlen(str) - 1;

  while (isspace((unsigned char)str[begin]))
    begin++;

  while ((end >= begin) && isspace((unsigned char)str[end]))
    end--;

  // Shift all characters back to the start of the string array.
  for (i = begin; i <= end; i++)
    str[i - begin] = str[i];

  str[i - begin] = '\0';  // Null terminate string.
}

int openDb(const char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
       Serial.printf("Opened database successfully\n");
   }
   return rc;
}

void setup() {
  Serial.begin(115200);
  SerialPort.begin(GPSBaud, SERIAL_8N1, 16, 17);

#ifdef OLED
  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_16);
#endif

  SPI.begin();

  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
#ifdef OLED
    display.clear();
    display.drawString(0, OLED_OFFSET, " BAD SD");
    display.display();
#endif 
    return;
  }

  #ifdef OLED
    display.clear();
    display.drawString(0, OLED_OFFSET, " SD Okay");
    display.display();
    delay(500);
#endif 

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  sqlite3_initialize();

  // Open database 1
  if (openDb("/sd/wifi.db", &db1))
    return;

  rc = db_exec(db1, "CREATE TABLE IF NOT EXISTS wifi_data (MAC TEXT PRIMARY KEY, SSID TEXT, AuthMode TEXT, FirstSeen TEXT, LastSeen TEXT, Channel INTEGER, RSSI INTEGER, CurrentLatitude REAL, CurrentLongitude REAL, AltitudeMeters REAL, AccuracyMeters REAL, Type TEXT)");
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }

}

const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql) {
   Serial.println(sql);
   long start = micros();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   //Serial.print(F("Time taken:"));
   //Serial.println(micros()-start);
   return rc;
}

void loop() {
  int n = 0;
  char *enc_type;
  char mac[19] = { 0 };
  long my_bssid = 0;
  sqlite3_stmt *stmt;
  int count = 0;

  while (SerialPort.available() > 0) {
    if (gps.encode(SerialPort.read())) {
      dumpInfo();
    }
  }
 
  n = WiFi.scanNetworks();

  for (int i = 0; i < n; ++i) {

    switch (WiFi.encryptionType(i)) {
    case WIFI_AUTH_OPEN:
      enc_type = "OPEN";
      break;
    case WIFI_AUTH_WEP:
      enc_type = "WEP";
      break;
    case WIFI_AUTH_WPA_PSK:
      enc_type = "WPA_PSK";
      break;
    case WIFI_AUTH_WPA2_PSK:
      enc_type = "WPA2_PSK";
      break;
    case WIFI_AUTH_WPA_WPA2_PSK:
      enc_type = "WPA_WPA2_PSK";
      break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
      enc_type = "WPA2";
      break;
    case WIFI_AUTH_WPA3_PSK:
      enc_type = "WPA3_PSK";
      break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
      enc_type = "WPA2_WPA3_PSK";
      break;
    case WIFI_AUTH_WAPI_PSK:
      enc_type = "WAPI_PSK";
      break; 
    default:
      enc_type = "UNDEFINED";
    }

    char my_ssid[32] = { 0 };

    snprintf(my_ssid, sizeof(my_ssid) - 1, "%s", WiFi.SSID(i).c_str());
    trim(my_ssid);

    uint8_t *my_mac = WiFi.BSSID(i);
    snprintf(mac, sizeof(mac) - 1, "%02x:%02x:%02x:%02x:%02x:%02x", my_mac[0], my_mac[1], my_mac[2], my_mac[3], my_mac[4], my_mac[5]);
    snprintf(my_timestamp, sizeof(my_timestamp) - 1, timestamp_string, year, month, day, hour, minute, second);

    if((gps.location.isValid()) && (year > 2023)) 
    {
      if (sqlite3_prepare_v2(db1, "SELECT * FROM wifi_data WHERE MAC=?", -1, &stmt, NULL) == SQLITE_OK) 
      {
        sqlite3_bind_text(stmt, 1, mac, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
          int rssi = WiFi.RSSI(i);
          int existingRssi = sqlite3_column_int(stmt, 6);
          // If RSSI is better, update location data
          if(rssi > existingRssi)
          {
            sqlite3_finalize(stmt);
            sqlite3_prepare_v2(db1, "UPDATE wifi_data SET SSID=?, AuthMode=?, LastSeen=?, Channel=?, RSSI=?, CurrentLatitude=?, CurrentLongitude=?, AltitudeMeters=?, AccuracyMeters=?, Type=? WHERE MAC=?", -1, &stmt, NULL);
            sqlite3_bind_text(stmt, 1, my_ssid, strlen(my_ssid), SQLITE_STATIC); // SSID
            sqlite3_bind_text(stmt, 2, enc_type, strlen(enc_type), SQLITE_STATIC); // AuthMode
            sqlite3_bind_text(stmt, 3, my_timestamp, strlen(my_timestamp), SQLITE_STATIC); // LastSeen
            sqlite3_bind_int(stmt, 4, WiFi.channel(i)); // Channel
            sqlite3_bind_int(stmt, 5, WiFi.RSSI(i)); // RSSI
            sqlite3_bind_double(stmt, 6, lat); // CurrentLatitude
            sqlite3_bind_double(stmt, 7, lng); // CurrentLongitude
            sqlite3_bind_double(stmt, 8, alt); // AltitudeMeters
            sqlite3_bind_double(stmt, 9, acc); // AccuracyMeters
            sqlite3_bind_text(stmt, 10, "WIFI", 7, SQLITE_STATIC); // Type
            sqlite3_bind_text(stmt, 11, mac, strlen(mac), SQLITE_STATIC); // MAC
            if (sqlite3_step(stmt) != SQLITE_DONE) 
            {
              Serial.println("Failed to execute update statement");
            }
          } else { 
            // If RSSI isn't better, don't update location
            sqlite3_finalize(stmt);
            sqlite3_prepare_v2(db1, "UPDATE wifi_data SET SSID=?, AuthMode=?, LastSeen=?, Channel=? WHERE MAC=?", -1, &stmt, NULL);
            sqlite3_bind_text(stmt, 1, my_ssid, strlen(my_ssid), SQLITE_STATIC); // SSID
            sqlite3_bind_text(stmt, 2, enc_type, strlen(enc_type), SQLITE_STATIC); // AuthMode
            sqlite3_bind_text(stmt, 3, my_timestamp, strlen(my_timestamp), SQLITE_STATIC); // LastSeen
            sqlite3_bind_int(stmt, 4, WiFi.channel(i)); // Channel
            sqlite3_bind_text(stmt, 5, mac, strlen(mac), SQLITE_STATIC); // MAC
            if (sqlite3_step(stmt) != SQLITE_DONE) {
              Serial.println("Failed to execute update statement");
            }
          }
        } else {
          sqlite3_finalize(stmt);
          sqlite3_prepare_v2(db1, "INSERT INTO wifi_data (MAC, SSID, AuthMode, FirstSeen, LastSeen, Channel, RSSI, CurrentLatitude, CurrentLongitude, AltitudeMeters, AccuracyMeters, Type) VALUES (?,?,?,?,?,?,?,?,?,?,?,?)", -1, &stmt, NULL);
          sqlite3_bind_text(stmt, 1, mac, strlen(mac), SQLITE_STATIC); // MAC
          sqlite3_bind_text(stmt, 2, my_ssid, strlen(my_ssid), SQLITE_STATIC); // SSID
          sqlite3_bind_text(stmt, 3, enc_type, strlen(enc_type), SQLITE_STATIC); // AuthMode
          sqlite3_bind_text(stmt, 4, my_timestamp, strlen(my_timestamp), SQLITE_STATIC); // FirstSeen
          sqlite3_bind_text(stmt, 5, my_timestamp, strlen(my_timestamp), SQLITE_STATIC); // LastSeen
          sqlite3_bind_int(stmt, 6, WiFi.channel(i)); // Channel
          sqlite3_bind_int(stmt, 7, WiFi.RSSI(i)); // RSSI
          sqlite3_bind_double(stmt, 8, lat); // CurrentLatitude
          sqlite3_bind_double(stmt, 9, lng); // CurrentLongitude
          sqlite3_bind_double(stmt, 10, alt); // AltitudeMeters
          sqlite3_bind_double(stmt, 11, acc); // AccuracyMeters
          sqlite3_bind_text(stmt, 12, "WIFI", 7, SQLITE_STATIC); // Type
          if (sqlite3_step(stmt) != SQLITE_DONE) {
            Serial.println("Failed to execute insert statement");
          }
        }
        sqlite3_finalize(stmt);
      }
    }
    SerialPort.flush();
  }

  char my_count[40] = { 0 };

#ifdef OLED
  if(oled_mode == 5)
    oled_mode = 0;

  switch(oled_mode)
  {
    case 0: // Display GPS Lat and Lng
      if(gps.location.isValid()) {
        snprintf(my_count, sizeof(my_count) - 1, "%f\n%f alt: %d", lat, lng, alt);
        display.setFont(ArialMT_Plain_10);
        display.clear();
        display.drawString(0, 0, my_count);
        display.display();
        display.setFont(ArialMT_Plain_16);
      } else {
        display.clear();
        display.drawString(0, OLED_OFFSET, " NO GPS LOCK");
        display.display();
      }
      break;
    case 1: // Display last seen count
      snprintf(my_count, sizeof(my_count) - 1, "Last: %d", n);
      display.clear();
      display.drawString(0, OLED_OFFSET, my_count);
      display.display();
      break;
    case 2: // Display total from DB
      // rc = db_exec(db1, "SELECT * from wifi_data");
      rc = sqlite3_prepare_v2(db1, "SELECT COUNT(*) from wifi_data", -1, &stmt, 0);
      if (rc == SQLITE_OK) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
          count = sqlite3_column_int(stmt, 0);
        }
      }
      sqlite3_finalize(stmt);
      snprintf(my_count, sizeof(my_count) - 1, "%d", count);
      display.clear();
      display.drawString(0, OLED_OFFSET, my_count);
      display.display();
      break;
    case 3: // Display total OPEN AuthMode from DB
      rc = sqlite3_prepare_v2(db1, "SELECT COUNT(*) from wifi_data WHERE AuthMode = 'OPEN'", -1, &stmt, 0);
      if (rc == SQLITE_OK) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
          count = sqlite3_column_int(stmt, 0);
        }
      }
      sqlite3_finalize(stmt);
      snprintf(my_count, sizeof(my_count) - 1, "Open: %d", count);
      display.clear();
      display.drawString(0, OLED_OFFSET, my_count);
      display.display();
      break;
    case 4: // Display UTC Date and Time...
      if(year > 2023) 
      {
        display.setFont(ArialMT_Plain_10);
        snprintf(my_count, sizeof(my_count) - 1, "%04d-%02d-%02d\n%02d:%02d:%02d", year, month, day, hour, minute, second);
        display.clear();
        display.drawString(0, 0, my_count);
        display.display();
        display.setFont(ArialMT_Plain_16);
      } else {
        display.clear();
        display.drawString(0, OLED_OFFSET, " NO GPS TIME");
        display.display();
      }
      break;
  }

  oled_mode++;
#endif
}


