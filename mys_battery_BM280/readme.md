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
|202 | SystemConfig | | |
|    |             |Var1| ParentNode|
|    |             |Var2| Radio Power Level|
|250 | SystemOnline | | |
|    |             |Status| Online|
|251 | SystemReport-TX | | |
|    |             |Var1| FailedTransmissions|

## Remote Config Values

| Name       | Sensor-ID          | VAR | Payload  |  Default |
| ------------- |:-------------:|:----:|:-----:|:-----:
| Sleeptime     | 201 | VAR1 | 1-60 | 2 |

## System Config Values

| Name       | Sensor-ID          | VAR | Payload  |  Default | Description |
| ------------- |:-------------:|:----:|:-----:|:-----:| :-----:|
| ParentNode     | 202 | VAR1 | 0-254 | 0 | Defines parent node, 0 means automatic |
| Radio Power Level | 202 | VAR2 | 1-4| 3 | Set Radio Power Level<br/>1 = Min<br/>2 = Low<br/>3 = High<br/>4 = Max |

## OpenHAB samples

### things
temperature     node01   "MySensorNode(01)"	[ nodeId=1, childId=1, smartSleep=true, nodeUpdateTimeout=70 ]

humidity        node01   "MySensorNode(01)"	[ nodeId=1, childId=2, smartSleep=true, nodeUpdateTimeout=70 ]

multimeter      node01   "MySensorNode(01)"	[ nodeId=1, childId=200, smartSleep=true, nodeUpdateTimeout=70 ]

customSensor    node01   "MySensorNode(01)"	[ nodeId=1, childId=201, smartSleep=true, nodeUpdateTimeout=70 ]


### items
DateTime bedroom_sensor_update				"Update [%1$tR]"					<clock>				(bedroom_group, group_sensor_update)							{ channel="mysensors:temperature:gateway:node01:lastupdate" }

Number	 bedroom_sensor_battery				"Schlafzimmer [%.0f %%]"			<battery>			(bedroom_group, group_sensor_battery)                           { channel="mysensors:temperature:gateway:node01:battery" }

Number	 bedroom_sensor_vcc					 "Schlafzimmer [%.2f V]"			 <energy>			 (bedroom_group, group_sensor_vcc)      						{ channel="mysensors:multimeter:gateway:node01:volt" }

Number   bedroom_sensor_sleeptime            "Schlafzimmer [%.0f Min]"          <sleeptime>         (bedroom_group,group_sensor_sleeptime)							{ channel="mysensors:customSensor:gateway:node01:var1" }
