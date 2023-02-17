/* Author: Mason Lopez
   Created: 12/15/2021
   Purpose: This code creates a simple interface for displaying data on small screen
*/

#include <Arduino.h>
#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>
#include "Org_01.h"

// joystick inputs i used
const int right_joy_x = 36;
const int right_joy_y = 39;
const int left_joy_x = 35;
const int left_joy_y = 34;
const int right_button = 23;
const int left_button = 22;

// outside circle calculations
byte O_x_radius_pixel[180];
byte O_y_radius_pixel[180];

// outside inside circle calculations
byte Oin_x_radius_pixel[180];
byte Oin_y_radius_pixel[180];


// inside circle calculations
byte I_x_radius_pixel[180];
byte I_y_radius_pixel[180];

// inside inside circle calculations
byte Iin_x_radius_pixel[180];
byte Iin_y_radius_pixel[180];

// the center of the circles of the guages
#define X_CENTER 63
#define Y_CENTER 63

// radius of the circles
#define O_CIRCLE_RADIUS 49      // outside (larger)
#define I_CIRCLE_RADIUS 29      // inside (smaller)


// Parameters for the ssd1306 , 128x 64 deep
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// change these values for your specific sensors!
#define SENSOR_MIN_VAL  1890    // CHANGE BASED ON INPUT OF SENSOR, THIS IS SET TO MIDDLE SINCE JOYSTICK GOES 0-4095 AND MIDDLE IS 1890 ABOUT
#define SENSOR_MAX_VAL  4095    // CHANGE BASED ON MAX RESOLUTION OF SENSOR
#define RAD_PI          180     // DONT CHANGE!
#define RAD_0           0       // DONT CHANGE!


// draws the circles on the screen
void drawCircles();

// calculates the radius of circles
void calculateRadiusPixels();

void setup()
{

  Serial.begin(115200);                                        // begin serial communications

  // set joysticks as inputs
  pinMode(right_joy_x, INPUT);
  pinMode(right_joy_y, INPUT);

  // pullup for the button pins
  pinMode(left_button, INPUT_PULLUP);
  pinMode(right_button, INPUT_PULLUP);

  // change the sda and sck pins for oled ( i used different pins then standard esp32 config)
  Wire.begin(17, 16);

  // if the display does not intialize then cause an error
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.clearDisplay();                                   // clear display
  display.setTextSize(1);
  display.setFont(&Org_01);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  calculateRadiusPixels();
  // draw the outlines of the guages one time
  drawCircles();
}

void loop()
{

  int rightValue = analogRead(right_joy_x);
  if (rightValue < 1890) // incase the values go over hardcode the min
  {
    rightValue = 1890;
  }
    
    
  int angleValue = map(rightValue, SENSOR_MIN_VAL, SENSOR_MAX_VAL, RAD_0, RAD_PI);         // GO ABOVE TO CHANGE SENSOR_XXX_VAL!

  for (int i = 0; i < angleValue; i++)                                                                                             // based on mapped value(angleValue) loop and draw lines
  {
    display.drawLine(Oin_x_radius_pixel[i], Oin_y_radius_pixel[i], Iin_x_radius_pixel[i], Iin_y_radius_pixel[i], SSD1306_WHITE);
  }
  for (int i = 179; i >= angleValue; i--)
  {   
    display.drawLine(Oin_x_radius_pixel[i], Oin_y_radius_pixel[i], Iin_x_radius_pixel[i], Iin_y_radius_pixel[i], SSD1306_BLACK);
  }
  static unsigned long lastRefresh;
    
  // if the last refresh was greater than .4 update the boost value, to reduce screen flickering
  if (millis() - lastRefresh > 100) // this value should be changed based on MCU speed
  {
    display.fillRect(45, 51, 22, 12, 0);
    display.setTextSize(2);
    display.setCursor(45, 60);
    display.printf("%02d", map(angleValue, 0, 180, 0, 30));

    display.setTextSize(1);
    display.fillRect(62, 0, 30, 7, 0);
    display.setCursor(62, 4);
    display.printf("%04d", map(angleValue, 0, 180, 0, 6000));
    lastRefresh = millis();
  }

  display.display();
  // display.clearDisplay();
}


// min 1890
// max 4095
// map these values
/**
 * @brief Draws the two concentric half circles onto the display,
 * along with other information that does not get wiped from the screen
 *
 */
void drawCircles()
{
  for (int i = 0; i < 180; i++)
  {
    display.drawPixel(O_x_radius_pixel[i], O_y_radius_pixel[i], SSD1306_WHITE);
    display.drawPixel(I_x_radius_pixel[i], I_y_radius_pixel[i], SSD1306_WHITE);
  }
  display.setCursor(40, 4);
  display.print("RPM: ");
  display.print("0000");
  display.setCursor(51, 46);
  display.print("Boost");
  display.setCursor(7, 63);
  display.print("0");
  display.setCursor(115, 63);
  display.print("30");
  display.setCursor(60, 12);
  display.print("15");
  display.setTextSize(2);
  display.setCursor(45, 60);
  display.print("00");
  display.setTextSize(1);
  display.print("PSI");

  display.display();
}

/**
 * @brief Calculates the pixels of the circle for inside and outside circle using simple maths
 *
 */
void calculateRadiusPixels()
{
  const int OFFSET_VALUE = 3; // the amount of space to leave from the edges
  for (int i = 0; i < 180; i++)
  {
    O_x_radius_pixel[i] = sin(radians(i - 90)) * O_CIRCLE_RADIUS + X_CENTER;
    I_x_radius_pixel[i] = sin(radians(i - 90)) * I_CIRCLE_RADIUS + X_CENTER;
    Oin_x_radius_pixel[i] = sin(radians(i - 90)) * (O_CIRCLE_RADIUS - OFFSET_VALUE) + X_CENTER;
    Iin_x_radius_pixel[i] = sin(radians(i - 90)) * (I_CIRCLE_RADIUS + OFFSET_VALUE) + X_CENTER;

    O_y_radius_pixel[i] = -cos(radians(i - 90)) * O_CIRCLE_RADIUS + Y_CENTER;
    I_y_radius_pixel[i] = -cos(radians(i - 90)) * I_CIRCLE_RADIUS + Y_CENTER;
    Oin_y_radius_pixel[i] = -cos(radians(i - 90)) * (O_CIRCLE_RADIUS - OFFSET_VALUE) + Y_CENTER;
    Iin_y_radius_pixel[i] = -cos(radians(i - 90)) * (I_CIRCLE_RADIUS + OFFSET_VALUE) + Y_CENTER;
  }
}
