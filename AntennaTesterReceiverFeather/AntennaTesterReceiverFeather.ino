#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <LoRa.h>

static const uint32_t GPSBaud = 9600;

#define SS      A5
#define RST     20
#define DI0     21


#define spreadingFactor 9
#define SignalBandwidth 125E3
#define preambleLength 8
#define codingRateDenominator 8

// The TinyGPS++ object
TinyGPSPlus gps;

// Using Serial1 on M0 to communicate with U-Blox GPS
HardwareSerial & GPSserial = Serial1;

#define FREQUENCY 868.9E6

#define LEDPIN 13


double retFrequency;
int retSF;

struct locStruct
{
  bool isValid;
  bool isUpdated;
  uint32_t age;
  double lat;
  double lng;
  double altitude;
  double hdop;
  int power;
  int rssi;
} sendLoc, recLoc;

struct GPStimeStruct
{
  bool isValid;
  bool isUpdated;
  uint32_t age;
  uint32_t value;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t centisecond;
} GPStime;


void displayVersion() {
  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  Serial.print(F("Testing TinyGPS++ library v. "));
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
}

void getPosition() {
  unsigned long GPSentry = millis();
  while (GPSserial.available() > 0)
    if (gps.encode(GPSserial.read()))
      if (millis() > GPSentry + 5000 && gps.charsProcessed() < 10)
      {
        Serial.println(F("No GPS detected: check wiring."));
        while (true);
      }
  if (gps.location.isValid())
  {
    if (gps.location.isValid()) {
      sendLoc.isUpdated = (gps.location.isUpdated());
      sendLoc.lat = gps.location.lat();
      sendLoc.lng = gps.location.lng();
      sendLoc.altitude = gps.altitude.meters();
      sendLoc.hdop = gps.hdop.value();
    }
    if (gps.time.isValid()) {
      GPStime.year = gps.date.year();
      GPStime.month = gps.date.month();
      GPStime.day = gps.date.day();
      GPStime.hour = gps.time.hour();
      GPStime.minute = gps.time.minute();
      GPStime.second = gps.time.second();
      GPStime.centisecond = gps.time.centisecond();
    }
  }
}

void receivingMessage () {
  unsigned long eentry = millis();
  StaticJsonBuffer<500> jsonBuffer;
  char json[500];
  Serial.println("HIGH");
  digitalWrite(LEDPIN, LOW);

  // Receive Message

  //  Serial.print("Start receiving ");
  //  Serial.println(millis());
  while (LoRa.parsePacket() == 0) yield();
  // received a packet
  // Serial.println("Received packet");

  // read packet
  json[0] = '\0';
  while (LoRa.available()) {
    byte hi = strlen(json);
    json[hi] = (char)LoRa.read();
    json[hi + 1] = '\0';
  }
  //  Serial.println(json);
  JsonObject& recMessage = jsonBuffer.parse(json);

  if (recMessage.success()) {

    recMessage["RSSI"] = LoRa.packetRssi();

    recMessage["rLat"] = sendLoc.lat;
    recMessage["rLng"] = sendLoc.lng;
    recMessage["rAlt"] = sendLoc.altitude;
    recMessage["rHdop"] = sendLoc.hdop;

    //    Serial.println("-------");
    //    Serial.print("Received  Data / Start sending ");
    //    Serial.println(millis());
    recMessage.printTo(Serial);
    //    Serial.println();
    Serial.println("LOW");
    digitalWrite(LEDPIN, HIGH);
    delay(200);

    // Send packet back
    String sendMessage;
    recMessage.printTo(sendMessage);
    LoRa.setTxPower(17);
    LoRa.beginPacket();
    LoRa.print(sendMessage);
    LoRa.endPacket();
    Serial.println("Package sent");
  } else Serial.println("parseObject() failed");
  Serial.println(millis() - eentry);
}

void initLoRa() {
  SPI.begin();
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
    while (1) {
      digitalWrite(LEDPIN, 1);
      delay(200);
      digitalWrite(LEDPIN, 0);
      delay(200);
    }
  }
  Serial.print("FREQUENCY ");
  Serial.println(FREQUENCY);
  LoRa.setTxPower(1);
  Serial.print("LoRa Spreading Factor: ");
  Serial.println(spreadingFactor);
  LoRa.setSpreadingFactor(spreadingFactor);
  Serial.print("LoRa Signal Bandwidth: ");
  Serial.println(SignalBandwidth);
  LoRa.setSignalBandwidth(SignalBandwidth);
  Serial.println("LoRa Initial OK!");
}

void setup() {
  Serial.begin(115200);
  //while(!Serial.available()){}
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, 1);
  delay(1000);
  digitalWrite(LEDPIN, 0);
  GPSserial.begin(GPSBaud);
  initLoRa();
}

void loop() {
  getPosition();
  receivingMessage();
  yield();
}
