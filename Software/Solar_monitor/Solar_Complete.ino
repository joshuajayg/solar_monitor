#include <Wire.h>
#include <LTC4151.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

const char* ssid = "Your_Access_Point";
const char* password = "Your_Password";
const char* mqtt_server = "Server_IP";

const float batteryEfficiency = .97;  // How much of the charge the batteries absorb
const float batteryCapacity = 450;  // Amp Hours

float initialBatteryAmpHours = 450;
float initialInverterWattHours = -2265;
float initialInverterAmpHours = -88.15;
float initialChargeControllerAmpHours = 163.54;
float initialChargeControllerWattHours = 4666;

// Input pins
//#define floatPin D3
  
// Tracking time
unsigned long lastTime1;
unsigned long lastTime2;
unsigned long lastTime3;
unsigned long lastTime4;
unsigned long length1;
unsigned long length2;
unsigned long length3;
unsigned long length4;

// MQTT setup
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg1[8];
char msg2[8];
char msg3[8];
char msg4[8];
char msg5[8];
char msg6[8];
char msg7[8];
char msg8[8];
char msg9[8];
char msg10[8];
char msg11[8];
char msg12[8];
char msg13[8];
char msg14[8];
char msg15[8];
char msg16[8];
char msg17[8];
char msg18[6];

// Define sensors
LTC4151 sensor1;
LTC4151 sensor2;
LTC4151 sensor3;
LTC4151 sensor4;
LTC4151 sensor5;
LTC4151 sensor6;

//Array setup
const int numReadings1 = 20;
float readings1[numReadings1];
int readIndex1 = 0;
float total1 = 0;
float amp1 = 0;

const int numReadings2 = 20;
float readings2[numReadings2];
int readIndex2 = 0;
float total2 = 0;
float amp2 = 0;

const int numReadings3 = 20;
float readings3[numReadings3];
int readIndex3 = 0;
float total3 = 0;
float amp3 = 0;

const int numReadings4 = 20;
float readings4[numReadings4];
int readIndex4 = 0;
float total4 = 0;
float amp4 = 0;

const int numReadings5 = 20;
float readings5[numReadings5];
int readIndex5 = 0;
float total5 = 0;
float amp5 = 0;

const int numReadings6 = 20;
float readings6[numReadings6];
int readIndex6 = 0;
float total6 = 0;
float amp6 = 0;

