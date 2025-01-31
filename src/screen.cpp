#include "screen.h"
#include <TFT_eSPI.h>
#include <PNGdec.h>
#include "kn_bg.h"
#include "kn_bg_op.h"
#include "wifi_bad.h"
#include "wifi_ok.h"
#include "eth_bad.h"
#include "eth_ok.h"
#include "aws_bad.h"
#include "aws_ok.h"
#include "info.h"
#include "ARIALNB14pt7b.h"
#include "smallwarning.h"

#define pin_external_power 15

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

TaskHandle_t GearTask;

#define DISPLAY_WIDTH tft.width()
#define DISPLAY_HEIGHT tft.height()
#define MAX_IMAGE_WIDTH 320

PNG png;

static int16_t xpos = 0;
static int16_t ypos = 0;

String wifi_ssid = "";
bool wifi_status = false;
String wifi_info = "";
String thing_name = "";
bool aws_status = false;
String plc_ip = "";
String plc_port = "";
bool ethernet_status = false;
String modbus_id = "";
String firmware_version = "";
String firmware_name = "";
String firmware_date = "";
bool showParameters = false;

static bool errorReceived = false;
static uint16_t infoCounter = 0;
static bool wifiInfoUpdatedOnScreen = true;
static bool awsInfoUpdatedOnScreen = true;
static bool plcInfoUpdatedOnScreen = true;
static bool firmwareInfoUpdatedOnScreen = true;
static bool kinnilLogoOnScreen = false;
static bool smallLogoOnScreen = false;

static uint16_t blue = tft.color565(102, 178, 255);
static uint16_t gray = tft.color565(192, 192, 192);
static uint16_t statusGreen = tft.color565(102, 255, 102);
static uint16_t statusRed = tft.color565(255, 102, 102);

static void openPNG(const uint8_t* image, size_t image_size);
static void pngDraw(PNGDRAW *pDraw);
static void showSmallKinnilLogo();
static void showAwsIcon(bool show);
static void showEthernetIcon(bool show);
static void showWifiIcon(bool show);
static void showStatusIcon(bool show);
static void showKinnilLogo();
static void showInfo(uint8_t type);

void screen_setup(){
    pinMode(pin_external_power, OUTPUT);
    digitalWrite(pin_external_power, HIGH);
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    showKinnilLogo();
    delay(1000);
    
}

void screen_loop(){
  if (showParameters){
    if (!kinnilLogoOnScreen){
      showKinnilLogo();
      kinnilLogoOnScreen = true;
    }
    if (!wifi_ssid.isEmpty() && !wifiInfoUpdatedOnScreen){
      Serial.println("updating wifi info on screen");
      showInfo(1);
      wifiInfoUpdatedOnScreen = true;
    }
    if (!thing_name.isEmpty() && !awsInfoUpdatedOnScreen){
      Serial.println("updating aws info on screen");
      showInfo(2);
      awsInfoUpdatedOnScreen = true;
    }
    if (!plc_ip.isEmpty() && !plcInfoUpdatedOnScreen){
      Serial.println("updating plc info on screen");
      showInfo(3);
      plcInfoUpdatedOnScreen = true;
    }
    if (!firmware_date.isEmpty() && !firmwareInfoUpdatedOnScreen){
      Serial.println("updating firmware info on screen");
      showInfo(4);
      firmwareInfoUpdatedOnScreen = true;
    }

    delay(10);
    infoCounter++;
    if (infoCounter == 500)kinnilLogoOnScreen = false;
    if (infoCounter == 999)showSmallKinnilLogo();
    if (infoCounter == 1000)wifiInfoUpdatedOnScreen = false;
    if (infoCounter == 1500)awsInfoUpdatedOnScreen = false;
    if (infoCounter == 2000)plcInfoUpdatedOnScreen = false;
    if (infoCounter == 2500)firmwareInfoUpdatedOnScreen = false;
    if (infoCounter >= 3000)infoCounter = 0;
  }
}

void showMessage(String message, String value){
  showSmallKinnilLogo();
  sprite.createSprite(320,120);
  sprite.fillSprite(TFT_BLACK);
  sprite.setFreeFont(&ARIALNB14pt7b);
  sprite.setTextColor(blue);
  sprite.drawString(message,10,20);
  sprite.drawString(value,10,60);
  sprite.pushSprite(0,45);
  sprite.deleteSprite();
  errorReceived = false;
  showStatusIcon(false);
}

void showError(String message, String value){
  showSmallKinnilLogo();
  sprite.createSprite(320,120);
  sprite.fillSprite(TFT_BLACK);
  sprite.setFreeFont(&ARIALNB14pt7b);
  sprite.setTextColor(statusRed);
  sprite.drawString(message,10,20);
  sprite.drawString(value,10,60);
  sprite.pushSprite(0,55);
  sprite.deleteSprite();
  errorReceived = true;
  showStatusIcon(true);
}

