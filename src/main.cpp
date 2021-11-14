/*
 * Displays text sent over the serial port (e.g. from the Serial Monitor) on
 * an attached LCD.
 * YWROBOT
 *Compatible with the Arduino IDE 1.0
 *Library version:1.1
 */
#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <avr/interrupt.h>

#define DETECT_TRESHOLD 10
#define ADC_DATA_SIZE 200

enum STATE:uint8_t{
  IDLE,
  IDLE_ADC_READY,
  COIN_DETECTED,
  CALCULATE,
  UPDATE,
  END
};

STATE state = IDLE;

float coin_dim[5] = {13.4, 15.4,882,916,957};
float coin_nom[5] = {1,2,5,0.05,1};
float coins_sum = 0;

struct EEPROM_DATA{
  uint8_t notNew;
  float coins_sum;
};

EEPROM_DATA local_data;

float normal_val=0;
uint8_t adc_count = 0;
uint16_t adc_new = 0;
uint16_t adc_last = 0;
uint16_t adc_data [ADC_DATA_SIZE];

uint16_t coins_diam[]= {0,0};

extern "C" {
  #include "buttons.h"
}

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int lookforcoin(float);
uint16_t largest(uint16_t*, uint8_t);
void ADC_Init();

void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  /*
  EEPROM.get(0,local_data);
  if(local_data.notNew == 0){
    local_data.notNew = 1;
  }
  */

  Serial.begin(115200);

    lcd.clear();
      lcd.print("Value: ");
      lcd.setCursor(0,1);
      lcd.print("Total: ");
      lcd.print(coins_sum);

}


void loop()
{

  for (int i = 0; i < 10; i++)
  {
    normal_val += (float)analogRead(0);
  }
  normal_val /= 10.0;
  
  adc_new = analogRead(0);
  adc_last = adc_new;
  state = IDLE_ADC_READY;

  while(state == IDLE_ADC_READY){
    adc_new = analogRead(0);
    if( adc_new > normal_val + 3){
      state = COIN_DETECTED;
      Serial.print("coin!!!");

      int max_num=analogRead(0);
      int new_max=max_num;
      while(state==COIN_DETECTED){
        new_max = analogRead(0);
        if(max_num < new_max )
        {
            max_num = new_max;
        }
        if (max_num - new_max > 5){
          state = UPDATE;
        } 
      }
      
      Serial.print("max:");
      Serial.println(max_num-normal_val);

      int coin = lookforcoin((float)max_num-normal_val);
      float coin_nominal = coin_nom[coin];
      Serial.print(coin);
      Serial.print("--");
      Serial.println(coin_nominal);

      lcd.clear();
      lcd.print("Value: ");
      lcd.println(coin_nominal);
      lcd.setCursor(0,1);
      lcd.print("Total: ");
      coins_sum += coin_nominal;
      lcd.print(coins_sum);

    }
    adc_last = adc_new;
  }
  delay(500);
}

int lookforcoin(float diam){
  for(int i=0;i<5; i++){
    if(fabs(diam - (float)coin_dim[i])<1.5*(i+1)){
      return i;
    }
  }
}



