#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FirebaseArduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>

// firebase data
#define FIREBASE_HOST "switcher-7b8e7.firebaseio.com"
#define FIREBASE_AUTH "PsxEQVAIaRUwYCG38NOrk9CJbLdLn5qlW9FBXmsd"
#define WIFI_SSID "TP-LINK"
#define WIFI_PASSWORD "syntax-x"
#define PATH "/status/pompa"

// send data each 5 minutes
#define limaMenit (1000UL * 60 * 5)
unsigned long rolltime = millis() + limaMenit;

#define ON 0  //nilai relay ketika hidup
#define OFF 1 //nilai relay ketika mati
#define ANALOG_INPUT A0
#define servoMicroBubble D8
#define servoKeluar D7
#define relayHeater D6
#define relayPompa D5
#define MUX_A D4
#define MUX_B D3
#define MUX_C D2
#define pinSuhu D1

// variable kontrol relayPompa
int last_read;

//variable sensor pH
float calibration = 6.00;
unsigned long int avgValue;
int buf[10], temp;

//variable sensor TDS
int sensorValue;               //adc value
float outputValueConductivity; //conductivity value
float outputValueTDS;          //TDS value
String statusSalinitas = "";

//variable sensor Ketinggian Air
int sensorAirVal = 0;
const int airNormal = 1;
const int airPenuh = 5;
String statusAir = "";

//variable sensor Suhu
float Celcius = 0;
float Fahrenheit = 0;

Servo servoAir;
Servo servoMb;
OneWire oneWire(pinSuhu);
DallasTemperature sensors(&oneWire);

// Json data
StaticJsonBuffer<1000> jsonBuffer;
JsonObject &root = jsonBuffer.createObject();
JsonObject &waktu = root.createNestedObject("waktu");

void changeMux(int c, int b, int a)
{
  digitalWrite(MUX_A, a);
  digitalWrite(MUX_B, b);
  digitalWrite(MUX_C, c);
}

float getTingiAir()
{
  changeMux(LOW, HIGH, HIGH);
  sensorAirVal = analogRead(ANALOG_INPUT);
  return sensorAirVal;
}

float getTDS()
{
  //read the analog in value:
  changeMux(HIGH, HIGH, HIGH);
  sensorValue = analogRead(ANALOG_INPUT);
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
    changeMux(HIGH, LOW, HIGH);
    buf[i] = analogRead(ANALOG_INPUT);
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
  float phValue = 1.5 * pHVol;
  return phValue;
}

void setup()
{
  Serial.begin(115200);
  servoMb.attach(servoMicroBubble);
  servoAir.attach(servoKeluar);
  sensors.begin();
  pinMode(relayHeater, OUTPUT);
  pinMode(relayPompa, OUTPUT);
  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);
  digitalWrite(relayHeater, OFF);
  digitalWrite(relayPompa, OFF);
  servoAir.write(0);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.stream(PATH);
}

void loop()
{
  root["suhu"] = getSuhu();
  root["salinitas"] = getTDS();
  root["tAir"] = getTingiAir();
  root["pH"] = getPh();
  waktu[".sv"] = "timestamp";
  root.printTo(Serial);
  Serial.println();

  // control ketinggian air
  if (tAir < airNormal)
  {
    statusAir = "RENDAH";
  }
  else if (tAir >= airNormal && tAir <= airPenuh)
  {
    statusAir = "NORMAL";
  }
  else
  {
    statusAir = "TINGGI";
  }

  // control salinitas
  if ((salinitas >= 28) && (salinitas <= 33))
  {
    statusSalinitas = "NORMAL";
  }
  else if (salinitas < 28)
  {
    statusSalinitas = "RENDAH";
  }
  else
  {
    statusSalinitas = "TINGGI";
  }

  // control heater
  if (root["suhu"] < 28.5)
  {
    digitalWrite(relayHeater, ON);
  }
  else if (root["suhu"] > 29)
  {
    digitalWrite(relayPompa, ON);
    servoAir.write(120);
  }
  else
  {
    digitalWrite(relayHeater, OFF);
  }

  // control pompa
  if (root["pH"] < 7.59 )
  {
    digitalWrite(relayPompa, ON);
    servoAir.write(0);
  }
  else if(root["pH"] > 8.17)
  {
    digitalWrite(relayPompa, OFF);
    servoAir.write(120);
  }

  if (Firebase.failed())
  {
    Serial.println("stream error");
    Serial.println(Firebase.error());
    delay(1000);
    Firebase.stream(PATH);
    return;
  }

  if (Firebase.available())
  {
    FirebaseObject event = Firebase.readEvent();
    int data = event.getInt("data");
    if (last_read != data)
    {
      // action to controll servo
      // check otomatis
      if (data == 0)
      {
        digitalWrite(relayPompa, OFF);
      }
      else if (data == 1)
      {
        digitalWrite(relayPompa, ON);
      }
    }
    last_read = data;
    Firebase.stream(PATH);
  }

  if ((long)(millis() - rolltime) >= 0)
  {
    Firebase.push("sensor/", root);
    if (Firebase.failed())
    {
      Serial.print("pushing /sensor failed:");
      Serial.println(Firebase.error());
      return;
    }
    rolltime += limaMenit;
  }

  delay(100);
}
