#include <SPI.h>
#include "nRF24L01.h" //https://github.com/maniacbug/RF24
#include "RF24.h"     //https://github.com/maniacbug/RF24

#define DEBUGMODE //Comment this out to turn off debug messages and use of serial

#ifdef DEBUGMODE
  #include "printf.h"
  //  Note if the printf.h is not found copy it from the RF24 examples in the ping pair demo
  //  there is a printf.h, copy to the RF24 folder in your libraries and restart arduino.
#endif
 
//These values are used to keep track of bounds of the analog input
//the values are translated into a range of 1-255 to transmit to the
//receiver, on that side it will map the value to 1500-2000 micro seconds
//for the ESC

//*** ADJUST THESE FOR YOUR CONTROLLER ***//
int max = 488;
int min = 342;

// Everything below here can remain the same assuming you use analog pin 1 and standard pin layout for the rf module
// https://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo See "Arduino pin for RF24 Library"

int msg[1];                             //Stores 1 byte message payload
RF24 radio(9, 10);                      //Creates the radio object for config and tranmission
const uint64_t pipe = 0xE8E8F0F0E1LL;   //ID for the connection
int SW1 = 7;

int sensorPin = A1;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor


void setup() {
  pinMode(A1, INPUT);

  //Start up and setup the radio configuration
  //TODO: there are lots of forks of this library see which one appears most active consider switching.
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);    //Set power level, lower consumes less power but is more easily obstructed
  radio.openWritingPipe(pipe);      //Controller only transmits for the time being so we open a connection to send data
  radio.setDataRate(RF24_250KBPS);  //Using a relatively low data rate, should have a bit better reliability and is sufficient for small payload
  radio.setChannel(108);            //Uses the last possible channel on the nrf module


#ifdef DEBUGMODE
   Serial.begin(9600);  //uncomment for debugging
   printf_begin();      //uncomment for debugging
#endif
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);

  //Map value from 1 to 255 (skipping 0 since receiver seems to see that as bad data), was 10-bit from analog read mapping to 8-bit
  //to fit in the payload as a single byte.
  int mappedValue = map(sensorValue, min, max, 1, 255);
  if (mappedValue < 1)
    mappedValue = 1;
  if (mappedValue > 255)
    mappedValue = 255;

#ifdef DEBUGMODE
  radio.printDetails();
  Serial.println(mappedValue);
#endif

  msg[0] = mappedValue;
  radio.write(msg, 1); //Message is in the bottle
}
