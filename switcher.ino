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
#define relayHeater 7
#define relayPompa 8
#define pinSuhu 4

// variable kontrol relayPompa
int data;
int lastData;

//variable sensor pH
float calibration = 0.00;
unsigned long int avgValue;
int buf[10], temp;

//variable sensor TDS
int sensorValue;               //adc value
float outputValueConductivity; //conductivity value
float outputValueTDS;          //TDS value
string statusSalinitas = "";

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
SoftwareSerial uno(5, 6);

// Json
StaticJsonBuffer<1000> jsonBuffer;
JsonObject &root = jsonBuffer.createObject();

float getTingiAir()
{
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

float getPh()
{
  for (int i = 0; i < 10; i++)
  {
    buf[i] = analogRead(sensorPH);
    delay(30);
  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)
    avgValue += buf[i];
  float pHVol = (float)avgValue * 5.0 / 1024 / 6;
  float phValue = -5.70 * pHVol + calibration;
  return phValue;
}

void setup()
{
  uno.begin(115200);
  servoAir.attach(9);
  sensors.begin();
  pinMode(relayHeater, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(relayHeater, OFF);
  pinMode(relayPompa, OUTPUT);
  digitalWrite(relayPompa, OFF); 
  servoAir.write(0);
}

void loop()
{
  float suhu = getSuhu();
  float salinitas = getTDS();
  float tAir = getTingiAir();
  float pH = getPh(); 

  if (uno.available() > 0)
  {
    data = uno.read();
  }

  // control ketinggian air
  if(tAir<airNormal){
    statusAir = "RENDAH";
  } else if(tAir<=airNormal && tAir!>airPenuh){
    statusAir = "NORMAL";
  } else {
    statusAir = "TINGGI";
  }

  // control salinitas
  if((salinitas<=28)&&(salinitas !> 33)){
    statusSalinitas = "NORMAL";
  } else if(salinitas < 28 ) {
    statusSalinitas = "RENDAH";
  } else {
    statusSalinitas = "TINGGI";
  }

  // control heater
  if((suhu <=28.5) || (suhu !> 29)){
    digitalWrite(relayHeater,ON);
  }
  else
  {
    digitalWrite(relayHeater,OFF);
  }
  
  // control pompa
  if((pH<=7.59) || (pH !> 8.17)){
    digitalWrite(relayPompa,ON);
    servoAir.write(120);
  } else {
    digitalWrite(relayPompa,OFF);
    servoAir.write(0);
  }

  // check otomatis
  if(data==0){
    digitalWrite(relayPompa,OFF);
  } else if(data==1){
    digitalWrite(relayPompa,ON);
  }

  // check if data is number or not
  if (isnan(suhu) || isnan(salinitas) || isnan(pH))
  {
    return;
  } 
  // make json data structure
  root["suhu"] = suhu;
  root["salinitas"] = salinitas;
  root["pH"] = pH;
  //send data to NodeMcu with serial communication
  if (uno.available() > 0)
  {
    root.printTo(uno);
  }
}
