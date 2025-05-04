/*
Arduino home microclimate station
by Taras Lynnyk & Dmyto Lynnyk
version 1.0
*/

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

#include <LM35.h>

#include <Buzzer.h>

#include <MQUnifiedsensor.h>

/************************Hardware Related Macros************************************/
#define         Board                   ("Arduino UNO")
#define         Pin                     (A2)  //Analog input 3 of your arduino
/***********************Software Related Macros************************************/
#define         Type                    ("MQ-2") //MQ2
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define         RatioMQ2CleanAir        (9.83) //RS / R0 = 9.83 ppm

MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

LiquidCrystal_PCF8574 lcd(0x27);

LM35 tempSensot(0);

Buzzer buzzer(11);

void setupMQ2()
{
  //Set math model to calculate the PPM concentration and the value of constants
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setA(574.25); MQ2.setB(-2.222); // Configure the equation to to calculate LPG concentration
  /*
    Exponential regression:
    Gas    | a      | b
    H2     | 987.99 | -2.162
    LPG    | 574.25 | -2.222
    CO     | 36974  | -3.109
    Alcohol| 3616.1 | -2.675
    Propane| 658.71 | -2.168
  */

  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ2.init(); 
  /* 
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ2.setRL(10);
  */
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
    // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
  // and on clean air (Calibration conditions), setting up R0 value.
  // We recomend executing this routine only on setup in laboratory conditions.
  // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  
  lcd.setCursor(0, 0);
  lcd.print("Calibrating     ");
  lcd.setCursor(0, 1);
  lcd.print("please wait.    ");

  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
  }
  MQ2.setR0(calcR0/10);

  lcd.home();
  lcd.clear();
  /*****************************  MQ CAlibration ********************************************/ 

}

void setup()
{
  
  lcd.begin(16, 2);
  lcd.home();
  lcd.clear();
  lcd.setBacklight(255);

  setupMQ2();

}

void loop()
{
  double temp = tempSensot.getTemp(CELCIUS);

  MQ2.update();
  float ppm = MQ2.readSensor();

  lcd.setCursor(0, 0);
  lcd.print("Temp.: ");
  lcd.print(temp, 1);
  lcd.print(" C  ");
  
  lcd.setCursor(0, 1);
  lcd.print("Air: ");
  lcd.print(ppm, 2);
  lcd.print(" PPM");

  if (ppm > 10)
  {
    lcd.print(" !!   ");
    buzzer.begin(10);
    buzzer.sound(NOTE_A4, 500);
    buzzer.end(10);
  }
  else
  {
    lcd.print(" OK   ");
  }

  delay(500);

}
