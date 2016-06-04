<?php
// WEBSENSOR CONFIG WIZARD
//
// Copyright (c) 2008-2014 Nagios Enterprises, LLC. All rights reserved.
//
// $Id: esensors-websensor.inc.php 965 2012-12-27 19:20:41Z yribbens $

include_once(dirname(__FILE__) . '/../configwizardhelper.inc.php');

esensors_websensor_configwizard_init();

function esensors_websensor_configwizard_init()
{
    $name="esensors_websensor";

    $args = array(
        CONFIGWIZARD_NAME => $name,
        CONFIGWIZARD_VERSION => "1.1.3b",
        CONFIGWIZARD_TYPE => CONFIGWIZARD_TYPE_MONITORING,
        CONFIGWIZARD_DESCRIPTION => gettext("Monitor different parameters of Esensors device."),
        CONFIGWIZARD_DISPLAYTITLE => "Esensors Websensor",
        CONFIGWIZARD_FUNCTION => "esensors_websensor_configwizard_func",
        CONFIGWIZARD_PREVIEWIMAGE => "em01.png",
        CONFIGWIZARD_FILTER_GROUPS => array('network'),
    );

    register_configwizard($name, $args);
}



function esensors_websensor_configwizard_func($mode = "", $inargs = null, &$outargs, &$result)
{
    $wizard_name="esensors_websensor";

    // initialize return code and output
    $result = 0;
    $output = "";

    // initialize output args - pass back the same data we got
    $outargs[CONFIGWIZARD_PASSBACK_DATA] = $inargs;

    
    switch($mode){
        case CONFIGWIZARD_MODE_GETSTAGE1HTML:

            $address = grab_array_var($inargs, "address", "");

            $output = '

<div class="sectionTitle">'.gettext('Websensor Information').'</div>

<table>
    <tr>
        <td valign="top">
            <label>' . gettext('Address') . ':</label><br class="nobr" />
        </td>
        <td>
            <input type="text" size="40" name="address" id="address" value="' . htmlentities($address) . '" class="textfield" /><br class="nobr" />
            ' . gettext('The IP address or FQDNS name of the Websensor device you\'d like to monitor') . '.<br><br>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <label>' . gettext('Port') . ':</label><br class="nobr" />
        </td>
        <td>
            <input type="text" size="3" name="port" id="port" value="' . '80' . '" class="textfield" /><br class="nobr" />
            ' . gettext('Port of device (default value should work in the majority of cases)') . '.<br><br>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <label>API type:</label><br class="nobr" />
        </td>
        <td>
            <select name="model" id="model">
                <option value="xml">XML based interface (EM08, EM32 and others)</option>
                <option value="text">Plain text interface (EM01B)</option>
            </select>
            <br class="nobr" />
            ' . gettext('The API of the device - XML based interface suits most of the devices') . '.
        </td>
    </tr>
    <tr>
        <td valign="top">
            <label>API url:</label><br class="nobr" />
        </td>
        <td>
            <input type="text" size="15" name="api_url" id="api_url" value="' . '' . '" class="textfield" /><br class="nobr" />
            ' . gettext('leave empty to try default URL (/ssetings.xml for xml api, /index.html?345678 for text api)') . '.<br><br>
        </td>
    </tr>
</table>

<p>
    <br class="nobr" />
    ' . gettext('For more information on ESensor\'s environmental monitoring products, or to place an order for a Websensor, visit') .
    ' <a href="http://www.eesensors.com/" target="_blank">www.eesensors.com</a>.
