/*
  SD card test

  This example shows how use the utility libraries on which the'
  SD library is based in order to get info about your SD card.
  Very useful for testing a card when you're not sure whether its working or not.

  The circuit:
    SD card attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila
 ** CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila
 ** CS - depends on your SD card shield or module.
 		Pin 4 used here for consistency with other Arduino examples


  created  28 Mar 2011
  by Limor Fried
  modified 9 Apr 2012
  by Tom Igoe
*/
// include the SD library:
#include <SPI.h>
#include <SD.h>
#include <SDConfig.h>
#include <SoftwareSerial.h>
//#include <TinyGPS++.h>
#include <Ethernet.h>

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: SDCARD_SS_PIN
const int SD_SELECT_PIN = 4;
const int ETHERNET_SELECT_PIN = 10;

IPAddress ip, gateway, subnet, serverIP;
int serverPort;
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

EthernetClient client;

SoftwareSerial gpsSerial(2, 3); //rx, tx 

boolean readConfiguration() {
  /*
   * Length of the longest line expected in the config file.
   * The larger this number, the more memory is used
   * to read the file.
   * You probably won't need to change this number.
   */
  int maxLineLength = 127;
  SDConfig cfg;
  char fileName[] = "setup.cfg";
  const char* CONFIG_HMAC = "HMAC";  
  const char* CONFIG_IP = "IP";
  const char* CONFIG_GATEWAY = "GATEWAY";
  const char* CONFIG_SUBNET = "SUBNET";
  const char* CONFIG_SERVER_IP = "SERVER_IP";
  const char* CONFIG_SERVER_PORT = "SERVER_PORT";

  // Setup the SD card 
  Serial.println("Calling SD.begin()...");
  
  if (!SD.begin(SD_SELECT_PIN)) {
    Serial.println("SD.begin() failed. Check: ");
    Serial.println("  card insertion,");
    Serial.println("  SD shield I/O pins and chip select,");
    Serial.println("  card formatting.");
    return;
  }
  
  Serial.println("...succeeded.");
  // Read our configuration from the SD card file.
  
  // Open the configuration file.
  if (!cfg.begin(fileName, maxLineLength)) {
    Serial.print("Failed to open configuration file: ");
    Serial.println(fileName);
    return false;
  }
  
  // Read each setting from the file.
  while (cfg.readNextSetting()) {
    if (cfg.nameIs(CONFIG_HMAC)) {
      String hmacStr = cfg.getValue();
      Serial.print("HMAC: ");
      Serial.println(hmacStr);      
    } else if (cfg.nameIs(CONFIG_IP)) {
      ip = cfg.getIPAddress();
      Serial.print("IP: ");
      Serial.println(ip);
    } else if (cfg.nameIs(CONFIG_GATEWAY)) {
      gateway = cfg.getIPAddress();
      Serial.print("GATEWAY: ");
      Serial.println(gateway);
    } else if (cfg.nameIs(CONFIG_SUBNET)) {
      subnet = cfg.getIPAddress();
      Serial.print("SUBNET: ");
      Serial.println(subnet);
    } else if (cfg.nameIs(CONFIG_SERVER_IP)) {
      serverIP = cfg.getIPAddress();
      Serial.print("SERVER IP: ");
      Serial.println(serverIP);
    } else if (cfg.nameIs(CONFIG_SERVER_PORT)) {
      serverPort = cfg.getIntValue();
      Serial.print("SERVER PORT: ");
      Serial.println(serverPort);
    } else {
      Serial.print("Unknown name in config: ");
      Serial.println(cfg.getName());
    }
  }
  cfg.end();
  return true;
}

void setup() {
  gpsSerial.begin(9600);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (readConfiguration() == false) {
    return;
  }

  //Init Ethernet
  Ethernet.init(ETHERNET_SELECT_PIN);
  Ethernet.begin(mac, ip, gateway, subnet);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
}

void printToSerial(void) {
  while(gpsSerial.available() > 0) {
    Serial.print(gpsSerial.read());
  }
}

void loop(void) {
  while (!client.connected()) {
    if (client.connect(serverIP, serverPort)) {
      Serial.println("connected");
      break;
    } else {
      Serial.println("connection failed");
      printToSerial();
      delay(1000);
    }
  }

  if (client.connected()) {
    while(gpsSerial.available() > 0) {
      client.print(gpsSerial.read());
    }    
  }
  else {
    client.stop();
    printToSerial();
  } 
  delay(1000);
}
