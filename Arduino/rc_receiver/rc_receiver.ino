#include <nRF24L01.h> //https://github.com/maniacbug/RF24
#include <RF24.h> //https://github.com/maniacbug/RF24
#include <SPI.h>
#include <elapsedMillis.h>
#include "Servo.h"

#define DEBUGMODE //Comment this out to turn off debug messages and use of serial

#ifdef DEBUGMODE
#include <printf.h>     //uncomment for debug info
#endif

//Millisconds that can occur between received packets before an idle signal is sent to ESC
const int TIMEOUT_AND_IDLE = 1000;

//Microseconds to pulse high for pwm signal to ESC
const int ESC_MIN = 1500;
const int ESC_MAX = 2000;
const int ESC_IDLE = 1750;

#ifdef DEBUGMODE
//Just used to give some basic live debug info, stays high when getting bad signals
const int LED1 = 13;
#endif

//Used to write PWM signals to the ESC (has writeMicroseconds method for setting a pwm signal)
Servo myServo; 

//Used to keep track of how long it has been since receiving a signal, sends idle on servo if greater than 1 second
elapsedMillis timeElapsed;

//nrf module related constants and variables below
//Chip enable pin
const int CE_PIN = 9;
//Chip select pin
const int CS_PIN = 10;

// Radio object for receiving data
RF24 radio(CE_PIN, CS_PIN);

//Pipe matches pipe in transmitter
const uint64_t PIPE = 0xE8E8F0F0E1LL;
uint8_t PIPE_NUM =1;

// Buffer for reading in the payload sent by the transmitter/controller
int msg[1]; 


void setup(void){
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  radio.openReadingPipe(PIPE_NUM,PIPE);
  radio.startListening();
  pinMode(5, OUTPUT);

  myServo.attach(5);
  myServo.writeMicroseconds(ESC_IDLE);
  
  #ifdef DEBUGMODE
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  printf_begin();
  #endif
}
void loop(void){
  
  if (radio.available(&PIPE_NUM)){
  
    #ifdef DEBUGMODE 
    radio.printDetails();
    #endif
    
    bool done = false;
    while (!done){
      done = radio.read(msg, 1);
      if (msg[0] <= 0 || msg[0] > 254){
        #ifdef DEBUGMODE
        Serial.println("Bad signal received");
        digitalWrite(LED1, HIGH);
        #endif
        
      } else {
        // Translates value sent over wifi to microseconds in range for ESC
        int msgInMicroSeconds = map(msg[0],1,254, ESC_MIN, ESC_MAX);
        myServo.writeMicroseconds(msgInMicroSeconds);
        timeElapsed = 0;
        
        #ifdef DEBUGMODE
        digitalWrite(LED1, LOW);
        Serial.println(msg[0]);
        #endif
      }
    }
  }
  // If no message was received and a second has passed put the ESC in neutral
  if(timeElapsed > TIMEOUT_AND_IDLE){
    
    myServo.writeMicroseconds(ESC_IDLE);
    
    #ifdef DEBUGMODE
    Serial.println("frack, we've lost signal captain");
    radio.printDetails();
    #endif
  }
  delay(10);
}