// Values
float inverterAmps;
float inverterVolts;
float inverterWatts;
float inverterWattHours;
float inverterAmpHours;
float chargeControllerAmps;
float chargeControllerVolts;
float chargeControllerWatts;
float chargeControllerWattHours;
float chargeControllerAmpHours;
float auxiliaryAmps;
float auxiliaryVolts;
float auxiliaryWatts;
float auxiliaryWattHours;
float auxiliaryAmpHours;
float batteryAmps;
float batteryWatts;
float batteryAmpHours;
float batteryWattHours;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("midnight");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void OTASetup()
{
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("Solar");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setInitialValues() {
  
    // Set initial values
    batteryAmpHours = initialBatteryAmpHours;
    inverterWattHours = initialInverterWattHours;
    inverterAmpHours = initialInverterAmpHours;
    chargeControllerAmpHours = initialChargeControllerAmpHours;
    chargeControllerWattHours = initialChargeControllerWattHours;
}

/*
void checkForFloat() {
  bool floatState = digitalRead(floatPin);
  if(floatState){
    batteryAmpHours = batteryCapacity;
  }
}
*/

void getReadings()
{
  unsigned long now = millis();

//array for amp smoothing
  total1 = total1 - readings1[readIndex1];
  readings1[readIndex1] = (sensor1.getLoadCurrent(0.000375));
  total1 = total1 + readings1[readIndex1];
  readIndex1 = readIndex1 + 1;

  if (readIndex1 >= numReadings1) {
    readIndex1 = 0;
  }
  amp1 = total1 / numReadings1;

  //Max input voltage is 102.4V full scale and 25mV resolution.
  float volt1 = sensor1.getInputVoltage();
  //Calculated watts from amp and volt.
  float watt1 = amp1*volt1;

  total2 = total2 - readings2[readIndex2];
  readings2[readIndex2] = (sensor2.getLoadCurrent(0.000375));
  total2 = total2 + readings2[readIndex2];
  readIndex2 = readIndex2 + 1;

  if (readIndex2 >= numReadings2) {
    readIndex2 = 0;
  }
  amp2 = total2 / numReadings2;

  float volt2 = sensor2.getInputVoltage();
  float watt2 = amp2*volt2;

  total3 = total3 - readings3[readIndex3];
  readings3[readIndex3] = (sensor3.getLoadCurrent(0.000375));
  total3 = total3 + readings3[readIndex3];
  readIndex3 = readIndex3 + 1;

  if (readIndex3 >= numReadings3) {
    readIndex3 = 0;
  }
  amp3 = total3 / numReadings3;

  float volt3 = sensor3.getInputVoltage();
  float watt3 = amp3*volt3;

  total4 = total4 - readings4[readIndex4];
  readings4[readIndex4] = (sensor4.getLoadCurrent(0.000375));
  total4 = total4 + readings4[readIndex4];
  readIndex4 = readIndex4 + 1;

  if (readIndex4 >= numReadings4) {
    readIndex4 = 0;
  }
  amp4 = total4 / numReadings4;

  float volt4 = sensor4.getInputVoltage();
  float watt4 = amp4*volt4;

  total5 = total5 - readings5[readIndex5];
  readings5[readIndex5] = (sensor5.getLoadCurrent(0.000375));
  total5 = total5 + readings5[readIndex5];
  readIndex5 = readIndex5 + 1;

  if (readIndex5 >= numReadings5) {
    readIndex5 = 0;
  }
  amp5 = total5 / numReadings5;
  
  float volt5 = sensor5.getInputVoltage();
  float watt5 = amp5*volt5;

  total6 = total6 - readings6[readIndex6];
  readings6[readIndex6] = (sensor6.getLoadCurrent(0.000375));
  total6 = total6 + readings6[readIndex6];
  readIndex6 = readIndex6 + 1;

  if (readIndex6 >= numReadings6) {
    readIndex6 = 0;
  }
  amp6 = total6 / numReadings6;

  float volt6 = sensor6.getInputVoltage();
  float watt6 = amp6*volt6;

  // Choose the greater of the two readings and set them as positive or negative
  if (amp1 > amp2) {
    chargeControllerAmps = -amp1;
    chargeControllerWatts = -watt1;
  } else {
    chargeControllerAmps = amp2;
    chargeControllerWatts = watt2;
  }

  // Calculate WH and AH
  length1 = (now-lastTime1);
  lastTime1 = now;
  float chargeControllerWattHoursNow = (chargeControllerWatts*length1/3600000);
  chargeControllerWattHours = (chargeControllerWattHoursNow + chargeControllerWattHours);
  float chargeControllerAmpHoursNow = (chargeControllerAmps*length1/3600000);
  chargeControllerAmpHours = (chargeControllerAmpHoursNow + chargeControllerAmpHours);

  if (volt1 > volt2) {
    chargeControllerVolts = volt1;
  } else{
    chargeControllerVolts = volt2;
  }

  if (amp3 > amp4) {
    inverterAmps = -amp3;
    inverterWatts = -watt3;
  } else {
    inverterAmps = amp4;
    inverterWatts = watt4;
  }

  length2 = (now-lastTime2);
  lastTime2 = now;
  float inverterWattHoursNow = (inverterWatts*length2/3600000);
  inverterWattHours = (inverterWattHoursNow + inverterWattHours);
  float inverterAmpHoursNow = (inverterAmps*length2/3600000);
  inverterAmpHours = (inverterAmpHoursNow + inverterAmpHours);
  
  if (volt3 > volt4) {
    inverterVolts = volt3;
  } else{
    inverterVolts = volt4;
  }

  if (amp5 > amp6) {
    auxiliaryAmps = -amp5;
    auxiliaryWatts = -watt5;
  } else {
    auxiliaryAmps = amp6;
    auxiliaryWatts = watt6;
  }

  length3 = (now-lastTime3);
  lastTime3 = now;
  float auxiliaryWattHoursNow = (auxiliaryWatts*length3/3600000);
  auxiliaryWattHours = (auxiliaryWattHoursNow + auxiliaryWattHours);
  float auxiliaryAmpHoursNow = (auxiliaryAmps*length3/3600000);
  auxiliaryAmpHours = (auxiliaryAmpHoursNow + auxiliaryAmpHours);
  
  if (volt5 > volt6) {
    auxiliaryVolts = volt5;
  } else{
    auxiliaryVolts = volt6;
  }

  batteryWatts = (chargeControllerWatts + inverterWatts + auxiliaryWatts);
  batteryAmps = (chargeControllerAmps + inverterAmps + auxiliaryAmps);
  
  length4 = (now-lastTime4);
  lastTime4 = now;
  float batteryWattHoursNow = (batteryWatts*length4/3600000);
  batteryWattHours = (batteryWattHoursNow + batteryWattHours);
  float batteryAmpHoursNow = (batteryAmps*length4/3600000);

  // Correct battery charge for battery inefficiency
  if (batteryAmpHoursNow > 0) {
    batteryAmpHours = (batteryAmpHoursNow*batteryEfficiency + batteryAmpHours);
  }
  else {
    batteryAmpHours = (batteryAmpHoursNow + batteryAmpHours);
  }
  
}

void printSerial()
{
  Serial.print(chargeControllerAmps);
  Serial.print("  ");
  Serial.print(chargeControllerWatts);
  Serial.print("  ");
  Serial.print(inverterAmps);
  Serial.print("  ");
  Serial.println(inverterWatts);
  
}

void sendMQTT()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  float(heap) = ESP.getFreeHeap();
  if (now - lastMsg > 5000) {  // send MQTT every 5000 ms
    lastMsg = now;
    snprintf (msg1, 7, "%3.2f", chargeControllerAmps);
    snprintf (msg2, 7, "%3.2f", chargeControllerVolts);
    snprintf (msg3, 8, "%4.2f", chargeControllerWatts);
    snprintf (msg4, 7, "%3.2f", inverterAmps);
    snprintf (msg5, 7, "%3.2f", inverterVolts);
    snprintf (msg6, 8, "%4.2f", inverterWatts);
    snprintf (msg7, 7, "%3.2f", auxiliaryAmps);
    snprintf (msg8, 7, "%3.2f", auxiliaryVolts);
    snprintf (msg9, 8, "%4.2f", auxiliaryWatts);
    snprintf (msg10, 7, "%5.0f", chargeControllerWattHours);
    snprintf (msg11, 8, "%4.2f", chargeControllerAmpHours);
    snprintf (msg12, 7, "%5.0f", inverterWattHours);
    snprintf (msg13, 8, "%4.2f", inverterAmpHours);
    snprintf (msg14, 7, "%3.2f", batteryAmps);
    snprintf (msg15, 8, "%4.2f", batteryAmpHours);
    snprintf (msg16, 7, "%5.0f", auxiliaryWattHours);
    snprintf (msg17, 8, "%4.2f", auxiliaryAmpHours);
    snprintf (msg18, 6, "%6.0f", heap); 
    
    client.publish("solar/CC/amps", msg1);
    client.publish("solar/CC/volts", msg2);
    client.publish("solar/CC/watts", msg3);
    client.publish("solar/inverter/amps", msg4);
    client.publish("solar/inverter/volts", msg5);
    client.publish("solar/inverter/watts", msg6);
    client.publish("solar/auxiliary/amps", msg7);
    client.publish("solar/auxiliary/volts", msg8);
    client.publish("solar/auxiliary/watts", msg9);
    client.publish("solar/CC/watthours", msg10);
    client.publish("solar/CC/amphours", msg11);
    client.publish("solar/inverter/watthours", msg12);
    client.publish("solar/inverter/amphours", msg13);
    client.publish("solar/battery/amps", msg14);
    client.publish("solar/battery/amphours", msg15);
    client.publish("solar/auxiliary/watthours", msg16);
    client.publish("solar/auxiliary/amphours", msg17);
    client.publish("solar/memory", msg18);
  }
}

