/*
  CCS811 Air Quality Sensor Example Code
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 7th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Reads the baseline number from the CCS811. You are supposed to gather this value
  when the sensor is in normalized clean air and use it to adjust (calibrate) the sensor 
  over the life of the sensor as it changes due to age.

  Hardware Connections (Breakoutboard to Arduino):
  3.3V = 3.3V
  GND = GND
  SDA = A4
  SCL = A5
  WAKE = D5 - Optional, can be left unconnected

  Serial.print it out at 9600 baud to serial monitor.
*/

#include <Wire.h>

#define CCS811_ADDR 0x5B //7-bit unshifted default I2C Address

#define WAKE 13 //!Wake on breakout connected to pin 5 on Arduino

//Register addresses
#define CSS811_STATUS 0x00
#define CSS811_MEAS_MODE 0x01
#define CSS811_ALG_RESULT_DATA 0x02
#define CSS811_RAW_DATA 0x03
#define CSS811_ENV_DATA 0x05
#define CSS811_NTC 0x06
#define CSS811_THRESHOLDS 0x10
#define CSS811_BASELINE 0x11
#define CSS811_HW_ID 0x20
#define CSS811_HW_VERSION 0x21
#define CSS811_FW_BOOT_VERSION 0x23
#define CSS811_FW_APP_VERSION 0x24
#define CSS811_ERROR_ID 0xE0
#define CSS811_APP_START 0xF4
#define CSS811_SW_RESET 0xFF

//These are the air quality values obtained from the sensor
unsigned int tVOC = 0;
unsigned int CO2 = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("CCS811 Read Example");

  pinMode(WAKE, OUTPUT);
  digitalWrite(WAKE, LOW);

  Wire.begin();

  configureCCS811(); //Turn on sensor

  unsigned int result = getBaseline();
  Serial.print("baseline for this sensor: 0x");
  Serial.println(result, HEX);
}

void loop()
{
  Serial.println("Done");
  delay(1000);
}

//Turns on the sensor and configures it with default settings
void configureCCS811()
{
  //Verify the hardware ID is what we expect
  byte hwID = readRegister(0x20); //Hardware ID should be 0x81
  if (hwID != 0x81)
  {
    Serial.println("CCS811 not found. Please check wiring.");
    while (1); //Freeze!
  }

  //Check for errors
  if (checkForError() == true)
  {
    Serial.println("Error at Startup");
    printError();
    while (1); //Freeze!
  }

  //Tell App to Start
  if (appValid() == false)
  {
    Serial.println("Error: App not valid.");
    while (1); //Freeze!
  }

  //Write to this register to start app
  Wire.beginTransmission(CCS811_ADDR);
  Wire.write(CSS811_APP_START);
  Wire.endTransmission();

  //Check for errors
  if (checkForError() == true)
  {
    Serial.println("Error at AppStart");
    printError();
    while (1); //Freeze!
  }

  //Set Drive Mode
  setDriveMode(1); //Read every second

  //Check for errors
  if (checkForError() == true)
  {
    Serial.println("Error at setDriveMode");
    printError();
    while (1); //Freeze!
  }
}

//Checks to see if error bit is set
boolean checkForError()
{
  byte value = readRegister(CSS811_STATUS);
  return (value & 1 << 0);
}

//Displays the type of error
//Calling this causes reading the contents of the ERROR register
//This should clear the ERROR_ID register
void printError()
{
  byte error = readRegister(CSS811_ERROR_ID);

  Serial.print("Error: ");
  if (error & 1 << 5) Serial.print("HeaterSupply ");
  if (error & 1 << 4) Serial.print("HeaterFault ");
  if (error & 1 << 3) Serial.print("MaxResistance ");
  if (error & 1 << 2) Serial.print("MeasModeInvalid ");
  if (error & 1 << 1) Serial.print("ReadRegInvalid ");
  if (error & 1 << 0) Serial.print("MsgInvalid ");

  Serial.println();
}

//Returns the baseline value
//Used for telling sensor what 'clean' air is
//You must put the sensor in clean air and record this value
unsigned int getBaseline()
{
  Wire.beginTransmission(CCS811_ADDR);
  Wire.write(CSS811_BASELINE);
  Wire.endTransmission();

  Wire.requestFrom(CCS811_ADDR, 2); //Get two bytes

  byte baselineMSB = Wire.read();
  byte baselineLSB = Wire.read();

  unsigned int baseline = ((unsigned int)baselineMSB << 8) | baselineLSB;
  return (baseline);
}

//Checks to see if DATA_READ flag is set in the status register
boolean dataAvailable()
{
  byte value = readRegister(CSS811_STATUS);
  return (value & 1 << 3);
}

//Checks to see if APP_VALID flag is set in the status register
boolean appValid()
{
  byte value = readRegister(CSS811_STATUS);
  return (value & 1 << 4);
}

//Mode 0 = Idle
//Mode 1 = read every 1s
//Mode 2 = every 10s
//Mode 3 = every 60s
//Mode 4 = RAW mode
void setDriveMode(byte mode)
{
  if (mode > 4) mode = 4; //Error correction

  byte setting = readRegister(CSS811_MEAS_MODE); //Read what's currently there

  setting &= ~(0b00000111 << 4); //Clear DRIVE_MODE bits
  setting |= (mode << 4); //Mask in mode
  writeRegister(CSS811_MEAS_MODE, setting);
}

//Reads from a give location from the CSS811
byte readRegister(byte addr)
{
  Wire.beginTransmission(CCS811_ADDR);
  Wire.write(addr);
  Wire.endTransmission();

  Wire.requestFrom(CCS811_ADDR, 1);

  return (Wire.read());
}

//Write a value to a spot in the CCS811
void writeRegister(byte addr, byte val)
{
  Wire.beginTransmission(CCS811_ADDR);
  Wire.write(addr);
  Wire.write(val);
  Wire.endTransmission();
}
