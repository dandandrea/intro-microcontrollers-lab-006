#include "Adafruit_MPU6050_counterfeit.h"
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH_PIXELS  128
#define OLED_HEIGHT_PIXELS 64

#define OLED_I2C_ADDRESS 0x3C

Adafruit_SSD1306 oled(OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS, &Wire, -1);
Adafruit_MPU6050 mpu;

#define GAME_MAX_SECONDS 60
#define TARGET_MATCH_THRESHOLD_PIXELS 5

uint32_t score = 0;
uint32_t gameStartMilliseconds = 0;
uint8_t targetX = 0;
uint8_t targetY = 0;

void flashScreenNTimes(uint16_t numFlashes);
void gameReset(bool flashScreen);
void nextDotLocation();
int32_t getSecondsRemaining();
void redrawMainScreenArea();

#define PLAY_AREA_OFFSET_HEIGHT_PIXELS 18

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);

  Serial.println(F("[setup()] Starting"));

  delay(250);

  if (oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS))
  {
    Serial.println(F("[setup()] GOOD: OLED display initialized"));
  }
  else
  {
    while (true)
    {
      Serial.println(F("[setup()] ERROR: Unable to initialize OLED display"));
      delay(1000);
    }
  }

  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 16);
  oled.println(F("  Starting"));
  oled.display();

  delay(250);

  if (mpu.begin())
  {
    Serial.println(F("[setup()] GOOD: MPU6050 initialized"));
  }
  else
  {
    while (true)
    {
      Serial.println(F("[setup()] ERROR: Unable to initialize MPU6050"));
      delay(1000);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  oled.clearDisplay();
  oled.display();

  randomSeed(analogRead(0));

  nextDotLocation();
  gameReset(false);

  Serial.println(F("[setup()] Done"));
}

void loop() {
  // put your main code here, to run repeatedly:

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float x = a.acceleration.y;
  float y = a.acceleration.x;

  // ~ +/- 9.8 m/s^2 is a good enough metric for a full tilt on any axis
  // Bound to [-10, 10] for convenience of mapping to OLED display's coordinate system
  if (x > 10) x = 10;
  if (x < -10) x = -10;
  if (y > 10) y = 10;
  if (y < -10) y = -10;

  // Map x and y from [-10, 10] to [0, 20]
  x = (x + 10);
  y = (y + 10);

  // Need to account for play area starting at Y pixel = 18

  // Map X from [0, 20] to [0, 63 - 18] and y from [0, 20] to [0, 127] (minus 18 because we are not using the full height of the display for the play area)
  x = x * 6.35; // 20 * 6.35 = 127
  y = y * 2.25; // 20 * 2.25 = 45 = 63 - 18

  int16_t xInt = x;
  int16_t yInt = y;

  redrawMainScreenArea();
  oled.drawCircle(xInt, yInt + PLAY_AREA_OFFSET_HEIGHT_PIXELS, 2, WHITE);
  oled.drawCircle(targetX, targetY, 1, WHITE);
  oled.display();

  bool match = false;
  if (abs(xInt - targetX) <= TARGET_MATCH_THRESHOLD_PIXELS && abs((yInt + PLAY_AREA_OFFSET_HEIGHT_PIXELS) - targetY) <= TARGET_MATCH_THRESHOLD_PIXELS) match = true;

  if (match)
  {
    flashScreenNTimes(3);
    score++;
    nextDotLocation();

    redrawMainScreenArea();
    oled.drawCircle(xInt, yInt + PLAY_AREA_OFFSET_HEIGHT_PIXELS, 2, WHITE);
    oled.drawCircle(targetX, targetY, 1, WHITE);
    oled.display();
  }

  if (getSecondsRemaining() <= 0)
  {
    gameReset(true);
    nextDotLocation();

    redrawMainScreenArea();
    oled.drawCircle(xInt, yInt + PLAY_AREA_OFFSET_HEIGHT_PIXELS, 2, WHITE);
    oled.drawCircle(targetX, targetY, 1, WHITE);
    oled.display();
  }
}

void flashScreenNTimes(uint16_t numFlashes)
{
  for (uint16_t i = 0; i < numFlashes; i++)
  {
    oled.invertDisplay(true);
    oled.display();
    delay(50);
    oled.invertDisplay(false);
    oled.display();
    delay(50);
  }
}

void gameReset(bool flashScreen)
{
  if (flashScreen)
  {
    flashScreenNTimes(10);

    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.setCursor(0, 16);
    oled.println(F("Game Over"));
    oled.println(F(""));
    oled.print(F("Score: "));
    oled.print(score);
    oled.display();

    delay(2500);
  }

  gameStartMilliseconds = millis();
  score = 0;
}

void nextDotLocation()
{
  // +/- 2 so that we don't draw directly on the border
  targetX = (uint8_t)random(2, OLED_WIDTH_PIXELS - 2);
  targetY = (uint8_t)random(PLAY_AREA_OFFSET_HEIGHT_PIXELS + 2, OLED_HEIGHT_PIXELS - 2);

  uint8_t verticalCrosshairX0 = targetX;
  uint8_t verticalCrosshairY0 = PLAY_AREA_OFFSET_HEIGHT_PIXELS;

  uint8_t verticalCrosshairX1 = targetX;
  uint8_t verticalCrosshairY1 = OLED_HEIGHT_PIXELS;

  uint8_t horizontalCrosshairX0 = 0;
  uint8_t horizontalCrosshairY0 = targetY;

  uint8_t horizontalCrosshairX1 = OLED_WIDTH_PIXELS;
  uint8_t horizontalCrosshairY1 = targetY;

  redrawMainScreenArea();
  oled.drawLine(verticalCrosshairX0, verticalCrosshairY0, verticalCrosshairX1, verticalCrosshairY1, WHITE);
  oled.drawLine(horizontalCrosshairX0, horizontalCrosshairY0, horizontalCrosshairX1, horizontalCrosshairY1, WHITE);
  oled.display();

  delay(500);
}

void redrawMainScreenArea()
{
  oled.clearDisplay();
  oled.drawRect(0, PLAY_AREA_OFFSET_HEIGHT_PIXELS, OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS - PLAY_AREA_OFFSET_HEIGHT_PIXELS, WHITE);
  oled.setCursor(0, 0);
  oled.print(score);
  oled.setCursor(OLED_WIDTH_PIXELS / 2, 0);
  oled.print(getSecondsRemaining());
}

int32_t getSecondsRemaining()
{
  return GAME_MAX_SECONDS - ((millis() - gameStartMilliseconds) / 1000);
}