## Sensor with BME280 Temperature Sensor

## Sensor IDs
| ID | Typ |
|:--- |:-------:|
| 1  | Temperature |
| 2  | Humidity |
| 3  | Barometer |
|201 | RemoteConfig |

## Remote Config Values

| Description       | Sensor-ID          | VAR | Payload  |  Default |
| ------------- |:-------------:|:----:|:-----:|:-----:
| Sleeptime     | 201 | VAR1 | 1-240 | 2 |
| Force Reads      | 201      | VAR2|    1-240 | 30 |
| Outdoor | 201      | VAR3 |    0/1 | 0 |