void showInfo(uint8_t type){
  sprite.createSprite(320,120);
  sprite.fillSprite(TFT_BLACK);
  sprite.setFreeFont(&ARIALNB14pt7b);
  sprite.setTextColor(blue);
  String status = "";
  switch(type){
    case 1: //Wifi info
      sprite.drawString("WiFi network info",0,2);
      sprite.setTextColor(TFT_WHITE);
      sprite.drawString(wifi_ssid,0,32);
      sprite.setTextColor(gray);
      sprite.drawString("Status: ",0,62);
      if (wifi_status){
        sprite.setTextColor(statusGreen);
        status = "Connected";
      }else{
        sprite.setTextColor(statusRed);
        status = "Disconnected";
      } 
      sprite.drawString(status,80,62);
      sprite.setTextColor(TFT_WHITE);
      sprite.drawString(wifi_info,0,92);
      showWifiIcon(true);
      showAwsIcon(true);
      showEthernetIcon(true);
      showStatusIcon(true);
      break; 
    case 2: //AWS info
      sprite.drawString("AWS service info",0,2);
      sprite.setTextColor(TFT_WHITE);
      sprite.drawString(thing_name,0,32);
      sprite.setTextColor(gray);
      sprite.drawString("Status: ",0,62);
      if (aws_status){
        sprite.setTextColor(statusGreen);
        status = "Connected";
      }else{
        sprite.setTextColor(statusRed);
        status = "Disconnected";
      } 
      sprite.drawString(status,80,62);
      showWifiIcon(true);
      showAwsIcon(true);
      showEthernetIcon(true);
      showStatusIcon(true);
      break; 
    case 3: //PLC & Modbus info
      sprite.drawString("PLC & Modbus info",0,2);
      sprite.setTextColor(gray);
      sprite.drawString("PLC IP:",0,32);
      sprite.setTextColor(TFT_WHITE);
      sprite.drawString(plc_ip,85,32);
      sprite.setTextColor(gray);
      sprite.drawString("Port: ",0,62);
      sprite.setTextColor(TFT_WHITE);
      sprite.drawString(plc_port,60,62);
      sprite.setTextColor(gray);
      sprite.drawString("Modbus ID: ",120,62);
      sprite.setTextColor(TFT_WHITE);
      sprite.drawString(modbus_id,255,62);
      sprite.setTextColor(gray);
      sprite.drawString("Status: ",0,92);
      if (ethernet_status){
        sprite.setTextColor(statusGreen);
        status = "Connected";
      }else{
        sprite.setTextColor(statusRed);
        status = "Disconnected";
      } 
      sprite.drawString(status,80,92);
      showWifiIcon(true);
      showAwsIcon(true);
      showEthernetIcon(true);
      showStatusIcon(true);
      break;
    case 4: //Firmware info
      sprite.drawString("Firmware info",0,2);
      sprite.setTextColor(gray);
      sprite.drawString("Version:",0,32);
      sprite.setTextColor(TFT_WHITE);
      sprite.drawString(firmware_version,100,32);
      sprite.drawString(firmware_name,0,62);
      sprite.drawString(firmware_date,0,92);
      break; 
  }
  sprite.pushSprite(0,45);
  sprite.deleteSprite();
}

void showKinnilLogo(){
  xpos = 0;
  ypos = 0;
  openPNG(kn_bg, sizeof(kn_bg));
  smallLogoOnScreen = false;
}

void showSmallKinnilLogo(){
  if (smallLogoOnScreen)return;
  smallLogoOnScreen = true;
  xpos = 0;
  ypos = 0;
  openPNG(kn_bg_op, sizeof(kn_bg_op));
}

void showAwsIcon(bool show){
  xpos = 220;
  if (show){
    if (!aws_status)openPNG(aws_bad, sizeof(aws_bad));
    else openPNG(aws_ok, sizeof(aws_ok));
  }else{
    sprite.createSprite(31,31);
    sprite.fillSprite(TFT_BLACK);
    sprite.pushSprite(xpos,0);
    sprite.deleteSprite();
  }
}

void showEthernetIcon(bool show){
  xpos = 255;
  if (show){
    if (!ethernet_status)openPNG(eth_bad, sizeof(eth_bad));
    else openPNG(eth_ok, sizeof(eth_ok));
  }else{
    sprite.createSprite(31,31);
    sprite.fillSprite(TFT_BLACK);
    sprite.pushSprite(xpos,0);
    sprite.deleteSprite();
  }
}

void showWifiIcon(bool show){
  xpos = 290;
  if (show){
    if (!wifi_status)openPNG(wifi_bad, sizeof(wifi_bad));
    else openPNG(wifi_ok, sizeof(wifi_ok));
  }else{
    sprite.createSprite(31,31);
    sprite.fillSprite(TFT_BLACK);
    sprite.pushSprite(xpos,0);
    sprite.deleteSprite();
  }
}

void showStatusIcon(bool show){
  xpos = 150;
  if (show){
    if (errorReceived){
      openPNG(smallwarning, sizeof(smallwarning));
    }
    if (!wifi_status || !aws_status || !ethernet_status){
      openPNG(smallwarning, sizeof(smallwarning));
    }
  }else{
    sprite.createSprite(36,36);
    sprite.fillSprite(TFT_BLACK);
    sprite.pushSprite(xpos,0);
    sprite.deleteSprite();
  }
}

void openPNG(const uint8_t* image, size_t image_size)
{
    int16_t rc = png.openFLASH((uint8_t *)image, image_size, pngDraw);

  if (rc == PNG_SUCCESS) {
    //Serial.println("Successfully opened png file");
    //Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    tft.startWrite();
    uint32_t dt = millis();
    rc = png.decode(NULL, 0);
    Serial.print(millis() - dt); Serial.println("ms");
    tft.endWrite();
  }
}

void pngDraw(PNGDRAW *pDraw)
{
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}
