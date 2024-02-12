#include <Arduino.h>
#include <MIDI.h>



#include <WiFi.h>
#include <TimeLib.h>
#include <TelnetStream.h>

#define LED 2


const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h


// Create and bind the MIDI interface to the default hardware Serial port
struct CustomBaudRateSettings : public MIDI_NAMESPACE::DefaultSerialSettings {
  static const long BaudRate = 115200;
};

#if defined(ARDUINO_SAM_DUE) || defined(USBCON) || defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__)
    // Leonardo, Due and other USB boards use Serial1 by default.
    MIDI_NAMESPACE::SerialMIDI<HardwareSerial, CustomBaudRateSettings> serialMIDI(Serial1);
    MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial, CustomBaudRateSettings>> MIDI((MIDI_NAMESPACE::SerialMIDI<HardwareSerial, CustomBaudRateSettings>&)serialMIDI);
#else
    MIDI_NAMESPACE::SerialMIDI<HardwareSerial, CustomBaudRateSettings> serialMIDI(Serial);
    MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial, CustomBaudRateSettings>> MIDI((MIDI_NAMESPACE::SerialMIDI<HardwareSerial, CustomBaudRateSettings>&)serialMIDI);
#endif


static void log(byte pitch) {
  static int i = 0;

  char timeStr[20];
  sprintf(timeStr, "%02d-%02d-%02d %02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());

  TelnetStream.print(i++);
  TelnetStream.print(" ");
  TelnetStream.print(timeStr);
  TelnetStream.print(" Pitch: ");
  TelnetStream.println(pitch);
}


void doSomeStuffWithNoteOn(byte channel, byte pitch, byte velocity)
{
  //digitalWrite(LED, !digitalRead(LED));
  log (pitch);  
  digitalWrite(LED, HIGH);
  // TelnetStream.println(pitch);
}

void doSomeStuffWithNoteOff(byte channel, byte pitch, byte velocity)
{
  digitalWrite(LED, LOW);
}


void setup() {
  // Serial.begin(115200);

  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);


  // Serial.print("Attempting to connect to WPA SSID: ");
  // Serial.println(ssid);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    // Serial.println("Failed to connect.");
    while (1) {
      delay(10);
    }
  }

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < SECS_YR_2000) {
    delay(100);
    now = time(nullptr);
  }
  setTime(now);

  IPAddress ip = WiFi.localIP();
  // Serial.println();
  // Serial.println("Connected to WiFi network.");
  // Serial.print("Connect with Telnet client to ");
  // Serial.println(ip);

  TelnetStream.begin();
  // Serial.end();

  digitalWrite(LED,HIGH);

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
  MIDI.setHandleNoteOn(doSomeStuffWithNoteOn);
  MIDI.setHandleNoteOff(doSomeStuffWithNoteOff);
}

void loop() {

  MIDI.read();
  byte i = 0;
  switch (TelnetStream.read()) {
    case 'L':
      log(i);
      digitalWrite(LED,LOW);
      delay(100);
      digitalWrite(LED,HIGH);
      break;
    case 'R':
      TelnetStream.stop();
      delay(100);
      ESP.restart();
      break;
    case 'C':
      TelnetStream.println("bye bye");
      TelnetStream.stop();
      break;
  }

}

