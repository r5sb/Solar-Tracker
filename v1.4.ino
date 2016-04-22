/**********************************************************************************
***********************************************************************************
***************Solar Tracker code for my Third Year Engineering Mini Project*******
***************Written by Sidharth Makhija on 7-Apr-2016***************************************
***************Code is provided AS-IS and under MIT Licence************************/

//Adding Libraries
#include <Servo.h>
#include <SPI.h>
#include <SD.h>

//Defining Variables and Pin Nos for LDRs, Temp Sensor and LED.
#define LDR1 A2
#define LDR2 A3
#define LDR3 A4
#define LDR4 A5
#define temp A0
#define led 9

const int chipSelect = 10;
String dataString = "";
int left=0;
int right=0;
int up=0;
int down=0;
int diffH=0;
int diffV=0;
int thresh=15;   // set threshold value to 15
int nthresh;
float val5=0;
float battV=0;

//Initializing Servos
Servo servoX;
Servo servoY;
int deg=0;

void setup()
{ 
Serial.begin(9600);
pinMode(led,OUTPUT);
pinMode(chipSelect,OUTPUT);
Serial.println("Setup");

//Assigning servos to pins 5,6. 
servoX.attach(6);
servoY.attach(5);

//Setting Y-axis to 90 
servoY.write(90);
//Stopping X-axis motor
servoX.write(83);
delay(1000);
//Setting Y-axis to 0
servoY.write(deg);
delay(20);

//Calling function to Initialize SD Card
InitializeSD();
dataString = "";//Readings to be written on SD Card go in this String
dataString="Temp and Solar Panel Readings";
//Calling function to write String to SD Card
writeSD(dataString);
Serial.println();
delay(3000); //wait 3 seconds
}

void loop()
{
dataString=""; //Emptying the String
takeReadings();//Calling function that takes readings and writes it to SD Card 
//Total delay = 1.15S (1150mS)

digitalWrite(led,LOW);
  
diffH=left-right; //Taking difference of Right and Left LDRs (X-axis) //-1*(right-left);
diffV=down-up;//Taking Difference of Up and Down LDRs (Y-axis) //-1*(up-down);
nthresh = -1*thresh; //negative threshold

Serial.print("DiffV= ");
Serial.println(diffV);
Serial.print("DiffH= ");
Serial.println(diffH);
Serial.println();

//For Y-axis
if(diffV>thresh) //if value is greater than threshold move up
{
  Serial.println("up");
  servoYup();// Calling function to turn Y-axis Servo up
}
else if(diffV<nthresh) //if value is lesser than negative threshold move down
{
 Serial.println("down"); // Calling function to turn Y-axis Servo down
 servoYdown();
}

//(For X-axis)
if(diffH>thresh) //if value is greater than threshold move right  
  {
   servoXRight(); //Calling Function to turn X-axis Servo right.
   Serial.println("right");
  }
else if(diffH<nthresh) //if value is less than negative threshold move left
  {
    servoXLeft(); //Calling function to turn X-axis Servo left.
    Serial.println("left");
  }
  
else
{
  servoX.write(83);//remain stationary
}
delay(1000);
}


//////////////////////////////FUNCTIONS//////////////////////////////////////////////////////

void servoYup()
{
   deg+=5; // Increment degree by 5
   servoY.write(deg);//Make Servo go to the assigned degree (clockwise)
   if(deg>=115) //If Servo reaches max limit
   {
     servoY.write(115);//keep servo in limit
   }
}

void servoYdown()
{
     deg-=5;//decrease degree by 5
     servoY.write(deg);//Make servo go to assigned degree
     if(deg<5)//If servo reaches minimum limit
     {
       servoY.write(5);//keep servo in limit
         
     }
}

void servoXRight()
{
  servoX.write(70);//Make servo turn to right
  delay(200);//keep on for 200mS
  servoX.write(83);//Stop servo
  delay(500);//keep off for 500mS
}

void servoXLeft()
{
   servoX.write(95);//Make servo turn left
  delay(200);//Keep on for 200mS
  servoX.write(83);//Stop servo
  delay(500);//Keep off for 500mS
}

void takeReadings()
{

digitalWrite(led,HIGH);
//Take Reading from Right LDR
right=map(analogRead(LDR1),0,1023,0,255); //Change range of LDR readings from 0-1023 to 0-255
Serial.print("right= ");
Serial.println(right);
delay(20);

//Take Reading from Left LDR
left=map(analogRead(LDR2),0,1023,0,255);
Serial.print("left= ");
Serial.println(left);
delay(20);

//Take Reading from Up LDR
up=map(analogRead(LDR3),0,1023,0,255);
Serial.print("up= ");
Serial.println(up);
delay(20);

//Take Reading from Down LDR
down=map(analogRead(LDR4),0,1023,0,255);
Serial.print("down= ");
Serial.println(down);
delay(20);
Serial.println();

//Take average reading of all LDR
int avg = (up+down+left+right)/4;
//Take percentage of sunlight available
float perc=(avg/255)*100;

//Add percentage sunlight to String
dataString+=String(perc);
dataString+="%";
dataString+=",";

//Take Temperature reading
val5=(5.0*analogRead(temp)*100.0)/1024;
delay(10);
val5=(5.0*analogRead(temp)*100.0)/1024;
delay(10);

//Add temperature to String
dataString+=String(val5);
dataString+=",";
Serial.print("Temp= ");
Serial.println(val5);
delay(20);

//Call Function to check voltage from solar panel
battV=batteryCheck();
delay(10);
battV=batteryCheck();

//Add solar panel Voltage to String
dataString+=String(battV);
Serial.print("Battery= ");
Serial.println(battV);
delay(20);

//Write all Data to the SD Card
writeSD(dataString);
delay(1000);
}

float batteryCheck()// Function to check voltage of Solar Panel
{
  //(val / resistorFactor) * referenceVolts*4; // calculate the ratio
  
  const float referenceVolts = 5;  // the default reference on a 5-volt board
//const float referenceVolts = 3.3;  // use this for a 3.3-volt board

const float R1 = 1000;//955 // value for a maximum voltage of 10 volts
const float R2 = 1000;//955
// determine by voltage divider resistors, see text
const float resistorFactor = 1023.0 / (R2/(R1 + R2));  
const int batteryPin = A1; // +V from battery is connected to analog pin 0
int val1 = analogRead(batteryPin);
delay(10);
int val2=analogRead(batteryPin);
float volts1 = val1*(5.0/1023.0)*2;
float volts2 = val2*(5.0/1023.0)*2;
if(volts1==volts2)
  {
    return volts1;
  }
else
  {
    volts2=volts2-volts1;
    volts1=volts1-volts2;
    return volts1;
  }
}

void InitializeSD()
{
   Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

void writeSD(String a)
{
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) 
  {
    dataFile.println(a);
    dataFile.close();
    // print to the serial port too:
    Serial.println(a);
  }
  // if the file isn't open, pop up an error:
  else 
  {
    Serial.println("error opening datalog.txt");
  }
}




