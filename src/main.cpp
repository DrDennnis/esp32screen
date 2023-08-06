#include <Arduino.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <AiEsp32RotaryEncoder.h>
#include <EMUSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include "option.h"
#include "OneButton.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <bootscreen.h>

// SCREEN
#define TFT_MOSI MOSI
#define TFT_SCLK SCK
#define TFT_RST -1
#define TFT_DC 4
#define TFT_CS 2

// ROTARY ENCODER
#define ROTARY_ENCODER_A_PIN 35
#define ROTARY_ENCODER_B_PIN 18
#define ROTARY_ENCODER_BUTTON_PIN 16
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4

// EMU
#define EMU_RX RX
#define EMU_TX TX

uint16_t SCREEN_WIDTH = 240;
uint16_t SCREEN_HEIGHT = 320;

const char *ssid = "EmuScreen";
const char *password = "12345678";
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, -1, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
OneButton button(ROTARY_ENCODER_BUTTON_PIN, true, true);
Adafruit_NeoPixel strip(1, RGB_BUILTIN, NEO_GRB + NEO_KHZ800);
EMUSerial emu(Serial1);
AsyncWebServer server(80);
Preferences preferences;

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

bool intSuboptionSelected = false;
int selectedSubOption = 0;

bool updateMode = LOW;
bool updateScreen = LOW;
unsigned long endTime;

int bootAnimation = 1500;
bool booted = false;
bool slashHasBeenDrawn = false;

int ledPeriod = 10;
unsigned long ledTimeNow = 0;

void emuRpm()
{
  tft.printf("%04d", emu.emu_data.RPM);
}

void emuBatt()
{
  float value = emu.emu_data.Batt;
  if (value < 10)
  {
    tft.print(0);
  }
  tft.printf("%.2fV", emu.emu_data.Batt);
}

void emuIat()
{
  tft.printf("%03dC", emu.emu_data.IAT);
}

void emuOilPresure()
{
  float value = emu.emu_data.oilPressure;
  if (value < 10)
  {
    tft.print(0);
  }
  tft.printf("%.2f", emu.emu_data.oilPressure);
}

void emuOilTemp()
{
  tft.printf("%03dC", emu.emu_data.oilTemperature);
}

void emuCoolantTemp()
{
  tft.printf("%03dC", emu.emu_data.CLT);
}

