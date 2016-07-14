#include <SoftwareSerial.h>
#include "pwm.hpp"

const char* VERSION = "1.0-dev";

const int PIN_BLUETOOTH_RX = 14;
const int PIN_BLUETOOTH_TX = 15;

const int PIN_DRIVE_LED = 8;
const int PIN_DRIVE_MOTOR = 9;
const int PIN_INDICATOR_LED = 10;

SoftwareSerial mySerial(PIN_BLUETOOTH_RX, PIN_BLUETOOTH_TX);

PWMDriver pwm_motor(PIN_DRIVE_MOTOR, 0, 30);
PWMDriver pwm_light(PIN_DRIVE_LED);

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
unsigned long lastCommand;

enum Mode {
  REMOTE, AUTO
};

struct State {
  int mode = Mode::REMOTE;
  int move = 0;
  int light = 0;

  bool blink_on = false;
} state; 

struct Settings {
  // Drive
  int drive_deadPower = 95;

  // Communication
  int  comm_stateSendDelay = 1000;
  bool comm_idleSendState = false;
  int  comm_shutdownDelay = 500;

  // Blinker
  int blink_duty = 10;
  int blink_rate = 2000;
  
  // Autopilot settings
  int auto_turnMax = 100;
  int auto_reactionDelay = 5;
} settings;

int sign(int v) {
  return v == 0 ? 0 : (v < 0 ? -1 : 1);
}


void blink() {
  bool last_state = state.blink_on;
  if (settings.blink_rate == 0) {
    state.blink_on == true;
  } else {
    long cycle = millis() % settings.blink_rate;
    long phase = 100 * cycle / settings.blink_rate;
    
    state.blink_on = settings.blink_duty < phase;
  }

  // Only change when state changes and save 2 microseconds
  if (last_state ^ state.blink_on)
    digitalWrite(PIN_INDICATOR_LED, state.blink_on ? HIGH : LOW);
}

/**
 * Shutdown all activity
 */
void stop() {
  pwm_motor.set(0);
  pwm_light.set(0);
}

/** 
 *  Apply the current state to the hardware.
 */
void execute() {
  pwm_motor.setDeadZone(settings.drive_deadPower);
  pwm_motor.set(state.move);

  pwm_light.set(state.light);
}

void sendState() {
  Serial.print("StateMTS: ");
  Serial.print(state.move);
  Serial.println();
}

void listAllParameters() {
  Serial.print("MAX_TIME_TO_IDLE ");
  Serial.println(-1);
  
  Serial.print("MAX_MESSAGE_LENGTH ");
  Serial.println(199);
  
  Serial.print("IDLE_BLINK_RATE ");
  Serial.println(settings.blink_rate);
  
  Serial.print("IDLE_DUTY_CYCLE ");
  Serial.println(settings.blink_duty);
  
  Serial.print("BT_BAUD_RATE ");
  Serial.println(9600);
  
  Serial.print("BT_PIN ");
  Serial.print(PIN_BLUETOOTH_RX);
  Serial.print(" ");
  Serial.println(PIN_BLUETOOTH_RX);
}

long lastStateUpdate = 0;
void processCommand(String command, String args) {
  if (command.compareTo("M") == 0) {
    int value = args.toInt();
    int out = map(value, 0, 100, 0, 255);
    state.move = out;
  }

  if (command.compareTo("L") == 0) {
    int value = args.toInt();
    int out = map(value, 0, 100, 0, 255);
    state.light = out;
  }

  if (command.compareTo("S") == 0) {
    // TODO: Change parameters
    listAllParameters();
  }

  if (command.compareTo("R") == 0) {
    // TODO: Factory reset
  }

  if (command.compareTo("V") == 0) {
    Serial.print("Version: ");
    Serial.println(VERSION);
  }
  
  // Send back the state.
  if (lastStateUpdate + settings.comm_stateSendDelay < millis()) {
    sendState();
    lastStateUpdate = millis();
  }
}

/* 
 *  Process a line to a command. 
 *  Input parameter is mutated in this function. Beware.
 */
void processLine(String s) {
  if (s.length() <= 1)
    return;

  s.trim();
  
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

void setup() {
  // initialize serial:
  // 15/14 + 9
  Serial.begin(9600);
  mySerial.begin(9600);
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  
  // Connect to the Motor 
  pinMode(PIN_DRIVE_MOTOR, OUTPUT);
  pinMode(PIN_DRIVE_LED, OUTPUT);
  pinMode(PIN_INDICATOR_LED, OUTPUT);
  
  // Timing
  lastCommand = millis();
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
