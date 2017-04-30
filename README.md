# nagiosxi-wizard

## Adding new sensors

At the location /usr/local/nagiosxi/html/includes/configwizards/
Following files have to be changed in order to add new sensor:
* esensors-websensor/plugins/check-esensors.pl
* esensors-websensor/templates/websensor.cfg
* esensors-websensor/esensors-websensor.inc.php

### plugins/check-esensors.pl
This is the script which actually implements all the checks.

It contains the following structure which defines all sensors
(and corresponding XML node names):
```
my $sensors = {
    'temperature'  => { 'name' => 'Temperature',  'flag' => 'sht',  'value' => 'tm0', 'uom' => 'tun0', },
    'humidity'     => { 'name' => 'Humidity',     'flag' => 'sht',  'value' => 'hu0', },
    'illumination' => { 'name' => 'Illumination', 'flag' => 'ilum', 'value' => 'il0', },
    'voltage'      => { 'name' => 'Voltage',      'flag' => 'evin', 'value' => 'vin', },
    'thermistor'   => { 'name' => 'Thermistor',   'flag' => 'ethm', 'value' => 'thm', },
    'contact'      => { 'name' => 'Contact',      'flag' => 'ecin', 'value' => 'cin', 'type' => 'boolean' },
    'flood'        => { 'name' => 'Flood',        'flag' => 'efld', 'value' => 'fin', 'type' => 'boolean' },
    };
```

### esensors-websensor.inc.php
Esensors wizard user interface. Contains the following structure
which maps UI elements to Nagios service checks:
```
            $sensors = array (
                "temp" => array (
                    "wl" => "temp_warning_low",
                    "wh" => "temp_warning_high",
                    "cl" => "temp_critical_low",
                    "ch" => "temp_critical_high",
                    "service_description" => "Temperature",
                    "xml_check_command" => "check_esensor_temp",
                    "text_check_command" => "check_esensor_temp_txt",
                ),
                "humidity" => array (
                    "wl" => "humidity_warning_low",
                    "wh" => "humidity_warning_high",
                    "cl" => "humidity_critical_low",
                    "ch" => "humidity_critical_high",
                    "service_description" => "Humidity",
                    "xml_check_command" => "check_esensor_humidity",
                    "text_check_command" => "check_esensor_humidity_txt",
                ),
                "illumination" => array (
                    "wl" => "illumination_warning_low",
                    "wh" => "illumination_warning_high",
                    "cl" => "illumination_critical_low",
                    "ch" => "illumination_critical_high",
                    "service_description" => "Illumination",
                    "xml_check_command" => "check_esensor_light",
                    "text_check_command" => "check_esensor_light_txt",
                ),
                "thermistor" => array (
                    "wl" => "thm_warning_low",
                    "wh" => "thm_warning_high",
                    "cl" => "thm_critical_low",
                    "ch" => "thm_critical_high",
                    "service_description" => "Thermistor",
                    "xml_check_command" => "check_esensor_thermistor",
                ),
                "voltage" => array (
                    "wl" => "volt_warning_low",
                    "wh" => "volt_warning_high",
                    "cl" => "volt_critical_low",
                    "ch" => "volt_critical_high",
                    "service_description" => "Voltage",
                    "xml_check_command" => "check_esensor_voltage",
                ),
                "contact" => array (
                    "warn" => "contact",
                    "crit" => "contact",
                    "service_description" => "Contact sensor",
                    "xml_check_command" => "check_esensor_contact",
                ),
                "floodSensor" => array (
                    "warn" => "floodSensor",
                    "crit" => "floodSensor",
                    "service_description" => "Flood sensor",
                    "xml_check_command" => "check_esensor_flood",
                ),
            );
```

### templates/websensor.cfg
Contains templates of Nagios service checks.

Every service check links check defined in Nagios wizard UI
(esensors-websensor.inc.php, either xml_check_command or txt_check_command)
with a command line used to implement actual check.

Sample xml_check_command:
```
define command{
  command_name check_esensor_temp
  command_line $USER1$/check_esensor.pl --host $HOSTADDRESS$ --port $ARG3$ --device xml --sensor temp --limits="$ARG1$,$ARG2$" --timeout default $ARG4$
  }
```

Sample txt_check_command:
```
define command{
  command_name check_esensor_temp_txt
  command_line $USER1$/check_esensor.pl --host $HOSTADDRESS$ --port $ARG3$ --device em01 --sensor temp --limits="$ARG1$,$ARG2$" --timeout default
  }
```
