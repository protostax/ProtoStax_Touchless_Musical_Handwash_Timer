/*************************************************** 
  ProtoStax Touchless Musical Handwash Timer

  This is a example sketch for a Touchless Musical Handwash Timer using Arduino Uno,
  Adafruit 0.56" 4-Digit 7-Segment Display w/I2C Backpack - Blue,
  Piezo Buzzer PS1240,
  HC-SR04 Ultrasonic Sensor,
  and
  ProtoStax Enclosure for Arduino --> https://www.protostax.com/products/protostax-for-arduino
  ProtoStax Enclosure for Breadboards/Custom --> https://www.protostax.com/products/protostax-for-breadboard
  ProtoStax Horizontal Stacking Kit --> https://www.protostax.com/products/protostax-horizontal-stacking-kit
  ProtoStax Kit for Ultrasonic Sensor HC-SR04 --> https://www.protostax.com/products/protostax-kit-for-ultrasonic-sensor-hc-sr04

  The following pins are used in this example - 
  SCL and SDA pins for I2C communications with the 7-Segment, pins 5&6 for the HC-SR04 
  and pin 12 for the piezo buzzer. This uses the tone
  library, which uses Timer2 which will conflict with pins 3 and 11. 
  You can also use the replacement NewTone library (tone --> NewTone, and noTone --> noNewTone), which is smaller, 
  has faster execution and uses Timer1 which can resolve conflicts with pins 3 and 11 
  (but which conflicts with pins 9 and 10 instead)

  A lot of time and effort has gone into providing this and other code. 
  Please support ProtoStax by purchasing products from us!
 
  Written by Sridhar Rajagopal for ProtoStax
  BSD license. All text above must be included in any redistribution
 */


#include <Wire.h>
#include "pitches.h"
#include "noteDurations.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_7segment matrix = Adafruit_7segment();

const int trigPin = 6; 
const int echoPin = 5;

float duration, distance;
unsigned long currentTime;
int countDown;


// pitches.h defines all the frequencies for the different notes in different octaves
// Here we pick a default octave for a note without octave specified, so it is easy to change it in one place
// For example, if you want to go down an octave, change NOTE_C7 to NOTE_C6 and so on

#define NOTE_C  NOTE_C7
#define NOTE_CS NOTE_CS7
#define NOTE_D  NOTE_D7
#define NOTE_DS NOTE_DS7
#define NOTE_E  NOTE_E7
#define NOTE_F  NOTE_F7
#define NOTE_FS NOTE_FS7
#define NOTE_G  NOTE_G7
#define NOTE_GS NOTE_GS7
#define NOTE_A  NOTE_A7
#define NOTE_AS NOTE_AS7
#define NOTE_B  NOTE_B7


// The Piezo uses pin 12 in our example. Change as needed. Remember that you cannot use pins 3 and 11 
// when using the tone library
#define TONE_PIN 12

// All of our tunes have the same characteristics 
// A Note in a melody has a frequency and a duration (1 = whole note, 2 = half note, etc)
// A melody consists of an array of such Notes (the melody)
// playTunes takes a pointer to the melody array of Notes, as well as a speedup factor. 
// The speedup factor is to facilitate playing at a higher speed (1 plays at intended speed, 
// 2 at twice the speed and so on) without having to modify the noteDurations array

typedef struct Note {
  int frequency;
  float duration;   
} Note;

typedef struct Melody {
  Note *notes;
  int numNotes;
  int wholeNoteDurationMs;  
} Melody;

// This melody is playing all the notes of the octave from C up to B and back down to C

