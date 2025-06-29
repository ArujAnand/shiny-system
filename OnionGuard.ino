//FOR AMMONIA SENSOR MQ 137
#define DEBUG //debug messages

/******** Hardware Related Macros ********/
#define MQ_PIN (A0) //define which analog channel you are going to use; needs analog pin
#define RL_VALUE (10) //define the load resistance on the board in kilo ohms
#define RO_CLEAN_AIR_FACTOR (3.6)
//RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO; (derived from the chart in datasheet)

/******** Software Related Macros ********/
#define CALIBARAION_SAMPLE_TIMES (50) //samples being taken in the calibration phase
#define CALIBRATION_SAMPLE_INTERVAL (500) //time interal(in milisecond) between each samples in the cablibration phase

#define READ_SAMPLE_INTERVAL (50) //samples you are going to take in normal operation
#define READ_SAMPLE_TIMES (5) //time interal(in milisecond) between each samples in normal operation

/********** Graph Related Macros *********/
#define slope_m (-0.350)
#define intercept_b (0.584)

/****************Globals***************/
float Ro; //Fixed Resistance of the sensor that will be set during calibration
float humid; //for storing humidity from DHT
float tempC; //for storing temperature in C from DHT

/**********Graph & Globals end**********/

//FOR TEMPERATURE AND HUMIDITY SENSOR
#include <DHT.h> //Pre-existing library
#define TYPE DHT11 //Type of sensor used
int THsensePin = 2; //Temp & Humidity Sensor Pin
DHT HT(THsensePin, TYPE);

//FOR DC MOTOR
/*******Deciding Pin Numbers for DC Motor********/
int SpeedPin=5; //pin from which speed input of fan will be given
int dir1=4; //selection of direction pins for fan movement
int dir2=3; //selection of direction pins for fan movement
/*int mSpeed=150; //for speed of fan select value b/w 0-255 bcz 8 bits so 2^8
//Commented out because the down in the code

//temp & humidity condition governing the DC motor*/

void setup() {
/************ Initialising *************/

  Serial.begin(9600); //baud rate of serial monitor
  HT.begin(); //Start Reading DHT11
  delay(500); //With delay of 500ms
  pinMode(LED_BUILTIN, OUTPUT); //To check state of the program using builtin LED

/**********Configuring Pins DC Motor***********/
  pinMode(SpeedPin,OUTPUT);
  pinMode(dir1,OUTPUT);
  pinMode(dir2,OUTPUT);

/*********Calibration for MQ 137**********/
  #ifdef DEBUG //START OF DEBUG
  Serial.println("Program is running, enter 0 to stop this prompt");
  Serial.print("Calibrating...\n");
  Ro = MQCalibration(MQ_PIN); //sensor calibration to set the Ro value, to be used only first or when using fresh air

//Calibrating the sensor. Please make sure the sensor is in clean air when you perform the calibration

  Serial.print("Calibration is done...\n");
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");
  #endif //END OF DEBUG
}

void loop() {
  #ifdef DEBUG
  LedBlink(); //Start LED blinking
  #endif
  delay(2000);
  DHT_Sensor(); //Start DHT sensor reading
//Motor(255); //Run motor at certain speed (0-255) for loop check

/****************** Calculating Ammonia Concentration ****************************/
  Serial.print("Ammonia:");
  Serial.print(round(MQGetGasConcentration(MQRead(MQ_PIN)/Ro)*100)/100.0);
  //Calling calculating function and rounding it off to 2 decimal places
  Serial.println( "ppm" );
/*********************************************************************************/
}

/******************************* LedBlink ****************************************
Input: none, works on function call
Output: Blinks built in LED with delay of 1 second
Remarks: On function call the it starts blinking led with delay if 1s.
************************************************************************************/
#ifdef DEBUG
void LedBlink()
{
  digitalWrite(LED_BUILTIN, HIGH); //debug Statement
  delay(1000); //debug Statement
  digitalWrite(LED_BUILTIN, LOW); //debug Statement
  delay(1000); //debug Statement
}
#endif

/******************************* DHT_Sensor ****************************************
Input: none, works on function call
Output: Giving output of temp in C, humidity, heat index and turning DC Motor on or off
depending upon the parameters given
Remarks: On function call the it starts reading values from sensor using member
functions
from DHT library.
************************************************************************************/
void DHT_Sensor()
{
  //DHT SENSING
  humid = HT.readHumidity();
  tempC = HT.readTemperature();
  // Check if any reads failed and exit early (to try again).
  /*if (isnan(humid) || isnan(tempC))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }*/
// Compute heat index in Celsius (isFahreheit = false)
  float hic = HT.computeHeatIndex(tempC, humid, false);

  Serial.print(F("Humidity: "));
  Serial.print(humid);
  Serial.print(F("% Temperature: "));
  Serial.print(tempC);
  Serial.print(F("°C "));
  Serial.print(F(" Heat index: "));
  Serial.print(hic);
  Serial.println(F("°C "));
  if(tempC >= 35 || humid >= 75)
  {
    Motor(255);
  }
  else
  {
    Motor(0);
  }
}

/************************************ MOTOR ****************************************
Input: speed at which you want to run the DC motor
Output: running the DC motor at 255(max speed) or 0
Remarks: the mspeed parameter is given by DHT sensor function depending upon the
temp
and humidity, currently no speed control, if temp or humidity greater than
specified parameters run fan at full speed if not then turn it off by giving 0.
************************************************************************************/
void Motor(int mSpeed)
{
  digitalWrite(dir1,HIGH); //dir1 HIGH & dir2 LOW makes it run
  counter-clockwise
  digitalWrite(dir2,LOW);
  analogWrite(SpeedPin,mSpeed); //Writing Speed to Speed Pin
}

/****************** MQResistanceCalculation ****************************************
Input: raw_adc - raw value read from adc, which represents the voltage
Output: the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
across the load resistor and its resistance, the resistance of the sensor
could be derived.
************************************************************************************/
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*((1023-raw_adc)/raw_adc))); //Rs = RL *(Vin - VL)
} // --------------- Mathematical Derivation (Caution: Check whether VL in being caluclated correctly)
// VL

/***************************** MQCalibration ****************************************
Input: mq_pin - analog channel
Output: Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use
MQResistanceCalculation to calculates the sensor resistance in clean air
and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is
about
3.6, which differs slightly between different sensors.
************************************************************************************/
float MQCalibration(int mq_pin)
{
  int i;
  float val=0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) { //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES; //calculate the average value

  val = val/RO_CLEAN_AIR_FACTOR; //divided by RO_CLEAN_AIR_FACTOR yields the Ro according to the chart in the datasheet

  return val;
}

/***************************** MQRead *********************************************
Input: mq_pin - analog channel
Output: Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistance (Rs).
The Rs changes as the sensor is in the different consentration of the target
gas. The sample times and the time interval between samples could be configured
by changing the definition of the macros.
************************************************************************************/
float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;
}

/***************************** MQGetGasConcentration ************************************
Input: mq_pin - analog channel
Output: Concentration of gas in ppm
Remarks: This function use MQRead to caculate the sensor resistenc (Rs).
The Rs changes as the sensor is in the different consentration of the target
gas. The sample times and the time interval between samples could be configured
by changing the definition of the macros. Calculates ppm according to the
logarithmic graph.
************************************************************************************/

double MQGetGasConcentration(float Rs_Ro_Ratio)
{
  return (pow(10,(((log10(Rs_Ro_Ratio))-intercept_b)/slope_m)));
}