</p>
      ';
            break;

        case CONFIGWIZARD_MODE_VALIDATESTAGE1DATA:

            // get variables that were passed to us
            $address = grab_array_var($inargs, "address", "");
            $port = grab_array_var($inargs, "port", "");
            $api_url = grab_array_var($inargs, "api_url", "");
            $model = grab_array_var($inargs, "model", "");

            // check for errors
            $errors = 0;
            $errmsg = array();
            if (have_value($address) == false)
                $errmsg[$errors++] = gettext("No address specified.");
            if (have_value($port) == false)
                $errmsg[$errors++] = gettext("No port specified.");

                if($errors>0){
                    $outargs[CONFIGWIZARD_ERROR_MESSAGES] = $errmsg;
                    $result = 1;
                }
            
            break;

        case CONFIGWIZARD_MODE_GETSTAGE2HTML:

            // get variables that were passed to us
            $address = grab_array_var($inargs, "address");
            $port = grab_array_var($inargs, "port", "");
            $api_url = grab_array_var($inargs, "api_url", "");
            $model = grab_array_var($inargs, "model", "");

            $ha = @gethostbyaddr($address);
            if ($ha == "")
                $ha = $address;
            $hostname = grab_array_var($inargs, "hostname", $ha);

            $temp_warning_low = "60";
            $temp_warning_high = "85";
            $temp_critical_low = "50";
            $temp_critical_high = "95";

            $humidity_warning_low = "15";
            $humidity_warning_high = "80";
            $humidity_critical_low = "10";
            $humidity_critical_high = "90";

            $illumination_warning_low = "";
            $illumination_warning_high = "1000";
            $illumination_critical_low = "";
            $illumination_critical_high = "800";

            $output = '
<input type="hidden" name="address" value="' . htmlentities($address) . '">
<input type="hidden" name="port" value="' . htmlentities($port) . '">
<input type="hidden" name="api_url" value="' . htmlentities($api_url) . '">
<input type="hidden" name="model" value="' . htmlentities($model) . '">

<div class="sectionTitle">' . gettext('Websensor Details') . '</div>

<table>
    <tr>
        <td>
            <label>' . gettext('API type') . ':</label>
            <br class="nobr" />
        </td>
        <td>
          ' . htmlentities($model) . '
          <br class="nobr" />
        </td>
    </tr>
    <tr>
        <td>
            <label>' . gettext('Address') . ':</label><br class="nobr" />
        </td>
        <td>
            <input type="text" size="20" name="address" id="address" value="' . htmlentities($address) . '" class="textfield" disabled/>
            <br class="nobr" />
        </td>
    </tr>
    <tr>
        <td valign=top>
            <label>Host Name:</label><br class="nobr" />
        </td>
        <td>
            <input type="text" size="20" name="hostname" id="hostname" value="' . htmlentities($hostname) . '" class="textfield" />
            <br class="nobr" />
            ' . gettext('The name you\'d like to have associated with this Websensor').'.
        </td>
    </tr>
    <tr>
        <td>
            <label>' . gettext('Port') . ':</label><br class="nobr" />
        </td>
        <td>
            <input type="text" size="20" name="port" id="port" value="' . htmlentities($port) . '" class="textfield" disabled/>
            <br class="nobr" />
        </td>
    </tr>
    <tr>
        <td>
            <label>' . gettext('API URL') . ':</label><br class="nobr" />
        </td>
        <td>
            <input type="text" size="20" name="api_url" id="api_url" value="' . htmlentities($api_url) . '" class="textfield" disabled/>
            <br class="nobr" />
        </td>
    </tr>
</table>

