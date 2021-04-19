#include <LiquidCrystal.h>
#include <Servo.h>
#include "pitches.h"
#define PLAYER_WAIT_TIME 3000 // The time allowed between button presses - 2s
// Define RGB LED Pins
#define BLUE 8
#define GREEN 7
#define RED 6
#define BLUE2 12
#define GREEN2 10
#define RED2 9
LiquidCrystal lcd(42, 44, 46, 48, 50, 52); // initialize the library with the numbers of the interface pins
Servo myservo;
byte sequence[100];           // Storage for the light sequence
byte curLen = 0;              // Current length of the sequence
byte winLen = 10;              // Number of successful rounds before winning the game
byte inputCount = 0;          // The number of times that the player has pressed a (correct) button in a given turn 
byte lastInput = 0;           // Last input from the player
byte expRd = 0;               // The LED that's suppose to be lit by the player
bool btnDwn = false;          // Used to check if a button is pressed
bool wait = false;            // Is the program waiting for the user to press a button
bool resetFlag = false;       // Used to indicate to the program that once the player lost
byte soundPin = 11;            // Speaker output
byte noPins = 5;              // Number of buttons/LEDs (While working on this, I was using only 2 LEDs)
byte pins[] = {22, 24, 26, 28, 30}; // Button input pins and LED ouput pins - change these vaules if you want to connect your buttons to other pins                              
long inputTime = 0;           // Timer variable for the delay between user inputs
bool candy = true;
int data = 3;//7-segment LED display data pin
int latch = 4;//7-segment LED display latch pin
int clock = 5;//7-segment LED display clock pin
// define RGB variables
int redValue;
int greenValue;
int blueValue;

// This is for the 7-segment LED display
unsigned char table[]=
{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c
,0x39,0x5e,0x79,0x71,0x00};

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

void setup() {
  pinMode(latch,OUTPUT);
  pinMode(clock,OUTPUT);
  pinMode(data,OUTPUT);
  delay(1000);                // This is to give me time to breathe after connection the arduino - can be removed if you want
  //Serial.begin(9600);         // Start Serial monitor. This can be removed too as long as you remove all references to Serial below
  lcd.begin(16, 2);// set up the LCD's number of columns and rows:
  myservo.attach(2);// Pin connected to the swervo
  // Setup RGB LEDs
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, HIGH);
  pinMode(RED2, OUTPUT);
  pinMode(GREEN2, OUTPUT);
  pinMode(BLUE2, OUTPUT);
  digitalWrite(RED2, LOW);
  digitalWrite(GREEN2, LOW);
  digitalWrite(BLUE2, HIGH);
  //Run the reset thingy right off the bat
  Reset();
}

///
/// Sets all the pins as either INPUT or OUTPUT based on the value of 'dir'
///
void setPinDirection(byte dir){
  for(byte i = 0; i < noPins; i++){
    pinMode(pins[i], dir); 
  }
}

//Display rounds that have been won
void Display(unsigned char num)
{

  digitalWrite(latch,LOW);
  shiftOut(data,clock,MSBFIRST,table[num]);
  digitalWrite(latch,HIGH);
  
}

//send the same value to all the LED pins
void writeAllPins(byte val){
  for(byte i = 0; i < noPins; i++){
    digitalWrite(pins[i], val); 
  }
}

//Makes a (very annoying :) beep sound
void beep(byte freq){
  analogWrite(soundPin, 2);
  delay(freq);
  analogWrite(soundPin, 0);
  delay(freq);
}

///
/// Flashes all the LEDs together
/// freq is the blink speed - small number -> fast | big number -> slow
///
void flash(short freq){
  lcd.setCursor(0, 1);
  setPinDirection(OUTPUT); /// We're activating the LEDS now
  for(int i = 0; i < 5; i++){
    lcd.print("*");
    // Also flash RGB LEDs red
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);
    digitalWrite(RED2, HIGH);
    digitalWrite(GREEN2, LOW);
    digitalWrite(BLUE2, LOW);
    writeAllPins(HIGH);
    beep(50);
    delay(freq);
    digitalWrite(RED, LOW);
    digitalWrite(RED2, LOW);
    writeAllPins(LOW);
    delay(freq);
  }
}

