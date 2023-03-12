#include <Wire.h>
#include "MAX30105.h"
#include <LiquidCrystal_I2C.h>
#include "heartRate.h"
#include <Keypad.h>
LiquidCrystal_I2C lcd(0x26,16,2);
MAX30105 particleSensor;

const byte rows = 4; //so' hang' 
const byte columns = 4; //so' cot.
byte rowPins[rows] = {9, 8, 7, 6}; //các chân nối với arduino
byte columnPins[columns] = {5, 4, 3, 2};
char key = 0;
int vitri=10;
int xoa=0;
bool trangthai=false;
char stt_arr[4];
int mang=0;
//bảng keypad các giá trị
char keys[rows][columns] =
{ 
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}, //* :"OK", #: "Clr"
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
 long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;
void nhaplieu(){
  lcd.setCursor(0,0);
  lcd.print("Nhap stt:");
  lcd.setCursor(0,1);
  lcd.print("[Ok]");
  lcd.setCursor(8,1);
  lcd.print("[Clr]");
}

float Z;
float SpO2;


void nhapthanhcong(){
    Serial.print("SpO2=");
    Serial.print(SpO2);
    Serial.print("%");
    Serial.print(";  ");
    Serial.print("Avg BPM=");     
    Serial.println(beatAvg);
  lcd.setCursor(0,0);
  lcd.print("stt:");
  lcd.setCursor(4,0);
  lcd.print(stt_arr);
  lcd.setCursor(9,0);
  lcd.print("BMP:");
  lcd.setCursor(0,1);
  lcd.print("SpO2:");
  lcd.setCursor(14,0);
  lcd.print(beatAvg);
  lcd.setCursor(6,1);
  lcd.print(SpO2);
  
}

void setup()
{ 
  Serial.begin(115200);
  Serial.println("waitting ...");
  lcd.init();
  lcd.backlight();

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("Khong tim thay MAX30102... ");
    while (1);
  }
  Serial.println("Dat ngon tay vao");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop()
{
  //=========BAN PHIM===========================//
 char key = keypad.getKey();

  if (key) 
  {
    //Xuất kết quả trên máy tính
    if(vitri<17 && key!='*' && key!='#' && key!='D')
    {
      lcd.setCursor(vitri,0);
      lcd.print(key);
      vitri++;
      if(mang<5)
      {
        stt_arr[mang]=key;
        mang++;
      }
    }
    if(key=='*')
    { // qua trang hiện thị thông tin 
      trangthai=true;
      lcd.clear();
    }
    if(key=='#')
    {
      vitri--;
      lcd.setCursor(vitri,0);
      mang--;
      lcd.print(' ');
    }
    if(key=='D')
    { // trở về trang đầu
      trangthai=false;
      resetBoard();
    }
 }
  /////////////////////////////////////////
   long irValue = particleSensor.getIR();
   long rValue = particleSensor.getRed();

/*  Serial.print("IR=");
  Serial.println(irValue);
  Serial.print("R=");
  Serial.println(rValue);*/
  //========= Đăng nhập ====================//
    if (trangthai==false) 
    {
      nhaplieu();
    }
    else
    {
      if (irValue < 50000)
      { 
          Serial.print(" No finger?");
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("No finger?");
          delay(1000);
          lcd.clear();
      }
      else 
      { 
          if (checkForBeat(irValue) == true)
          {
            //We sensed a beat!
             long delta = millis() - lastBeat;
            lastBeat = millis();
            beatsPerMinute = 60 / (delta / 1000.0);
        
            if (beatsPerMinute < 255 && beatsPerMinute > 20)
            {
              if (irValue > 50000)
              {
                 float Z=(float(rValue))/(float(irValue));      
                 SpO2= Z*(-46.05*Z+30.354) + 94.845;
                    rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
                    //rateSpot %= RATE_SIZE; //Wrap variable   
                    //Take average of readings
                    beatAvg = 0;
                    for (byte x = 0 ; x < RATE_SIZE ; x++)
                      beatAvg += rates[x];
                    beatAvg /= RATE_SIZE;
              }
              else
              {
                 Z=0;
                 SpO2= 0;
                 beatAvg =0;
              }
            }
          }
        nhapthanhcong();
      }
   }
    //========================================//
  Serial.println();
}

void resetBoard()
{
  asm volatile ("jmp 0");
}
