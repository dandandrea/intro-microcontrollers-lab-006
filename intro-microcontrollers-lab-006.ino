#include "Adafruit_MPU6050_counterfit.h"
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH_PIXELS  128
#define OLED_HEIGHT_PIXELS 64

#define OLED_I2C_ADDRESS 0x3C

Adafruit_SSD1306 oled(OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS, &Wire, -1);
Adafruit_MPU6050 mpu;

#define Z_FORCE_TO_RESET_DISPLAY 25

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

  oled.setTextSize(3);
  oled.clearDisplay();
  oled.display();

  Serial.println(F("[setup()] Done"));
}

void loop() {
  // put your main code here, to run repeatedly:

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  int16_t x = a.acceleration.x * 10;
  int16_t y = a.acceleration.y * 10;

  if (a.acceleration.z >= Z_FORCE_TO_RESET_DISPLAY)
  {
    for (uint16_t i = 0; i < 5; i++)
    {
      oled.invertDisplay(true);
      oled.display();
      delay(50);
      oled.invertDisplay(false);
      oled.display();
      delay(50);
    }

    // oled.clearDisplay();
    // oled.setCursor(0, 16);
    // oled.print(F(" CLEAR"));
    // oled.display();
    // delay(1000);
    oled.clearDisplay();
    oled.display();
  }

  if (x > 100) x = 100;
  if (x < -100) x = -100;
  if (y > 100) y = 100;
  if (y < -100) y = -100;

  x = (x + 100) * 0.315;
  y = (y + 100) * 0.635;

  oled.writePixel(y, x, WHITE);
  oled.display();

  delay(5);
}