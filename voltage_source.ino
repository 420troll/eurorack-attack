


//For Cascadence

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
int outputValue   = 0;
int outputValueB  = 0; 


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

void loop() {

// read the analog in value:
  sensorValue  = analogRead(0);
  sensorValueB = analogRead(1);
  // map it to the range of the analog out:
  outputValue  = map(sensorValue , 0, 1023, 0, 4095);
  outputValueB = map(sensorValueB, 0, 1023, 0, 4095);

  setOutput(A, GAIN_2, NO_SHTDWN, outputValue);
  setOutput(B, GAIN_2, NO_SHTDWN, outputValueB);

 
  updatevalues();      //subroutine to grab the 4 pot values

}


void updatevalues(void)
{
  unsigned char x;
   for(x=0;x<4;x++)
  {
    values[x]=analogRead(POTS[x]);  
  }

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
