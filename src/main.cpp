#include <Arduino.h>
#include <EEPROM.h>
#include <AiEsp32RotaryEncoder.h>
#include <EMUSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "option.h"
#include "OneButton.h"


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

uint16_t SCREEN_WIDTH = 240;
uint16_t SCREEN_HEIGHT = 320;

unsigned long previousScreenMillis = 0;
unsigned long previousMillis = 0;
const long screenRefreshRate = 12; // screen refresh rate
const long debounce = 10; 

int lastEncoderPos = 0;
int optionSelected = 0;
static unsigned long lastTimePressed = 0; // Soft debouncing
int buttonState = LOW;
int lastMenuState = LOW;
int menuState = LOW;
int lastSubMenuState = LOW;
int subMenuState = LOW;
int blockColor = ST77XX_WHITE;


// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Initialize AiEsp32RotaryEncoder Library
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN,ROTARY_ENCODER_B_PIN,-1,1);


// Setup a new OneButton on pin A1.  
OneButton button1(ROTARY_ENCODER_BUTTON_PIN, true);

// PinButton myButton(ROTARY_ENCODER_BUTTON_PIN, INPUT);

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

void noValidation()
{
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  blockColor = ST77XX_WHITE;
}

void validationBatt()
{
  if (emu.emu_data.Batt < 11.0)
  {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    blockColor = ST77XX_RED;
  }
}

void validationOilTemp()
{
  if (emu.emu_data.oilTemperature < 80)
  {
    tft.setTextColor(ST77XX_ORANGE, ST77XX_BLACK);
    blockColor = ST77XX_ORANGE;
  }
  
  if (emu.emu_data.oilTemperature > 120)
  {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    blockColor = ST77XX_RED;
  }
}

void validationOilPresure()
{
  if (emu.emu_data.oilPressure < 2.0)
  {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    blockColor = ST77XX_RED;
  }
}

void emptyCallBack(){}
void back()
{
  menuState = LOW;
  subMenuState = LOW;
}

// subOption subOpt1 = subOption("Position",1);
// subOption subOpt2 = subOption("Active",2);
int subOptionCount = 2;
info positionInfo = info(false);
info activeInfo = info(true, 0);
info fullscreenInfo = info(true, 1);

subOption subOptions[] = {
  subOption("Position", &positionInfo),
  subOption("Active", &activeInfo),
  subOption("FullScreen", &activeInfo),
};
int noOptionCount = 0;
subOption noOptions[] = {
};

// option opt1 = option("Rpm", 1, emuRpm, noValidation, subOptions);
// option opt2 = option("Batt", 5, emuBatt, validationBatt);
// option opt3 = option("Iat", 10, emuIat, noValidation);
// option opt4 = option("Oil Temp", 15, emuOilTemp, validationOilTemp);
// option opt5 = option("Oil Press", 20, emuOilPresure, validationOilPresure);
// option opt6 = option("CL Temp", 25, emuCoolantTemp, noValidation);
// option opt7 = option("BACK", 250, back, emptyCallBack, false);

// // opt1.addSubOption(subOpt1);

// option options[] = {
//   opt1,
//   opt2,
//   opt3,
//   opt4,
//   opt5,
//   opt6,
//   opt7,
// };

option options[] = {
  option("Rpm", 1, emuRpm, noValidation, subOptions,subOptionCount),
  option("Batt", 5, emuBatt, validationBatt, subOptions, subOptionCount),
  option("IAT", 10, emuIat, noValidation, subOptions,subOptionCount),
  option("Oil Temp", 15, emuOilTemp, validationOilTemp, subOptions,subOptionCount),
  option("Oil Press", 20, emuOilPresure, validationOilPresure, subOptions,subOptionCount),
  option("CLT", 25, emuCoolantTemp, noValidation, subOptions,subOptionCount),
};


bool comp(const option& lhs, const option& rhs)
{
  return lhs.position < rhs.position;
}
 
