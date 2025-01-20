#include <Arduino.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>
#include <AnimatedGIF.h>
#include <HardwareSerial.h>

#define BAUD_RATE 9600
#define TX_1  2
#define RX_1  3

#include "kn_bg.h"
#include "kn_bg_op.h"
#include "wifi_bad.h"
#include "wifi_ok.h"
#include "eth_bad.h"
#include "eth_ok.h"
#include "aws_bad.h"
#include "aws_ok.h"
#include "Inter_10pt.h"
#include "Inter_7pt.h"
#include "info.h"

HardwareSerial CommPort(1);

#define pin_external_power 15

PNG png;
#define MAX_IMAGE_WIDTH 320
int16_t xpos = 0;
int16_t ypos = 0;

AnimatedGIF gif;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

#define DISPLAY_WIDTH tft.width()
#define DISPLAY_HEIGHT tft.height()
#define BUFFER_SIZE 256

uint16_t usTemp[1][BUFFER_SIZE];
bool dmaBuf = 0;

void openPNG(const uint8_t* image, size_t image_size);
void pngDraw(PNGDRAW *pDraw);
void showBaseBackGround(uint8_t wifi, uint8_t eth, uint8_t aws);
void showVersionFooter(char* version);
void showInfo(String infoA, String infoB, String infoC);
void setCenterSprite(uint16_t w, uint16_t h, String text, uint16_t x, uint16_t y);

uint8_t loading_counter = 0;

void setup() 
{
  pinMode(pin_external_power, OUTPUT);
  digitalWrite(pin_external_power, HIGH);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  gif.begin(BIG_ENDIAN_PIXELS);
  openPNG(kn_bg, sizeof(kn_bg));
  Serial.begin(115200);
  Serial.println("Started!!");
  CommPort.begin(BAUD_RATE,SERIAL_8N1,RX_1,TX_1);
  Serial.begin(BAUD_RATE);
  delay(2000);
  showBaseBackGround(1,1,1);
  showVersionFooter("");
  showInfo("Unconfigured credentials","Please create a hotspot","ID: KinnilHP PW:KN1818");
}

void loop() 
{

}

void showInfo(String infoA, String infoB, String infoC)
{
  if (!infoC.isEmpty()){
    setCenterSprite(320,24,infoA,0,50);
    setCenterSprite(320,24,infoB,0,80);
    setCenterSprite(320,24,infoC,0,110);
  }else if (!infoB.isEmpty()){
    setCenterSprite(320,24,infoA,0,50);
    setCenterSprite(320,24,infoB,0,80);
  }else{
    setCenterSprite(320,24,infoA,0,80);
  }
}

void setCenterSprite(uint16_t w, uint16_t h, String text, uint16_t x, uint16_t y)
{
  sprite.createSprite(w,h);
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_RED);
  sprite.setFreeFont(&Inter_18pt_Bold10pt7b);
  int16_t width = (320 - sprite.textWidth(text)) / 2;
  sprite.drawString(text, width, 0);
  sprite.pushSprite(x,y);
  sprite.deleteSprite();
}

void showBaseBackGround(uint8_t wifi, uint8_t eth, uint8_t aws)
{
  ypos = 0;
  openPNG(kn_bg_op, sizeof(kn_bg_op));
  xpos = 290;
  if (wifi == 0)openPNG(wifi_bad, sizeof(wifi_bad));
  else openPNG(wifi_ok, sizeof(wifi_ok));
  xpos = 255;
  if (eth == 0)openPNG(eth_bad, sizeof(eth_bad));
  else openPNG(eth_ok, sizeof(eth_ok));
  xpos = 220;
  if (aws == 0)openPNG(aws_bad, sizeof(aws_bad));
  else openPNG(aws_ok, sizeof(aws_ok));
}

void showVersionFooter(char* version){
  sprite.createSprite(120, 20);
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_GREEN);
  sprite.setTextFont(1);
  sprite.drawString("FIRMWARE: 2.1.3.3", 10, 0);
  sprite.pushSprite(200, 160);
  sprite.deleteSprite();
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
