/*
  Shout-out to
  Tom Igoe's code found on https://forum.arduino.cc/index.php?topic=621855.0
  who based on Sparkfun examples by Nathan Seidle
  and ST examples

  This custom component is based on:
  https://esphome.io/components/sensor/custom.html */
#include "esphome.h"
#include "SparkFun_VL53L1X.h"

class VL53L1X_people_counter : public Component, public Sensor {
 public:

  //Esphome component set up  
  Sensor *people_counter = new Sensor();
  VL53L1X_people_counter() : PollingComponent(500) {}
  // float get_setup_priority() const override { return esphome::setup_priority::XXXX; }

  // Library
  SFEVL53L1X sensor;

  // Definitions
  // People Counting defines (from ST example):
  static int NOBODY = 0;
  static int SOMEONE = 1;
  static int LEFT = 0;
  static int RIGHT = 1;
  
  static int DIST_THRESHOLD_MAX[] = {1500, 1500};   // treshold of the two zones
  
  static int PathTrack[] = {0, 0, 0, 0};
  static int PathTrackFillingSize = 1; // init this to 1 as we start from state where nobody is any of the zones
  static int LeftPreviousStatus = NOBODY;
  static int RightPreviousStatus = NOBODY;
  
  // settings for optical center of each ROI. These may need adjustment
  // depending on your distance from the sensor, and your application:
  static int center[2] = {175, 239}; /* center of the two zones */
  static int Zone = 0; // An int for switching the zones, left (0) or right(1):
  static int PeopleCount = 0;
  
  static int ROI_height = 5;
  static int ROI_width = 5;

  void setup() override {
  {
    Wire.begin(4, 5);
  
    Serial.begin(115200);
    sensor.setI2CAddress((uint8_t)29);
  
    // sensor.begin works the reverse of many libraries: when it succeeds, it returns 0:
    if (sensor.init() == true)
      ESP_LOGD("custom", "VL53L1X not connected, please check your wiring.");
      while (true);
    }
    
    // settings from Sparkfun's recommendations for short/medium length:
    sensor.setDistanceModeShort();
    sensor.setTimingBudgetInMs(33);
    sensor.setIntermeasurementPeriod(33);
    // sensor.setDistanceModeLong();

    // publishSerialData(0);
    // publishPeopleCountData(PeopleCount);
    //update state to 0
  
  }

  void loop() override {
    // This will be called by App.loop()
    uint16_t distance;
  
    sensor.setROI(ROI_height, ROI_width, center[Zone]);  // first value: height of the zone, second value: width of the zone
    // delay(50);
    sensor.startRanging(); //Write configuration bytes to initiate measurement
    // See if the sensor has a reading:
    while (!sensor.checkForDataReady());
    distance = sensor.getDistance(); //Get the result of the measurement from the sensor
    // clear the sensor's interrupt and turn off ranging:
    sensor.clearInterrupt();
    sensor.stopRanging();
  
    //  Serial.println(distance);
    //  publishSerialDataDistance(distance);
    
    //  inject the new ranged distance in the people counting algorithm
    // processPeopleCountingData(distance, Zone);

    int PeopleCounter = ProcessPeopleCountingData(distance, zone);
    
    people_counter->publish_state(PeopleCounter)
    ESP_LOGD("custom", "PeopleCount = %f", this->state);
  
    // Switch zones from 0 to 1 or 1 to 0:
    Zone++;
    Zone = Zone % 2;
    // zone = !zone;
  }
  
void ProcessPeopleCountingData(int16_t thisDistance, uint8_t thisZone) {
    int CurrentZoneStatus = NOBODY;
    int AllZonesCurrentStatus = 0;
    int AnEventHasOccured = 0;
  
    if (thisDistance < DIST_THRESHOLD_MAX[Zone]) {
      // Someone is in !
      CurrentZoneStatus = SOMEONE;
    }
  
    // left zone
    if (thisZone == LEFT) {
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
          if ((PathTrack[1] == 1)  && (PathTrack[2] == 3) && (PathTrack[3] == 2)) {
            // This an entry
            PeopleCount ++;
            // Serial.println("left");
          } else if ((PathTrack[1] == 2)  && (PathTrack[2] == 3) && (PathTrack[3] == 1)) {
            // This an exit
            PeopleCount --;
            // Serial.println("right");
          }
          PeopleCount = max(0, PeopleCount); // filter so only values >= 0
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
    return PeopleCount
    }
  }
// private:
};