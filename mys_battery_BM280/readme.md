## Sensor with BME280 Temperature Sensor

## Sensor IDs
| ID | Typ | Child-Item | openhab |
|:--- |:-------:|:--------:|:--------:|
| 1  | Temperature | | |
|    |             |Temperature| Temperature|
|    |             |Battery| Battery|
|    |             |Last Update| Update|
| 2  | Humidity | | |
|    |             |Humidity| Humidity|
| 3  | Barometer |
|    |             |Pressure| Air Pressure|
|    |             |Forecast| Forecast|
|201 | RemoteConfig | | |
|    |             |Var1| Sleeptime|
|    |             |Var2| ForceReads|
|    |             |Var3| Read Air Pressure|

## Remote Config Values

| Description       | Sensor-ID          | VAR | Payload  |  Default |
| ------------- |:-------------:|:----:|:-----:|:-----:
| Sleeptime     | 201 | VAR1 | 1-240 | 2 |
| Force Reads      | 201      | VAR2|    1-240 | 30 |
| Use Barometer | 201      | VAR3 |    0/1 | 0 |
