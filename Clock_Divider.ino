
//For Cascadence by cctv.fm
//Developed with Modular Seattle for Velocity 2019



#include <tinySPI.h>

//You will need to install the TinySPI library Using the 'Manage Libraries' panel
//DAC Definitions 
const int GAIN_1 = 0x1;
const int GAIN_2 = 0x0;
const int NO_SHTDWN = 1;
const int SHTDWN = 0;

const int A=0;  //Use A or B to write to DAC
const int B=1;  //This makes it easy to remember which output is which

int pulseWidth     = 10;
int pulseWidthB    = 10;

unsigned int count  = 0;
unsigned int countB = 0; 
byte clockPulse     = 0;
byte clockPulseB    = 0;

//pin definitions
const int POT0 = 0;
const int POT1 = 1;
const int POT2 = 2;
const int POT3 = 3;
const int CLK_IN = 8;
const int SW = 7;

//DAC pins
const int MOSI = 6;
const int SCK = 4;
const int PIN_CS = 5;

int switchState = 0;
const int LED_BLUE = 7;

unsigned int potValues[4]; //Global array to store potentiometer values


void setup()
{
  pinMode(POT0, INPUT);  //rate
  pinMode(POT1, INPUT);  //pulseWidth
  pinMode(POT2, INPUT);  //rateB
  pinMode(POT3, INPUT);  //pulseWidthB
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


void loop() 
{
  switchState          = digitalRead(SW);
  byte clockState  = digitalRead(CLK_IN);
  byte clockStateB = digitalRead(CLK_IN);
  int rateValue    = analogRead(POT0);
  int rateValueB   = analogRead(POT2);
  rateValue        = map(rateValue , 0, 1024, 0, 8);
  rateValueB       = map(rateValueB, 0, 1024, 0, 16);
  int pulseLength  = analogRead(POT1);
  int pulseLengthB = analogRead(POT3);
  pulseLength      = map(pulseLength , 0, 1024, 2, 100);
  pulseLengthB     = map(pulseLengthB, 0, 1024, 2, 100);

  
  if (clockState == LOW )
  {
    if (!clockPulse)
    {
      clockPulse = true;
      count++;
    } 
  } else 
  { // clockState == HIGH
    if (clockPulse)
    {
      // input pulse is over
      clockPulse = false;
    }
  }

    if (switchState == LOW) {

  count = 0;
  }
  

  if (count > rateValue)
  { 
    SendPulse(A,pulseLength);            
    count = 0; 
  }


    if (clockStateB == LOW )
  {
    if (!clockPulseB)
    {
      clockPulseB = true;
      countB++;
    } 
  } else 
  { // clockState == HIGH
    if (clockPulseB)
    {
      // input pulse is over
      clockPulseB = false;
    }
  }

    if (switchState == LOW) {

  countB= 0;
  }

  if (countB > rateValueB)
  { 
    SendPulse(B,pulseLengthB);            
    countB = 0; 
  }

   if (switchState == HIGH) {

  countB= 0;
  }
  
}


void SendPulse(boolean chan, char pulseLength) //Sends a  trigger. This function blocks
{
  setOutput(chan, GAIN_2, NO_SHTDWN, 0xFFF);
  delay(pulseLength);
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
