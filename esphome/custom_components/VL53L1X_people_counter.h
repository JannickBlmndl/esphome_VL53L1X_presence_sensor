/* https://esphome.io/components/sensor/custom.html */
#include "esphome.h"
#include <SparkFun_VL53L1X.h>

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
  #define NOBODY 0
  #define SOMEONE 1
  #define LEFT 0
  #define RIGHT 1
  #define DIST_THRESHOLD_MAX  1000
  
  // settings for optical center of each ROI. These may need adjustment
  // depending on your distance from the sensor, and your application:
  unsigned int opticalCenter[] = {159, 231};
  
  // An int for switching the zones, left (0) or right(1):
  int zone = 0;
  
  // width and height of the ROI; These may need adjustment as well:
  const int roiWidth = 7;
  const int roiHeight = 16;
  
  // moved this from ST function to global for use in loop():
  int PeopleCounter = 0;

  void setup() override {
    // This will be called by App.setup()
   
    Wire.begin();
   
    // sensor.begin works the reverse of many libraries: when it succeeds, it returns 0:
    if (sensor.begin() != 0) {
      ESP_LOGD("custom", "VL53L1X not connected, please check your wiring.");
      while (true);
    }
  
    // settings from Sparkfun's recommendations for short/medium length:
    sensor.setDistanceModeShort();
    sensor.setTimingBudgetInMs(33);
    sensor.setIntermeasurementPeriod(33);
  
    // set the ROI for next time:
    sensor.setROI(roiWidth, roiHeight,  opticalCenter[zone]);

  }
  void loop() override {
    // This will be called by App.loop()

    //initiate measurement:
    sensor.startRanging();
    // See if the sensor has a reading:
    while (!sensor.checkForDataReady());
    // get the distance in mm:
    int distance = sensor.getDistance();
    // clear the sensor's interrupt and turn off ranging:
    sensor.clearInterrupt();
    sensor.stopRanging();
  
    // set the ROI for next time:
    sensor.setROI(roiWidth, roiHeight,  opticalCenter[zone]);
  
    PeopleCounter = ProcessPeopleCountingData(distance, zone);

    people_counter->publish_state(PeopleCounter)
    ESP_LOGD("custom", "PeopleCount = %f", this->state);
    
    // Switch zones from 0 to 1 or 1 to 0:
    zone = !zone;
  }

 private:
  
  int ProcessPeopleCountingData(unsigned int thisDist, int thisZone) {
    static int PathTrack[] = {0, 0, 0, 0};
    static int PathTrackFillingSize = 1; // init this to 1 as we start from state where nobody is any of the zones
    static int LeftPreviousStatus = NOBODY;
    static int RightPreviousStatus = NOBODY;
    static int PeopleCount = 0;
  
  
    int CurrentZoneStatus = NOBODY;
    int AllZonesCurrentStatus = 0;
    int AnEventHasOccured = 0;
  
    if (thisDist < DIST_THRESHOLD_MAX) {
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
            Serial.println("left");
  
          } else if ((PathTrack[1] == 2)  && (PathTrack[2] == 3) && (PathTrack[3] == 1)) {
            // This an exit
            PeopleCount --;
            Serial.println("right");
          }
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
  
    // output debug data to main host machine
    return (PeopleCount);
  
  }
};