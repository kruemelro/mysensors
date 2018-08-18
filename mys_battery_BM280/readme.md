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
|200 | Multimeter |
|    |             |Voltage| Battery Voltage|
|201 | RemoteConfig | | |
|    |             |Var1| Sleeptime|

## Remote Config Values

| Description       | Sensor-ID          | VAR | Payload  |  Default |
| ------------- |:-------------:|:----:|:-----:|:-----:
| Sleeptime     | 201 | VAR1 | 1-60 | 2 |

## OpenHAB samples

### things
<code>
temperature     node01   "MySensorNode(01)"	[ nodeId=1, childId=1, smartSleep=true, nodeUpdateTimeout=70 ]<br>
humidity        node01   "MySensorNode(01)"	[ nodeId=1, childId=2, smartSleep=true, nodeUpdateTimeout=70 ]<br>
multimeter      node01   "MySensorNode(01)"	[ nodeId=1, childId=200, smartSleep=true, nodeUpdateTimeout=70 ]<br>
customSensor    node01   "MySensorNode(01)"	[ nodeId=1, childId=201, smartSleep=true, nodeUpdateTimeout=70 ]<br>
</code>

### items
<code>
DateTime bedroom_sensor_update				"Update [%1$tR]"					<clock>				(bedroom_group, group_sensor_update)							{ channel="mysensors:temperature:gateway:node01:lastupdate" }
Number	 bedroom_sensor_battery				"Schlafzimmer [%.0f %%]"			<battery>			(bedroom_group, group_sensor_battery)                           { channel="mysensors:temperature:gateway:node01:battery" }
Number	 bedroom_sensor_vcc					 "Schlafzimmer [%.2f V]"			 <energy>			 (bedroom_group, group_sensor_vcc)      						{ channel="mysensors:multimeter:gateway:node01:volt" }
Number   bedroom_sensor_sleeptime            "Schlafzimmer [%.0f Min]"          <sleeptime>         (bedroom_group,group_sensor_sleeptime)							{ channel="mysensors:customSensor:gateway:node01:var1" }
</code>