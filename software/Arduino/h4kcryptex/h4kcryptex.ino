/*
    Hak4Kidz 2019 Tyro badge sketch for DEF CON 27 and BSides Las Vegas.

    Version 0.5 of sketch was used for final code at Hak4Kidz Chicago.
    Version 0.6 of sketch was combining core code, Cryptex CLI, and sponsors.
    Version 0.75 added more to the Cryptex CLI
    Version 0.85 added the improved light show with the LEDs
    Version 0.90 some code clean up and passed beta testing

    Contributors: Michael Whitely, Dave Schwartzberg, various Internet resources such as Bodmer & Adafruit.com.
    Handle: @compukidmike, @DSchwartzberg, @Hak4Kidz
*/

// declare included libraries
#include <Wire.h>
#include <pgmspace.h>
#include "SPIFFS.h"
#include "SPI.h"
#include <FS.h>
#include <TFT_eSPI.h>
#include "game.h"

// define constant values for LEDs
#define FS_NO_GLOBALS
#define Addr_VCC 0x78 //7 bit format is 0x3F;;

//drawing and font color constant values
#define ILI9341_BLACK 0x0000
#define ILI9341_BLUE  0x001F
#define ILI9341_PINK  0XEBD5
#define ILI9341_RED   0xF800
#define ILI9341_WHITE 0xFFFF

#define D3R 0x7
#define D3G 0x8
#define D3B 0x9
#define D4R 0xa
#define D4G 0xb
#define D4B 0xc
#define D5R 0xd
#define D5G 0xe
#define D5B 0xf
#define D6R 0x10
#define D6G 0x11
#define D6B 0x12
#define D7R 0x13
#define D7G 0x14
#define D7B 0x15
#define D8R 0x16
#define D8G 0x17
#define D8B 0x18
#define D9  25  // left button
#define D10 26  // right button
#define D11 27  // First H4K chest light
#define D12 28
#define D13 29
#define D14 30
#define D15 31
#define D16 32  // Last H4K chest light

// Create task handlers
//TaskHandle_t LCD = NULL;
TaskHandle_t Cryptex = NULL;


// beginning of pong demo variables
int16_t h = 240;
int16_t w = 320;

int dly = 5;

int16_t paddle_h = 30;
int16_t paddle_w = 4;

int16_t lpaddle_x = 0;
int16_t rpaddle_x = w - paddle_w;

int16_t lpaddle_y = 0;
int16_t rpaddle_y = h - paddle_h;

int16_t lpaddle_d = 1;
int16_t rpaddle_d = -1;

int16_t lpaddle_ball_t = w - w / 4;
int16_t rpaddle_ball_t = w / 4;

int16_t target_y = 0;

int16_t ball_x = 2;
int16_t ball_y = 2;
int16_t oldball_x = 2;
int16_t oldball_y = 2;

int16_t ball_dx = 1;
int16_t ball_dy = 1;

int16_t ball_w = 6;
int16_t ball_h = 6;

int16_t dashline_h = 4;
int16_t dashline_w = 2;
int16_t dashline_n = h / dashline_h;
int16_t dashline_x = w / 2 - 1;
int16_t dashline_y = dashline_h / 2;

int16_t lscore = 12;
int16_t rscore = 4;
// end of pong demo variables

// a variable to hold a random number
long randomNumber;

// assign temporary values to variables
int x;
int dim = 50;
int rotation = 3;
int threshold = 40;
int touchcounter = 0;
int displayCounter = 0;
bool touch1detected = false;
bool touch2detected = false;
bool touch3detected = false;
bool touch4detected = false;
bool touch5detected = false;
bool touch6detected = false;

// assign tft to the TFT_eSPI library
// moved to game.h to use in both
//TFT_eSPI tft = TFT_eSPI();

// begin starfield classes and variables

// 1bpp Sprites are economical on memory but slower to render
#define COLOR_DEPTH 1  // Colour depth (1, 8 or 16 bits per pixel)

// Rotation angle increment and start angle for starfield
#define ANGLE_INC 3
int angle = 0;

TFT_eSprite img = TFT_eSprite(&tft);  // Sprite class

#define RADIUS 40      // Radius of completed symbol = 40
#define WAIT 0         // Loop delay

// With 1024 stars the update rate is ~65 frames per second
#define NSTARS 1024
uint8_t sx[NSTARS] = {};
uint8_t sy[NSTARS] = {};
uint8_t sz[NSTARS] = {};