Note melodyStart[] = {
  {NOTE_C, NOTE_WHOLE}, {NOTE_CS, NOTE_WHOLE}, {NOTE_D, NOTE_WHOLE}, {NOTE_DS, NOTE_WHOLE}, {NOTE_E, NOTE_WHOLE}, {NOTE_F, NOTE_WHOLE}, {NOTE_FS, NOTE_WHOLE}, {NOTE_G, NOTE_WHOLE}, {NOTE_GS, NOTE_WHOLE}, {NOTE_A, NOTE_WHOLE}, {NOTE_AS, NOTE_WHOLE}, {NOTE_B, NOTE_WHOLE},
  {NOTE_AS, NOTE_WHOLE}, {NOTE_A, NOTE_WHOLE}, {NOTE_GS, NOTE_WHOLE}, {NOTE_G, NOTE_WHOLE}, {NOTE_FS, NOTE_WHOLE}, {NOTE_F, NOTE_WHOLE}, {NOTE_E, NOTE_WHOLE}, {NOTE_DS, NOTE_WHOLE}, {NOTE_D, NOTE_WHOLE}, {NOTE_CS, NOTE_WHOLE}, {NOTE_C, NOTE_WHOLE}
};


// Happy Birthday

Note melody[] = { 
  {NOTE_G6, DOTTED(NOTE_EIGHTH)}, {NOTE_G6, NOTE_SIXTEENTH}, {NOTE_A6, NOTE_QUARTER}, {NOTE_G6, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, {NOTE_B6, NOTE_HALF}, 
  {NOTE_G6, DOTTED(NOTE_EIGHTH)}, {NOTE_G6, NOTE_SIXTEENTH}, {NOTE_A6, NOTE_QUARTER}, {NOTE_G6, NOTE_QUARTER}, {NOTE_D7, NOTE_QUARTER}, {NOTE_C7, NOTE_HALF},
  {NOTE_G6, DOTTED(NOTE_EIGHTH)}, {NOTE_G6, NOTE_SIXTEENTH}, {NOTE_E7, NOTE_QUARTER}, {NOTE_D7, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, {NOTE_B6, NOTE_QUARTER}, {NOTE_A6, NOTE_HALF}, 
  {NOTE_F7, DOTTED(NOTE_EIGHTH)}, {NOTE_F7, NOTE_SIXTEENTH}, {NOTE_E7, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, {NOTE_D7, NOTE_QUARTER}, {NOTE_C7, NOTE_HALF},      
};

// Do a deer - Sound of Music

Note melody2[] = {
  {NOTE_C6, DOTTED(NOTE_QUARTER)}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, DOTTED(NOTE_QUARTER)}, {NOTE_C6, NOTE_EIGHTH}, {NOTE_E6, NOTE_QUARTER}, {NOTE_C6, NOTE_QUARTER}, {NOTE_E6, NOTE_HALF}, 
  {NOTE_D6, DOTTED(NOTE_QUARTER)}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_F6, NOTE_EIGHTH}, {NOTE_F6, NOTE_EIGHTH}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_F6, NOTE_WHOLE}, 
  {NOTE_E6, DOTTED(NOTE_QUARTER)}, {NOTE_F6, NOTE_EIGHTH}, {NOTE_G6, DOTTED(NOTE_QUARTER)}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_G6, NOTE_QUARTER}, {NOTE_E6, NOTE_QUARTER}, {NOTE_G6, NOTE_HALF}, 
  {NOTE_F6, DOTTED(NOTE_QUARTER)}, {NOTE_G6, NOTE_EIGHTH}, {NOTE_A6, NOTE_EIGHTH}, {NOTE_A6, NOTE_EIGHTH}, {NOTE_G6, NOTE_EIGHTH}, {NOTE_F6, NOTE_EIGHTH}, {NOTE_A6, NOTE_WHOLE}, 
  {NOTE_G6, DOTTED(NOTE_QUARTER)}, {NOTE_C6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_F6, NOTE_EIGHTH}, {NOTE_G6, NOTE_EIGHTH}, {NOTE_A6, NOTE_WHOLE},
  {NOTE_A6, DOTTED(NOTE_QUARTER)}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_F6, NOTE_EIGHTH}, {NOTE_G6, NOTE_EIGHTH}, {NOTE_A6, NOTE_EIGHTH}, {NOTE_B6, NOTE_WHOLE},
  {NOTE_B6, DOTTED(NOTE_QUARTER)}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_F6, NOTE_EIGHTH}, {NOTE_G6, NOTE_EIGHTH}, {NOTE_A6, NOTE_EIGHTH}, {NOTE_B6, NOTE_EIGHTH}, {NOTE_C7, NOTE_WHOLE},
  {NOTE_C7, NOTE_EIGHTH}, {NOTE_B6, NOTE_EIGHTH}, {NOTE_A6, NOTE_QUARTER}, {NOTE_F6, NOTE_QUARTER}, {NOTE_B6, NOTE_QUARTER}, {NOTE_G6, NOTE_QUARTER}, 
  {NOTE_C7, NOTE_QUARTER}, {NOTE_G6, NOTE_QUARTER}, {NOTE_E6, NOTE_QUARTER}, {NOTE_C6, NOTE_QUARTER},
  {NOTE_C6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_F6, NOTE_EIGHTH}, {NOTE_G6, NOTE_EIGHTH}, {NOTE_A6, NOTE_EIGHTH}, {NOTE_B6, NOTE_QUARTER}, 
  {NOTE_C7, NOTE_QUARTER}, {NOTE_G6, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, 
 

};

