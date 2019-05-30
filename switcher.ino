#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>

#define ON 0  //nilai relay ketika hidup
#define OFF 1 //nilai relay ketika mati
#define sensorTDS A0
#define sensorPH A1
#define relayHeater 2
#define relayPompa 3
#define pinSuhu 4

#define Offset -5.77 //deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth 40    //times of collection
int pHArray[ArrayLenth]; //Store the average value of the sensor feedback
int pHArrayIndex = 0;

//variable sensor TDS
int sensorValue;               //adc value
float outputValueConductivity; //conductivity value
float outputValueTDS;          //TDS value

//variable sensor Suhu
float Celcius = 0;
float Fahrenheit = 0;

Servo servoAir;
OneWire oneWire(pinSuhu);
DallasTemperature sensors(&oneWire);

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

float getPH()
{
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(sensorPH);
    if (pHArrayIndex == ArrayLenth)
      pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
    pHValue = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
  if (millis() - printTime > printInterval) //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
    return pHValue;
    printTime = millis();
  }
}

double avergearray(int *arr, int number)
{
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0)
  {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number < 5)
  { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++)
    {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  }
  else
  {
    if (arr[0] < arr[1])
    {
      min = arr[0];
      max = arr[1];
    }
    else
    {
      min = arr[1];
      max = arr[0];
    }
    for (i = 2; i < number; i++)
    {
      if (arr[i] < min)
      {
        amount += min; //arr<min
        min = arr[i];
      }
      else
      {
        if (arr[i] > max)
        {
          amount += max; //arr>max
          max = arr[i];
        }
        else
        {
          amount += arr[i]; //min<=arr<=max
        }
      } //if
    }   //for
    avg = (double)amount / (number - 2);
  } //if
  return avg;
}

void setup()
{
  Serial.begin(115200);
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
  //  float sensorVal = getPH();
  //  Serial.println(sensorVal);
  //    servoAir.write(120);
  //    digitalWrite(10, HIGH);
  //    digitalWrite (relayHeater,ON);
  //    digitalWrite (relayPompa,ON);
  //    Serial.println("relay nyala");
  //    delay(1000);
  //    servoAir.write(0);
  //    digitalWrite(10, LOW);
  //    digitalWrite (relayHeater,OFF);
  //    digitalWrite (relayPompa,OFF);
  //    Serial.println("relay mati");
  //  delay(1000);
}
