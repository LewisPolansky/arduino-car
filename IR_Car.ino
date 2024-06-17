/*
 *  File: IR_remote_control_decoder_v0.2_FINAL
 * 
 *  Author: Rex Qualis Tutorials
 *  Modified: Casey Gilbertson aka ItsCaseyDambit
 *  Date: 01/29/2020
 *  Adapted for Arduino Nano: [Your Name]
 *
 *  Hardware Required
 *    1 ea.   Arduino Nano
 *    1 ea.   IR receiver module  ( I used RQ-S003 )
 *    1 ea.   IR remote               ( unidentified )
 *    3 ea.   F-M Jumper Wires
 *
 *  Purpose: This program is used to report which infrared (IR) remote control buttons are being pressed.
 *    The original "project" did not bother establishing any serial connection, so, there was ZERO report
 *    being accomplished. I included the serial connection and baud rate, cleaned-up the reports so they
 *    declare what's happening when the buttons are being held down OR when two-or-more buttons are
 *    being pressed at once.
 *    
 *    Note: controllers vary and your controller may transmit different signals than the one I used 
 *      (and, I have no ID for my controller). The signal values MAY be universal, I dunno.
 *      This is only a test. See if your controller's signals match these signals.
 *      
 *    I also documented the operations of the program so humans can (with hope) understand. PLEASE DOCUMENT EVERYTHING!
 *    
 *    If you're curious and like to experiment, try changing the REPEAT_READ_DELAY value and see how short
 *    a delay you can use before the "You're holding the button down too long..." message occurs.
 *    It's all about learning.
 *    
 *    Enjoy!
 */

/****************************************************************************************************************/
#include "IRremote.h"                           // include the IRremote.h file from our library for using predefined IR goodies
#include "Servo.h"

#define REPEAT_READ_DELAY 0x32                  // declares how long to delay before reading the next button being pressed...
                                                // ( 0x32 == 50/1000 of one second ) - too short of a delay and repeat reads occur

int receiver = 0xb;                             // set Arduino digital pin-11 as the IR signal receiver 

                                                // declare the objects...                                                  
IRrecv irrecv(receiver);                        // create an instance of 'irrecv'
decode_results results;                         // create an instance of 'decode_results'

Servo steeringServo;

/****************************************************************************************************************/
const int speakerPin = 8;                       // Define the pin for the piezo speaker

// Define the note frequencies (in Hertz)
const int C4 = 262;
const int D4 = 294;
const int E4 = 330;
const int F4 = 349;
const int G4 = 392;
const int A4s = 440;
const int B4 = 494;
const int C5 = 523;
const int D5 = 587;
const int E5 = 659;
const int F5 = 698;
const int G5 = 784;
const int A5s = 880;
const int B5 = 988;
const int C6 = 1047;

// Define an array with the notes to be played
const int melody[] = {A4s, E4};
const int resetTune[] = {A5s, B5};

// Define the duration for each note (in milliseconds)
const int noteDuration = 100;

// Function to play a single note
void playNote(int frequency, int duration) {
  int period = 1000000 / frequency; // Period in microseconds
  int halfPeriod = period / 2;
  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(halfPeriod);
  }
}

// Function to play the melody
void playMelody() {
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
    playNote(melody[i], noteDuration);
    delay(noteDuration); // Wait for the note to finish
  }
  delay(1000); // Pause before repeating the melody
}

void playResetTune() {
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
    playNote(resetTune[i], noteDuration);
    delay(noteDuration); // Wait for the note to finish
  }
  delay(1000); // Pause before repeating the melody
}

const int motorPin = 3;                         // Define the motor control pin

bool reverseSteering = false;
int currentDegree = 90;
int targetDegree = 90;
int turningSpeed = 90/6;
int servoOffset = 0; //this will correct the wheel's steering to make sure it's actually straight
int turningIncrement = 1; //degree change of incremental adjustment
int servoDeltaTime = 5; //time between incremental adjustments, in ms

//bounds for the max and min degrees the servo can turn, middle is 90
int upperBound = 150; //max is 180
int lowerBound = 30; //lowest is 0

int targetSpeed = 0; //use this to increment and decrement
int speedIncrement = 30;

void updateSpeed(int newSpeed)
{
  //speed can be between 0-255
  if (targetSpeed > 255) { targetSpeed = 255; }
  if (targetSpeed < 0) { targetSpeed = 0; }
  
  newSpeed = targetSpeed;

  analogWrite(motorPin, newSpeed);                     // Start the motor with 0/255 speed
}