void renderMainScreen() {
  tft.setTextWrap(false);
  tft.setCursor(0, 10);
  
  tft.setTextSize(3);
  
  int blockHeight = 50;
  int x = 2;
  int y = 0;
  int itemCount = 0;

  std::sort(std::begin(options), std::end(options), comp);
  for (int i=0; i < (sizeof(options) / sizeof(options[0])); i++) {
    option item = options[i];
    item.readMemoryData();

    if (item.isActive())
    {
      itemCount++;
      if (itemCount > 1)
      {
        if ( itemCount % 2 == 0)
        {
          x = (SCREEN_WIDTH / 2) +2;
        } else {
          x = 2;
          y += blockHeight;
        }
      }
      item.validate();
      tft.drawRect(x, y, (SCREEN_WIDTH / 2) - 2, (blockHeight - 2), blockColor);
      tft.setCursor(4 + x, 2 + y);

      tft.setTextSize(2);
      tft.setTextColor(blockColor, ST77XX_BLACK);
      tft.print(item.getName());
      tft.setCursor(4 + x, 20 + y);
      tft.setTextSize(3);
      item.getEmuDataT();
      tft.println();
    }
  }
}

void renderMenu()
{
  tft.setTextSize(3);
  tft.setCursor(0, 10);
  std::sort(std::begin(options), std::end(options), comp);
    for (int i=0; i < (sizeof(options) / sizeof(options[0])); i++) {
    option item = options[i];
    item.readMemoryData();

    int16_t color = 0x52AA;
    if (i == lastEncoderPos)
    {
      color = 0x94F2;
    }

    if (item.isActive())
    {
      tft.setTextColor(ST77XX_GREEN);
    } else {
      tft.setTextColor(ST77XX_RED);
    }
    int blockHeight = 28;
    tft.drawRect(1, (i*blockHeight), SCREEN_WIDTH - 2, (blockHeight - 2), color);
    tft.setCursor(4, 1+(i*blockHeight));

    tft.print(item.getName());
    tft.println();
  }
}

void renderSubMenu()
{
  tft.setTextSize(3);
  tft.setCursor(0, 10);

  option item = options[optionSelected];
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print(item.getName());
  
  int yOffset = 40;
  if (item.hasSubOption())
  {
    subOption* subOptions = item.getSubOptions();
    for (int i=0; i < item.getSubOptionCount(); i++) {
      subOption subOption = subOptions[i];
      int16_t color = 0x52AA;
      if (i == lastEncoderPos)
      {
        color = 0x94F2;
      }

      // if (item.isActive())
      // {
      //   tft.setTextColor(ST77XX_GREEN);
      // } else {
      //   tft.setTextColor(ST77XX_RED);
      // }
      
      // if (!item.isMainScreen())
      // {
      //   tft.setTextColor(ST77XX_BLACK);
      // }
      int blockHeight = 28;
      tft.drawRect(2, yOffset + (i*blockHeight), SCREEN_WIDTH - 6, (blockHeight - 2), color);
      tft.setCursor(6, yOffset + 1+(i*blockHeight));

      tft.print(subOption.getName());
      tft.println();
    } 
  }
}

bool debugIndicator = false;

void renderDebugInfo()
{
  tft.setCursor(0, 300);
  tft.setTextSize(2);
  tft.print("B");
  tft.print(buttonState);
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  tft.print("C");
  tft.print(lastEncoderPos);
  tft.print(options[lastEncoderPos].getName());
  
  if (debugIndicator)
  {
    tft.fillRect(SCREEN_WIDTH-2, SCREEN_HEIGHT-2, 2, 2, ST77XX_WHITE);
    debugIndicator = false;
  } else {
    tft.fillRect(SCREEN_WIDTH-2, SCREEN_HEIGHT-2, 2, 2, ST77XX_RED);
    debugIndicator = true;
  }
  
}


void rotary_loop() {
  lastEncoderPos = rotaryEncoder.readEncoder();
  if (rotaryEncoder.encoderChanged()) {
    Serial.print("SCROLLLL : ");
    Serial.println(rotaryEncoder.readEncoder());
    // if (rotaryEncoder.readEncoder() < lastEncoderPos)
    // {
    // } else {
    // }
    lastEncoderPos = rotaryEncoder.readEncoder();
    
  }
}

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

