/*
  Button

  Turns on and off a light emitting diode(LED) connected to digital pin 13,
  when pressing a pushbutton attached to pin 2.

  The circuit:
  - LED attached from pin 13 to ground
  - pushbutton attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground

  - Note: on most Arduinos there is already an LED on the board
    attached to pin 13.

  created 2005
  by DojoDave <http://www.0j0.org>
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Button
*/

// constants won't change. They're used here to set pin numbers:
const int button1Pin = 6;     // the number of the pushbutton pin
const int button2Pin = 7;     // the number of the pushbutton pin
const int button3Pin = 8;     // the number of the pushbutton pin
const int button4Pin = 9;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin

// variables will change:
int buttonState1 = 0;         // variable for reading the pushbutton status
int buttonState2 = 0;
int buttonState3 = 0;
int buttonState4 = 0;

void setup() {
  Serial.begin(9600);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  pinMode(button4Pin, INPUT);
}

void loop() {
  // read the state of the pushbutton value:
  buttonState1 = digitalRead(button1Pin);
  buttonState2 = digitalRead(button2Pin);
  buttonState3 = digitalRead(button3Pin);
  buttonState4 = digitalRead(button4Pin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState1 == LOW) {
    // turn LED on:
    Serial.println("button1");
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
    // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState2 == LOW) {
    // turn LED on:
    Serial.println("button2");
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
    // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState3 == LOW) {
    // turn LED on:
    Serial.println("button3");
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
    // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState4 == LOW) {
    // turn LED on:
    Serial.println("button4");
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
}
