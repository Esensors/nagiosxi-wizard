<?php
//
// Websensor Config Wizard
// Copyright (c) 2008-2015 Nagios Enterprises, LLC. All rights reserved.
//
// $Id$

include_once(dirname(__FILE__) . '/../configwizardhelper.inc.php');

esensors_websensor_configwizard_init();

function esensors_websensor_configwizard_init()
{
    $name = "esensors_websensor";
    $args = array(
        CONFIGWIZARD_NAME => $name,
        CONFIGWIZARD_VERSION => "1.1.2",
        CONFIGWIZARD_TYPE => CONFIGWIZARD_TYPE_MONITORING,
        CONFIGWIZARD_DESCRIPTION => _("Monitor temperature, humidity, and light levels on a Esensors Websensor."),
        CONFIGWIZARD_DISPLAYTITLE => _("Esensors Websensor"),
        CONFIGWIZARD_FUNCTION => "esensors_websensor_configwizard_func",
        CONFIGWIZARD_PREVIEWIMAGE => "em01.png",
        CONFIGWIZARD_FILTER_GROUPS => array('network'),
        CONFIGWIZARD_REQUIRES_VERSION => 500
    );
    register_configwizard($name, $args);
}


/**
 * @param string $mode
 * @param null   $inargs
 * @param        $outargs
 * @param        $result
 *
 * @return string
 */
function esensors_websensor_configwizard_func($mode = "", $inargs = null, &$outargs, &$result)
{
    $wizard_name = "esensors_websensor";

    // Initialize return code and output
    $result = 0;
    $output = "";

    // Initialize output args - pass back the same data we got
    $outargs[CONFIGWIZARD_PASSBACK_DATA] = $inargs;


    switch ($mode) {
        case CONFIGWIZARD_MODE_GETSTAGE1HTML:

            $address = grab_array_var($inargs, "address", "");

            $output = '

<p>' . _('For more information on ESensor\'s environmental monitoring products, or to place an order for a Websensor, visit') . ' <a href="http://www.eesensors.com/" target="_blank">www.eesensors.com</a>.</p>

<h5 class="ul">' . _('Websensor Information') . '</h5>
<table class="table table-condensed table-no-border table-auto-width">
    <tr>
        <td class="vt">
            <label>' . _('Address') . ':</label>
        </td>
        <td>
            <input type="text" size="40" name="address" id="address" value="' . htmlentities($address) . '" class="textfield form-control">
            <div class="subtext">' . _('The IP address or FQDNS name of the Websensor device you\'d like to monitor') . '.</div>
        </td>
    </tr>
    <tr>
        <td valign="top">
            <label>'._('Websensor Model').':</label>
        </td>
        <td>
            <select name="model" id="model" class="form-control">
                <option value="EM01B">EM01B</option>
                <option value="EM01B-STN">EM01B-STN</option>
                <option value="EM01B-VLT">EM01B-VLT</option>
                <option value="EM01B-THM">EM01B-THM</option>
                <option value="EM08-T">EM08-T</option>
            </select>
            <div class="subtext">' . _('The model number of the Websensor device you\'d like to monitor') . '.</div>
        </td>
    </tr>
</table>';
            break;

        case CONFIGWIZARD_MODE_VALIDATESTAGE1DATA:

            // Get variables that were passed to us
            $address = grab_array_var($inargs, "address", "");
            $model = grab_array_var($inargs, "model", "");

            // Check for errors
            $errors = 0;
            $errmsg = array();
            if (have_value($address) == false)
                $errmsg[$errors++] = _("No address specified.");

            if ($errors > 0) {
                $outargs[CONFIGWIZARD_ERROR_MESSAGES] = $errmsg;
                $result = 1;
            }

            break;

        case CONFIGWIZARD_MODE_GETSTAGE2HTML:

            // Get variables that were passed to us
            $address = grab_array_var($inargs, "address");
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
            $illumination_warning_high = "70";
            $illumination_critical_low = "";
            $illumination_critical_high = "80";

            $output = '
