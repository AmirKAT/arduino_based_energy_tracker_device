#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>

WiFiClient myClient;
int status = WL_IDLE_STATUS;
// Replace with your network credentials
const char* ssid     = "REPLACE_WITH_YOUR_SSID";
const char* pass = "REPLACE_WITH_YOUR_PASSWORD";
float ACCurrentValue;
float Power;
float Price;
// REPLACE with your Domain name and URL path or IP address with path
const char* serverName = "https://stormy-inlet-66435.herokuapp.com/api/energyusage";
int port = 8080;

const int ACPin = A2;         //seting the arduino signal read pin
#define ACTectionRange 10;    //setting Non-invasive AC Current Sensor tection range to 10A

#define VREF 5.0

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
  peakVoltage = peakVoltage / 5;   
  voltageVirtualValue = peakVoltage * 0.707;    //change the peak voltage to the Virtual Value of voltage

  /*The circuit is amplified by 2 times, so it is divided by 2.*/
  voltageVirtualValue = (voltageVirtualValue / 1024 * VREF ) / 2;  

  ACCurrtntValue = voltageVirtualValue * ACTectionRange;

  return ACCurrtntValue;
}

void setup() 
{
  Serial.begin(115200);
  pinMode(13, OUTPUT);
 
  Serial.println("Connecting");
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();
}

void loop() 
{
   ACCurrentValue = readACCurrentValue(); //read AC Current Value
  Serial.print(ACCurrentValue);
  //1 amp =240 watts or 
  Serial.println(" A");
  Serial.print("Power KwH : ");
  Power = 0.24*ACCurrentValue;
  Serial.println(Power);
  Serial.print("Price :");
  Price = 0.1245 * Power;
  Serial.println(Price);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}

void _Servercall()
{
  if(WiFi.status()== WL_CONNECTED){
    HttpClient http = HttpClient(myClient, serverName, port);
    
    // Your Domain name with URL path or IP address with path
  //  http.begin(serverName);
    
    // Specify content-type header
  String contentType = "application/x-www-form-urlencoded";
    
    // Prepare your HTTP POST request data
    String httpRequestData = "id=" + (String)"5" + (String)"&Kw=" + (String)Power
                          + "&date=" + "22/03/21" + "&price" + (String)Price;
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    
   
    // Send HTTP POST request
    int httpResponseCode = http.post("/",contentType, httpRequestData);
     
    // If you need an HTTP request with a content type: text/plain
    //http.addHeader("Content-Type", "text/plain");
    //int httpResponseCode = http.POST("Hello, World!");
    
    // If you need an HTTP request with a content type: application/json, use the following:
    //http.addHeader("Content-Type", "application/json");
    //int httpResponseCode = http.POST("{\"value1\":\"19\",\"value2\":\"67\",\"value3\":\"78\"}");
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    //http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
  //Send an HTTP POST request every 30 seconds
  delay(30000);  
}


void printWifiData() 
{
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  delay(1000);
}

void printCurrentNet() 
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());



  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}