void click1() {
  if(menuState == LOW)
  {
    rotaryEncoder.setEncoderValue(0);
    menuState = HIGH;
  } else if(subMenuState == LOW) {
      option item = options[lastEncoderPos];
      optionSelected = lastEncoderPos;
      rotaryEncoder.setEncoderValue(0);
      lastEncoderPos = rotaryEncoder.readEncoder();
      subMenuState = HIGH;
  } else {
      option item = options[optionSelected];
      if (item.hasSubOption() && item.isInRange(lastEncoderPos +1))
      {
        subOption subOption = item.getSubOptions()[lastEncoderPos];
        if ( subOption.getInfo().isUpdateMemory)
        {
          item.updateMemory(subOption.getInfo().memoryAddressModifier, false);
          item.readMemoryData();
        }
        
      }
      
  }
} // click1


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick1() {
  if (subMenuState == HIGH)
  {
      subMenuState = LOW;
  } else if (menuState == HIGH)
  {
      menuState = LOW;
  }
} // doubleclick1


// This function will be called once, when the button1 is pressed for a long time.
void longPressStart1() {
  Serial.println("Button 1 longPress start");
} // longPressStart1


// This function will be called often, while the button1 is pressed for a long time.
void longPress1() {
  Serial.println("Button 1 longPress...");
} // longPress1


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStop1() {
  Serial.println("Button 1 longPress stop");
} // longPressStop1

ICACHE_RAM_ATTR void checkTicks()
{
  // include all buttons here to be checked
  button1.tick(); // just call tick() to check the state.
}


void setup(void) {
  pinMode(ROTARY_ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ROTARY_ENCODER_B_PIN, INPUT_PULLUP);
  Serial.begin(9600);

  tft.init(SCREEN_WIDTH, SCREEN_HEIGHT, SPI_MODE2);    // Init ST7789 display 135x240 pixel
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  tft.invertDisplay(true);
  
  //we must initialize rotary encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  //set boundaries and if values should cycle or not
  //in this example we will set possible values between 0 and 1000;
  bool circleValues = true;
  rotaryEncoder.setBoundaries(0, (sizeof(options) / sizeof(options[0]) - 1), circleValues); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)


  // encoder.setButtonHeldEnabled(true);
  // encoder.setDoubleClickEnabled(true);
  /*Rotary acceleration introduced 25.2.2021.
  * in case range to select is huge, for example - select a value between 0 and 1000 and we want 785
  * without accelerateion you need long time to get to that number
  * Using acceleration, faster you turn, faster will the value raise.
  * For fine tuning slow down.
  */
  rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
  // rotaryEncoder.setAcceleration(0); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

  Serial1.begin(19200, SERIAL_8N1, 19, 21); //EMU Serial setup, 8 Data Bits 1 Stopbit, RX Pin, TX Pin
    
  EEPROM.begin(512);
  
  // pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_BUTTON_PIN), checkTicks, CHANGE);
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  // button1.attachLongPressStart(longPressStart1);
  // button1.attachLongPressStop(longPressStop1);
  // button1.attachDuringLongPress(longPress1);
}


 
void loop()
{
	emu.checkEmuSerial();

  unsigned long currentMillis = millis();
  if (currentMillis - previousScreenMillis >= screenRefreshRate) {
    previousScreenMillis = currentMillis;
    if (lastMenuState && !menuState)
    {
      tft.fillScreen(ST77XX_BLACK);
      lastMenuState = LOW;
    }

    if (lastSubMenuState && !subMenuState)
    {
      tft.fillScreen(ST77XX_BLACK);
      lastSubMenuState = LOW;
      rotaryEncoder.setBoundaries(0, (sizeof(options) / sizeof(options[0])), true);
    }

    if (!lastMenuState && menuState)
    {
      lastMenuState = HIGH;
      rotaryEncoder.setBoundaries(0, (sizeof(options) / sizeof(options[0])), true);
      tft.fillScreen(ST77XX_BLACK);
    }
    
    if (!lastSubMenuState && subMenuState)
    {
      lastSubMenuState = HIGH;
      // selected
      
      option item = options[optionSelected];
      rotaryEncoder.setBoundaries(0, item.getSubOptionCount() - 1, true);
      tft.fillScreen(ST77XX_BLACK);
    }
    
    if (menuState)
    {
      if (subMenuState)
      {
        renderSubMenu();
      } else {
        renderMenu();
      }
    } else {
      renderMainScreen();
    }
    renderDebugInfo();

  if (currentMillis - previousMillis >= debounce) {
    previousMillis = currentMillis;
    button1.tick(); 
    rotary_loop();
  }
}
