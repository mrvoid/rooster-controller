#include <SoftwareSerial.h>
#include "pwm.hpp"

const int DRIVE_FORWARD_PIN = 9;
const int ACTIVE_PIN = 10;

SoftwareSerial mySerial(14, 15); // RX, TX

PWMDriver pwm;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
unsigned long lastCommand;

enum Mode {
  REMOTE, AUTO
};

struct State {
  int mode = Mode::REMOTE;
  int move = 0;
} state; 

struct Settings {
  // Drive
  int drive_deadPower = 95;

  // Communication
  int comm_stateSendDelay = 1000;
  bool comm_idleSendState = false;
  int comm_shutdownDelay = 500;
  
  // Autopilot settings
  int auto_turnMax = 100;
  int auto_reactionDelay = 5;
} settings;

int sign(int v) {
  return v == 0 ? 0 : (v < 0 ? -1 : 1);
}

void setup() {
  // initialize serial:
  // 15/14 + 9
  Serial.begin(9600);
  mySerial.begin(9600);
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  
  // Connect to the Motor 
  pinMode(DRIVE_FORWARD_PIN, OUTPUT);
  pinMode(ACTIVE_PIN, OUTPUT);
  
  // Timing
  lastCommand = millis();
}

void _drive(int f) {
  pwm.set(DRIVE_FORWARD_PIN, f);
}

void move(int val, int ms) {
  int v = abs(val);
  if (v < 1)
    v = 0;
  else 
    v = map(v, 1, 255, settings.drive_deadPower, 255);

  _drive(v);
  
  delay(ms);
}

void stop() {
  move(0, 0);
}

void sendState() {
  Serial.print("StateMTS: ");
  Serial.print(state.move);
  Serial.println();
}

/** 
 *  Apply the current state to the hardware.
 */
void execute() {
  move(state.move, 0);
}

long lastStateUpdate = 0;
void processCommand(String command, String args) {
  if (command.compareTo("M") == 0) {
    int value = args.toInt();
    int out = constrain(map(value, -100, 100, -255, 255), -255, 255);
    state.move = out;
  }

  // Send back the state.
  if (lastStateUpdate + settings.comm_stateSendDelay < millis()) {
    sendState();
    lastStateUpdate = millis();
  }
}

void processLine(String s) {
  if (s.length() <= 1)
    return;
  
  int toCommand = s.indexOf(' ');
  if (toCommand == -1) {
      String command = s.substring(0, s.length()-1);
      Serial.println("Command: " + command + " no args.");  
      processCommand(command, "");  
  }
  else {
    String command = s.substring(0, toCommand);
    String args = s.substring(toCommand + 1, s.length()-1);
    Serial.println("Command: " + command + " args: " + args);
    processCommand(command, args);
  }
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEventX() {
  while (mySerial.available()) {
    // get the new byte:
    char inChar = (char)mySerial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

bool blink_state = false;
long blink_last = 0;
void blink() {
  if (!blink_state && millis() - blink_last < 1000)
    return;
  if (blink_state && millis() - blink_last < 100)
    return;

  blink_last = millis();
  blink_state = !blink_state;
  digitalWrite(ACTIVE_PIN, blink_state ? HIGH : LOW);
}

void loop() {
  serialEvent();
  serialEventX();
  
  if (settings.comm_idleSendState && 
      (millis() - lastCommand > settings.comm_shutdownDelay) && 
      (state.mode == Mode::REMOTE)) {
    /// state.move = 0;
    lastCommand = millis();
    sendState();
  }

  execute();
  blink();
  
  // print the string when a newline arrives:
  if (stringComplete) {
    processLine(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}




/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
