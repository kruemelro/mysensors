## Sensor with Reed Sensor

## Sensor IDs
| ID | Typ | Child-Item | openhab |
|:--- |:-------:|:--------:|:--------:|
| 1  | Door | | |
|    |             |Tripped| Window|
|    |             |Battery| Battery|
|    |             |Last Update| Update|
| 2  | Door | | |
|    |             |Tripped| Window|
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
|    |             |Var3| Retries |


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
    door            node011  "MySensorNode(01_1)"	[ nodeId=1, childId=1, smartSleep=true, nodeUpdateTimeout=70 ]
    door            node012  "MySensorNode(01_2)"	[ nodeId=1, childId=2, smartSleep=true, nodeUpdateTimeout=70 ]
    multimeter      node01   "MySensorNode(01)"	[ nodeId=1, childId=200, smartSleep=true, nodeUpdateTimeout=70 ]
    customSensor    node01   "MySensorNode(01)"	[ nodeId=1, childId=201, smartSleep=true, nodeUpdateTimeout=70 ]
    light           node01   "MySensorNode(01)"	[ nodeId=1, childId=54, smartSleep=true, nodeUpdateTimeout=70 ]


### items
    Contact  bedroom_window               "Schlafzimmer [MAP(window.map):%s]"       <window>      (bedroom_group, group_window)             { channel="mysensors:door:gateway:node112:tripped" }
    DateTime bedroom_sensor_update        "Update [%1$tR]"                          <time>        (bedroom_group, group_sensor_update)      { channel="mysensors:temperature:gateway:node01:lastupdate" }
    Number	 bedroom_sensor_battery       "Schlafzimmer [%.0f %%]"                  <battery>     (bedroom_group, group_sensor_battery)     { channel="mysensors:temperature:gateway:node01:battery" }
    Number	 bedroom_sensor_vcc           "Schlafzimmer [%.2f V]"                   <energy>      (bedroom_group, group_sensor_vcc)         { channel="mysensors:multimeter:gateway:node01:volt" }
    Number	 bedroom_sensor_parent        "Schlafzimmer [%.0f]"                                   (bedroom_group)                           { channel="mysensors:customSensor:gateway:node01:var1" }
    Number	 bedroom_sensor_radiopower    "Schlafzimmer [MAP(radiopower.map):%s]"                 (bedroom_group)                           { channel="mysensors:customSensor:gateway:node01:var2" }
    Number	 bedroom_sensor_staticparent  "Schlafzimmer [MAP(staticparent.map):%s]"               (bedroom_group)                           { channel="mysensors:customSensor:gateway:node01:var3" }
    Number	 bedroom_sensor_sendretry     "Schlafzimmer [%.0f]"                                   (bedroom_group)                           { channel="mysensors:customSensor:gateway:node01:var4" }
    Number   bedroom_sensor_sleeptime     "Schlafzimmer [%.0f Min]"                 <sleeptime>   (bedroom_group, group_sensor_sleeptime)   { channel="mysensors:customSensor:gateway:node01:var5" }
    Switch   bedroom_sensor_online        "Schlafzimmer"                            <online>      (bedroom_group, group_sensor_online)      { channel="mysensors:light:gateway:node01:status", expire="70m,command=OFF"  }
    Number	 bedroom_sensor_tx            "Schlafzimmer [%.0f]"                                   (bedroom_group)                           { channel="mysensors:light:gateway:node01:var1" }
    Number	 bedroom_sensor_txfailed      "Schlafzimmer [%.0f]"                                   (bedroom_group)                           { channel="mysensors:light:gateway:node01:var2" }
    Number	 bedroom_sensor_txretries     "Schlafzimmer [%.0f]"                                   (bedroom_group)                           { channel="mysensors:light:gateway:node01:var3" }
