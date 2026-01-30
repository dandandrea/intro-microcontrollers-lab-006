#include "Adafruit_MPU6050_counterfeit.h"
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH_PIXELS  128
#define OLED_HEIGHT_PIXELS 64

#define OLED_I2C_ADDRESS 0x3C

#define SCREEN_AREA_OFFSET_HEIGHT_PIXELS 18

Adafruit_SSD1306 oled(OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS, &Wire, -1);
Adafruit_MPU6050 mpu;

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

  oled.clearDisplay();
  oled.drawRect(0, SCREEN_AREA_OFFSET_HEIGHT_PIXELS, OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS - SCREEN_AREA_OFFSET_HEIGHT_PIXELS, WHITE);
  oled.drawCircle(xInt, yInt + SCREEN_AREA_OFFSET_HEIGHT_PIXELS, 2, WHITE);
  oled.display();
}