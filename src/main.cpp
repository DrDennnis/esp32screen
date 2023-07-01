#include <Arduino.h>
#include <EEPROM.h>
#include <ClickEncoder.h>
#include <EMUSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "option.h"
#include "subOption.h"
#include "SimpleMenu.h"
#include <PinButton.h>


// SCREEN
#define TFT_MOSI 23  // SDA Pin on ESP32
#define TFT_SCLK 18  // SCL Pin on ESP32
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

//ROTARY  ENCODER
#define ROTARY_ENCODER_A_PIN 14
#define ROTARY_ENCODER_B_PIN 12
#define ROTARY_ENCODER_BUTTON_PIN 34
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4

uint16_t SCREEN_WIDTH = 240;
uint16_t SCREEN_HEIGHT = 320;

unsigned long previousScreenMillis = 0;
unsigned long previousMillis = 0;
const long screenRefreshRate = 250; // screen refresh rate
const long debounce = 50; 

int lastEncoderPos = 0;
int optionSelected = 0;
static unsigned long lastTimePressed = 0; // Soft debouncing
int buttonState = LOW;
int menuState = LOW;
int subMenuState = LOW;
int blockColor = ST77XX_WHITE;


// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Initialize AiEsp32RotaryEncoder Library
ClickEncoder encoder = ClickEncoder(ROTARY_ENCODER_A_PIN,ROTARY_ENCODER_B_PIN,ROTARY_ENCODER_BUTTON_PIN,1);

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
subOption subOptions[] = {
  subOption("Position",1),
  subOption("Active",2),
};
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
  option("Rpm", 1, emuRpm, noValidation, subOptions),
  option("Batt", 5, emuBatt, validationBatt, subOptions),
  option("IAT", 10, emuIat, noValidation, subOptions),
  option("Oil Temp", 15, emuOilTemp, validationOilTemp, subOptions),
  option("Oil Press", 20, emuOilPresure, validationOilPresure, subOptions),
  option("CLT", 25, emuCoolantTemp, noValidation, subOptions),
  option("BACK", 250, back, emptyCallBack, noOptions, false), // should not be here and should be handle diffrently but i dont know how 
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

    if (item.isActive() && item.isMainScreen())
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
      tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    } else {
      tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    }
    
    if (!item.isMainScreen())
    {
      tft.setTextColor(ST77XX_BLACK, ST77XX_BLACK);
    }
    int blockHeight = 28;
    tft.fillRect(1, (i*blockHeight), SCREEN_WIDTH - 2, (blockHeight - 2), color);
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
    
    subOption* subOptions = item.itemSubOptions;
    Serial.print(sizeof(subOptions) / sizeof(int));
    Serial.print("/");
    Serial.print(item.hasSubOption());
    Serial.println("");
    
  for (int i=0; i < (sizeof(subOptions) / sizeof(int)); i++) {
    subOption subOption = subOptions[i];
    // item.readMemoryData();

    Serial.print(sizeof(subOptions) / sizeof(int));
    Serial.print("/");
    Serial.print(lastEncoderPos);
    Serial.print(":");
    Serial.print(subOption.getName());
    Serial.println("");
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
    tft.fillRect(2, (i*blockHeight), SCREEN_WIDTH - 6, (blockHeight - 2), color);
    tft.setCursor(6, 2+(i*blockHeight));

    tft.print(subOption.getName());
    tft.println();
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


// void rotary_onButtonClick()
// {
//     Serial.print("OpenMenu : ");
//     Serial.print(menuState);
//     Serial.print(" : ");
//     Serial.print(subMenuState);
//     Serial.print(" : ");
//     Serial.println(lastTimePressed);
//     if (millis() - lastTimePressed < 500)
//     {
//             return;
//     }
//     if(menuState == LOW)
//     {
//       rotaryEncoder.setEncoderValue(0);
//       lastEncoderPos = rotaryEncoder.readEncoder();
//       menuState = HIGH;
//     } else {
//       option item = options[lastEncoderPos];
//       if (!item.isMainScreen()) {
//         menuState = LOW;
//       } else {
//         if (item.isActive()) {
//           item.setInActive();
//         } else {
//           item.setActive();
//         }
//       }
//       item.readMemoryData();
//     }
// }

// void rotary_loop()
// {
  
//   unsigned long currentMillis = millis();
//   //dont print anything unless value changed
//   if (rotaryEncoder.encoderChanged())
//   {
//     // if (rotaryEncoder.readEncoder() < lastEncoderPos)
//     // {
//     // } else {
//     // }
//     lastEncoderPos = rotaryEncoder.readEncoder();
    
//   }
  // if (rotaryEncoder.isEncoderButtonClicked()) {
  //   buttonState = HIGH;
  //   if (menuState == HIGH && millis() - lastTimePressed < 500 && millis() - lastTimePressed > 50)
  //   {
  //     Serial.print("OpenSubMenu : with Menu state :");
  //     Serial.print(menuState);
  //     Serial.print(" : ");
  //     Serial.print(subMenuState);
  //     Serial.print(" : ");
  //     Serial.println(lastTimePressed);
  //     option item = options[lastEncoderPos];
  //     if(menuState == HIGH && sizeof(item.itemSubOptions) > 1)
  //     {
  //       subMenuState = HIGH;
  //       optionSelected = lastEncoderPos;
  //       rotaryEncoder.setEncoderValue(0);
  //       lastEncoderPos = rotaryEncoder.readEncoder();
  //     }
  //   } else {
  //     rotary_onButtonClick();
  //   }
  //   lastTimePressed = currentMillis;
  // } else {
  //   buttonState = LOW;
  // }
  // if (rotaryEncoder.isEncoderButtonClicked())
  // {
  //   buttonState = HIGH;
  //   rotary_onButtonClick();
  // } else {
  //   buttonState = LOW;
  // }
    
    
// }

// void IRAM_ATTR readEncoderISR()
// {
//     rotaryEncoder.readEncoder_ISR();
// }


void setup(void) {
  Serial.begin(9600);

  tft.init(SCREEN_WIDTH, SCREEN_HEIGHT, SPI_MODE2);    // Init ST7789 display 135x240 pixel
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  
  //we must initialize rotary encoder
  // rotaryEncoder.begin();
  // rotaryEncoder.setup(readEncoderISR);
  //set boundaries and if values should cycle or not
  //in this example we will set possible values between 0 and 1000;
  bool circleValues = true;
  // rotaryEncoder.setBoundaries(0, (sizeof(options) / sizeof(options[0]) - 1), circleValues); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)


  encoder.setButtonHeldEnabled(true);
  encoder.setDoubleClickEnabled(true);
  /*Rotary acceleration introduced 25.2.2021.
  * in case range to select is huge, for example - select a value between 0 and 1000 and we want 785
  * without accelerateion you need long time to get to that number
  * Using acceleration, faster you turn, faster will the value raise.
  * For fine tuning slow down.
  */
  // rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
  // rotaryEncoder.setAcceleration(250); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

  Serial1.begin(19200, SERIAL_8N1, 19, 21); //EMU Serial setup, 8 Data Bits 1 Stopbit, RX Pin, TX Pin
    
  EEPROM.begin(512);
  
  Serial.print((sizeof(options) / sizeof(options[0])));
  Serial.print("/");
  Serial.print((sizeof(subOptions) / sizeof(int)));
  Serial.print(std::size_t(subOptions));
  Serial.print("/");
  // Serial.print((sizeof(opt1.itemSubOptions) / sizeof(opt1.itemSubOptions[0])));
  Serial.print("/");
  Serial.println("");
  for (option option : options) {
        Serial.print(option.getName());
        Serial.print(">");
    subOption *subOptions = option.itemSubOptions;
      for (int i = 0; i < sizeof(subOptions) / sizeof(int); i++) {
        Serial.print(subOptions[i].getName());
        Serial.print("|");
      }
      Serial.println("");
  }
  // Serial.print(opt1.itemSubOptions[0].getName());

  // for (int i=0; i < (sizeof options / sizeof(options[0])); i++) {
  //   option item = options[i];
  //   item.addSubOption(subOpt1);
  // }
}


 
void loop()
{
	emu.checkEmuSerial();

  unsigned long currentMillis = millis();
  if (currentMillis - previousScreenMillis >= screenRefreshRate) {
    previousScreenMillis = currentMillis;
    tft.invertDisplay(true);
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
  }

  if (currentMillis - previousScreenMillis >= 1000) {
    encoder.service();  
  }

  // rotary_loop();
  // myButton.update();
  
  // if (myButton.isClick()) {
  //   buttonState = HIGH;
  //   Serial.print(currentMillis);
  //   Serial.println(" click");
  // } else {
  //   buttonState = LOW;
  // }
  // if (myButton.isSingleClick()) {
  //   Serial.print(currentMillis);
  //   Serial.println(" single");
  // }
  // if (myButton.isDoubleClick()) {
  //   Serial.print(currentMillis);
  //   Serial.println(" double");
  // }
  // if (myButton.isLongClick()) {
  //   Serial.print(currentMillis);
  //   Serial.println(" long");
  // }
  // if (myButton.isReleased()) {
  //   Serial.print(currentMillis);
  //   Serial.println(" up");
  // }
  
  ClickEncoder::Button b = encoder.getButton();
  if (b != ClickEncoder::Open) {
    Serial.print("Button: ");
    switch (b) {
      case ClickEncoder::Pressed:
          Serial.println("ClickEncoder::Pressed");
      break;
      case ClickEncoder::Held:
          Serial.println("ClickEncoder::Held");
      break;
      case ClickEncoder::Released:
          Serial.println("ClickEncoder::Released");
      break;
      case ClickEncoder::Clicked:
          Serial.println("ClickEncoder::Clicked");
          if(menuState == LOW)
          {
            // rotaryEncoder.setEncoderValue(0);
            lastEncoderPos = encoder.getValue();
            menuState = HIGH;
          } else if(options[lastEncoderPos].isMainScreen()) {
            option item = options[lastEncoderPos];
            if (!item.isMainScreen()) {
              menuState = LOW;
            } else {
              // open submenu if it has it 
              if (subMenuState)
              {
                if (item.isActive()) {
                  item.setInActive();
                } else {
                  item.setActive();
                }
              }
            }
            item.readMemoryData();
          }
      break;
      case ClickEncoder::DoubleClicked:
          Serial.println("ClickEncoder::DoubleClicked");
          if (subMenuState == HIGH)
          {
              subMenuState = LOW;
          } else if (menuState == HIGH)
          {
              menuState = LOW;
          }
        break;
    }
  }

  // if (myButton.isSingleClick()) {
  //   if(menuState == LOW)
  //   {
  //     // rotaryEncoder.setEncoderValue(0);
  //     lastEncoderPos = encoder->getValue();
  //     menuState = HIGH;
  //   } else if(options[lastEncoderPos].isMainScreen()) {
  //     option item = options[lastEncoderPos];
  //     if (!item.isMainScreen()) {
  //       menuState = LOW;
  //     } else {
  //       // open submenu if it has it 
  //       if (subMenuState)
  //       {
  //         if (item.isActive()) {
  //           item.setInActive();
  //         } else {
  //           item.setActive();
  //         }
  //       }
        
  //     }
  //     item.readMemoryData();
  //   }
  // }

  // if (myButton.isDoubleClick()) {
  //   if (subMenuState == HIGH)
  //   {
  //       subMenuState = LOW;
  //   } else if (menuState == HIGH)
  //   {
  //       menuState = LOW;
  //   }
  // }
  
  if (currentMillis - previousMillis >= debounce) {
    previousMillis = currentMillis;
  }
}