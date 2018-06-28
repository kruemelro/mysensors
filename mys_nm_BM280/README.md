# Sensor with BME280 Temperature Sensor

## Sensor IDs

| ID | Typ | Child-Item | openhab |
|:--- |:-------:|:--------:|:--------:|
| 1  | Temperature | | |
|    |             |Temperature| Temperature|
|    |             |Battery| Battery|
|    |             |Last Update| Update|
| 2  | Humidity | | |
|    |             |Humidity| Humidity|
| 3  | Pressure | | |
|    |             |Pressure| Air Pressure|
| 4  | Forecast | | |
|    |             |Forecast| Forecast|
|200 | Remote API | | |
|201 | Battery | | |
|    |             | Voltage | |
|203 | openHAB CFG | | |
|    |             |Var1| Sleeptime|

## Remote Config Values

| Description       | Sensor-ID          | VAR | Payload  |  Default |
| ------------- |:-------------:|:----:|:-----:|:-----:|
| Sleeptime     | 201 | VAR1 | 1-60 | 2 |

## Power consumption

| MOD  | Power Consumption Awake | Power Consumption Sleep |
| ------------- |:-------------:|:----:|
| NodeManager standard | 15.24mA | 0.35mA |
| BME280 Forced Mode | 15.24mA | 0.014mA |

## calculated Runtime

<https://oregonembedded.com/batterycalc.htm>

| Config | Sleep time in minutes | calculated runtime in days |
| ------------- |:-------------:|:----:|
| NodeManager standard | 2 | 202 |
| NodeManager standard | 5 | 240 |
| NodeManager standard | 15 | 260 |
| BME280 Forced Mode | 2 | 679 |
| BME280 Forced Mode | 5 | 1477 |
| BME280 Forced Mode | 15 | 3093 |
