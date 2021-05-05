#include <ESP8266WiFi.h>
#include <Wire.h>
#include "SparkFun_VL53L1X.h"
#include <PubSubClient.h>

#define ssid "TP-Link 2,4G" //type your information inside the quotes
#define password "RSJTM2019@"
#define mqtt_server "192.168.0.30"
#define mqtt_user "pi"
#define mqtt_pass "Julianalaan1DALTO"
#define mqtt_port 1883

#define mqtt_serial_publish_ch "/dev/peopleCounter/serialdata/tx"
#define mqtt_people_counter_ch "/dev/peopleCounter/people"
#define mqtt_serial_publish_distance_ch "/dev/peopleCounterDistance/serialdata/tx"

WiFiClient espClient;
PubSubClient client(espClient);

char peopleCounterArray[50];

//Optional interrupt and shutdown pins. 
#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3

SFEVL53L1X sensor();

static int NOBODY = 0;
static int SOMEONE = 1;
static int LEFT = 0;
static int RIGHT = 1;

static int DIST_THRESHOLD_MAX[] = {1500, 1500};   // treshold of the two zones

static int PathTrack[] = {0, 0, 0, 0};
static int PathTrackFillingSize = 1; // init this to 1 as we start from state where nobody is any of the zones
static int LeftPreviousStatus = NOBODY;
static int RightPreviousStatus = NOBODY;

static int center[2] = {175, 239}; /* center of the two zones */
static int Zone = 0;
static int PeopleCount = 0;

static int ROI_height = 5;
static int ROI_width = 5;

void setup(void)
{
  Wire.begin(4, 5);

  Serial.begin(115200);
  sensor.setI2CAddress((uint8_t)29);

  if (sensor.init() == false)
    Serial.println("Sensor online!");
  sensor.setDistanceModeLong();

  Serial.setTimeout(500);// Set time out for setup_wifi();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  delay(1000);
  publishSerialData(0);
  publishPeopleCountData(PeopleCount);
  reconnect();

}

void setup_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void publishSerialData(int serialData) {
  serialData = max(0, serialData);
  if (!client.connected()) {
    reconnect();
  }
  String stringaCounter = String(serialData);
  stringaCounter.toCharArray(peopleCounterArray, stringaCounter.length() + 1);
  client.publish(mqtt_serial_publish_ch, peopleCounterArray);
}


void publishPeopleCountData(int thisPeopleCount) {
  if (!client.connected()) {
    reconnect();
  }
  String stringaCounter = String(thisPeopleCount);
  stringaCounter.toCharArray(peopleCounterArray, stringaCounter.length() + 1);
  client.publish(mqtt_people_counter_ch, peopleCounterArray);
}

void publishSerialDataDistance(int serialData) {
  //serialData = max(0, serialData);
  if (!client.connected()) {
    reconnect();
  }
  String stringaCounter = String(serialData);
  stringaCounter.toCharArray(peopleCounterArray, stringaCounter.length() + 1);
  client.publish(mqtt_serial_publish_distance_ch, peopleCounterArray);
}


void loop(void)
{
  uint16_t distance;
  client.loop();
  if (!client.connected())
  {
    reconnect();
  }

  sensor.setROI(ROI_height, ROI_width, center[Zone]);  // first value: height of the zone, second value: width of the zone
  delay(50);
  sensor.setTimingBudgetInMs(50); // Could this be set into setup as well?
  sensor.startRanging(); //Write configuration bytes to initiate measurement
  distance = sensor.getDistance(); //Get the result of the measurement from the sensor
  sensor.stopRanging();

//  Serial.println(distance);
//  publishSerialDataDistance(distance);

//  inject the new ranged distance in the people counting algorithm
 processPeopleCountingData(distance, Zone);

  Zone++;
  Zone = Zone % 2;

}

// NOBODY = 0, SOMEONE = 1, LEFT = 0, RIGHT = 1

void processPeopleCountingData(int16_t Distance, uint8_t zone) {

  int CurrentZoneStatus = NOBODY;
  int AllZonesCurrentStatus = 0;
  int AnEventHasOccured = 0;

  if (Distance < DIST_THRESHOLD_MAX[Zone]) {
    // Someone is in !
    CurrentZoneStatus = SOMEONE;
  }

  // left zone
  if (zone == LEFT) {

    if (CurrentZoneStatus != LeftPreviousStatus) {
      // event in left zone has occured
      AnEventHasOccured = 1;

      if (CurrentZoneStatus == SOMEONE) {
        AllZonesCurrentStatus += 1;
      }
      // need to check right zone as well ...
      if (RightPreviousStatus == SOMEONE) {
        // event in left zone has occured
        AllZonesCurrentStatus += 2;
      }
      // remember for next time
      LeftPreviousStatus = CurrentZoneStatus;
    }
  }
  // right zone
  else {

    if (CurrentZoneStatus != RightPreviousStatus) {

      // event in left zone has occured
      AnEventHasOccured = 1;
      if (CurrentZoneStatus == SOMEONE) {
        AllZonesCurrentStatus += 2;
      }
      // need to left right zone as well ...
      if (LeftPreviousStatus == SOMEONE) {
        // event in left zone has occured
        AllZonesCurrentStatus += 1;
      }
      // remember for next time
      RightPreviousStatus = CurrentZoneStatus;
    }
  }

  // if an event has occured
  if (AnEventHasOccured) {
    if (PathTrackFillingSize < 4) {
      PathTrackFillingSize ++;
    }

    // if nobody anywhere lets check if an exit or entry has happened
    if ((LeftPreviousStatus == NOBODY) && (RightPreviousStatus == NOBODY)) {

      // check exit or entry only if PathTrackFillingSize is 4 (for example 0 1 3 2) and last event is 0 (nobobdy anywhere)
      if (PathTrackFillingSize == 4) {
        // check exit or entry. no need to check PathTrack[0] == 0 , it is always the case
        Serial.println();
        if ((PathTrack[1] == 1)  && (PathTrack[2] == 3) && (PathTrack[3] == 2)) {
          // this is an entry
          publishSerialData(1);
          PeopleCount++;
        } else if ((PathTrack[1] == 2)  && (PathTrack[2] == 3) && (PathTrack[3] == 1)) {
          // This an exit
          publishSerialData(2);
          PeopleCount--;
        }
        PeopleCount = max(0, PeopleCount); // only values >= 0
        publishPeopleCountData(PeopleCount);
      }
      for (int i = 0; i < 4; i++) {
        PathTrack[i] = 0;
      }
      PathTrackFillingSize = 1;
    }
    else {
      // update PathTrack
      // example of PathTrack update
      // 0
      // 0 1
      // 0 1 3
      // 0 1 3 1
      // 0 1 3 3
      // 0 1 3 2 ==> if next is 0 : check if exit
      PathTrack[PathTrackFillingSize - 1] = AllZonesCurrentStatus;
    }
  }
}
 