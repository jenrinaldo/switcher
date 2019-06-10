#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#define ON 0  //nilai relay ketika hidup
#define OFF 1 //nilai relay ketika mati
#define sensorTDS A0
#define sensorPH A1
#define sensorAir A2
#define relayHeater 53
#define relayPompa 52
#define pinSuhu 4


//variable sensor pH
float calibration = 0.00;
unsigned long int avgValue; 
int buf[10],temp;

//variable sensor TDS
int sensorValue;               //adc value
float outputValueConductivity; //conductivity value
float outputValueTDS;          //TDS value

//variable sensor Ketinggian Air 
int sensorAirVal = 0;
const int airNormal = 410;
const int airPenuh = 560;

//variable sensor Suhu
float Celcius = 0;
float Fahrenheit = 0;

Servo servoAir;
OneWire oneWire(pinSuhu);
DallasTemperature sensors(&oneWire);
SoftwareSerial s(11, 10); // (Rx, Tx)
// Json
StaticJsonBuffer<1000> jsonBuffer;
JsonObject &root = jsonBuffer.createObject();

float getTingiAir(){
  sensorAirVal = analogRead(sensorAir);
  return sensorAirVal;
}

float getTDS()
{
  //read the analog in value:
  sensorValue = analogRead(sensorTDS);

  //Mathematical Conversion from ADC to TDS (ppm)
  //rumus berdasarkan datasheet
  outputValueTDS = (0.3417 * sensorValue) + 281.08;

  return outputValueTDS;
}

float getSuhu()
{
  sensors.requestTemperatures();
  Celcius = sensors.getTempCByIndex(0);
  return Celcius;
}

float getPh(){
  for(int i=0;i<10;i++) 
 { 
 buf[i]=analogRead(sensorPH);
 delay(30);
 }
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buf[i]>buf[j])
 {
 temp=buf[i];
 buf[i]=buf[j];
 buf[j]=temp;
 }
 }
 }
 avgValue=0;
 for(int i=2;i<8;i++)
 avgValue+=buf[i];
 float pHVol=(float)avgValue*5.0/1024/6;
 float phValue = -5.70 * pHVol + calibration; 
 return phValue;
}

void setup()
{
  s.begin(9600);
  servoAir.attach(9);
  sensors.begin();
  pinMode(relayHeater, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(relayHeater, OFF);
  pinMode(relayPompa, OUTPUT);
  digitalWrite(relayPompa, OFF);
}

void loop()
{
  float suhu = getSuhu();
  float salinitas = getTDS();
  float tAir = getTingiAir();
  float pH = analogRead(sensorPH); //getPH();
  //    servoAir.write(120);
  //    digitalWrite(10, HIGH);
     digitalWrite (relayHeater,ON);
  //    digitalWrite (relayPompa,ON);
  //    servoAir.write(0);
  //    digitalWrite(10, LOW);
    //  digitalWrite (relayHeater,OFF);
  //    digitalWrite (relayPompa,OFF);
  if (isnan(suhu) || isnan(salinitas) || isnan(pH))
  {
    return;
  }
  root["suhu"] = suhu;
  root["salinitas"] = salinitas;
  root["pH"] = pH;

  if (s.available() > 0)
  {
    root.printTo(s);
  }
}