<div class="sectionTitle">' . gettext('Device Metrics') . '</div>
<p>' . gettext('Specify which metrics you\'d like to monitor on the Websensor') . '.</p>
<table>
    <tr>
        <td valign="top">
            <input type="checkbox" class="checkbox" name="services[ping]" checked>
        </td>
        <td>
            <b>' . gettext('Ping') . '</b><br>
            ' . gettext('Monitors the Websensor with an ICMP ping.  Useful for watching network latency and general uptime') . '.<br><br>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <input type="checkbox" class="checkbox" name="services[temp]" checked>
        </td>
        <td>
            <b>' . gettext('Temperature') . '</b><br>
            ' . gettext('Monitors the temperature readings from the device') . '.<br>
            <label>' . gettext('Warning Below') . ':</label> <input type="text" size="3" name="serviceargs[temp_warning_low]" value="' . htmlentities($temp_warning_low) . '" class="textfield" />
            <label> ' . gettext('Above') . ':</label> <input type="text" size="3" name="serviceargs[temp_warning_high]" value="' . htmlentities($temp_warning_high) . '" class="textfield" /> Deg. F<br>
            <label>' . gettext('Critical Below') . ':</label> <input type="text" size="3" name="serviceargs[temp_critical_low]" value="' . htmlentities($temp_critical_low) . '" class="textfield" />
            <label>' . gettext('Above') . ':</label> <input type="text" size="3" name="serviceargs[temp_critical_high]" value="' . htmlentities($temp_critical_high) . '" class="textfield" /> Deg. F
            <br><br>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <input type="checkbox" class="checkbox" name="services[humidity]" checked>
        </td>
        <td>
            <b>' . gettext('Humidity') . '</b><br>
            ' . gettext('Monitors the humidity readings from the device') . '.<br>
            <label>' . gettext('Warning Below') . ':</label> <input type="text" size="3" name="serviceargs[humidity_warning_low]" value="' . htmlentities($humidity_warning_low) . '" class="textfield" />
            <label> ' . gettext('Above') . ':</label> <input type="text" size="3" name="serviceargs[humidity_warning_high]" value="' . htmlentities($humidity_warning_high) . '" class="textfield" /> %<br>
            <label>' . gettext('Critical Below') . ':</label> <input type="text" size="3" name="serviceargs[humidity_critical_low]" value="' . htmlentities($humidity_critical_low) . '" class="textfield" />
            <label>' . gettext('Above') . ':</label> <input type="text" size="3" name="serviceargs[humidity_critical_high]" value="' . htmlentities($humidity_critical_high) . '" class="textfield" /> %
            <br><br>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <input type="checkbox" class="checkbox" name="services[illumination]" checked>
        </td>
        <td>
            <b>' . gettext('Illumination') . '</b><br>
            ' . gettext('Monitors the illumination (light level) readings from the device') . '.<br>
            <label>' . gettext('Warning Below').':</label> <input type="text" size="3" name="serviceargs[illumination_warning_low]" value="' . htmlentities($illumination_warning_low) . '" class="textfield" />
            <label> ' . gettext('Above').':</label> <input type="text" size="3" name="serviceargs[illumination_warning_high]" value="' . htmlentities($illumination_warning_high) . '" class="textfield" /><br>
            <label>' . gettext('Critical Below').':</label> <input type="text" size="3" name="serviceargs[illumination_critical_low]" value="' . htmlentities($illumination_critical_low) . '" class="textfield" />
            <label>' . gettext('Above').':</label> <input type="text" size="3" name="serviceargs[illumination_critical_high]" value="' . htmlentities($illumination_critical_high) . '" class="textfield" />
            <br><br>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <input type="checkbox" class="checkbox" name="services[thermistor]" >
        </td>
        <td>
            <b>'.gettext('Thermistor').'</b><br>
            '.gettext('Monitors the Thermistor Sensor readings from the device').'.<br>
            <label>'.gettext('Warning Below').':</label> <input type="text" size="3" name="serviceargs[thm_warning_low]" value="'.htmlentities($temp_warning_low).'" class="textfield" />
            <label> '.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[thm_warning_high]" value="'.htmlentities($temp_warning_high).'" class="textfield" /> Deg. F<br>
            <label>'.gettext('Critical Below').':</label> <input type="text" size="3" name="serviceargs[thm_critical_low]" value="'.htmlentities($temp_critical_low).'" class="textfield" />
            <label>'.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[thm_critical_high]" value="'.htmlentities($temp_critical_high).'" class="textfield" /> Deg. F
            <br><br>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <input type="checkbox" class="checkbox" name="services[voltage]" >
            </td>
            <td>
            <b>'.gettext('AC Line Voltage / UPS').'</b><br>
            '.gettext('Monitors your UPS or AC Line Voltage Levels').'.<br>
            <label>'.gettext('Warning Below').':</label> <input type="text" size="3" name="serviceargs[volt_warning_low]" value="" class="textfield" />
            <label> '.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[volt_warning_high]" value="" class="textfield" /> Volts<br>
            <label>'.gettext('Critical Below').':</label> <input type="text" size="3" name="serviceargs[volt_critical_low]" value="" class="textfield" />
            <label>'.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[volt_critical_high]" value="" class="textfield" /> Volts
            <br><br>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <input type="checkbox" class="checkbox" name="services[contact]" >
        </td>
        <td>
            <b>'.gettext('Contact Switch / Door sensor').'</b><br>
            '.gettext('Monitors Contact switch, Door open/close status').'.<br><br>
            <input type="radio" name="serviceargs[contact]" value="0" checked />Alarm if there\'s no contact<br>
            <input type="radio" name="serviceargs[contact]" value="1" /> Alarm if there\'s contact<br>
            <br><br>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <input type="checkbox" class="checkbox" name="services[floodSensor]" >
            <input type="hidden" name="serviceargs[floodSensor]" value="1" />
        </td>
        <td>
            <b>'.gettext('Flood/Water sensor').'</b><br>
            '.gettext('Monitors water spills and leakeages').'.<br> 
            <br><br>
        </td>
    </tr>
