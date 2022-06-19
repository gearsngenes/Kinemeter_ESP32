
/****** These are WIFI Parameters */
const char* ssid = "wifi-name";
const char* password = "wifi-password";
const char* mqtt_server = "rpi-ip-address";
/****** ***** ***** ***** ***** ***** */


//#define BUTTON_B 6
int neopin = 5;
#include "01_WIFI_support.h"

String model = "Model 1";


//Number of sensors and their pin-locations on the feather
int const sensors = 4;
int const slots = 2 * sensors;
int pinArray [sensors] = { 12, 11, 10, 9}; //, 5,4,3};

int buttonCount = 0;


unsigned long timeArray[slots]; //Times each sensor was un/broken
float deltaArray[sensors]; // Time traveling between sensors
unsigned long brokenTime[sensors]; //Time each sensor remained broken

float AvgTime[sensors]; // average time coordinate between sensors
float instTime[sensors]; // expected times each velocity was reached
float speedArray[sensors]; // average velocity between adjacent sensors
float accl[sensors]; // average acceleration between adjacent velocities

boolean printVel = true;
boolean printAcc = true;
int expnumber = 0;

//Arrays to handle logic of which kinemeter sensors have been crossed
boolean passArray[sensors];
int sensStateArray[sensors];
int lastStateArray[sensors];


float disp[sensors] = {0, 5, 10, 10}; //Displacement between adjacent sensors, starting with 0 due to it being the start


float accArray[sensors]; //UNUSED. DO NOT USE THIS

unsigned long curTime;
unsigned long startTime;
//float speed1;

float accSum = 0.0;
float avgAcc = 0.0; //Average acceleration over the acceleration values



void setup() {
  // initialize the LED pin as an output:
  pinMode(neopin, OUTPUT);

  // initialize the sensor pin as an input:
  for (int j1 = 0; j1 < sensors; j1++)
  {
    if (pinArray[j1] == 0) {
      pinMode(pinArray[j1], INPUT);
      digitalWrite(pinArray[j1], HIGH); // turn on the pullup
    }
    else {
      pinMode(pinArray[j1], INPUT_PULLUP);
    }
    passArray[j1] = false;
  }
  Serial.begin(115200);
  setup_wifi();
  setupTFT ();

  Serial.println(F("Set up Complete."));
  //BTSetup();
  //oledSetup();
}




boolean allDone() //all sensors have been crossed by passing object
{
  boolean output = true;

  for (int j2 = 0; j2 < sensors; j2++)
  {
    if (passArray[j2] == false) {
      output = false; // if even one is false, the total result is false.
    }
  }
  return output;
}





void resetAllDone()
{
  for (int j1 = 0; j1 < sensors; j1++)
  {
    passArray[j1] = false;
  }
}



void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  for (int x = 0; x < sensors; x++)
  {
    processSensorPinat (x);
  }

  if  (allDone())
  {
    Serial.println("----");
    expnumber = expnumber + 1; Serial.print("Exp number ="); Serial.println(expnumber);
    if (expnumber == 2) {
      printSelfConfig();
    };
    processAcc();
    SerialPrint();
    String s = "";
    float pos = 0.0;
    for (int i = 0; i < sensors; i++){
      pos += disp[i];
      s += (String)instTime[i] + "," + (String)pos + ";";//"Average Acc: " + (String) avgAcc + "cm/Sec^2";
    }
    printTFT (s);
    char Buf[200];
    s.toCharArray(Buf, 200);
    client.publish("postime", Buf);

    s = "";
    for (int i = 1; i < sensors; i++){
      s += (String)AvgTime[i] + "," + (String)speedArray[i] + ";";//"Average Acc: " + (String) avgAcc + "cm/Sec^2";
    }
    printTFT (s);
    s.toCharArray(Buf, 200);
    client.publish("veltime", Buf);
    s = "";
    for (int i = 1; i < sensors-1; i++){
      s += (String)accl[i] + ";";//"Average Acc: " + (String) avgAcc + "cm/Sec^2";
    }
    s+=(String)avgAcc;
    printTFT (s);
    s.toCharArray(Buf, 200);
    client.publish("accs", Buf);
    resetAllDone();

  }

}