void chargeState() {
  if (batteryAmpHours > batteryCapacity) {
    batteryAmpHours = batteryCapacity;
  }
  
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    chargeControllerAmpHours = 0;
    chargeControllerWattHours = 0;
    inverterAmpHours = 0;
    inverterWattHours = 0;
    auxiliaryAmpHours = 0;
    auxiliaryWattHours = 0;
  }
}

void setup()
{   
    // Start serial connection
    Serial.begin(9600);
    while (!Serial);

//    pinMode(floatPin, INPUT);

    // Connect to WiFi
    setup_wifi();

    // Setup MQTT server
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // OTA setup
    OTASetup();
    
    Wire.begin();

    // Initiate sensors (Address pins: L=Low, H=High, F=Float)
    sensor1.init(LTC4151::H, LTC4151::F);
    sensor2.init(LTC4151::F, LTC4151::H);
    sensor3.init(LTC4151::L, LTC4151::F);
    sensor4.init(LTC4151::F, LTC4151::L);
    sensor5.init(LTC4151::H, LTC4151::H);
    sensor6.init(LTC4151::L, LTC4151::L);

    //Arrays
    for (int thisReading1 = 0; thisReading1 < numReadings1; thisReading1++) {
      readings1[thisReading1] = 0;
    }
    for (int thisReading2 = 0; thisReading2 < numReadings2; thisReading2++) {
      readings2[thisReading2] = 0;
    }
    for (int thisReading3 = 0; thisReading3 < numReadings3; thisReading3++) {
      readings3[thisReading3] = 0;
    }
    for (int thisReading4 = 0; thisReading4 < numReadings4; thisReading4++) {
      readings4[thisReading4] = 0;
    }
    for (int thisReading5 = 0; thisReading5 < numReadings5; thisReading5++) {
      readings5[thisReading5] = 0;
    }
    for (int thisReading6 = 0; thisReading6 < numReadings6; thisReading6++) {
      readings6[thisReading6] = 0; 
    }               

    // Setup time
    lastTime1 = 0;
    lastTime2 = 0;
    lastTime3 = 0;

    setInitialValues();  //disable this if you want to start at zero
}

void loop()
{
//  checkForFloat();
  getReadings();
  chargeState();
  printSerial();
  sendMQTT();
  
  ArduinoOTA.handle();
  delay(250);
}