</table>
      ';
            break;

        case CONFIGWIZARD_MODE_VALIDATESTAGE2DATA:

            // get variables that were passed to us
            $address = grab_array_var($inargs, "address");
            $hostname = grab_array_var($inargs, "hostname");
            $port = grab_array_var($inargs, "port");
            $api_url = grab_array_var($inargs, "api_url");
            $model = grab_array_var($inargs, "model");

            // check for errors
            $errors = 0;
            $errmsg = array();
            if (is_valid_host_name($hostname) == false)
                $errmsg[$errors++] = gettext("Invalid host name.");

            if ($errors > 0) {
                $outargs[CONFIGWIZARD_ERROR_MESSAGES] = $errmsg;
                $result = 1;
            }

            break;


        case CONFIGWIZARD_MODE_GETSTAGE3HTML:

            // get variables that were passed to us
            $address = grab_array_var($inargs, "address");
            $hostname = grab_array_var($inargs, "hostname");
            $port = grab_array_var($inargs, "port");
            $api_url = grab_array_var($inargs, "api_url");
            $model = grab_array_var($inargs, "model");
            $services = grab_array_var($inargs, "services");
            $serviceargs = grab_array_var($inargs, "serviceargs");

            $output='

        <input type="hidden" name="address" value="' . htmlentities($address) . '">
        <input type="hidden" name="hostname" value="' . htmlentities($hostname) . '">
        <input type="hidden" name="port" value="' . htmlentities($port) . '">
        <input type="hidden" name="api_url" value="' . htmlentities($api_url) . '">
        <input type="hidden" name="model" value="' . htmlentities($model) . '">
        <input type="hidden" name="services_serial" value="' . htmlentities(base64_encode(serialize($services))) . '">
        <input type="hidden" name="serviceargs_serial" value="' . htmlentities(base64_encode(serialize($serviceargs))) . '">

            ';
            break;

        case CONFIGWIZARD_MODE_VALIDATESTAGE3DATA:
            break;

        case CONFIGWIZARD_MODE_GETFINALSTAGEHTML:
            $output = '
            ';
            break;

        case CONFIGWIZARD_MODE_GETOBJECTS:
            $hostname = grab_array_var($inargs, "hostname", "");
            $address = grab_array_var($inargs, "address", "");
            $port = grab_array_var($inargs, "port", "");
            $api_url = grab_array_var($inargs, "api_url", "");
            $hostaddress = $address;

            $model = grab_array_var($inargs, "model");

            $services_serial = grab_array_var($inargs, "services_serial", "");
            $serviceargs_serial = grab_array_var($inargs, "serviceargs_serial", "");

            $services = unserialize(base64_decode($services_serial));
            $serviceargs = unserialize(base64_decode($serviceargs_serial));

            // save data for later use in re-entrance
            $meta_arr = array();
            $meta_arr["hostname"] = $hostname;
            $meta_arr["address"] = $address;
            $meta_arr["model"] = $model;
            $meta_arr["port"] = $port;
            $meta_arr["services"] = $services;
            $meta_arr["serivceargs"] = $serviceargs;
            save_configwizard_object_meta($wizard_name, $hostname, "", $meta_arr);

            $objs = array();

            if (!host_exists($hostname)) {
                $objs[] = array(
                    "type" => OBJECTTYPE_HOST,
                    "use" => "xiwizard_websensor_host",
                    "host_name" => $hostname,
                    "address" => $hostaddress,
                    "icon_image" => "em01.png",
                    "statusmap_image" => "em01.png",
                    "_xiwizard" => $wizard_name,
                );
            }

            $sensors = array (
                "temp" => array (
                    "wl" => "temp_warning_low",
                    "wh" => "temp_warning_high",
                    "cl" => "temp_critical_low",
                    "ch" => "temp_critical_high",
                    "service_description" => "Temperature",
                    "xml_check_command" => "check_em08_temp",
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

            // see which services we should monitor
            foreach($services as $svc => $svcstate) {

                if ($svc == "ping") {
                    $objs[] = array(
                        "type" => OBJECTTYPE_SERVICE,
                        "host_name" => $hostname,
                        "service_description" => "Ping",
                        "use" => "xiwizard_websensor_ping_service",
                        "_xiwizard" => $wizard_name,
                    );
                }
                elseif (isset($sensors[$svc])) {
                    if (isset($sensors[$svc]["wl"])) {
                        $wl = grab_array_var($serviceargs, $sensors[$svc]["wl"]);
                        if ($wl == "")
                            $wl = "x";
                        $wh = grab_array_var($serviceargs, $sensors[$svc]["wh"]);
                        if ($wh == "")
                            $wh = "x";
                        $cl = grab_array_var($serviceargs, $sensors[$svc]["cl"]);
                        if ($cl == "")
                            $cl = "x";
                        $ch = grab_array_var($serviceargs, $sensors[$svc]["ch"]);
                        if ($ch == "")
                            $ch = "x";

                        $warn = $wl . " " . $wh;
                        $crit = $cl . " " . $ch;
                    }
                    elseif (isset($sensors[$svc]["warn"])) {
                        $warn = grab_array_var($serviceargs, $sensors[$svc]["warn"]);
                        if ($warn == "")
                            $warn = "x";

                        $crit = grab_array_var($serviceargs, $sensors[$svc]["crit"]);
                        if ($crit == "")
                            $crit = "x";
                    }
                            
                    if ($model == "xml") {
                        $objs[] = array(
                            "type" => OBJECTTYPE_SERVICE,
                            "host_name" => $hostname,
                            "service_description" => $sensors[$svc]["service_description"],
                            "use" => "xiwizard_websensor_service",
                            "check_command" => $sensors[$svc]["xml_check_command"] . "!" . $warn . "!" . $crit .
                                "!" . $port . ($api_url == "" ? "" : "!--url " . $api_url),
                            "_xiwizard" => $wizard_name,
                        );
                    }
                    elseif ($model == "text" && isset($sensors[$svc]["text_check_command"])) {
                        $objs[] = array(
                            "type" => OBJECTTYPE_SERVICE,
                            "host_name" => $hostname,
                            "service_description" => $sensors[$svc]["service_description"],
                            "use" => "xiwizard_websensor_service",
                            "check_command" => $sensors[$svc]["text_check_command"] . "!" . $warn . "!" . $crit . "!" . $port,
                            "_xiwizard" => $wizard_name,
                        );
                    }
                }
            }

            //echo "OBJECTS:<BR>";
            //print_r($objs);
            //exit();

            // return the object definitions to the wizard
            $outargs[CONFIGWIZARD_NAGIOS_OBJECTS] = $objs;

            break;

        default:
            break;
    }

    return $output;
}

?>