uint8_t za, zb, zc, zx;

// Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
uint8_t __attribute__((always_inline)) rng()
{
  zx++;
  za = (za ^ zc ^ zx);
  zb = (zb + za);
  zc = (zc + (zb >> 1)^za);
  return zc;
}

// end starfield variables

//  Create a task as a function to run on Core 0
void Cryptexcode(void * pvParameters) {
  //while (1) {
  disableCore0WDT();
  //TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
  //TIMERG0.wdt_feed = 1;
  //TIMERG0.wdt_wprotect = 0;
  for (;;) {
    vTaskDelay(10);
    badge_cli();

  }
  if (Cryptex != NULL) {
    vTaskDelete(Cryptex);
  }
  //}
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);             // address the serial interface and tell it the speed
  Wire.begin(16, 17);               // tell MCU which pins to assdress the LED driver with
  Wire.setClock(400000);            // set I2C to 400kHz
  Wire.beginTransmission(60);       // start sending data to the LED driver
  Wire.write(0);                    //Register 0
  Wire.write(1);                    //Enable LED Driver PWM Value Register
  Wire.endTransmission();           // stop sending data to the LED driver

  randomSeed(1337);

  // setting the interrupt pins
  Serial.println("");
  Serial.println("\n\nMonitoring interrupts: ");
  //touchAttachInterrupt(0, gotTouch1, threshold);   // TP1 disabled due to USB short
  touchAttachInterrupt(4, gotTouch2, threshold);     // TP2
  touchAttachInterrupt(2, gotTouch3, threshold);     // TP3
  touchAttachInterrupt(27, gotTouch4, threshold);    // TP4
  touchAttachInterrupt(32, gotTouch5, threshold);    // TP5
  touchAttachInterrupt(33, gotTouch6, threshold);    // TP6

  // Read sponsor files
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
    while (1) yield(); // Stay here and wait
    //return;
  }
  Serial.println("SPIFFS initialization complete.");

  File file = SPIFFS.open("/sekritdata.txt");
  if (!file) {
    Serial.println("Failed to open file for reading.");
    return;
  }

  Serial.println("Firmware checksum:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();

  tft.begin();                       // enable the LCD using tft
  tft.setRotation(3);                // change LCD orientation to be readable on badge

  // random variable assignment for starfield
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);

  // prepare display for use
  tft.fillScreen(ILI9341_WHITE);               // clears tft to be white
  tft.setTextColor(ILI9341_BLACK);             // text will be written in black
  tft.println(" Booting ESP32 USB power / H4K 2019 Badge...\n");
  delay(500);

  Wire.beginTransmission(60);
  Wire.write(1);
  for (x = 0; x < 32; x++) {
    Wire.write(0);                   // Turn all LEDs off
  }
  for (x = 0; x < 4; x++) {
    Wire.write(127);                 // Turn LCD backlight on half brightness
  }
  Wire.write(0);                     //PWM Update
  for (x = 0; x < 36; x++) {
    Wire.write(1);                   // Turn all LED Controls to on, max current
  }
  Wire.endTransmission();

  randomSeed(digitalRead(0x00));     // used to reduce predictability.

  int x, y, w = tft.width(), h = tft.height();
  tft.fillRect(0, 0, w, 10, ILI9341_WHITE);    // clear USB Boot message
  tft.print("Initializing display... ");
  delay(300);
  tft.print("Done!\n\n\n");
  delay(750);

  tft.print("Initializing illumination... ");
  delay(300);

  Init_FL3236A();                              // initialize the LED Driver
  tft.print("Done!\n\n\n");
  delay(300);

  tft.println("Initialization complete.\n\n");
  delay(750);

  tft.setTextColor(ILI9341_WHITE);
  splash();                                    // who are we?
  delay(750);

  tft.fillScreen(ILI9341_BLACK);               // clears tft to be black

  LBlueEye();                                  // turn on the left eye LED

  RBlueEye();                                  // mturn on the right eye LED

  Buttons();                                   // turn on the button LEDs
  H4Kon();                                       // turn on the chest LEDs
  delay(250);
  LBlueEyeClose();
  delay(25);
  LBlueEye();

  // onboard cryptex game init
  cli_init();

  // create a task that will be executed in the Cryptexcode() function with
  // priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Cryptexcode,  /* Task function */
    "Cryptex",    /* name of the task */
    10000,    /* stack size of the task */
    NULL,     /* parameter of the task */
    1,        /* task priority */
    &Cryptex,     /* task handler to keep track of created task */
    0);       /* pin task to core 0 */
  delay(500);

}  // end setup

