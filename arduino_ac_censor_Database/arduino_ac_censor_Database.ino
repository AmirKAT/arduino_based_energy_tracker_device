#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <RTCZero.h>
#include <WiFiUdp.h>

RTCZero rtc;

const char* ssid = "ASUS";  //SSID
const char* pass = "patillo2020"; //Password
const char* serverName = "stormy-inlet-66435.herokuapp.com"; //Database Link
int port = 80; //Port over which database will communicate
int keyIndex = 0;

const int GMT = 2; //timezone

int status = WL_IDLE_STATUS;
String Date = "22/5/23";
int _id = 90;
float ACCurrentValue;
float Power;
float Price;

const int ACPin = A2;         //seting the arduino signal read pin
#define ACTectionRange 10    //setting Non-invasive AC Current Sensor tection range to 10A

#define VREF 5.0 //Refrence voltage for ADC

float readACCurrentValue()
{
  float ACCurrtntValue = 0;
  float peakVoltage = 0;
  float voltageVirtualValue = 0;  //Vrms
  for (int i = 0; i < 5; i++)
  {
    peakVoltage += analogRead(ACPin);   //read peak voltage
    delay(1);
  }
  peakVoltage = peakVoltage / 5; //Averaging voltage
  voltageVirtualValue = peakVoltage * 0.707;    //change the peak voltage to the Virtual Value of voltage

  /*The circuit is amplified by 2 times, so it is divided by 2.*/
  voltageVirtualValue = (voltageVirtualValue / 1024 * VREF ) / 2;

  ACCurrtntValue = voltageVirtualValue * ACTectionRange;

  return ACCurrtntValue;
}

void setup()
{
  Serial.begin(115200);

  Serial.println("Connecting to Wifi");
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) //Keep on trying till we get connected
  {

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass); //Connect to Wifi

    // wait 0.5 seconds for connection:
    delay(500);
  }

  // connected, print out the data
  Serial.print("You're connected to the network");

  delay(5000);
  rtc.begin(); //Enabling the real time clock

  unsigned long epoch;

  int numberOfTries = 0, maxTries = 6; //Max tries to read data from online NPT source

  do {

    epoch = WiFi.getTime();

    numberOfTries++;

  }

  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {

    Serial.print("NTP unreachable!!");

    while (1);

  }

  else {

    Serial.print("Epoch received: ");

    Serial.println(epoch);

    rtc.setEpoch(epoch); //Setting epoch

    Serial.println();

  }

}


void loop()
{
  ACCurrentValue = readACCurrentValue(); //read AC Current Value
  Serial.print(ACCurrentValue); //Printing the current value
  //1 amp =240 watts or
  Serial.println(" A");
  Serial.print("Power KwH : ");
  Power = 0.24 * ACCurrentValue; //Calculating Kw
  Serial.println(Power);
  Serial.print("Price :");
  Price = 0.1245 * Power; //Calculating price per Kw
  Serial.println(Price);
  digitalWrite(13, HIGH); //Blinking the internal LED to show program progress
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  printDate(); //Printing Date read from RTC
  _Servercall(); //Sending data to server

}


void _Servercall()
{
  WiFiClient wifi;
  if (WiFi.status() == WL_CONNECTED) { //Check if wifi is connected or not

    HttpClient client = HttpClient(wifi, serverName); //Make a HTTP client to generate a POST request

    String contentType = "application/json"; //Define the Application or packet type.
//Packet
    String msg = "{\"id\":\"" + (String)(_id) + "\",\"Kwh\":\"" + (String)Power + "\",\"date\":\"" + (String)Date + "\",\"price\":\"" + (String)Price + "\"}";
    client.post("/api/energyusage", contentType, msg); //Where to send msg/Packet

    client.beginRequest(); //Open connection with Client
    client.sendHeader("Content-Type", "application/json"); //Set Application type
    client.post( msg); //Post packet

  }
  else {
    Serial.println("WiFi Disconnected");
  }
  //Send an HTTP POST request every 30 seconds
  delay(5000); //Wait 5s between each request so the server has ample time to process request after receiving
}


void printWifiData()
{
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  delay(1000);
}


void printTime()
{

  print2digits(rtc.getHours() + GMT); //Getting hours and converting to respective Timezone

  Serial.print(":");

  print2digits(rtc.getMinutes());

  Serial.print(":");

  print2digits(rtc.getSeconds());

  Serial.println();
}

void printDate()
{

  Date = (String)rtc.getDay() + (String)"/" + (String)rtc.getMonth() + (String)"/" + rtc.getYear();
  Serial.println(Date);
}

void print2digits(int number) {

  if (number < 10) {

    Serial.print("0"); //Add zero i.e 12:01:02 time so that a time and date format can be followed while showing it on serial monitor

  }

  Serial.print(number);
}
