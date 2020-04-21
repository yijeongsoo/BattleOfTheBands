#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include <WaveHC.h>
#include <WaveUtil.h>

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader file;      // This holds the information for the file we're play
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

Adafruit_MMA8451 mma = Adafruit_MMA8451();

uint8_t dirLevel; // indent level for file/dir names    (for prettyprinting)
dir_t dirBuf;     // buffer for directory reads

#define DEBOUNCE 100  // button debouncer

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

/*
 * Define macro to put error messages in flash memory
 */
#define error(msg) error_P(PSTR(msg))


// Function definitions (we define them here, but the code is below)
void play(FatReader &dir);
void playcomplete(char *name);
void sdErrorCheck(void);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  putstring_nl("\nWave test!");  // say we woke up!
  
  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(FreeRam());

    //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
  
  // Whew! We got past the tough parts.
  putstring_nl("Files found (* = fragmented):");
// Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  😞
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }

  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
    // Print out all of the files in all the directories.
  root.ls(LS_R | LS_FLAG_FRAGMENTED);

  
  // Whew! We got past the tough parts.
  putstring_nl("Ready!");


  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("MMA8451 found!");
  
  mma.setRange(MMA8451_RANGE_2_G);
  
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  
  // read the state of the pushbutton value:
  buttonState1 = digitalRead(button1Pin);
  buttonState2 = digitalRead(button2Pin);
  buttonState3 = digitalRead(button3Pin);
  buttonState4 = digitalRead(button4Pin);
 
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mma.getEvent(&event);
  
  // Read the 'raw' data in 14-bit counts
  mma.read();
  //Serial.print(" X:\t"); Serial.print(mma.x); 
  //Serial.print(" X: \t"); Serial.print(event.acceleration.x); Serial.println("\t");
  int value1 = mma.x;
  
  if (value1 < -500){
    // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
    if (buttonState1 == LOW) {
      Serial.println("C chord");
      playcomplete("C_chord.wav");
      Serial.println();
    }
    else if (buttonState2 == LOW) {
      Serial.println("F chord");
      playcomplete("F_chord.wav");
      Serial.println();
    }
    if (buttonState3 == LOW) {
      Serial.println("G chord");
      playcomplete("G_chord.wav");
      Serial.println();
    }
    if (buttonState4 == LOW) {
      Serial.println("Am chord");
      playcomplete("Am_chord.wav");
      Serial.println();
    }
  }
}
/////////////////////////////////// HELPERS
/*
 * print error message and halt
 */
void error_P(const char *str) {
  PgmPrint("Error: ");
  SerialPrint_P(str);
  sdErrorCheck();
  while(1);
}
/*
 * print error message and halt if SD I/O error, great for debugging!
 */
void sdErrorCheck(void) {
  if (!card.errorCode()) return;
  PgmPrint("\r\nSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  PgmPrint(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

// Plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
  }
  // now its done playing
}

void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!file.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(file)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
} 