void ArmPad(void)
{
  //uint8_t m = 0;
  int8_t n = 0;

  //assign a random number to variable
  randomNumber = random(7, 25);              // find a pseudo random number between 7 and 24

  Wire.beginTransmission(60);

  for (n = 0; n <= 63; n++)                   // all LED lights go on
  {
    // ArmPad RGB LEDs
    IS_IIC_WriteByte(Addr_VCC, randomNumber, PWM_Gamma64[n]); // set  random  PWM
  }
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);    // update  PWM  &  congtrol  registers
  delay(20);                                 // 20ms delay

  randomSeed(digitalRead(0x00));
  randomNumber = random(7, 25);              // find a pseudo random number between 7 and 24

  for (n = 63; n >= 0; n--)                  // all LED lights go off
  {
    IS_IIC_WriteByte(Addr_VCC, randomNumber, PWM_Gamma64[n]); //set  D9  PWM

    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00); // update  PWM  &  congtrol  registers
    delay(20);
  }
}  // end ArmPad

void loop() {
  // put your main code here, to run repeatedly:
  LBlueEye();                                  // turn on the left eye LED
  RBlueEye();                                  // mturn on the right eye LED
  Buttons();                                   // turn on the button LEDs
  H4Kon();                                       // turn on the chest LEDs
  ArmPad();

  if (displayCounter < 301) {

    menu();
    displayCounter++;
    delay(1000);  // 1 second delay to make the idle wait any duration
  }
  else {
    // launch sponsor logos
    ArmPad();
    sponsors();
  }

  // checking for when a touch pad is touched.
  // this should be a case statement
  if (touch1detected && touchcounter > 1)
  {
    touch1detected = false;
    touchcounter = 0;
    //Serial.println("Touch 1 detected");
  }

  if (touch2detected && touchcounter > 1)
  {
    touch2detected = false;
    touchcounter = 0;
    //Serial.println("Touch 2 detected");
    tft.fillScreen(ILI9341_BLACK);               // clears tft to be black
    menu();
  }

  if (touch3detected && touchcounter > 1)
  {
    touch3detected = false;
    touchcounter = 0;
    //Serial.println("Touch 3 detected");
    lights();
  }

  if (touch4detected && touchcounter > 1)
  {
    touch4detected = false;
    touchcounter = 0;
    //Serial.println("Touch 4 detected");
    pong();
  }

  if (touch5detected && touchcounter > 1)
  {
    touch5detected = false;
    touchcounter = 0;
    //Serial.println("Touch 5 detected");
    //draw();
    sponsors();
  }

  if (touch6detected && touchcounter > 1)
  {
    touch6detected = false;
    touchcounter = 0;
    //Serial.println("Touch 6 detected");
    about();
  }

  // tft.drawPixel(200, 200, ILI9341_PINK);                 // example how to draw a pink single pixel
}  // end loop


void menu() {
  tft.setCursor(15, 0);
  tft.setTextSize(2);
  tft.println("Hak4Kidz Cryptex Badge\n");
  //tft.setTextSize(2);
  //tft.setCursor(0, 0);
  tft.println("\n[E] Canceled by DEF CON");
  tft.println("[S] Return to menu");
  tft.println("[R] Light Show");
  tft.println("[W] Pong Demo");
  tft.println("[H] Believers");
  tft.println("[A] Contributors\n\n");
  // Customize Black Badge Owner's Name
  tft.print("Owner: ");
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.println("Compukidmike");
   tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  //tft.fillScreen(ILI9341_BLACK);
  //displayCounter = 0;
}

// *************************
// *         PONG          *
// *************************
void pong()
{
  /*

    Pong
    Original Code from https://github.com/rparrett/pongclock

  */

  // Demo only - not playable
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREY  0x5AEB

  int ballcount = 0;

  //TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
  tft.fillScreen(ILI9341_BLACK);
  //tft.fillScreen(GREY);
  tft.setTextSize(1);

  initgame();

  tft.setTextColor(WHITE, BLACK);

  do
  {
    delay(dly);

    lpaddle();
    rpaddle();

    midline();

    ball();
  } while (!touch2detected);
  tft.fillScreen(ILI9341_BLACK);
}

