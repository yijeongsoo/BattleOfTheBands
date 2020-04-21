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
FatReader f;      // This holds the information for the file we're play
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

Adafruit_MMA8451 mma = Adafruit_MMA8451();

uint8_t dirLevel; // indent level for file/dir names    (for prettyprinting)
dir_t dirBuf;     // buffer for directory reads

#define DEBOUNCE 100  // button debouncer

/*
 * Define macro to put error messages in flash memory
 */
#define error(msg) error_P(PSTR(msg))

// Function definitions (we define them here, but the code is below)
void play(FatReader &dir);

//////////////////////////////////// SETUP
void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps for debugging
  
  putstring_nl("\nWave test!");  // say we woke up!
  
  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(FreeRam());

// enable pull-up resistors on switch pins (analog inputs)
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  digitalWrite(14, HIGH);
  digitalWrite(15, HIGH);

  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    error("Card init. failed!");  // Something went wrong, lets print out why
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
  
  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {   // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                           // we found one, lets bail
  }
  if (part == 5) {                     // if we ended up not finding one  :(
    error("No valid FAT partition!");  // Something went wrong, lets print out why
  }
  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(), DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    error("Can't open root dir!");      // Something went wrong,
  }
  
  // Whew! We got past the tough parts.
  putstring_nl("Files found (* = fragmented):");

  // Print out all of the files in all the directories.
  root.ls(LS_R | LS_FLAG_FRAGMENTED);

  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("MMA8451 found!");
  
  mma.setRange(MMA8451_RANGE_2_G);
  
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");
}

//////////////////////////////////// LOOP
void loop() {

  /* Get a new sensor event */ 
  sensors_event_t event; 
  mma.getEvent(&event);
  
  // Read the 'raw' data in 14-bit counts
  mma.read();
  //Serial.print(" X:\t"); Serial.print(mma.x); 
  //Serial.print(" X: \t"); Serial.print(event.acceleration.x); Serial.println("\t");
  int value1 = mma.x;
  

    /* Get the orientation of the sensor */
  uint8_t o = mma.getOrientation();
 
  //if (value1 < -500){
    switch (check_switches()) {
      case 1:
        Serial.println("C chord");
        playcomplete("C_chord.wav");
        break;
      case 2:
        Serial.println("F chord");
        playcomplete("F_chord.wav");
        break;
      case 3:
        Serial.println("G chord");
        playcomplete("G_chord.wav");
        break;
      case 4:
        Serial.println("Am chord");
        playcomplete("Am_chord.wav");
        break;
    }
// }
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

byte check_switches()
{
  static byte previous[4];
  static long time[41];
  byte reading;
  byte pressed;
  byte index;
  pressed = 0;

  for (byte index = 0; index < 4; ++index) {
    reading = digitalRead(6 + index);
    if (reading == LOW && previous[index] == HIGH && millis() - time[index] > DEBOUNCE)
    {
      // switch pressed
      Serial.println("switch pressed");
      time[index] = millis();
      pressed = index + 1;
      break;
    }
    previous[index] = reading;
  }
  // return switch number (1 - 4)
  //Serial.println(pressed);
  return (pressed);
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
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
} 
