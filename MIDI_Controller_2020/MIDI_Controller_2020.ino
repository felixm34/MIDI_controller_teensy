#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>

#define PIN 15
#define N_LEDS 4

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

Bounce debouncer = Bounce(); 

int j = 0;
int k = 0;
byte counter;
byte CLOCK = 248;
byte START = 250;
byte CONTINUE = 251;
byte STOP = 252;

bool togglestate[13];
int channel = 2;

int colours [6][3] = {
  {255, 0, 0},      //Red
  {0, 255, 155},    //Cyan
  {0, 0, 255},      //Blue
  {255, 155, 0},    //Yellow
  {0, 255, 0},      //Green
  {255, 0, 155}     //Magenta
};

int sequence [2][2] = {
  {24, 12}, //quarter notes
  {12, 6} //eighth notes
};

bool onOff [4] = {false, false, false, false};

int button1 = 19;
int button2 = 20;
int button3 = 21;
int button4 = 22;

int bankA = 16;
int bankB = 17;
int bankC = 18;

int lastBank = 0;

int screenSaver = 1; // minutes of inactivity before 'screensaver' kicks in

unsigned long lastActivityTime = millis();

void setup() {
  Serial.begin(9600);
  Serial.println("Setup Complete");

  strip.begin();
  
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem);

  for (int i = 16; i <= 18; i++) {
    pinMode(i, INPUT);
  }
  for (int i = 19; i <= 22; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  
  //Run startup RGB animation
  uint32_t period = 2000;    // 2 seconds
  for (uint32_t tStart = millis();  (millis() - tStart) < period;) {
    rainbowCycle(7);
  }

  // Update display based on selected bank
  updateDisplay(whichBank());
}

void loop() {

  if (whichBank() == 1 or whichBank() == 2) {
    usbMIDI.read();
    //Serial.println("Clock Mode");
  }

  if (whichBank() == 3) {
  //Serial.println("MIDI Mode");
  buttonPress();
  }

  int temp = whichBank();
  if (lastBank != temp) {
    lastBank = temp;
    updateDisplay(temp);
    lastActivityTime = millis();
  }

  // Check for last activity and update display accordingly
  lastActivity();

}

// =================== UTILITY ===================

void lastActivity() {
  if (millis() - lastActivityTime > 30000UL * screenSaver) {
    Serial.println("Screensaver applied");
      rainbowCycle(7);
    }
  else {
    Serial.println("Operation Mode");
    updateDisplay(whichBank());
  }
}

void buttonPress() {
  if (digitalRead(button1)) {
    Serial.println("Button 1 Pressed, Bank " + whichBank());
    onOff[0] = true;
    int com = (((whichBank() - 1) * 4) + 1);
    MIDIout(com, channel);
    updateDisplay(whichBank());
    while (digitalRead(button1));
    onOff[0] = false;
    updateDisplay(whichBank());
  }

  if (digitalRead(button2)) {
    Serial.println("Button 2 Pressed, Bank " + whichBank());
    onOff[1] = true;
    int com = (((whichBank() - 1) * 4) + 2);
    MIDIout(com, channel);
    updateDisplay(whichBank());
    while (digitalRead(button2));
    onOff[1] = false;
    updateDisplay(whichBank());
  }

  if (digitalRead(button3)) {
    Serial.println("Button 3 Pressed, Bank " + whichBank());
    onOff[2] = true;
    int com = (((whichBank() - 1) * 4) + 3);
    MIDIout(com, channel);
    updateDisplay(whichBank());
    while (digitalRead(button3));
    onOff[2] = false;
    updateDisplay(whichBank());
  }

  if (digitalRead(button4)) {
    Serial.println("Button 4 Pressed, Bank " + whichBank());
    onOff[3] = true;
    int com = (((whichBank() - 1) * 4) + 4);
    MIDIout(com, channel);
    updateDisplay(whichBank());
    while (digitalRead(button4));
    onOff[3] = false;
    updateDisplay(whichBank());
  }
}

// =================== DISPLAY ===================

// Update the LEDs based on selected bank
void updateDisplay(int bank) {
  for (int i = 0; i < 4; i++) {
    if (!onOff[i]) {
      strip.setPixelColor(i, 0, 0, 0);
    }
    else {
      strip.setPixelColor(i, strip.Color(colours[bank * 2 - 1][0], colours[bank * 2 - 1][1], colours[bank * 2 - 1][2]));
    }
    strip.show();
  }
}

// Determine which bank is currently selected
int whichBank() {
  if (digitalRead(bankA)) {
    //Serial.println("Bank A");
    return 1;
  }
  else if (digitalRead(bankB)) {
    //Serial.println("Bank B");
    return 2;
  }
  else if (digitalRead(bankC)) {
    //Serial.println("Bank C");
    return 3;
  }
}
// Run a rainbow cycle effect
void rainbowCycle(int speedDelay) {
  byte *c;
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < N_LEDS; i++) {
      c = Wheel(((i * 256 / N_LEDS) + j) & 255);
      strip.setPixelColor(i, *c, *(c + 1), *(c + 2));
    }
    strip.show();
    delay(speedDelay);
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];

  if (WheelPos < 85) {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  }
  else {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }
  return c;
}

// =================== MIDI ===================

void RealTimeSystem(byte realtimebyte) { //receive and process midi clock messages from DAW
  lastActivityTime = millis();
  if (whichBank() == 1 or whichBank() == 2) {
    if (realtimebyte == 248) {
      counter++;
      if (counter == sequence[whichBank() - 1][0]) {
        j++;
        if (j == 4) {
          j = 0;
        }
        counter = 0;
        //Serial.println("Beat");
        strip.setPixelColor(j, colours[whichBank() - 1][0], colours[whichBank() - 1 ][1], colours[whichBank() - 1][2]);
      }
      if (counter == sequence[whichBank() - 1][1]) {
        //Serial.println("Offbeat");
        strip.setPixelColor(j, 0, 0, 0);
      }
    }

    if (realtimebyte == START || realtimebyte == CONTINUE) {
      counter = 0;
      j = 0;
      Serial.println("Start");

      whichBank();
      strip.setPixelColor(j, colours[whichBank() - 1][0], colours[whichBank() - 1 ][1], colours[whichBank() - 1][2]);
    }
    if (realtimebyte == STOP) {
      Serial.println("Stop");
      whichBank();
      strip.setPixelColor(j, 0, 0, 0);
    }
  }
  else (whichBank());
  strip.show();
}

void MIDIout(int CC, int chan) { //send MIDI CC data to DAW
  Serial.println("MIDI out");
  lastActivityTime = millis ();
  if (togglestate[CC]) {
    usbMIDI.sendControlChange (CC, 0, chan);
    togglestate[CC] = false;
  }
  else {
    usbMIDI.sendControlChange (CC, 127, chan);
    togglestate[CC] = true;
  }

}