//
// beginning of pong demo functions
//
void initgame() {
  lpaddle_y = random(0, h - paddle_h);
  rpaddle_y = random(0, h - paddle_h);

  // ball is placed on the center of the left paddle
  ball_y = lpaddle_y + (paddle_h / 2);

  calc_target_y();

  midline();

  tft.fillRect(0, h - 26, w, 239, GREY);

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(WHITE, GREY);
  tft.drawString("Pong Demo Only S 2 Exit", w / 2, h - 26 , 4);
}

void midline() {

  // If the ball is not on the line then don't redraw the line
  if ((ball_x < dashline_x - ball_w) && (ball_x > dashline_x + dashline_w)) return;

  tft.startWrite();

  // Quick way to draw a dashed line
  tft.setAddrWindow(dashline_x, 0, dashline_w, h);

  for (int16_t i = 0; i < dashline_n; i += 2) {
    tft.pushColor(WHITE, dashline_w * dashline_h); // push dash pixels
    tft.pushColor(BLACK, dashline_w * dashline_h); // push gap pixels
  }

  tft.endWrite();
}

void lpaddle() {

  if (lpaddle_d == 1) {
    tft.fillRect(lpaddle_x, lpaddle_y, paddle_w, 1, BLACK);
  }
  else if (lpaddle_d == -1) {
    tft.fillRect(lpaddle_x, lpaddle_y + paddle_h - 1, paddle_w, 1, BLACK);
  }

  lpaddle_y = lpaddle_y + lpaddle_d;

  if (ball_dx == 1) lpaddle_d = 0;
  else {
    if (lpaddle_y + paddle_h / 2 == target_y) lpaddle_d = 0;
    else if (lpaddle_y + paddle_h / 2 > target_y) lpaddle_d = -1;
    else lpaddle_d = 1;
  }

  if (lpaddle_y + paddle_h >= h && lpaddle_d == 1) lpaddle_d = 0;
  else if (lpaddle_y <= 0 && lpaddle_d == -1) lpaddle_d = 0;

  tft.fillRect(lpaddle_x, lpaddle_y, paddle_w, paddle_h, WHITE);
}

void rpaddle() {

  if (rpaddle_d == 1) {
    tft.fillRect(rpaddle_x, rpaddle_y, paddle_w, 1, BLACK);
  }
  else if (rpaddle_d == -1) {
    tft.fillRect(rpaddle_x, rpaddle_y + paddle_h - 1, paddle_w, 1, BLACK);
  }

  rpaddle_y = rpaddle_y + rpaddle_d;

  if (ball_dx == -1) rpaddle_d = 0;
  else {
    if (rpaddle_y + paddle_h / 2 == target_y) rpaddle_d = 0;
    else if (rpaddle_y + paddle_h / 2 > target_y) rpaddle_d = -1;
    else rpaddle_d = 1;
  }

  if (rpaddle_y + paddle_h >= h && rpaddle_d == 1) rpaddle_d = 0;
  else if (rpaddle_y <= 0 && rpaddle_d == -1) rpaddle_d = 0;

  tft.fillRect(rpaddle_x, rpaddle_y, paddle_w, paddle_h, WHITE);
}

void calc_target_y() {
  int16_t target_x;
  int16_t reflections;
  int16_t y;

  if (ball_dx == 1) {
    target_x = w - ball_w;
  }
  else {
    target_x = -1 * (w - ball_w);
  }

  y = abs(target_x * (ball_dy / ball_dx) + ball_y);

  reflections = floor(y / h);

  if (reflections % 2 == 0) {
    target_y = y % h;
  }
  else {
    target_y = h - (y % h);
  }
}