// We Will Rock You! Queen

Note melody3[] = {
  {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_QUARTER}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, 
  {NOTE_E6, NOTE_QUARTER}, {NOTE_E6, NOTE_QUARTER}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH},
  {NOTE_E6, NOTE_QUARTER}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_QUARTER},
  {NOTE_A6, NOTE_QUARTER}, {NOTE_G6, NOTE_QUARTER},  {NOTE_G6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_QUARTER},
  
  {NOTE_E6, NOTE_EIGHTH}, {NOTE_E6, NOTE_QUARTER}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_E6, NOTE_QUARTER}, {REST, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, 
  {NOTE_E6, NOTE_QUARTER}, {NOTE_E6, NOTE_QUARTER}, {NOTE_E6, NOTE_QUARTER}, {REST, NOTE_QUARTER}, 
  {NOTE_D6, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, {NOTE_D6, NOTE_QUARTER}, {NOTE_D6, NOTE_QUARTER}, {NOTE_B5, NOTE_QUARTER}, 
     
  {NOTE_A5, NOTE_EIGHTH}, {NOTE_G5, NOTE_EIGHTH}, {NOTE_E5, NOTE_EIGHTH}, {NOTE_E5, NOTE_EIGHTH}, {NOTE_E5, NOTE_QUARTER}, {NOTE_B5, NOTE_EIGHTH}, {NOTE_D6, NOTE_EIGHTH}, 

  {NOTE_G7, NOTE_HALF}, {NOTE_FS7, NOTE_HALF}, {NOTE_E7, NOTE_HALF}, {NOTE_D7, NOTE_HALF}, {NOTE_E7, NOTE_QUARTER}, {NOTE_E7, NOTE_QUARTER}, {REST, NOTE_HALF}, {REST, NOTE_WHOLE},
  {NOTE_G7, NOTE_HALF}, {NOTE_FS7, NOTE_HALF}, {NOTE_E7, NOTE_HALF}, {NOTE_D7, NOTE_HALF}, {NOTE_E7, NOTE_QUARTER}, {NOTE_E7, NOTE_QUARTER}, {REST, NOTE_HALF}, {REST, NOTE_WHOLE},

};

// Jeopardy!

Note melody4[] = {
  {NOTE_G6, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, {NOTE_G6, NOTE_QUARTER}, {NOTE_C6, NOTE_QUARTER}, 
  {NOTE_G6, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, {NOTE_G6, NOTE_HALF},
  {NOTE_G6, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, {NOTE_G6, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, 
  {NOTE_E7, DOTTED(NOTE_QUARTER)}, {NOTE_D7, NOTE_EIGHTH}, {NOTE_C7, NOTE_EIGHTH}, {NOTE_B6, NOTE_EIGHTH}, {NOTE_AS6, NOTE_EIGHTH}, {NOTE_A6, NOTE_EIGHTH}, 
  
  {NOTE_G6, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, {NOTE_G6, NOTE_QUARTER}, {NOTE_C6, NOTE_QUARTER}, 
  {NOTE_G6, NOTE_QUARTER}, {NOTE_C7, NOTE_QUARTER}, {NOTE_G6, NOTE_HALF},
  {NOTE_C7, DOTTED(NOTE_QUARTER)}, {NOTE_A6, NOTE_EIGHTH}, {NOTE_G6, NOTE_QUARTER}, {NOTE_F6, NOTE_QUARTER},
  {NOTE_E6, NOTE_QUARTER}, {NOTE_D6, NOTE_QUARTER}, {NOTE_C6, NOTE_QUARTER}, {REST, NOTE_QUARTER}, 

  
  {NOTE_AS6, NOTE_QUARTER}, {NOTE_DS7, NOTE_QUARTER}, {NOTE_AS6, NOTE_QUARTER}, {NOTE_DS6, NOTE_EIGHTH}, {NOTE_DS6, NOTE_EIGHTH},
  {NOTE_AS6, NOTE_QUARTER}, {NOTE_DS7, NOTE_QUARTER}, {NOTE_AS6, NOTE_HALF},  
  {NOTE_AS6, NOTE_QUARTER}, {NOTE_DS7, NOTE_QUARTER}, {NOTE_AS6, NOTE_QUARTER}, {NOTE_DS7, NOTE_QUARTER},  
  {NOTE_G7, DOTTED(NOTE_QUARTER)}, {NOTE_F7, NOTE_EIGHTH}, {NOTE_DS7, NOTE_EIGHTH}, {NOTE_D7, NOTE_EIGHTH}, {NOTE_C7, NOTE_EIGHTH}, {NOTE_B6, NOTE_EIGHTH},

  {NOTE_AS6, NOTE_QUARTER}, {NOTE_DS7, NOTE_QUARTER}, {NOTE_AS6, NOTE_QUARTER}, {NOTE_DS6, NOTE_EIGHTH}, {NOTE_DS6, NOTE_EIGHTH},
  {NOTE_AS6, NOTE_QUARTER}, {NOTE_DS7, NOTE_QUARTER}, {NOTE_AS6, NOTE_HALF},  
  {NOTE_DS7, DOTTED(NOTE_QUARTER)}, {NOTE_C7, NOTE_EIGHTH}, {NOTE_AS6, NOTE_QUARTER}, {NOTE_GS6, NOTE_QUARTER},
  {NOTE_G6, NOTE_QUARTER}, {REST, NOTE_QUARTER}, {NOTE_F6, NOTE_QUARTER}, {REST, NOTE_QUARTER}, 
  {NOTE_DS6, NOTE_QUARTER}, {REST, NOTE_QUARTER}, {REST, NOTE_HALF},
};


#define MELODY_LENGTH(m) (sizeof(m)/sizeof(m[0]))

Melody melodies[] = {
  {melody, MELODY_LENGTH(melody), 1250}, {melody3, MELODY_LENGTH(melody3), 1000}, {melody4, MELODY_LENGTH(melody4), 1000}
};

#define NUM_MELODIES(m) ((sizeof(m)/sizeof(m[0])))

// TODO: Taking it further! 
// These 4 melodies pretty much max out the Arduino Uno's memory, and don't leave enough SRAM for the I2C calls to the 7-segment display. 
// The array of Melodys above has only 3 out of the 4 defined to leave enough RAM. 
// 
// There is plenty of FLASH/PROGMEM in comparison. To take this project further
// You can move the above Note and Melody struct arrays defined to PROGMEM to free up more space
// This is an advanced project, and you will have to understand how to store and retrieve structs from PROGMEM
// However, it will be well worth your time to get an understanding of the concepts!
// 
// To give you an appreciation for the differences, this program (with 3 melodies) takes up 31% of program storage and 76% of dynamic memory on an Uno. 
// With the above variables defined in PROGMEM, it occupies 32% of program space and only 22% of dynamic memory! That means you can 
// easily add plenty of songs to this Touchless Jingle-playing Hand Wash Timer once you move things to PROGMEM! :-) 
// I've not included the PROGMEM stuff here as the code will be more complex to understand, and plus to give you a challenge! :-)

void setup() {
  Serial.begin(9600);

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  randomSeed(analogRead(0));
  
  
  Serial.println(F("ProtoStax Touchless Handwash Timer Demo")); 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
    
  matrix.begin(0x70);
  
  playTune(melodyStart, 23, 48);
}


void loop() {
  static int melodyNum = 0;
  static int num = 0;
  static int startMusic = 0;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;  

  if (distance < 10 && !startMusic) {
    startMusic = 1;
    // initializeTimer1();
    countDown = 20;
    currentTime = millis();
    melodyNum = (melodyNum+1) % (NUM_MELODIES(melodies));
  }
  if (startMusic) {
    // Pick the melody to play
    Melody mel = melodies[melodyNum];
    Note *m = mel.notes;
    int mSize = mel.numNotes;
    // speedUp is an easy way to speed up the playing of the notes. The best way would be to 
    // set the wholeNoteDurationMs appropriately. 
    int speedUp = 1; 
    
    noTone(TONE_PIN); // Start with a clean slate
    for (int thisNote = 0; thisNote < mSize; thisNote++) {
  
      // to calculate the note duration, take the duration of the whole note and multiply by the note type.
      //e.g. quarter note = wholeNoteDurationMs * 0.25, eighth note = wholeNoteDurationMs * 1/8 (0.125), etc.
      // reduce the duration by the speedup factor to increase the speed of playing 
      // by an appropriate amount
      int noteDuration = mel.wholeNoteDurationMs * m[thisNote].duration / speedUp;
      unsigned long noteTime = millis();
      tone(TONE_PIN, m[thisNote].frequency, noteDuration);
  
      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      
      unsigned long nowTime = millis();
      countDown = 20 - (int)((nowTime - currentTime)/1000);

      // Look Ma No delay()!
      // Don't use delay(), as we still want to update the countDown timer 
      // and update the display
      while(millis() - noteTime <= pauseBetweenNotes) {
        countDown = 20 - (int)((millis() - currentTime)/1000);
        matrix.print(countDown);
        matrix.writeDisplay();  
        delay(10);      
      }
      noTone(TONE_PIN);
      matrix.print(countDown);
      matrix.writeDisplay();
      
    }
    delay(100); 
    if (countDown <= 0) { 
      startMusic = 0;  
      matrix.clear();
      matrix.writeDisplay();
    }
  }

  delay(100);

}

// This function is used to play a given melody
// Arguments are:
// array of Notes for the melody
// size of array
// speedup factor - higher number plays the melody faster
// This uses delay() and is shown for reference
// It is also used in setup() to play a quick initialization music 
// This assumes that the whole note is of duration 1000 milliseconds

void playTune(Note *m, int mSize, int speedUp) {
  noTone(TONE_PIN); // Start with a clean slate
  for (int thisNote = 0; thisNote < mSize; thisNote++) {

    // to calculate the note duration, take one second multiplies by the note type.
    //e.g. quarter note = 1000.0 * 0.25, eighth note = 1000 * 1/8 (0.125), etc.
    // reduce the duration by the speedup factor to increase the speed of playing 
    // by an appropriate amount
    int noteDuration = 1000.0 * m[thisNote].duration / speedUp;
    tone(TONE_PIN, m[thisNote].frequency, noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(TONE_PIN);
  }
  delay(100);
  
}
