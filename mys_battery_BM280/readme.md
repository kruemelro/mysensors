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
|202 | System-Config | | |
|    |             |Var1| Prefered Parent|
|    |             |Var2| Radio Power Level|
|    |             |Var3| Parent is static |
|    |             |Var4| Send Retry |
|    |             |Var5| Sleeptime|
|254 | System-Reporting | | |
|    |             |Status| Online|
|    |             |Var1| Transmissions |
|    |             |Var2| FailedTransmissions|


## System Config Values

| Name       | Sensor-ID          | VAR | Payload  |  Default | Description |
| ------------- |:-------------:|:----:|:-----:|:-----:| :-----:|
| Prefered Parent     | 202 | VAR1 | 0-254 | 0 | Defines parent node, 0 means automatic |
| Radio Power Level | 202 | VAR2 | 0-3| 2 | Set Radio Power Level<br/>0 = Min<br/>1 = Low<br/>2 = High<br/>3 = Max |
| Parent is static | 202 | VAR3 | 0-1 | 0 | Disable search for parent if prefered parent fails |
| Send Retry | 202 | VAR4 | 1-100 | 5 | Configures the amount of retrys if no ACK received |
| Sleeptime | 202 | VAR5 | 1-60 | 2 | Sleeptime of sensor in minutes |

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