void ball() {
  ball_x = ball_x + ball_dx;
  ball_y = ball_y + ball_dy;

  if (ball_dx == -1 && ball_x == paddle_w && ball_y + ball_h >= lpaddle_y && ball_y <= lpaddle_y + paddle_h) {
    ball_dx = ball_dx * -1;
    dly = random(5); // change speed of ball after paddle contact
    calc_target_y();
  } else if (ball_dx == 1 && ball_x + ball_w == w - paddle_w && ball_y + ball_h >= rpaddle_y && ball_y <= rpaddle_y + paddle_h) {
    ball_dx = ball_dx * -1;
    dly = random(5); // change speed of ball after paddle contact
    calc_target_y();
  } else if ((ball_dx == 1 && ball_x >= w) || (ball_dx == -1 && ball_x + ball_w < 0)) {
    dly = 5;
  }

  if (ball_y > h - ball_w || ball_y < 0) {
    ball_dy = ball_dy * -1;
    ball_y += ball_dy; // Keep in bounds
  }

  //tft.fillRect(oldball_x, oldball_y, ball_w, ball_h, BLACK);
  tft.drawRect(oldball_x, oldball_y, ball_w, ball_h, BLACK); // Less TFT refresh aliasing than line above for large balls
  tft.fillRect(   ball_x,    ball_y, ball_w, ball_h, WHITE);
  oldball_x = ball_x;
  oldball_y = ball_y;
}
//
// end of pong demo functions
//

void starfield()
{
  img.setColorDepth(COLOR_DEPTH);
  img.createSprite(RADIUS * 2 + 1, RADIUS * 2 + 1);
  img.fillSprite(TFT_BLACK);

  uint8_t spawnDepthVariation = 255;

  for (int i = 0; i < NSTARS; ++i)
  {
    if (sz[i] <= 1)
    {
      sx[i] = 160 - 120 + rng();
      sy[i] = rng();
      sz[i] = spawnDepthVariation--;
    }
    else
    {
      int old_screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
      int old_screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;

      // This is a faster pixel drawing function for occassions where many single pixels must be drawn
      tft.drawPixel(old_screen_x, old_screen_y, TFT_BLACK);

      sz[i] -= 2;
      if (sz[i] > 1)
      {
        int screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
        int screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;

        if (screen_x >= 0 && screen_y >= 0 && screen_x < 320 && screen_y < 240)
        {
          uint8_t r, g, b;
          r = g = b = 255 - sz[i];
          tft.drawPixel(screen_x, screen_y, tft.color565(r, g, b));
        }
        else
          sz[i] = 0; // Out of screen, die.
      }

    }
    
    // Draw Yin and Yang symbol circles into Sprite
    // Saving for future feature
    //    yinyang(RADIUS, RADIUS, angle, RADIUS);
    //
    //    // Set the 2 pixel palette colours that 1 and 0 represent on the display screen
    //    img.setBitmapColor(TFT_WHITE, TFT_BLACK);
    //
    //    // Push Sprite image to the TFT screen at x, y
    //    img.pushSprite(tft.width() / 2 - RADIUS, 0); // Plot sprite
    //
    //    angle += 3;                 //Increment angle to rotate circle positions
    //    if (angle > 359) angle = 0; // Limit angle range
    //
    //    // Slow things down
    //    delay(WAIT);
  }

}

// a touch pad response - shout outs
void about()
{
  ArmPad();
  int yMin = 0;
  int xMin = 0;
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setCursor((yMin + 25), xMin);
  tft.println("Shouts to contributers\n\n");
  delay(1000);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(NULL);
  tft.println("Heal - design, cryptex art, code, logistics \(312\)\n");
  delay(1000);
  ArmPad();
  tft.println("Compukidmike - board design\n");
  delay(2000);
  tft.println("Mz Bat - Tinker \(robot\) art\n");
  delay(1000);
  ArmPad();
  tft.print("TwinkleTwinkie, supersat, & \#badgelife folks -\n");
  tft.println("for advice, solutions, and mentorship\n");
  delay(2000);
  tft.print("Bodmer, Adafruit, and other Internet resources\n");
  tft.println("for libraries, sample code, forums, and saving time\n");
  delay(2000);
  ArmPad();
  tft.println("Coffee and black cherry flavored Mio\n");
  delay(2000);
  tft.println("Heal's wife and kids for their patience \(312\)\n");
  ArmPad();
  delay(3000);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  displayCounter = 0;
}

