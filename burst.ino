//Template
//For Cascadence
//Developed with Modular Seattle for Velocity 2019

//This file is to give you all the functions you need to start writing your own Cascadence firmware

#include <tinySPI.h>
//You will need to install the TinySPI library Using the 'Manage Libraries' panel
    
//DAC Definitions 
const int GAIN_1 = 0x1;
const int GAIN_2 = 0x0;
const int NO_SHTDWN = 1;
const int SHTDWN = 0;

const int A=0;  //Use A or B to write to DAC
const int B=1;  //This makes it easy to remember which output is which

//pin definitions
const int POTS[4]={0,1,2,3};  //the 4 pots from top to bottom
const int CLK_IN = 8;
const int SW = 7;
//DAC pins
const int MOSI = 6;
const int SCK = 4;
const int PIN_CS = 5;

unsigned int values[4]; //Global array to store potentiometer values

int sensorValue   = 0;
int sensorValueB  = 0;
int sensorValueC  = 0;
int sensorValueD  = 0;
int bursts   = 0;
int space  = 0;
int trigCount = 0;
int trigState = 0;
int lastTrigState = 0;
int reps = 0;
int vol = 0;


void setup()
{


  pinMode(POTS[0], INPUT);  //Set up pins as inputs
  pinMode(POTS[1], INPUT);
  pinMode(POTS[2], INPUT);
  pinMode(POTS[3], INPUT);
  pinMode(SW, INPUT);
  pinMode(CLK_IN,INPUT_PULLUP); //Pullup might not be necessary


//mosi, sck, and pin_cs used for spi dac (mcp4822)
  pinMode(MOSI,OUTPUT);
  pinMode(SCK,OUTPUT);
  
  
  digitalWrite(PIN_CS,HIGH);  //prepare the dac CS line (active low)
  pinMode(PIN_CS, OUTPUT);

  setOutput(A, GAIN_2, NO_SHTDWN, 0); //initialize both DACs to 0
  setOutput(B, GAIN_2, NO_SHTDWN, 0);
  
}


void SendPulse (boolean chan) //Sends a 20ms trigger. This function blocks
{
  setOutput(chan, GAIN_2, NO_SHTDWN, vol);
  delay(bursts);
  setOutput(chan, GAIN_2, NO_SHTDWN, 0);
  
}
void setOutput(byte channel, byte gain, byte shutdown, unsigned int val)
{
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | channel << 7 | gain << 5 | shutdown << 4;
  //bit level stuff to make the dac work.
  //channel can be 0 or 1 (A or B)
  //gain can be 0 or 1 (x1 or x2 corresponding to 2V max or 4V max)
  //shutdown is to disable the DAC output, 0 for shutdown, 1 for no shutdown
   
  digitalWrite(PIN_CS, LOW);
  shiftOut(MOSI,SCK,MSBFIRST,highByte);
  shiftOut(MOSI,SCK,MSBFIRST,lowByte);
  digitalWrite(PIN_CS, HIGH);
}

void loop() {


// read the analog in value:
  sensorValue  = analogRead(0);
  sensorValueB = analogRead(1);
  sensorValueC = analogRead(2);
  sensorValueD = analogRead(3);
// map it to the range of the analog out:
  bursts  = map(sensorValue , 0, 1023, 7, 100);
  space   = map(sensorValueB, 0, 1023, 0, 250);
  reps    = map(sensorValueC, 0, 1023, 1, 16);
  vol     = map(sensorValueD, 0, 1023, 0, 4095);
// read the digital in value:  
  trigState = digitalRead(CLK_IN);

// compare to its previous state
  if (trigState != lastTrigState) {
    
    if (trigState == LOW) {
     
//  if(digitalRead(CLK_IN) == LOW)  //we've received a clock pulse!
    
    for(int i = 0; i < reps; i++){  
    SendPulse(A);
    delay(space);
    }

 
    }
  lastTrigState = trigState;    
  }
}

 
