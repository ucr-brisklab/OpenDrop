/*
  Basic Code to run the OpenDrop V4.1, Research platfrom for digital microfluidics
  Object codes are defined in the OpenDrop.h library
  Written by Urs Gaudenz from GaudiLabs, 2021
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OpenDrop.h>
#include <OpenDropAudio.h>
#include <hardware_def.h>
#include "Adafruit_ZeroTimer.h"

float freq = 1.0;

Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);

void TC3_Handler() {
  Adafruit_ZeroTimer::timerHandler(3);
}

volatile bool toggle = false;

void blinkLED(void) {
  digitalWrite(LED_BUILTIN, toggle);
  toggle = !toggle;
}


OpenDrop OpenDropDevice = OpenDrop();
Drop *myDrop = OpenDropDevice.getDrop();

bool FluxCom[16][8];
bool FluxBack[16][8];

int ControlBytesIn[16];
int ControlBytesOut[24];
int readbyte;
int writebyte;

int JOY_value;
int joy_x, joy_y;
int x, y;
int del_counter = 0;
int del_counter2 = 0;

bool SWITCH_state = true;
bool SWITCH_state2 = true;
bool idle = true;

bool Magnet1_state = false;
bool Magnet2_state = false;

int j = 0;

// the setup function runs once when you press reset or power the board
void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  OpenDropDevice.begin(nullptr);

  ControlBytesOut[23] = OpenDropDevice.get_ID();
  Serial.println(OpenDropDevice.get_ID());
  // OpenDropDevice.set_voltage(240,false,1000);

  OpenDropDevice.set_Fluxels(FluxCom);  // fluidic pixel

  pinMode(JOY_pin, INPUT);

  OpenDropAudio.begin(16000);
  OpenDropAudio.playMe(2);
  delay (2000);

  OpenDropDevice.drive_Fluxels();
  OpenDropDevice.update_Display();
  Serial.println("Welcome to OpenDrop");

  myDrop->begin(7, 4);
  OpenDropDevice.update();

  del_counter = millis();
<<<<<<< Updated upstream
=======

  Serial.println("setting up timer");

  uint16_t divider  = 1;
  uint16_t compare = 0;
  tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV1;

  if ((freq < 24000000) && (freq > 800)) {
    divider = 1;
    prescaler = TC_CLOCK_PRESCALER_DIV1;
    compare = 48000000/freq;
  } else if (freq > 400) {
    divider = 2;
    prescaler = TC_CLOCK_PRESCALER_DIV2;
    compare = (48000000/2)/freq;
  } else if (freq > 200) {
    divider = 4;
    prescaler = TC_CLOCK_PRESCALER_DIV4;
    compare = (48000000/4)/freq;
  } else if (freq > 100) {
    divider = 8;
    prescaler = TC_CLOCK_PRESCALER_DIV8;
    compare = (48000000/8)/freq;
  } else if (freq > 50) {
    divider = 16;
    prescaler = TC_CLOCK_PRESCALER_DIV16;
    compare = (48000000/16)/freq;
  } else if (freq > 12) {
    divider = 64;
    prescaler = TC_CLOCK_PRESCALER_DIV64;
    compare = (48000000/64)/freq;
  } else if (freq > 3) {
    divider = 256;
    prescaler = TC_CLOCK_PRESCALER_DIV256;
    compare = (48000000/256)/freq;
  } else if (freq >= 0.75) {
    divider = 1024;
    prescaler = TC_CLOCK_PRESCALER_DIV1024;
    compare = (48000000/1024)/freq;
  } else {
    Serial.println("Invalid frequency");
    while (1) delay(10);
  }
  Serial.print("Divider:"); Serial.println(divider);
  Serial.print("Compare:"); Serial.println(compare);
  Serial.print("Final freq:"); Serial.println((int)(48000000/compare));

  zerotimer.enable(false);
  zerotimer.configure(prescaler,       // prescaler
          TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
          TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
          );

  zerotimer.setCompare(0, compare);
  zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, blinkLED);
  zerotimer.enable(true);

>>>>>>> Stashed changes
}


void loop() {
  if (Serial.available() > 0)           //receive data from App
  {
    readbyte = Serial.read();
    if (x < FluxlPad_width)
      for (y = 0; y < 8; y++) FluxCom[x][y] = (((readbyte) >> (y)) & 0x01);
    else ControlBytesIn[x - FluxlPad_width] = readbyte;

    x++;
    digitalWrite(LED_Rx_pin, HIGH);
    if (x == (FluxlPad_width + 16))
    { OpenDropDevice.set_Fluxels(FluxCom);
      OpenDropDevice.drive_Fluxels();
      OpenDropDevice.update_Display();

      if ((ControlBytesIn[0] & 0x2) && (Magnet1_state == false)) {
        Magnet1_state = true;
        OpenDropDevice.set_Magnet(0, HIGH);
      };

      if (!(ControlBytesIn[0] & 0x2) && (Magnet1_state == true)) {
        Magnet1_state = false;
        OpenDropDevice.set_Magnet(0, LOW);
      } ;

      if ((ControlBytesIn[0] & 0x1) && (Magnet2_state == false)) {
        Magnet2_state = true;
        OpenDropDevice.set_Magnet(1, HIGH);
      };

      if (!(ControlBytesIn[0] & 0x1) && (Magnet2_state == true)) {
        Magnet2_state = false;
        OpenDropDevice.set_Magnet(1, LOW);
      } ;

      for (int x = 0; x < (FluxlPad_width) ; x++)
      { writebyte = 0;
        for (int y = 0; y < FluxlPad_heigth ; y++)
          writebyte = (writebyte << 1) + (int)OpenDropDevice.get_Fluxel(x, y);
        ControlBytesOut[x] = writebyte;
      }

      OpenDropDevice.set_Temp_1(ControlBytesIn[10]);
      OpenDropDevice.set_Temp_2(ControlBytesIn[11]);
      OpenDropDevice.set_Temp_3(ControlBytesIn[12]);

      OpenDropDevice.show_feedback(ControlBytesIn[8]);

      ControlBytesOut[17] = OpenDropDevice.get_Temp_L_1();
      ControlBytesOut[18] = OpenDropDevice.get_Temp_H_1();
      ControlBytesOut[19] = OpenDropDevice.get_Temp_L_2();
      ControlBytesOut[20] = OpenDropDevice.get_Temp_H_2();
      ControlBytesOut[21] = OpenDropDevice.get_Temp_L_3();
      ControlBytesOut[22] = OpenDropDevice.get_Temp_H_3();

      for (x = 0; x < 24; x++) Serial.write(ControlBytesOut[x]);
      x = 0;
    };
  }
  else digitalWrite(LED_Rx_pin, LOW);
  del_counter--;

  if (millis() - del_counter > 2000) {             //update Display
    OpenDropDevice.update_Display();
    del_counter = millis();
  }

  SWITCH_state = digitalRead(SW1_pin);
  SWITCH_state2 = digitalRead(SW2_pin);

  if (!SWITCH_state)                    //activate Menu
  {
    OpenDropAudio.playMe(1);
    Menu(OpenDropDevice);
    OpenDropDevice.update_Display();
    del_counter2 = 200;
  }


  if (!SWITCH_state2)                    //activate Reservoirs
  {
    if ((myDrop->position_x() == 15) && (myDrop->position_y() == 3))
    {
      myDrop->begin(14, 1);
      OpenDropDevice.dispense(1, 1200);
    }
    if ((myDrop->position_x() == 15) && (myDrop->position_y() == 4))
    {
      myDrop->begin(14, 6);
      OpenDropDevice.dispense(2, 1200);
    }

    if ((myDrop->position_x() == 0) && (myDrop->position_y() == 3))
    {
      myDrop->begin(1, 1);
      OpenDropDevice.dispense(3, 1200);
    }
    if ((myDrop->position_x() == 0) && (myDrop->position_y() == 4))
    {
      myDrop->begin(1, 6);
      OpenDropDevice.dispense(4, 1200);
    }

    if ((myDrop->position_x() == 10) && (myDrop->position_y() == 2))
    { if (Magnet1_state) {
        OpenDropDevice.set_Magnet(0, LOW);
        Magnet1_state = false;
      }
      else {
        OpenDropDevice.set_Magnet(0, HIGH);
        Magnet1_state = true;
      }
      while (!digitalRead(SW2_pin));
    }

    if ((myDrop->position_x() == 5) && (myDrop->position_y() == 2))
    {
      if (Magnet2_state) {
        OpenDropDevice.set_Magnet(1, LOW);
        Magnet2_state = false;
      }
      else {
        OpenDropDevice.set_Magnet(1, HIGH);
        Magnet2_state = true;
      }
      while (!digitalRead(SW2_pin));
    }

  }


  JOY_value = analogRead(JOY_pin);      //navigate using Joystick

  if ((JOY_value < 950) & (del_counter2 == 0))
  {
    if  (JOY_value < 256)
    {
      myDrop->move_right();
    }

    if  ((JOY_value > 725) && (JOY_value < 895))
    {
      myDrop->move_up();
    }

    if  ((JOY_value > 597) && (JOY_value < 725))
    {
      myDrop->move_left();
    }

    if  ((JOY_value > 256) && (JOY_value < 597))
    {
      myDrop->move_down();
    }

    OpenDropDevice.update_Drops();
    OpenDropDevice.update();
    if (idle) {
      del_counter2 = 1800;
      idle = false;
    } else del_counter2 = 400;

  }

  if (JOY_value > 950) {
    del_counter2 = 0;
    idle = true;
  }
  if (del_counter2 > 0) del_counter2--;

} // main loop