// run through all the LEDs and then fade out
void IS31FL3236A_mode1(void)
{
  uint8_t i = 0;
  int8_t j = 0;

  Wire.beginTransmission(60);

  for (i = 1; i < 13; i++)
  {
    IS_IIC_WriteByte(Addr_VCC, (i * 3 - 1), 0xff); // set PWM
    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);      // update PWM & congtrol  registers
    delay(100);
  }
  delay(50);

  for (i = 12; i > 0; i--)                       // LED  running
  {
    IS_IIC_WriteByte(Addr_VCC, (i * 3 - 2), 0xff); // set PWM
    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);      // update PWM & congtrol  registers
    delay(100);
  }
  delay(50);

  for (i = 1; i < 12; i++)                          // LED  running
  {
    IS_IIC_WriteByte(Addr_VCC, (i * 3 - 0), 0xff);  // set PWM
    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);         // update PWM & congtrol  registers
    delay(100);
  }
  delay(500);

  for (j = 63; j >= 0; j--)                         // all LED breath falling
  {
    for (i = 1; i < 33; i++)
    {
      IS_IIC_WriteByte(Addr_VCC, i, PWM_Gamma64[j]); // fade out
    }
    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);         //update  PWM  &  congtrol  registers
    delay(50);
  }
  for (j = 0; j <= 63; j++)                         // all LED breath falling
  {
    IS_IIC_WriteByte(Addr_VCC, 0x19, PWM_Gamma64[j]); // set  D9  PWM
    IS_IIC_WriteByte(Addr_VCC, 0x1a, PWM_Gamma64[j]); // set  D10  PWM

    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);         // update PWM & congtrol  registers
    delay(50);
  }
  delay(500);
  Wire.endTransmission();
}


// function to initialize the LED drivers and light show
void Init_FL3236A(void)
{
  uint8_t i = 0;

  //prepare ILI9341 for use
  for (i = 0x26; i <= 0x49; i++)
  {
    IS_IIC_WriteByte(Addr_VCC, i, 0xff);      // turn  on  all  LED
  } delay(1000);

  for (i = 0x01; i <= 0x24; i++)
  {
    IS_IIC_WriteByte(Addr_VCC, i, 0x00);        // write  all  PWM  set  0x00
  }

  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);     // update PWM & congtrol registers
  IS_IIC_WriteByte(Addr_VCC, 0x4B, 0x01);     // frequency  setting  22KHz
  IS_IIC_WriteByte(Addr_VCC, 0x00, 0x01);     // normal  operation

  IS31FL3236A_mode1();                        // breath of all the LEDs mode

}