void emuAfr()
{
  float value = emu.emu_data.wboAFR;
  if (value < 10)
  {
    tft.print(0);
  }

  tft.printf("%.2f", emu.emu_data.wboAFR);
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

int subOptionCount = 3;

subOption subOptions[] = {
    subOption("Active", {true, 0, info::subOptionType::Type_Bool}),
    subOption("FullWidth", {true, 1, info::subOptionType::Type_Bool}),
    subOption("Position", {true, 2, info::subOptionType::Type_Int}),
};
int noOptionCount = 0;
subOption noOptions[] = {};

option options[] = {
    option("AFR", 0, emuAfr, noValidation, subOptions, subOptionCount),
    option("OilP", 50, emuOilPresure, validationOilPresure, subOptions, subOptionCount),
    option("OilT", 100, emuOilTemp, validationOilTemp, subOptions, subOptionCount),
    option("IAT", 150, emuIat, noValidation, subOptions, subOptionCount),
    option("CLT", 200, emuCoolantTemp, noValidation, subOptions, subOptionCount),
    option("Batt", 250, emuBatt, validationBatt, subOptions, subOptionCount),
    option("Rpm", 300, emuRpm, noValidation, subOptions, subOptionCount),
};

bool comp(option &lhs, option &rhs)
{
  lhs.readMemoryData();
  rhs.readMemoryData();
  return lhs.position < rhs.position;
}

void sortOptions()
{
  std::sort(options, options + (sizeof(options) / sizeof(options[0])), comp);
}

void drawPercentbar(int x, int y, int width, int height, int progress)
{
  progress = progress > 100 ? 100 : progress;
  progress = progress < 0 ? 0 : progress;

  float bar = ((float)(width - 4) / 100) * progress;
  tft.drawRect(x, y, width, height, ST77XX_WHITE);
  tft.fillRect(x + 2, y + 2, bar, height - 4, ST77XX_WHITE);
}

void renderSplashScreen(unsigned long currentMillis)
{
  if (!slashHasBeenDrawn)
  {
    tft.drawBitmap(0, 0, epd_bitmap_Bitmap, SCREEN_WIDTH, SCREEN_HEIGHT, ST77XX_WHITE);
    slashHasBeenDrawn = true;
  }
  int timeRemaning = endTime - currentMillis;
  drawPercentbar(0, (SCREEN_HEIGHT - 30), SCREEN_WIDTH, 30, ((bootAnimation - timeRemaning) * 100) / bootAnimation);
  if (currentMillis > endTime)
  {
    booted = true;
    tft.fillScreen(ST77XX_BLACK);
  }
}

void renderMainScreen()
{
  tft.setTextWrap(false);
  tft.setCursor(0, 10);

  tft.setTextSize(3);

  int x = 2;
  int y = 0;
  int itemCount = 0;
  int blockHeight = 50;
  int textSizeTitle = 2;
  int textSizeValue = 3;

  for (int i = 0; i < (sizeof(options) / sizeof(options[0])); i++)
  {
    option item = options[i];
    item.readMemoryData();
    bool active = false;
    bool fullWidth = false;
    subOption *subOptions = item.getSubOptions();
    for (int i = 0; i < item.getSubOptionCount(); i++)
    {
      subOption subOption = subOptions[i];
      if (subOption.getName() == "Active")
      {
        active = item.readMemoryDataBool(subOption.getInfo().memoryAddressModifier);
      }
      if (subOption.getName() == "FullWidth")
      {
        fullWidth = item.readMemoryDataBool(subOption.getInfo().memoryAddressModifier);
      }
    }
    if (active)
    {
      itemCount++;
      if (itemCount > 1)
      {
        if (itemCount % 2 == 0)
        {
          x = (SCREEN_WIDTH / 2) + 2;
        }
        else
        {
          x = 2;
          y += blockHeight;
        }
      }
      int width = (SCREEN_WIDTH / 2);
      if (fullWidth)
      {
        width = SCREEN_WIDTH;
        if (itemCount % 2 == 0)
        {
          x = 2;
          y += blockHeight;
          itemCount++;
        }

        itemCount++;
        blockHeight = 50;
        textSizeTitle = 2;
        textSizeValue = 4;
      }
      else
      {
        blockHeight = 50;
        textSizeTitle = 2;
        textSizeValue = 3;
      }

      item.validate();
      tft.drawRect(x, y, width - 2, (blockHeight - 2), blockColor);
      tft.setCursor(4 + x, 2 + y + (blockHeight - 16) - 6);

      tft.setTextSize(textSizeTitle);
      tft.setTextColor(blockColor, ST77XX_BLACK);
      tft.print(item.getName());

      if (fullWidth)
      {
        tft.setCursor(70, 3 + y);
      }
      else
      {
        tft.setCursor(4 + x, 3 + y);
      }
      tft.setTextSize(textSizeValue);
      item.getEmuDataT();
      tft.println();
    }
  }
}

void renderMenu()
{
  tft.setTextSize(3);
  tft.setCursor(0, 10);
  for (int i = 0; i < (sizeof(options) / sizeof(options[0])); i++)
  {
    option item = options[i];

    int16_t color = 0x52AA;
    if (i == lastEncoderPos)
    {
      // a clear indicator of which option we have selected
      color = ST77XX_BLUE;
    }

    bool active = false;
    subOption *subOptions = item.getSubOptions();
    for (int i = 0; i < item.getSubOptionCount(); i++)
    {
      subOption subOption = subOptions[i];
      if (subOption.getName() == "Active")
      {
        active = item.readMemoryDataBool(subOption.getInfo().memoryAddressModifier);
      }
    }

    if (active)
    {
      tft.setTextColor(ST77XX_GREEN);
    }
    else
    {
      tft.setTextColor(ST77XX_RED);
    }

    int blockHeight = 28;
    tft.drawRect(1, (i * blockHeight), SCREEN_WIDTH - 2, (blockHeight - 2), color);
    tft.setCursor(4, 1 + (i * blockHeight));

    tft.print(item.getName());
    tft.setCursor((SCREEN_WIDTH - (24 * 2)) - 2, 1 + (i * blockHeight));
    tft.printf("%03d", item.getPosition());
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
  int blockHeight = 50;
  if (item.hasSubOption())
  {
    subOption *subOptions = item.getSubOptions();
    for (int i = 0; i < item.getSubOptionCount(); i++)
    {
      tft.setTextSize(3);
      subOption subOption = subOptions[i];
      int16_t color = 0x52AA; // greyish
      int subOptionPos = lastEncoderPos;
      if (intSuboptionSelected)
      {
        if (selectedSubOption == i)
        {
          color = ST77XX_GREEN;
        }
      }
      else
      {
        if (i == lastEncoderPos)
        {
          // a clear indicator of which option we have selected
          color = ST77XX_BLUE;
        }
      }

      tft.drawRect(2, yOffset + (i * blockHeight), SCREEN_WIDTH - 6, (blockHeight - 2), color);
      tft.setCursor(6, yOffset + 1 + (i * blockHeight));
      tft.print(subOption.getName());

      tft.setCursor(6, yOffset + 25 + (i * blockHeight));
      tft.setTextSize(2);
      if (subOption.getInfo().type == info::subOptionType::Type_Bool)
      {
        tft.print(item.readMemoryDataBool(subOption.getInfo().memoryAddressModifier) ? "True " : "False");
      }
      if (subOption.getInfo().type == info::subOptionType::Type_Int)
      {
        if (intSuboptionSelected)
        {
          tft.printf("%03d", lastEncoderPos);
        }
        else
        {
          tft.printf("%03d", item.readMemoryDataInt(subOption.getInfo().memoryAddressModifier));
        }
      }
    }
  }
}

void renderUpdateScreen()
{
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 10);
  tft.setTextSize(2);

  tft.println("Connect to:");
  tft.println(ssid);
  tft.println("With Password:");
  tft.println(password);
  tft.println("After go to url:");
  tft.printf("%s.local", ssid);
  tft.println();
}

void rotary_loop()
{
  lastEncoderPos = rotaryEncoder.readEncoder();
  if (rotaryEncoder.encoderChanged())
  {
    Serial.print("Scroll: ");
    Serial.println(lastEncoderPos);
  }
}

void IRAM_ATTR readEncoderISR()
{
  rotaryEncoder.readEncoder_ISR();
}

void buttonSingleClick()
{
  Serial.println("buttonSingleClick");

  if (intSuboptionSelected)
  {
    intSuboptionSelected = false;
    option item = options[optionSelected];
    subOption subOption = item.getSubOptions()[selectedSubOption];
    item.updateMemory(subOption.getInfo().memoryAddressModifier, lastEncoderPos);
    rotaryEncoder.setBoundaries(0, item.getSubOptionCount() - 1, false);
    rotaryEncoder.setEncoderValue(selectedSubOption);
    lastEncoderPos = rotaryEncoder.readEncoder();
    return;
  }

  if (menuState == LOW)
  {
    rotaryEncoder.setEncoderValue(0);
    menuState = HIGH;
  }
  else if (subMenuState == LOW)
  {
    option item = options[lastEncoderPos];
    optionSelected = lastEncoderPos;
    rotaryEncoder.setEncoderValue(0);
    lastEncoderPos = rotaryEncoder.readEncoder();
    subMenuState = HIGH;
  }
  else
  {
    option item = options[optionSelected];
    if (item.hasSubOption() && item.isInRange(lastEncoderPos + 1))
    {
      subOption subOption = item.getSubOptions()[lastEncoderPos];

      Serial.print("Suboption change:");
      Serial.println(subOption.getName());

      if (subOption.getInfo().isUpdateMemory)
      {
        if (subOption.getInfo().type == info::subOptionType::Type_Bool)
        {
          item.updateMemory(subOption.getInfo().memoryAddressModifier, !item.readMemoryDataBool(subOption.getInfo().memoryAddressModifier));
        }
        if (subOption.getInfo().type == info::subOptionType::Type_Int)
        {
          intSuboptionSelected = true;
          selectedSubOption = lastEncoderPos;
          rotaryEncoder.setBoundaries(0, 255, true);
          rotaryEncoder.setEncoderValue(item.readMemoryDataInt(subOption.getInfo().memoryAddressModifier));
        }
      }
    }
  }
}

void buttonDoubleClick()
{
  Serial.println("buttonDoubleClick");

  if (subMenuState == HIGH)
  {
    subMenuState = LOW;
  }
  else if (menuState == HIGH)
  {
    menuState = LOW;
  }
  else if (!updateScreen)
  {
    updateScreen = HIGH;
    updateMode = HIGH;

    Serial.println("Update screen enabled");
    tft.fillScreen(ST77XX_BLACK);
  }
  else if (updateScreen)
  {
    ESP.restart();
  }
}

ICACHE_RAM_ATTR void checkTicks()
{
  // include all buttons here to be checked
  button.tick(); // just call tick() to check the state.
}

void enableHotspot()
{
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->redirect("/update"); });

  AsyncElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();
  MDNS.begin(ssid);
}