<input type="hidden" name="address" value="' . htmlentities($address) . '">
<input type="hidden" name="model" value="' . htmlentities($model) . '">

<h5 class="ul">' . _('Websensor Details') . '</h5>
<table class="table table-condensed table-no-border table-auto-width">
    <tr>
        <td>
            <label>' . _('Websensor Model') . ':</label>
        </td>
        <td>
            ' . htmlentities($model) . '
        </td>
    </tr>
    <tr>
        <td>
            <label>' . _('Address') . ':</label>
        </td>
        <td>
            <input type="text" size="20" name="address" id="address" value="' . htmlentities($address) . '" class="textfield form-control" disabled>
        </td>
    </tr>
    <tr>
        <td class="vt">
            <label>' . _('Host Name') . ':</label>
        </td>
        <td>
            <input type="text" size="20" name="hostname" id="hostname" value="' . htmlentities($hostname) . '" class="textfield form-control">
            <div class="subtext">' . _('The name you\'d like to have associated with this Websensor') . '.</div>
        </td>
    </tr>
</table>

<h5 class="ul">' . _('Device Metrics') . '</h5>
<p>' . _('Specify which metrics you\'d like to monitor on the Websensor') . '.</p>
<table class="table table-no-border table-auto-width">
    <tr>
        <td class="vt">
            <input type="checkbox" class="checkbox" name="services[ping]" checked>
        </td>
        <td>
            <b>' . _('Ping') . '</b><br>
            ' . _('Monitors the Websensor with an ICMP ping. Useful for watching network latency and general uptime') . '.
        </td>
    </tr>
    <tr>
        <td class="vt">
            <input type="checkbox" class="checkbox" name="services[temp]" checked>
        </td>
        <td>
            <b>' . _('Temperature') . '</b><br>
            ' . _('Monitors the temperature readings from the device') . '.
            <div class="pad-t5">
                <label><img src="'.theme_image('error.png').'" class="tt-bind" title="'._('Warning Threshold').'"> ' . _('Below') . ':</label> <input type="text" size="3" name="serviceargs[temp_warning_low]" value="' . htmlentities($temp_warning_low) . '" class="textfield form-control condensed">
                <label> ' . _('Above') . ':</label> <input type="text" size="3" name="serviceargs[temp_warning_high]" value="' . htmlentities($temp_warning_high) . '" class="textfield form-control condensed"> Deg. F
            </div>
            <div class="pad-t5">
                <label><img src="'.theme_image('critical_small.png').'" class="tt-bind" title="'._('Critical Threshold').'"> ' . _('Below') . ':</label> <input type="text" size="3" name="serviceargs[temp_critical_low]" value="' . htmlentities($temp_critical_low) . '" class="textfield form-control condensed">
                <label>' . _('Above') . ':</label> <input type="text" size="3" name="serviceargs[temp_critical_high]" value="' . htmlentities($temp_critical_high) . '" class="textfield form-control condensed"> Deg. F
            </div>
        </td>
    </tr>
    <tr>
        <td class="vt">
            <input type="checkbox" class="checkbox" name="services[humidity]" checked>
        </td>
        <td>
            <b>' . _('Humidity') . '</b><br>
            ' . _('Monitors the humidity readings from the device') . '.
            <div class="pad-t5">
                <label><img src="'.theme_image('error.png').'" class="tt-bind" title="'._('Warning Threshold').'"> ' . _('Below') . ':</label> <input type="text" size="3" name="serviceargs[humidity_warning_low]" value="' . htmlentities($humidity_warning_low) . '" class="textfield form-control condensed">
                <label> ' . _('Above') . ':</label> <input type="text" size="3" name="serviceargs[humidity_warning_high]" value="' . htmlentities($humidity_warning_high) . '" class="textfield form-control condensed"> %
            </div>
            <div class="pad-t5">
                <label><img src="'.theme_image('critical_small.png').'" class="tt-bind" title="'._('Critical Threshold').'"> ' . _('Below') . ':</label> <input type="text" size="3" name="serviceargs[humidity_critical_low]" value="' . htmlentities($humidity_critical_low) . '" class="textfield form-control condensed">
                <label>' . _('Above') . ':</label> <input type="text" size="3" name="serviceargs[humidity_critical_high]" value="' . htmlentities($humidity_critical_high) . '" class="textfield form-control condensed"> %
            </div>
        </td>
    </tr>
    <tr>
        <td class="vt">
            <input type="checkbox" class="checkbox" name="services[illumination]" checked>
        </td>
        <td>
            <b>' . _('Illumination') . '</b><br>
            ' . _('Monitors the illumination (light level) readings from the device') . '.
            <div class="pad-t5">
                <label><img src="'.theme_image('error.png').'" class="tt-bind" title="'._('Warning Threshold').'"> ' . _('Below') . ':</label> <input type="text" size="3" name="serviceargs[illumination_warning_low]" value="' . htmlentities($illumination_warning_low) . '" class="textfield form-control condensed">
                <label> ' . _('Above') . ':</label> <input type="text" size="3" name="serviceargs[illumination_warning_high]" value="' . htmlentities($illumination_warning_high) . '" class="textfield form-control condensed">
            </div>
            <div class="pad-t5">
                <label><img src="'.theme_image('critical_small.png').'" class="tt-bind" title="'._('Critical Threshold').'"> ' . _('Below') . ':</label> <input type="text" size="3" name="serviceargs[illumination_critical_low]" value="' . htmlentities($illumination_critical_low) . '" class="textfield form-control condensed">
                <label>' . _('Above') . ':</label> <input type="text" size="3" name="serviceargs[illumination_critical_high]" value="' . htmlentities($illumination_critical_high) . '" class="textfield form-control condensed">
            </div>
        </td>
    </tr>