void LRedEye(void)
{
  // set eyes red; not used
  IS_IIC_WriteByte(Addr_VCC, 0x01, 0xff);     //set  D1 red
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void RRedEye(void)
{
  // set eyes red
  IS_IIC_WriteByte(Addr_VCC, 0x04, 0xff);     // set  D2 red
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void LRedEyeClose(void)
{
  // set eyes red; not used
  IS_IIC_WriteByte(Addr_VCC, 0x01, 0x00);     //set  D1 red
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void RRedEyeClose(void)
{
  // set eyes red
  IS_IIC_WriteByte(Addr_VCC, 0x04, 0x00);     // set  D2 red
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void Buttons(void)
{
  IS_IIC_WriteByte(Addr_VCC, 0x19, 0xff);     // set  D9  PWM
  IS_IIC_WriteByte(Addr_VCC, 0x1a, 0xff);     // set  D10  PWM
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void ButtonsOff(void)
{
  IS_IIC_WriteByte(Addr_VCC, 0x19, 0x00);     // set  D9  PWM
  IS_IIC_WriteByte(Addr_VCC, 0x1a, 0x00);     // set  D10  PWM
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void H4Kon(void)
{
  int8_t h = 0;

  for (h = 0x1b; h < 0x21; h++)
  {
    IS_IIC_WriteByte(Addr_VCC, h, 0xff);     // set  D11 - D16  PWM
  }
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void H4Koff(void)
{
  IS_IIC_WriteByte(Addr_VCC, D11, 0x00);
  IS_IIC_WriteByte(Addr_VCC, D12, 0x00);
  IS_IIC_WriteByte(Addr_VCC, D13, 0x00);
  IS_IIC_WriteByte(Addr_VCC, D14, 0x00);
  IS_IIC_WriteByte(Addr_VCC, D15, 0x00);
  IS_IIC_WriteByte(Addr_VCC, D16, 0x00);
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void Hon(void)
{
  IS_IIC_WriteByte(Addr_VCC, D11, 0xff);
  IS_IIC_WriteByte(Addr_VCC, D12, 0xff);
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void Hoff(void)
{
  IS_IIC_WriteByte(Addr_VCC, D11, 0x00);
  IS_IIC_WriteByte(Addr_VCC, D12, 0x00);
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void Fouron(void)
{
  IS_IIC_WriteByte(Addr_VCC, D13, 0xff);
  IS_IIC_WriteByte(Addr_VCC, D14, 0xff);
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void Fouroff(void)
{
  IS_IIC_WriteByte(Addr_VCC, D13, 0x00);
  IS_IIC_WriteByte(Addr_VCC, D14, 0x00);
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void Kon(void)
{
  IS_IIC_WriteByte(Addr_VCC, D15, 0xff);
  IS_IIC_WriteByte(Addr_VCC, D16, 0xff);
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void Koff(void)
{
  IS_IIC_WriteByte(Addr_VCC, D15, 0x00);
  IS_IIC_WriteByte(Addr_VCC, D16, 0x00);
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

void discoArm(void)
{
  //uint8_t m = 0;
  int8_t n = 0;

  //assign a random number to variable
  randomNumber = random(7, 25);              // find a pseudo random number between 7 and 24

  Wire.beginTransmission(60);
  randomNumber = random(7, 25);

  for (n = 0; n <= 63; n++)                   // all LED lights go on
  {
    // discoArm RGB LEDs
    IS_IIC_WriteByte(Addr_VCC, randomNumber, PWM_Gamma64[n]); // set  random  PWM
  }
  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);    // update  PWM  &  congtrol  registers
  //delay(5);                                 // 20ms delay

  for (n = 63; n >= 0; n--)                  // all LED lights go off
  {
    IS_IIC_WriteByte(Addr_VCC, randomNumber, PWM_Gamma64[n]); //set  D9  PWM

    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00); // update  PWM  &  congtrol  registers
    //delay(5);
  }
  delay(50);
}  // end DiscoArm

void aroundTheBases(void)
{
  // the lights will run alternating on and off
  IS_IIC_WriteByte(Addr_VCC, 0x19, 0xff);     // left button on
  delay(200);
  Hon();
  delay(200);
  IS_IIC_WriteByte(Addr_VCC, 0x19, 0x00);     // left button off
  delay(200);
  Fouron();
  delay(200);
  Hoff();
  delay(200);
  Kon();
  delay(200);
  Fouroff();
  delay(200);
  IS_IIC_WriteByte(Addr_VCC, 0x1a, 0xff);     // right button on
  delay(200);
  Koff();
  delay(200);
  RRedEye();
  delay(200);
  IS_IIC_WriteByte(Addr_VCC, 0x1a, 0x00);     // right button off
  delay(200);
  RRedEyeClose();
  delay(200);
  LRedEye();
  delay(200);
  LRedEyeClose();
  delay(200);
  for (int i = 0; i < 20; i++) {
    discoArm();
  }

  IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);
}

// function to display the splash screen
void splash(void)
{
  drawBmp("/h4k.bmp", 1, 1);
  delay(2000);
  tft.fillScreen(ILI9341_BLACK);
}

// a touch pad response
void lights()
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(20, 100);
  tft.setTextColor(ILI9341_RED);
  tft.println("Enjoy the light show!\n\n\n");
  tft.setTextColor(ILI9341_YELLOW);
  tft.println("Boots and cats and boots and cats and boots and cats and boots and cats and boots and cats boots and cats and boots and cats and boots and cats");
  for (int j = 63; j >= 0; j--)                    // all LED breath falling
  {
    for (int i = 1; i < 33; i++)
    {
      IS_IIC_WriteByte(Addr_VCC, i, PWM_Gamma64[j]); // fade out
    }
    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);         //update  PWM  &  congtrol  registers
    delay(50);
  }
  for (int j = 0; j <= 63; j++)                     //all LED fading out
  {
    IS_IIC_WriteByte(Addr_VCC, 0x19, PWM_Gamma64[j]); // set  D9  PWM
    IS_IIC_WriteByte(Addr_VCC, 0x1a, PWM_Gamma64[j]); // set  D10  PWM

    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);         // update  PWM  &  congtrol  registers
    delay(50);
  }

  //Wire.endTransmission(60);
  // blinking techno lights
  // Start with a steady beat
  for (int y = 0; y < 9; y++) {
    H4Koff();
    delay(250);
    H4Kon();
    delay(250);
  }

  // light chase
  ButtonsOff();
  delay(500);
  for (int y = 0; y < 3; y++) {
    aroundTheBases();
  }

  // switch between buttons and eyes
  for (int x = 0; x < 6; x++) {
    ButtonsOff();
    Hon();
    LRedEye();
    RRedEye();
    delay(200);
    discoArm();
    ///
    Buttons();
    LRedEyeClose();
    RRedEyeClose();
    Hoff();
    Fouron();
    delay(200);
    discoArm();
    //
    ButtonsOff();
    Fouroff();
    Kon();
    LRedEye();
    RRedEye();
    delay(200);
    discoArm();
    //
    Buttons();
    Koff();
    LRedEyeClose();
    RRedEyeClose();
    delay(200);
    discoArm();
    //
    ButtonsOff();
    LRedEye();
    RRedEye();
    delay(200);
    discoArm();
    //
    Buttons();
    LRedEyeClose();
    RRedEyeClose();
    delay(200);
    discoArm();
    //
    LRedEye();
    RRedEye();
    for (int y = 0; y < 5; y++) {
      H4Kon();
      delay(100);
      H4Koff();
      delay(100);
    }

    for (int y = 0; y < 9; y++) {
      H4Kon();
      delay(25);
      H4Koff();
      delay(25);
    }
  }
  delay(500);
discoArm();
  for (int y = 0; y < 20; y++)
  {
    H4Koff();
    delay(50);
    H4Kon();
    delay(50);
    H4Koff();
    delay(50);
    H4Kon();
  }
discoArm();
  for (int x = 0; x < 25; x++) {
    discoArm();
    if (x == 3) {
      H4Koff();
    }
    else if (x == 6) {
      H4Kon();
    }
    else if (x == 9) {
      H4Koff();
    }
    else if (x == 12) {
      H4Kon();
    }
    else if (x == 15) {
      H4Koff();
    }
    else if (x == 18) {
      H4Kon();
    }
    else if (x == 21) {
      H4Koff();
    }
  }

  Wire.endTransmission();
  Init_FL3236A();
  //splash();
  delay(500);
  tft.fillScreen(ILI9341_BLACK);
}


// this section is for the touch pad interrupt ISR
// this should really be a case statement
void gotTouch1()
{
  touch1detected = true;
  touchcounter++;
}

void gotTouch2()
{
  touch2detected = true;
  touchcounter++;
}

void gotTouch3()
{
  touch3detected = true;
  touchcounter++;
}
void gotTouch4()
{
  touch4detected = true;
  touchcounter++;
}
void gotTouch5()
{
  touch5detected = true;
  touchcounter++;
}

void gotTouch6()
{
  touch6detected = true;
  touchcounter++;
}


// *********************************************
// **                                          *
// **           BMP Functions                  *
// **                                          *
// *********************************************
//

// Bodmers BMP image rendering function

void drawBmp(const char *filename, int16_t x, int16_t y) {

  if ((x >= tft.width()) || (y >= tft.height())) return;

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS)
  {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();

  Wire.beginTransmission(60);

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      // Removed because it interferred with CLI game
      //Serial.print("Loaded in "); Serial.print(millis() - startTime);
      //Serial.println(" ms");
    }
    else {
      LBlueEyeClose();
      delay(100);
      LBlueEye();
    }
  }
  bmpFS.close();
  Wire.endTransmission(60);
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}


// *********************************************
// **                                          *
// **             SPONSOR LOGOS                *
// **                                          *
// *********************************************
//

void sponsors()
{
  //  scroll through the sponsor logos
  displayCounter = 0;
  do {
    ArmPad();                                    // start the ArmPad function
    drawBmp("/grrcon.bmp", 0, 0);
    delay(1500);
    ArmPad();
    drawBmp("/cyphercon.bmp", 0, 0);
    delay(1500);
    ArmPad();
    drawBmp("/cgs.bmp", 0, 0);
    delay(1500);
    ArmPad();
    drawBmp("/amc.bmp", 0, 0);
    delay(1500);
    ArmPad();
    drawBmp("/isn_netdoc.bmp", 0, 0);
    delay(1500);
    ArmPad();
    drawBmp("/ss_mhh.bmp", 0, 0);
    delay(1500);
    ArmPad();
    drawBmp("/kas_hk.bmp", 0, 0);
    delay(1500);
    ArmPad();
    drawBmp("/cnd.bmp", 0, 0);
    delay(1500);
  } while (!touch2detected);
}