void flash(short freq, int countDn){
  lcd.setCursor(0, 1);
  int printCt = countDn;
  setPinDirection(OUTPUT); /// We're activating the LEDS now
  for(int i = 0; i <= countDn; i++){
    lcd.print(printCt);
    // Also flash RGB LEDs red
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);
    digitalWrite(RED2, HIGH);
    digitalWrite(GREEN2, LOW);
    digitalWrite(BLUE2, LOW);
    writeAllPins(HIGH);
    beep(50);
    delay(freq);
    digitalWrite(RED, LOW);
    digitalWrite(RED2, LOW);
    writeAllPins(LOW);
    printCt--;
    lcd.setCursor(0, 1);
    delay(freq);
  }
}
///
///This function resets all the game variables to their default values
///
void Reset(){
  Display(0);
  lcd.clear();
  lcd.print("GET READY!!");// Print a message to the LCD.
  myservo.write(116);// Move servo to home position.
  //Reset RGB LEDs back to start color
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, HIGH);
  digitalWrite(RED2, LOW);
  digitalWrite(GREEN2, LOW);
  digitalWrite(BLUE2, HIGH);
  flash(500, 5);
  curLen = 0;
  inputCount = 0;
  lastInput = 0;
  expRd = 0;
  btnDwn = false;
  wait = false;
  resetFlag = false;
  candy = true;
}

///
/// User lost
///
void Lose(){
  flash(50);  
}

///
/// The arduino shows the user what must be memorized
/// Also called after losing to show you what you last sequence was
///
void playSequence(){
  //Loop through the stored sequence and light the appropriate LEDs in turn
  for(int i = 0; i < curLen; i++){
      //Serial.print("Seq: ");
      //Serial.print(i);
      //Serial.print("Pin: ");
      //Serial.println(sequence[i]);
      digitalWrite(sequence[i], HIGH);
      delay(500);
      digitalWrite(sequence[i], LOW);
      delay(250);
    } 
}

///
/// The events that occur upon a loss
///
void DoLoseProcess(){
  lcd.clear();
  lcd.print("YOU LOSE! BUMMER");
  tone(11, NOTE_B0, 1100);
  Lose();             // Flash all the LEDS quickly (see Lose function)
  delay(1000);
  playSequence();     // Shows the user the last sequence - So you can count remember your best score - Mine's 22 by the way :)
  delay(1000);
  Reset();            // Reset everything for a new game
}

