#include <Arduino.h>
#include <EEPROM.h>
#include "AiEsp32RotaryEncoder.h"
#include <EMUSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "menuItem.h"

// SCREEN
#define TFT_MOSI 23  // SDA Pin on ESP32
#define TFT_SCLK 18  // SCL Pin on ESP32
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

//ROTARY  ENCODER
#define ROTARY_ENCODER_A_PIN 14
#define ROTARY_ENCODER_B_PIN 12
#define ROTARY_ENCODER_BUTTON_PIN 13
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4

unsigned long previousScreenMillis = 0;
unsigned long previousMillis = 0;
const long screenRefreshRate = 250; // screen refresh rate
const long debounce = 50; 

int lastEncoderPos = 0;
int buttonState = 0;


// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Initialize AiEsp32RotaryEncoder Library
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

EMUSerial emu(Serial1);


void emuRpm()
{
  tft.print(emu.emu_data.RPM);
}

void emuBatt()
{
  tft.print(emu.emu_data.Batt);
  tft.print(" V");
}

void emuIat()
{
  tft.print(emu.emu_data.IAT);
  tft.print(" C");
}

void emuOilPresure()
{
  tft.print(emu.emu_data.oilPressure);
}

void emuOilTemp()
{
  tft.print(emu.emu_data.oilTemperature);
  tft.print(" C");
}

void emuCoolantTemp()
{
  tft.print(emu.emu_data.CLT);
  tft.print(" C");
}

option options[] = {
  {"Rpm", 1, emuRpm},
  {"Batt", 2, emuBatt},
  {"Iat", 3, emuIat},
  {"Oil", 4, emuOilTemp},
  {"Oil", 5, emuOilPresure},
  {"CLT", 6, emuCoolantTemp},
};

 
void renderMainScreen() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 10);
  
  tft.setTextSize(3);
  for (int i=0; i < (sizeof options / sizeof(options[0])); i++) {
    option item = options[i];
    if (item.isActive())
    {
      tft.print(item.getName());
      tft.print(":");
      item.getEmuDataT();
      tft.println();
    }
  }
  
  tft.setCursor(0, 300);
  tft.setTextSize(2);
  tft.print("B");
  tft.print(buttonState);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("C");
  tft.print(lastEncoderPos);
  tft.print(options[lastEncoderPos].getName());
}

void renderMenu()
{

}


void rotary_onButtonClick()
{
    static unsigned long lastTimePressed = 0; // Soft debouncing
    if (millis() - lastTimePressed < 500)
    {
      // write dubble click logic here ?
            return;
    }
    
    option item = options[lastEncoderPos];
    if (item.isActive()) {
      item.setInActive();
    } else {
      item.setActive();
    }
  
    
}

void rotary_loop()
{
    //dont print anything unless value changed
    if (rotaryEncoder.encoderChanged())
    {
            lastEncoderPos = rotaryEncoder.readEncoder();
    }
    if (rotaryEncoder.isEncoderButtonClicked())
    {
            buttonState = HIGH;
            rotary_onButtonClick();
    } else {
            buttonState = LOW;
    }
}

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

void setup(void) {
  Serial.begin(9600);

  tft.init(240, 320, SPI_MODE2);    // Init ST7789 display 135x240 pixel
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);;
  Serial.begin(9600);
  
    //we must initialize rotary encoder
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    //set boundaries and if values should cycle or not
    //in this example we will set possible values between 0 and 1000;
    bool circleValues = true;
    rotaryEncoder.setBoundaries(0, (sizeof(options) / sizeof(options[0]) - 1), circleValues); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

    /*Rotary acceleration introduced 25.2.2021.
   * in case range to select is huge, for example - select a value between 0 and 1000 and we want 785
   * without accelerateion you need long time to get to that number
   * Using acceleration, faster you turn, faster will the value raise.
   * For fine tuning slow down.
   */
    //rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
    rotaryEncoder.setAcceleration(250); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

    Serial1.begin(19200, SERIAL_8N1, 19, 21); //EMU Serial setup, 8 Data Bits 1 Stopbit, RX Pin, TX Pin
    
  EEPROM.begin(512);
}


 
void loop()
{
	emu.checkEmuSerial();

  unsigned long currentMillis = millis();
  if (currentMillis - previousScreenMillis >= screenRefreshRate) {
    previousScreenMillis = currentMillis;
    tft.invertDisplay(true);
    renderMainScreen();
  }

  rotary_loop();
  if (currentMillis - previousMillis >= debounce) {
    previousMillis = currentMillis;
  }

  
}