void setup(void)
{
  pinMode(ROTARY_ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ROTARY_ENCODER_B_PIN, INPUT_PULLUP);

  // USB
  Serial.begin(115200);

  // EMU
  Serial1.begin(19200, SERIAL_8N1, EMU_RX, EMU_TX);

  // Onboard led
  strip.begin();
  strip.show();
  strip.setBrightness(20);

  tft.init(SCREEN_WIDTH, SCREEN_HEIGHT, SPI_MODE2); // Init ST7789 display 135x240 pixel
  tft.setRotation(2);
  tft.invertDisplay(true);
  // fillScreen black to clear instead of a reset as we do not use this pin
  tft.fillScreen(ST77XX_BLACK);

  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(0, (sizeof(options) / sizeof(options[0]) - 1), true);
  rotaryEncoder.disableAcceleration();

  attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_BUTTON_PIN), checkTicks, CHANGE);
  button.attachClick(buttonSingleClick);
  button.attachDoubleClick(buttonDoubleClick);

  EEPROM.begin(512);
  preferences.begin("EmuScreen", false);

  sortOptions();
  endTime = millis() + bootAnimation;
}

int loopcount = 390;
void loop()
{
  unsigned long currentMillis = millis();

  rotary_loop();
  // keep watching the push button, even when no interrupt happens:
  button.tick();

  emu.checkEmuSerial();
  if (loopcount < 512 && Serial1.available())
  {
    if (emu.emu_data.RPM > 1000)
    {
      EEPROM.put(390, emu.emu_data);
      EEPROM.commit();
      loopcount + sizeof(emu.emu_data);
      Serial.println("Writing to eeprom");
    }
  }

  if (updateMode)
  {
    enableHotspot();
    updateMode = false;
  }

  if (!booted)
  {
    renderSplashScreen(currentMillis);
    return;
  }

  if (lastMenuState && !menuState)
  {
    tft.fillScreen(ST77XX_BLACK);
    lastMenuState = LOW;
    sortOptions();
  }

  if (lastSubMenuState && !subMenuState)
  {
    tft.fillScreen(ST77XX_BLACK);
    lastSubMenuState = LOW;
    rotaryEncoder.setBoundaries(0, (sizeof(options) / sizeof(options[0])), true);
    rotaryEncoder.setEncoderValue(optionSelected);
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
    rotaryEncoder.setBoundaries(0, item.getSubOptionCount() - 1, false);
    tft.fillScreen(ST77XX_BLACK);
  }

  if (updateScreen)
  {
    renderUpdateScreen();
  }
  else if (menuState)
  {
    if (subMenuState)
    {
      renderSubMenu();
    }
    else
    {
      renderMenu();
    }
  }
  else
  {
    renderMainScreen();
  }
}