///
/// Where the magic happens
///
void loop() {
  if(!wait){      
                            //****************//
                            // Arduino's turn //
                            //****************//
    lcd.clear();
    lcd.print("PAY ATTENTION!");
    // Change RGB led to red
    delay(1000);
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);
    digitalWrite(RED2, HIGH);
    digitalWrite(GREEN2, LOW);
    digitalWrite(BLUE2, LOW);
    setPinDirection(OUTPUT);                      // We're using the LEDs
    randomSeed(analogRead(A0));                   // https://www.arduino.cc/en/Reference/RandomSeed
    sequence[curLen] = pins[random(0,noPins)];    // Put a new random value in the next position in the sequence -  https://www.arduino.cc/en/Reference/random
    curLen++;                                     // Set the new Current length of the sequence
    playSequence();                               // Show the sequence to the player
    //beep(50);                                     // Make a beep for the player to be aware
    tone(11, NOTE_C4, 200);
    delay(100);
    tone(11, NOTE_E4, 200);
    delay(100);
    tone(11, NOTE_G4, 200);
    delay(100);
    noTone(11);
    wait = true;                                  // Set Wait to true as it's now going to be the turn of the player
    inputTime = millis();                         // Store the time to measure the player's response time
  
  }else if(curLen <= winLen){ 
                            //***************//
                            // Player's turn //
                            //***************//
    //Turn RGB LED green
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
    digitalWrite(BLUE, LOW);
    digitalWrite(RED2, LOW);
    digitalWrite(GREEN2, HIGH);
    digitalWrite(BLUE2, LOW);

    setPinDirection(INPUT);                       // We're using the buttons

    if(millis() - inputTime > PLAYER_WAIT_TIME){  // If the player takes more than the allowed time,
      DoLoseProcess();                            // All is lost :(
      return;
    }      
        
    if(!btnDwn){                                  // 
      expRd = sequence[inputCount];               // Find the value we expect from the player
      //Serial.print("Expected: ");                 // Serial Monitor Output - Should be removed if you removed the Serial.begin above
      //Serial.println(expRd);                      // Serial Monitor Output - Should be removed if you removed the Serial.begin above
      
      for(int i = 0; i < noPins; i++){           // Loop through the all the pins
        if(pins[i]==expRd)                        
          continue;                               // Ignore the correct pin
        if(digitalRead(pins[i]) == HIGH){         // Is the button pressed
          lastInput = pins[i];
          resetFlag = true;                       // Set the resetFlag - this means you lost
          btnDwn = true;                          // This will prevent the program from doing the same thing over and over again
          //Serial.print("Read: ");                 // Serial Monitor Output - Should be removed if you removed the Serial.begin above
          //Serial.println(lastInput);              // Serial Monitor Output - Should be removed if you removed the Serial.begin above
        }
      }      
    }

    if(digitalRead(expRd) == 1 && !btnDwn)        // The player pressed the right button
    {
      inputTime = millis();                       // 
      lastInput = expRd;
      inputCount++;                               // The user pressed a (correct) button again
      btnDwn = true;                              // This will prevent the program from doing the same thing over and over again
      //Serial.print("Read: ");                     // Serial Monitor Output - Should be removed if you removed the Serial.begin above
      //Serial.println(lastInput);                  // Serial Monitor Output - Should be removed if you removed the Serial.begin above
    }else{
      if(btnDwn && digitalRead(lastInput) == LOW){  // Check if the player released the button
        btnDwn = false;
        delay(20);
        if(resetFlag){                              // If this was set to true up above, you lost
          DoLoseProcess();                          // So we do the losing sequence of events
        }
        else{
          if(inputCount == curLen){                 // Has the player finished repeating the sequence
            wait = false;                           // If so, this will make the next turn the program's turn
            inputCount = 0;                         // Reset the number of times that the player has pressed a button
            Display(char(curLen));
            delay(1500);
            if(curLen == winLen){
                curLen++;
                wait = true;
            }
            }
          }
        }
      }
    }else{
    // Print a winner message on lcd.
    lcd.clear();
    lcd.print("WINNER WINNER!!!");
    lcd.setCursor(0, 1);
    lcd.print("CHICKEN DINNER!!");
    // Play a jolly good tune for the winner.
    for (int thisNote = 0; thisNote < 8; thisNote++) {
      // to calculate the note duration, take one second divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(11, melody[thisNote], noteDuration);
      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      // stop the tone playing:
      noTone(11);
    }

    // Dispense candy!!
    if(candy){
      int servoDelay = 200; //Delay between swervo moves, lower numbers shoots out candy faster but may jam more.
      int candyCount = 5; //Number of candys to dispense on win
      for(int i = 0; i < candyCount; i ++) { // 
      myservo.write(100);
      delay(servoDelay);
      myservo.write(116);
      delay(servoDelay);  
      }
      candy = false;
    }
    delay(500);
    lcd.clear();
    lcd.print("RESTARTING IN:");
    flash(250, 9);
    Reset();
  }
}
