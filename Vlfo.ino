


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

//DAC pins
const int MOSI = 6;
const int SCK = 4;
const int PIN_CS = 5;


//pin definitions
const int POTS[4]={0,1,2,3};  //the 4 pots from top to bottom
//const int CLK_IN = 8;
//const int SW = 7;
int LED = 8;

unsigned int values[4]; //Global array to store potentiometer values
 
int sine[120] = {


    0x0, 0x2, 0xb, 0x19, 0x2c, 0x45, 0x64, 0x87, 0xb1, 0xdf,
    0x112, 0x14a, 0x187, 0x1c8, 0x20d, 0x257, 0x2a5, 0x2f6, 0x34c, 0x3a4,
    0x3ff, 0x45d, 0x4be, 0x521, 0x586, 0x5ed, 0x655, 0x6bf, 0x729, 0x794,
    0x7ff, 0x86a, 0x8d5, 0x93f, 0x9a9, 0xa11, 0xa78, 0xadd, 0xb40, 0xba1,
    0xbff, 0xc5a, 0xcb2, 0xd08, 0xd59, 0xda7, 0xdf1, 0xe36, 0xe77, 0xeb4,
    0xeec, 0xf1f, 0xf4d, 0xf77, 0xf9a, 0xfb9, 0xfd2, 0xfe5, 0xff3, 0xffc,
    0xfff, 0xffc, 0xff3, 0xfe5, 0xfd2, 0xfb9, 0xf9a, 0xf77, 0xf4d, 0xf1f,
    0xeec, 0xeb4, 0xe77, 0xe36, 0xdf1, 0xda7, 0xd59, 0xd08, 0xcb2, 0xc5a,
    0xbff, 0xba1, 0xb40, 0xadd, 0xa78, 0xa11, 0x9a9, 0x93f, 0x8d5, 0x86a,
    0x7ff, 0x794, 0x729, 0x6bf, 0x655, 0x5ed, 0x586, 0x521, 0x4be, 0x45d,
    0x3ff, 0x3a4, 0x34c, 0x2f6, 0x2a5, 0x257, 0x20d, 0x1c8, 0x187, 0x14a,
    0x112, 0xdf, 0xb1, 0x87, 0x64, 0x45, 0x2c, 0x19, 0xb, 0x2};
    

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

  int  i = 0;
  int sensorValue   = 0;
  int sensorValueB  = 0;
  int sensorValueC  = 0;
  int rate   = 0;
  int ending = 0;
  int start  = 0;                   


void setup(){

 
  pinMode(POTS[0], INPUT);  //Set up pins as inputs
  pinMode(POTS[1], INPUT);
  pinMode(POTS[2], INPUT);
  pinMode(POTS[3], INPUT);
  
  pinMode(LED, OUTPUT);



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
  sensorValueC = analogRead(2);
  // map it to the range:
  rate   = map(sensorValue , 0, 1024, 100, 0);
  start  = map(sensorValueB , 0, 1024, 0, 119);
  ending = map(sensorValueC, 0, 1024, 5, 120);
  
  

 
  i++;
  
  if(i == ending) i = start;  // Reset the counter to repeat the wave
  if(i == 120) i = 1;  
  if(i < 25) {digitalWrite(LED, HIGH);}
  else {digitalWrite(LED, LOW);}    
  setOutput(A, GAIN_2, NO_SHTDWN, sine[i] );   
  
  delay(rate);
 
 

}
