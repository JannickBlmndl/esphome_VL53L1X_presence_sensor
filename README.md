# ESPHome VL53L1X presence sensor

### Goal:<br/>
The goal of this project to detect the presence of people in a room and also count how many there are present in the room. To, eventually, with that information toggle the state of the light(s) and do other automation 'stuff'.

### Pathway and logbook: <br/>
| Description     |  Expected date  | Actual date  | ☒ /<br/> ☐   | Result commentary |
|----------------| ---------------|------------|------------|------------|
| Making this pathway | February 8| February 8 |☒ | 
| Delivery of the VL53L1X time of flight sensors | February 9 - 10 | February 10 |  ☒ |
| Checking if hardware is nondefective | February 10| February 10 |  ☒ | The I2C-device address differed from the SparkFun and Polulu boards. This meant that the address was needed to be set before the initalisation of the sensor. 
| Dry testing using <a href="https://github.com/Andrea-Fox/peopleCounter">Andrea Fox's Arduino IDE sketch</a>| February 10 | February 10 |  ☒ | 
| Testing distance sensor esphome component <a href="https://github.com/esphome/esphome/pull/1447">PR #1447 </a>| February 10 | February 10 |  ☒ | See <a href="https://github.com/esphome/esphome/pull/1447#issuecomment-777426587">issue comment</a>
| Rough _"It'll either work or explode"_ VL53L1X presence detecting in ESPHome as a custom component | February 11| Never | - | Currently this doesn't look like the way to go. *Deprecated*| 
| Clean ESPHome integration as a component following the code guidelines etc.|?|?| ☐ |
| Get the software for the component merged to an official ESPHome component| ?| ?|  ☐ |

**Steps to an ESPHome VL53L1X presence component**

- pull the plain distance measuring code out from the libraries into esphome c++ (vl53l1x_distance)
- add possibilty to use and switch the ROI zones (vl53l1x_presence)
- add algorithm for presence and direction (vl53l1x_presence)
### Join on discord:<br/>

https://discord.gg/65eBamz7AS

