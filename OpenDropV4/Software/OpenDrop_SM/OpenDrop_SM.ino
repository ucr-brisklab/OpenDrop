/*
  Basic Code to run the OpenDrop V4.1, Research platfrom for digital
  microfluidics Object codes are defined in the OpenDrop.h library Written by
  Urs Gaudenz from GaudiLabs, 2021
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <ArduinoLowPower.h>
#include <OpenDrop.h>
#include <OpenDropAudio.h>
#include <hardware_def.h>
//#include "Adafruit_ZeroTimer.h"


//unsigned long int findGCD(unsigned long int a, unsigned long int b) {
//  unsigned long int c;
//  while (1) {
//    c = a % b;
//    if (c == 0) {
//      return b;
//    }
//    a = b;
//    b = c;
//  }
//  return 0;
//}

typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;

static task task1, task2, task3, task4;
task *tasks[] = {&task1, &task2, &task3, &task4};
const unsigned short tasksNum = sizeof(tasks) / sizeof(task *);

const char start = -1;
unsigned short i;
unsigned long sysPeriod = 1; // this should be set using GCD of all periods, but ISR interrupt routine seems to be breaking due to... reasons

OpenDrop OpenDropDevice = OpenDrop();
Drop *myDrop = OpenDropDevice.getDrop();

bool FluxCom[16][8];
bool FluxBack[16][8];

int ControlBytesIn[16];
int ControlBytesOut[24];
int readbyte;
int writebyte;

int JOY_value;
int x, y; // used for serial comm when running under opendropcontroller software

bool SWITCH_state = true;
bool SWITCH_state2 = true;

bool Magnet1_state = false;
bool Magnet2_state = false;

enum Task1State { start1, wait1 };
enum Task2States { start2, wait2 };
enum Task3States { start3, wait3 };
enum Task4States { start4, wait4, up4, down4, left4, right4 };

int Task_SerialComm_Tick(int state) {
  switch (state) {
  case start1:
    state = wait1;
    break;

  case wait1:
    state = wait1;
    break;
  }

  switch (state) {
  case start1:
    break;

  case wait1:
    if (Serial.available() > 0) // receive data from App
    {
      readbyte = Serial.read();
      if (x < FluxlPad_width)
        for (y = 0; y < 8; y++)
          FluxCom[x][y] = (((readbyte) >> (y)) & 0x01);
      else
        ControlBytesIn[x - FluxlPad_width] = readbyte;

      x++;
      digitalWrite(LED_Rx_pin, HIGH);
      if (x == (FluxlPad_width + 16)) {
        OpenDropDevice.set_Fluxels(FluxCom);
        OpenDropDevice.drive_Fluxels();
        OpenDropDevice.update_Display();

        if ((ControlBytesIn[0] & 0x2) && (Magnet1_state == false)) {
          Magnet1_state = true;
          OpenDropDevice.set_Magnet(0, HIGH);
        };

        if (!(ControlBytesIn[0] & 0x2) && (Magnet1_state == true)) {
          Magnet1_state = false;
          OpenDropDevice.set_Magnet(0, LOW);
        };

        if ((ControlBytesIn[0] & 0x1) && (Magnet2_state == false)) {
          Magnet2_state = true;
          OpenDropDevice.set_Magnet(1, HIGH);
        };

        if (!(ControlBytesIn[0] & 0x1) && (Magnet2_state == true)) {
          Magnet2_state = false;
          OpenDropDevice.set_Magnet(1, LOW);
        };

        for (int x = 0; x < (FluxlPad_width); x++) {
          writebyte = 0;
          for (int y = 0; y < FluxlPad_heigth; y++)
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

        for (x = 0; x < 24; x++)
          Serial.write(ControlBytesOut[x]);
        x = 0;
      }
    } else
      digitalWrite(LED_Rx_pin, LOW);
    break;
  }
  return state;
}

int Task_ActivateMenu_Tick(int state) {

  SWITCH_state = digitalRead(SW1_pin);
  
  switch (state) {
  case start2:
    state = wait2;
    break;

  case wait2:
    state = wait2;
    break;
  }

  switch (state) {
  case start2:
    break;

  case wait2:
    if (!SWITCH_state) // activate Menu
    {
      OpenDropAudio.playMe(1);
      Menu(OpenDropDevice);
      OpenDropDevice.update_Display();
    }
    break;
  }
  return state;
}

int Task_RightBtn_Tick(int state) {

  SWITCH_state2 = digitalRead(SW2_pin);
  
  switch (state) {
  case start3:
    state = wait3;
    break;

  case wait3:
    state = wait3;
    break;
  }

  switch (state) {
  case start3:
    break;

  case wait3:
    if (!SWITCH_state2) // activate Reservoirs
    {
    auto posx = myDrop->position_x();
    auto posy = myDrop->position_y();
      if ((posx == 15) && (posy == 3)) {
        myDrop->begin(14, 1);
        OpenDropDevice.dispense(1, 1200);
      }
      if ((posx == 15) && (posy == 4)) {
        myDrop->begin(14, 6);
        OpenDropDevice.dispense(2, 1200);
      }

      if ((posx == 0) && (posy == 3)) {
        myDrop->begin(1, 1);
        OpenDropDevice.dispense(3, 1200);
      }
      if ((posx == 0) && (posy == 4)) {
        myDrop->begin(1, 6);
        OpenDropDevice.dispense(4, 1200);
      }

      if ((posx == 10) && (posy == 2)) {
        if (Magnet1_state) {
          OpenDropDevice.set_Magnet(0, LOW);
          Magnet1_state = false;
        } else {
          OpenDropDevice.set_Magnet(0, HIGH);
          Magnet1_state = true;
        }
        while (!digitalRead(SW2_pin))
          ;
      }

      if ((posx == 5) && (posy == 2)) {
        if (Magnet2_state) {
          Serial.println("mag down");
          OpenDropDevice.set_Magnet(1, LOW);
          Magnet2_state = false;
        } else {
          Serial.println("mag up");
          OpenDropDevice.set_Magnet(1, HIGH);
          Magnet2_state = true;
        }
        while (!digitalRead(SW2_pin))
          ;
      }
    }
    break;
  }
  return state;
}

int Task_Joystick_Tick(int state) {

  JOY_value = analogRead(JOY_pin);
  
  switch (state) {
  case start4:
    state = wait4;
    break;

  case wait4:
    if ((JOY_value < 950)) {
      if (JOY_value < 256) {
        myDrop->move_right();
        state = right4;
      }

      else if ((JOY_value > 256) && (JOY_value < 597)) {
        myDrop->move_down();
        state = down4;
      }

      else if ((JOY_value > 597) && (JOY_value < 725)) {
        myDrop->move_left();
        state = left4;
      }

      else if ((JOY_value > 725) && (JOY_value < 950)) {
        myDrop->move_up();
        state = up4;
      }
      OpenDropDevice.update_Drops();
      OpenDropDevice.update();
      break;
    } else {
      state = wait4;
      break;
    }

  case right4:
    if (JOY_value > 950) {
      state = wait4;
    } else {
      state = right4;
    }
    break;

  case down4:
    if (JOY_value > 950) {
      state = wait4;
    } else {
      state = down4;
    }
    break;

  case left4:
    if (JOY_value > 950) {
      state = wait4;
    } else {
      state = left4;
    }
    break;

  case up4:
    if (JOY_value > 950) {
      state = wait4;
    } else {
      state = up4;
    }
    break;
  }
  return state;
}


void DeviceTick() {
  for (int i = 0; i < tasksNum; i++) {
    if (tasks[i]->elapsedTime >= tasks[i]->period) {
      tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
      tasks[i]->elapsedTime = 0;
    }
  }
  for (int i = 0; i < tasksNum; ++i) {
    tasks[i]->elapsedTime += sysPeriod;
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  OpenDropDevice.begin(nullptr);

  ControlBytesOut[23] = OpenDropDevice.get_ID();
  Serial.println(OpenDropDevice.get_ID());
  // OpenDropDevice.set_voltage(240,false,1000);

  OpenDropDevice.set_Fluxels(FluxCom); // fluidic pixel

  pinMode(JOY_pin, INPUT);

  OpenDropAudio.begin(16000);
  OpenDropAudio.playMe(2);
  delay(2000);

  OpenDropDevice.drive_Fluxels();
  OpenDropDevice.update_Display();
  Serial.println("Welcome to OpenDrop");

  myDrop->begin(7, 4);
  OpenDropDevice.update();

/* set up tasks */

  task1.state = start1;
  task1.period = 1;
  task1.elapsedTime = 0;
  task1.TickFct = &Task_SerialComm_Tick;
  task2.state = start2;
  task2.period = 200;
  task2.elapsedTime = 0;
  task2.TickFct = &Task_ActivateMenu_Tick;
  task3.state = start3;
  task3.period = 150;
  task3.elapsedTime = 0;
  task3.TickFct = &Task_RightBtn_Tick;
  task4.state = start4;
  task4.period = 100;
  task4.elapsedTime = 0;
  task4.TickFct = &Task_Joystick_Tick;


  //ISR breaking it seems due to Serial comm (opendropcontroller and magnets)
//  for(i = 1; i < tasksNum; i++) {
//    sysPeriod = findGCD(sysPeriod, tasks[i]->period);
//  }
}

void loop() {
  DeviceTick();
}
