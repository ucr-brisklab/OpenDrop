/*
  Basic Code to run the OpenDrop V4.1, Research platfrom for digital microfluidics
  Object codes are defined in the OpenDrop.h library
  Written by Urs Gaudenz from GaudiLabs, 2021
*/

#include <SPI.h>
#include <Wire.h>
#include <ArduinoLowPower.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OpenDrop.h>
#include <OpenDropAudio.h>
#include <hardware_def.h>
#include "Adafruit_ZeroTimer.h"

float freq = 10;
float sysPeriod = (1/freq)*1000;

Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);

void TC3_Handler() {
  Adafruit_ZeroTimer::timerHandler(3);
}

volatile bool toggle = false;

void blinkLED(void) {
  digitalWrite(LED_BUILTIN, toggle);
  toggle = !toggle;
}


typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;


const unsigned short tasksNum = 5;
task tasks[tasksNum];

OpenDrop OpenDropDevice = OpenDrop();
Drop *myDrop = OpenDropDevice.getDrop();


bool FluxCom[16][8];
bool FluxBack[16][8];

int ControlBytesIn[16];
int ControlBytesOut[24];
int readbyte;
int writebyte;

int JOY_value;
/*int joy_x, joy_y; unused */
int x, y;

//del_counters will be replaced with periods
/*
int del_counter = 0;
int del_counter2 = 0;
*/
bool SWITCH_state = true;
bool SWITCH_state2 = true;

//idle is probably unecessary, only affects del_counter2
bool idle = true;

bool Magnet1_state = false;
bool Magnet2_state = false;

/*int j = 0; unusued */

enum Task1State{start1, wait1};
enum Task2States{start2, wait2};
enum Task3States{start3, wait3};
enum Task4States{start4, wait4};
enum Task5States{start5, wait5, up5, down5, left5, right5};

int Task1Tick(int state) {
  switch(state) {
    case start1:
      state = wait1;
      break;

    case wait1:
      state = wait1;
      break;
  }

  switch(state) {
    case start1:
      break;

    case wait1:
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
      SWITCH_state = digitalRead(SW1_pin);
      SWITCH_state2 = digitalRead(SW2_pin);
      JOY_value = analogRead(JOY_pin);
      break; 
    
  }
  return state;
}


int Task2Tick(int state) {
  switch(state) {
    case start2:
      state = wait2;
      break;

    case wait2:
      state = wait2;
      break;
  }

  switch(state) {
    case start2:
      break;

    case wait2:
      if (!SWITCH_state)                    //activate Menu
      {
        OpenDropAudio.playMe(1);
        Menu(OpenDropDevice);
        OpenDropDevice.update_Display();
//        del_counter2 = 200;
      }
      break;
  }
  return state;
}


int Task3Tick(int state) {
  switch(state) {
    case start3:
      state = wait3;
      break;

    case wait3:
      state = wait3;
      break;
  }

  switch(state) {
    case start3:
      break;

    case wait3:
        if (!SWITCH_state)                    //activate Menu
        {
        OpenDropAudio.playMe(1);
        Menu(OpenDropDevice);
        OpenDropDevice.update_Display();
      //del_counter2 = 200;
  }
    
  }
  return state;
}


int Task4Tick(int state) {
  switch(state) {
    case start4:
      state = wait4;
      break;

    case wait4:
      state = wait4;
      break;
  }

  switch(state) {
    case start4:
      break;

    case wait4:
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
    break;
  }
  return state;
}

 
int Task5Tick(int state) {
  switch(state) {
    case start5:
      state = wait5;
      break;

    case wait5:
      if ((JOY_value < 950))
      {
        if  (JOY_value < 256)
        {
          myDrop->move_right();
          state = right5;
        }

        else if  ((JOY_value > 256) && (JOY_value < 597))
        {
          myDrop->move_down();
          state = down5;
        }

        else if  ((JOY_value > 597) && (JOY_value < 725))
        {
          myDrop->move_left();
          state = left5;
        }

        else if  ((JOY_value > 725) && (JOY_value < 950))
        {
          myDrop->move_up();
          state = up5;
        }
        OpenDropDevice.update_Drops();
        OpenDropDevice.update();
        break;
      }
      else {
        state = wait5;
        break;
      }

    case right5:
      if (JOY_value > 950) {
        state = wait5;
      }
      else {
        state = right5;
      }
      break;

    case down5:
      if (JOY_value > 950) {
        state = wait5;
      }
      else {
        state = down5;
      }
      break;
  
    case left5:
      if (JOY_value > 950) {
        state = wait5;
      }
      else {
        state = left5;
      }
      break;

    case up5:
      if (JOY_value > 950) {
        state = wait5;
      }
      else {
        state = up5;
      }
      break;
  }
  return state;
}

void DeviceTick() {
  for (int i = 0; i < tasksNum; i++) {
    if (tasks[i].elapsedTime >= tasks[i].period) {
        tasks[i].state = tasks[i].TickFct(tasks[i].state);
        tasks[i].elapsedTime = 0;
    }
  }
  for (int i = 0; i < tasksNum; ++i) {
    tasks[i].elapsedTime += sysPeriod;
  }
}


// the setup function runs once when you press reset or power the board
void setup() {
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

//  del_counter = millis();

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
  zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, DeviceTick);
  zerotimer.enable(true);
  
  tasks[0].state = start1;
  tasks[0].period = 100;
  tasks[0].elapsedTime = 0;
  tasks[0].TickFct = &Task1Tick;
  tasks[1].state = start2;
  tasks[1].period = 100;
  tasks[1].elapsedTime = 0;
  tasks[1].TickFct = &Task2Tick;
  tasks[2].state = start3;
  tasks[2].period = 100;
  tasks[2].elapsedTime = 0;
  tasks[2].TickFct = &Task3Tick;
  tasks[3].state = start4;
  tasks[3].period = 100;
  tasks[3].elapsedTime = 0;
  tasks[3].TickFct = &Task4Tick;
  tasks[4].state = start5;
  tasks[4].period = 100;
  tasks[4].elapsedTime = 0;
  tasks[4].TickFct = &Task5Tick;

}

void loop() {
  LowPower.idle();
  while(1){}
}