void translateIR()                              // begin method translateIR() 
                                                // prints the IR remote control's send codes....
{
  switch ( results.value ) {                    // begin switch() 
    
    case 0xFFA25D: Serial.println( "Power ON/OF" );   break;    // print which remote control button has been pressed...
    case 0xFFE21D: Serial.println( "Mute" );          break;
    case 0xFF629D: Serial.println( "Mode" );          break;
    case 0xFF22DD: Serial.println( "Pause" );         break;
    case 0xFF02FD: Serial.println( "Fast Back" );     break;
    case 0xFFC23D: Serial.println( "Fast Forward" );  break;
    case 0xFFE01F: Serial.println( "EQ" );            break;
    case 0xFFA857: Serial.println( "Vol -" );         break;
    case 0xFF906F: Serial.println( "Vol +" );         break;
    case 0xFF9867:
      Serial.println( "Return" );
      targetDegree = 90; // Set the target degree (0 to 180)
      break;
    case 0xFFB04F: Serial.println( "USB Scan" );      break;
    case 0xFF6897: Serial.println( "0" );             break;
    case 0xFF30CF: Serial.println( "1" );             break;
    case 0xFF18E7: Serial.println( "2" );
      targetSpeed += speedIncrement;
      updateSpeed(targetSpeed);
      break;
    case 0xFF7A85: Serial.println( "3" );             break;
    case 0xFF10EF: 
      Serial.println( "4" );
      targetDegree -= turningSpeed; // Set the target degree (0 to 180)
      break;
    case 0xFF38C7: Serial.println( "5" );             break;
    case 0xFF5AA5:
      Serial.println( "6" );
      targetDegree += turningSpeed; // Set the target degree (0 to 180)
      break;
    case 0xFF42BD: Serial.println( "7" );             break;
    case 0xFF4AB5: Serial.println( "8" );
      targetSpeed -= speedIncrement;
      updateSpeed(targetSpeed);
      break;
    case 0xFF52AD: Serial.println( "9" );             break;
    case 0xFFFFFFFF: Serial.println( 
      "You're holding the button down too long..." );    break;
    default:
      Serial.print("Target Speed: ");
      Serial.println(targetSpeed);
      Serial.print("Target Degree: ");
      Serial.println(targetDegree);
      
  }                                             // end switch()
  
  delay( REPEAT_READ_DELAY );                   // delay for the REPEAT_READ_DELAY amount of time #defined above
  
}                                               // end method translateIR()

/****************************************************************************************************************/

void setup() {                                  // begin setup()
  steeringServo.attach(2); // Attach the servo to pin 9
  steeringServo.write(90); // Rotate the servo to the specified angle

  pinMode(speakerPin, OUTPUT); // Set the speaker pin as an output

  pinMode(motorPin, OUTPUT);                    // Initialize the motor pin as an output
  targetSpeed = 0;
  updateSpeed(targetSpeed);

  Serial.begin( 9600 );                         // start the serial communications at a baud rate of 9600
  while ( !Serial );                            // wait for the USB cable to be connected
  Serial.println( 
    "Serial connection established..." );       // report serial connections were established
  pinMode( receiver, INPUT );                   // set the Arduino digital-pin-11 mode to INPUT 
  Serial.println(
    "IR Remote Control Button Decoder");        // print the report title
  irrecv.enableIRIn();                          // enable the IR receiver for input
  
}                                               // end setup()

/****************************************************************************************************************/

void loop() {                                   // begin loop()
  
  analogWrite(motorPin, 50);                     // Start the motor with 5/255 speed

  if ( irrecv.decode( &results ) ) {            // begin if()...if we received an IR signal from the remote control...
    
    translateIR();                              // translate the IR signal, then...
    irrecv.resume();                            // resume receiving the next IR signal from the remote control
    
  }                                             // end if()

  /*
  variable inits from above, just for reference so I don't have to keep scrolling:
    int currentDegree = 90;
    int targetDegree = 90;
    int turningSpeed = 90/3;
    int servoOffset = 0; //this will correct the wheel's steering to make sure it's actually straight
    int turningIncrement = 1; //degree change of incremental adjustment
    int servoDeltaTime = 1; //time between incremental adjustments, in ms
  */

  if (targetDegree > upperBound) { targetDegree = upperBound; playMelody(); }
  if (targetDegree < lowerBound) { targetDegree = lowerBound; playMelody(); }

  if ( targetDegree != currentDegree ) {
    if ( targetDegree > currentDegree )
    {
      //increase current degree
      currentDegree += turningIncrement;
      //wait time increment
      delay(servoDeltaTime);
    }
    if ( targetDegree < currentDegree )
    {
      //decrease current degree
      currentDegree -= turningIncrement;
      //wait time increment
      delay(servoDeltaTime);
    }

    if (currentDegree == 90) {
      playResetTune();
    }

    //update the servo itself
    steeringServo.write(currentDegree + servoOffset); // Rotate the servo to the specified angle
  }
  
}                                               // end loop()

/****************************************************************************************************************/

                                                // end IR_remote_control_decoder_v0.2_FINAL.ino file
