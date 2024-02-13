#include <Arduino.h>
#include <MIDI.h>



#include <WiFi.h>
#include <TimeLib.h>
#include <TelnetStream.h>
#include <math.h> 

#define LED 2
#define DIR_PIN 2
#define STEP_PIN 0

bool noteon = false;
bool dir = false;
int interval = 0;

const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
#include <secrets.h>

u_int32_t intervals[128];


// from https://gist.github.com/YuxiUx/ef84328d95b10d0fcbf537de77b936cd
// calculate Interval in microseconds
float pitchToInterval(int note) {
    float a = 440; //frequency of A (coomon value is 440Hz)
    return floor(1000000/((a / 32) * pow(2, ((note - 9) / 12.0))));
}

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
  TelnetStream.print(pitch);
  TelnetStream.print(" Freq: ");
  TelnetStream.println(intervals[pitch]);
}


void doSomeStuffWithNoteOn(byte channel, byte pitch, byte velocity)
{
  log (pitch);  
  //  digitalWrite(LED, HIGH);
  noteon=true;
  interval=intervals[pitch];
}

void doSomeStuffWithNoteOff(byte channel, byte pitch, byte velocity)
{
  //digitalWrite(LED, LOW);
  noteon=false;
  dir = !dir;
  digitalWrite(DIR_PIN, dir);
}


void setup() {
  // Serial.begin(115200);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(STEP_PIN,LOW);

  pinMode(DIR_PIN, OUTPUT);

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

  // create lookup table for frequencies
  for (int pitch = 1; pitch < 128; pitch++){
    intervals[pitch] = pitchToInterval(pitch);
    log(pitch);
  }

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
  MIDI.setHandleNoteOn(doSomeStuffWithNoteOn);
  MIDI.setHandleNoteOff(doSomeStuffWithNoteOff);
}

void loop() {

  MIDI.read();

  if (noteon){
    digitalWrite(STEP_PIN,HIGH);
    delayMicroseconds(interval);
    digitalWrite(STEP_PIN,LOW);
    delayMicroseconds(interval);
  }


  byte i = 0;
  switch (TelnetStream.read()) {
    case 'L':
      for (i=40; i<80; i++){
        log(i);
      }
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