</table>';
            break;

        case CONFIGWIZARD_MODE_VALIDATESTAGE2DATA:

            // get variables that were passed to us
            $address = grab_array_var($inargs, "address");
            $hostname = grab_array_var($inargs, "hostname");
            $model = grab_array_var($inargs, "model");

            // check for errors
            $errors = 0;
            $errmsg = array();
            if (is_valid_host_name($hostname) == false)
                $errmsg[$errors++] = _("Invalid host name.");

            if ($errors > 0) {
                $outargs[CONFIGWIZARD_ERROR_MESSAGES] = $errmsg;
                $result = 1;
            }

            break;


        case CONFIGWIZARD_MODE_GETSTAGE3HTML:

            // get variables that were passed to us
            $address = grab_array_var($inargs, "address");
            $hostname = grab_array_var($inargs, "hostname");
            $model = grab_array_var($inargs, "model");
            $services = grab_array_var($inargs, "services");
            $serviceargs = grab_array_var($inargs, "serviceargs");

            $output = '

        <input type="hidden" name="address" value="' . htmlentities($address) . '">
        <input type="hidden" name="hostname" value="' . htmlentities($hostname) . '">
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

            // see which services we should monitor
            foreach ($services as $svc => $svcstate) {

                //echo "PROCESSING: $svc -> $svcstate<BR>\n";

                switch ($svc) {

                    case "ping":
                        $objs[] = array(
                            "type" => OBJECTTYPE_SERVICE,
                            "host_name" => $hostname,
                            "service_description" => "Ping",
                            "use" => "xiwizard_websensor_ping_service",
                            "_xiwizard" => $wizard_name,
                        );
                        break;

                    case "temp":

                        $wl = grab_array_var($serviceargs, "temp_warning_low");
                        if ($wl == "")
                            $wl = "x";
                        $wh = grab_array_var($serviceargs, "temp_warning_high");
                        if ($wh == "")
                            $wh = "x";
                        $cl = grab_array_var($serviceargs, "temp_critical_low");
                        if ($cl == "")
                            $cl = "x";
                        $ch = grab_array_var($serviceargs, "temp_critical_high");
                        if ($ch == "")
                            $ch = "x";

                        switch ($model) {
                            case "EM08-T":
                                $warn = $wl . " " . $wh;
                                $crit = $cl . " " . $ch;
                                $objs[] = array(
                                    "type" => OBJECTTYPE_SERVICE,
                                    "host_name" => $hostname,
                                    "service_description" => "Temperature",
                                    "use" => "xiwizard_websensor_service",
                                    "check_command" => "check_em08_temp!" . $warn . "!" . $crit . "",
                                    "_xiwizard" => $wizard_name,
                                );
                                break;
                            default:
                                $warn = $wl . "/" . $wh;
                                $crit = $cl . "/" . $ch;
                                $objs[] = array(
                                    "type" => OBJECTTYPE_SERVICE,
                                    "host_name" => $hostname,
                                    "service_description" => "Temperature",
                                    "use" => "xiwizard_websensor_service",
                                    "check_command" => "check_em01_temp!" . $warn . "!" . $crit . "",
                                    "_xiwizard" => $wizard_name,
                                );
                                break;
                        }
                        break;

                    case "humidity":

                        $wl = grab_array_var($serviceargs, "humidity_warning_low");
                        if ($wl == "")
                            $wl = "x";
                        $wh = grab_array_var($serviceargs, "humidity_warning_high");
                        if ($wh == "")
                            $wh = "x";
                        $cl = grab_array_var($serviceargs, "humidity_critical_low");
                        if ($cl == "")
                            $cl = "x";
                        $ch = grab_array_var($serviceargs, "humidity_critical_high");
                        if ($ch == "")
                            $ch = "x";

                        switch ($model) {
                            case "EM08-T":
                                $warn = $wl . " " . $wh;
                                $crit = $cl . " " . $ch;
                                $objs[] = array(
                                    "type" => OBJECTTYPE_SERVICE,
                                    "host_name" => $hostname,
                                    "service_description" => "Humidity",
                                    "use" => "xiwizard_websensor_service",
                                    "check_command" => "check_em08_humidity!" . $warn . "!" . $crit . "",
                                    "_xiwizard" => $wizard_name,
                                );
                                break;
                            default:
                                $warn = $wl . "/" . $wh;
                                $crit = $cl . "/" . $ch;
                                $objs[] = array(
                                    "type" => OBJECTTYPE_SERVICE,
                                    "host_name" => $hostname,
                                    "service_description" => "Humidity",
                                    "use" => "xiwizard_websensor_service",
                                    "check_command" => "check_em01_humidity!" . $warn . "!" . $crit . "",
                                    "_xiwizard" => $wizard_name,
                                );
                                break;
                        }
                        break;

                    case "illumination":

                        $wl = grab_array_var($serviceargs, "illumination_warning_low");
                        if ($wl == "")
                            $wl = "x";
                        $wh = grab_array_var($serviceargs, "illumination_warning_high");
                        if ($wh == "")
                            $wh = "x";
                        $cl = grab_array_var($serviceargs, "illumination_critical_low");
                        if ($cl == "")
                            $cl = "x";
                        $ch = grab_array_var($serviceargs, "illumination_critical_high");
                        if ($ch == "")
                            $ch = "x";

                        switch ($model) {
                            case "EM08-T":
                                $warn = $wl . " " . $wh;
                                $crit = $cl . " " . $ch;
                                $objs[] = array(
                                    "type" => OBJECTTYPE_SERVICE,
                                    "host_name" => $hostname,
                                    "service_description" => "Illumination",
                                    "use" => "xiwizard_websensor_service",
                                    "check_command" => "check_em08_light!" . $warn . "!" . $crit . "",
                                    "_xiwizard" => $wizard_name,
                                );
                                break;
                            default:
                                $warn = $wl . "/" . $wh;
                                $crit = $cl . "/" . $ch;
                                $objs[] = array(
                                    "type" => OBJECTTYPE_SERVICE,
                                    "host_name" => $hostname,
                                    "service_description" => "Illumination",
                                    "use" => "xiwizard_websensor_service",
                                    "check_command" => "check_em01_light!" . $warn . "!" . $crit . "",
                                    "_xiwizard" => $wizard_name,
                                );
                                break;
                        }
                        break;
                    default:
                        break;
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