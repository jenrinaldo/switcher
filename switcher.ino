#include <Servo.h>

Servo servoAirTawar;
Servo servoAirLaut;
Servo servoAirKeluar;
 
int servoAirTawarPos = 0;
int servoAirLautPos = 0;
int servoAirKeluarPos = 0;

void setup() {
  servoAirTawar.attach (9);
  servoAirLaut.attach (10);
  servoAirKeluar.attach (11);
}

void loop() {
    servoAirTawar.write(120);
    delay(1000);
    servoAirTawar.write(0);
}
