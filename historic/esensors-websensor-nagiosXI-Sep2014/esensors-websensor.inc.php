<?php
// WEBSENSOR CONFIG WIZARD
//
// Copyright (c) 2008-2014 Nagios Enterprises, LLC.  All rights reserved.
//
// $Id: esensors-websensor.inc.php 965 2012-12-27 19:20:41Z yribbens $

include_once(dirname(__FILE__).'/../configwizardhelper.inc.php');

// run the initialization function
esensors_websensor_configwizard_init();

function esensors_websensor_configwizard_init(){

	$name="esensors_websensor";

	$args=array(
		CONFIGWIZARD_NAME => $name,
        CONFIGWIZARD_VERSION => "1.1",
		CONFIGWIZARD_TYPE => CONFIGWIZARD_TYPE_MONITORING,
		CONFIGWIZARD_DESCRIPTION => gettext("Monitor temperature, humidity, light levels, flood, Door status, AC/ UPS voltage on a")." <a href='http://www.eesensors.com' target='blank'>ESensors</a> Websensor.",
		CONFIGWIZARD_DISPLAYTITLE => "Websensor",
		CONFIGWIZARD_FUNCTION => "esensors_websensor_configwizard_func",
		CONFIGWIZARD_PREVIEWIMAGE => "em01.png",
		);

	register_configwizard($name,$args);
	}



function esensors_websensor_configwizard_func($mode="",$inargs=null,&$outargs,&$result){

	$wizard_name="esensors_websensor";

	// initialize return code and output
	$result=0;
	$output="";

	// initialize output args - pass back the same data we got
	$outargs[CONFIGWIZARD_PASSBACK_DATA]=$inargs;


	switch($mode){
		case CONFIGWIZARD_MODE_GETSTAGE1HTML:

			$address=grab_array_var($inargs,"address","");

			$output='

	<div class="sectionTitle">'.gettext('Websensor Information').'</div>


	<table>

	<tr>
	<td valign="top">
	<label>'.gettext('Address').':</label><br class="nobr" />
	</td>
	<td>
<input type="text" size="40" name="address" id="address" value="'.htmlentities($address).'" class="textfield" /><br class="nobr" />
	'.gettext('The IP address or FQDNS name of the Websensor device you\'d like to monitor').'.<br><br>
	</td>
	</tr>

	<tr>
	<td valign="top">
	<label>Websensor Model:</label><br class="nobr" />
	</td>
	<td>
	<select name="model" id="model">
	<option value="EM32">EM32 - Environ. Monitor</option>
	<option value="AQUO">AQUO - Flood Sensors</option>
	<option value="MOS">MoS - Motion Sensors</option>
	<option value="SM1">SM1 - GreenHouse Sensors</option>
	</select>
	<br class="nobr" />
	'.gettext('The make / model number of the Websensor device you\'d like to monitor').'.
	</td>
	</tr>

	</table>

	<p>
	'.gettext('For more information on ESensor\'s environmental monitoring products, or to place an order for a Websensor, visit').' <a href="http://www.eesensors.com/" target="_blank">www.eesensors.com</a>.
	</p>
			';
			break;

		case CONFIGWIZARD_MODE_VALIDATESTAGE1DATA:

			// get variables that were passed to us
			$address=grab_array_var($inargs,"address","");
			$model=grab_array_var($inargs,"model","");


			// check for errors
			$errors=0;
			$errmsg=array();
			//$errmsg[$errors++]="Address: '$address'";
			if(have_value($address)==false)
				$errmsg[$errors++]=gettext("No address specified.");
			//else if(!valid_ip($address))
				//$errmsg[$errors++]="Invalid IP address.";

			if($errors>0){
				$outargs[CONFIGWIZARD_ERROR_MESSAGES]=$errmsg;
				$result=1;
				}

			break;

		case CONFIGWIZARD_MODE_GETSTAGE2HTML:

			// get variables that were passed to us
			$address=grab_array_var($inargs,"address");
			$model=grab_array_var($inargs,"model","");

			$ha=@gethostbyaddr($address);
			if($ha=="")
				$ha=$address;
			$hostname=grab_array_var($inargs,"hostname",$ha);

			//$ipaddress=@gethostbyname($address);

			$temp_warning_low="60";
			$temp_warning_high="85";
			$temp_critical_low="50";
			$temp_critical_high="95";

			$humidity_warning_low="15";
			$humidity_warning_high="80";
			$humidity_critical_low="10";
			$humidity_critical_high="90";

			$illumination_warning_low="";
			$illumination_warning_high="1000";
			$illumination_critical_low="";
			$illumination_critical_high="800";

			


			$output='


		<input type="hidden" name="address" value="'.htmlentities($address).'">
		<input type="hidden" name="model" value="'.htmlentities($model).'">

	<div class="sectionTitle">'.gettext('Websensor Details').'</div>

	<table>

	<tr>
	<td>
	<label>'.gettext('Websensor Model').':</label><br class="nobr" />
	</td>
	<td>
'.htmlentities($model).'<br class="nobr" />
	</td>
	</tr>

	<tr>
	<td>
	<label>'.gettext('Address').':</label><br class="nobr" />
	</td>
	<td>
<input type="text" size="20" name="address" id="address" value="'.htmlentities($address).'" class="textfield" disabled/><br class="nobr" />
	</td>
	</tr>

	<tr>
	<td>
	<label>Host Name:</label><br class="nobr" />
	</td>
	<td>
<input type="text" size="20" name="hostname" id="hostname" value="'.htmlentities($hostname).'" class="textfield" /><br class="nobr" />
	'.gettext('The name you\'d like to have associated with this Websensor').'.
	</td>
	</tr>

	</table>

	<div class="sectionTitle">'.gettext('Device Metrics').'</div>

	<p>'.gettext('Specify which metrics you\'d like to monitor on the Websensor').'.</p>

	<table>

	<tr>
	<td valign="top">
	<input type="checkbox" class="checkbox" name="services[ping]" checked>
	</td>
	<td>
	<b>'.gettext('Ping').'</b><br>
	'.gettext('Monitors the Websensor with an ICMP ping.  Useful for watching network latency and general uptime').'.<br><br>
	</td>
	</tr>

	<tr>
	<td valign="top">
	<input type="checkbox" class="checkbox" name="services[temp]" checked>
	</td>
	<td>
	<b>'.gettext('Temperature').'</b><br>
	'.gettext('Monitors the temperature readings from the device').'.<br>
	<label>'.gettext('Warning Below').':</label> <input type="text" size="3" name="serviceargs[temp_warning_low]" value="'.htmlentities($temp_warning_low).'" class="textfield" />
	<label> '.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[temp_warning_high]" value="'.htmlentities($temp_warning_high).'" class="textfield" /> Deg. F<br>
	<label>'.gettext('Critical Below').':</label> <input type="text" size="3" name="serviceargs[temp_critical_low]" value="'.htmlentities($temp_critical_low).'" class="textfield" />
	<label>'.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[temp_critical_high]" value="'.htmlentities($temp_critical_high).'" class="textfield" /> Deg. F
	<br><br>
	</td>
	</tr>

	<tr>
	<td valign="top">
	<input type="checkbox" class="checkbox" name="services[humidity]" checked>
	</td>
	<td>
	<b>'.gettext('Humidity').'</b><br>
	'.gettext('Monitors the humidity readings from the device').'.<br>
	<label>'.gettext('Warning Below').':</label> <input type="text" size="3" name="serviceargs[humidity_warning_low]" value="'.htmlentities($humidity_warning_low).'" class="textfield" />
	<label> '.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[humidity_warning_high]" value="'.htmlentities($humidity_warning_high).'" class="textfield" /> %<br>
	<label>'.gettext('Critical Below').':</label> <input type="text" size="3" name="serviceargs[humidity_critical_low]" value="'.htmlentities($humidity_critical_low).'" class="textfield" />
	<label>'.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[humidity_critical_high]" value="'.htmlentities($humidity_critical_high).'" class="textfield" /> %
	<br><br>
	</td>
	</tr>

	<tr>
	<td valign="top">
	<input type="checkbox" class="checkbox" name="services[illumination]" checked>
	</td>
	<td>
	<b>'.gettext('Illumination').'</b><br>
	'.gettext('Monitors the illumination (light level) readings from the device').'.<br>
	<label>'.gettext('Warning Below').':</label> <input type="text" size="3" name="serviceargs[illumination_warning_low]" value="'.htmlentities($illumination_warning_low).'" class="textfield" />
	<label> '.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[illumination_warning_high]" value="'.htmlentities($illumination_warning_high).'" class="textfield" /><br>
	<label>'.gettext('Critical Below').':</label> <input type="text" size="3" name="serviceargs[illumination_critical_low]" value="'.htmlentities($illumination_critical_low).'" class="textfield" />
	<label>'.gettext('Above').':</label> <input type="text" size="3" name="serviceargs[illumination_critical_high]" value="'.htmlentities($illumination_critical_high).'" class="textfield" />
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
	<input type="checkbox" class="checkbox" name="services[doorSwitch]" >
	</td>
	<td>
	<b>'.gettext('Contact Switch / Door sensor').'</b><br>
	'.gettext('Monitors Contact switch, Door open/close status').'.<br><br>
	<input type="radio" name="serviceargs[doorSwitch]" value="0" checked />Alarm if Door is Open<br>
	<input type="radio" name="serviceargs[doorSwitch]" value="1" /> Alarm if Door is Closed<br>
	<br><br>
	</td>
	</tr>

	<tr>
	<td valign="top">
	<input type="checkbox" class="checkbox" name="services[floodSensor]" >
	</td>
	<td>
	<b>'.gettext('Flood/ Water sensor').'</b><br>
	'.gettext('Monitors water spills and leakeages').'.<br>	
	<br><br>
	</td>
	</tr>
	</table>

			';
			break;

		case CONFIGWIZARD_MODE_VALIDATESTAGE2DATA:

			// get variables that were passed to us
			$address=grab_array_var($inargs,"address");
			$hostname=grab_array_var($inargs,"hostname");
			$model=grab_array_var($inargs,"model");

			// check for errors
			$errors=0;
			$errmsg=array();
			if(is_valid_host_name($hostname)==false)
				$errmsg[$errors++]=gettext("Invalid host name.");

			if($errors>0){
				$outargs[CONFIGWIZARD_ERROR_MESSAGES]=$errmsg;
				$result=1;
				}

			break;


		case CONFIGWIZARD_MODE_GETSTAGE3HTML:

			// get variables that were passed to us
			$address=grab_array_var($inargs,"address");
			$hostname=grab_array_var($inargs,"hostname");
			$model=grab_array_var($inargs,"model");
			$services=grab_array_var($inargs,"services");
			$serviceargs=grab_array_var($inargs,"serviceargs");

			$output='

		<input type="hidden" name="address" value="'.htmlentities($address).'">
		<input type="hidden" name="hostname" value="'.htmlentities($hostname).'">
		<input type="hidden" name="model" value="'.htmlentities($model).'">
		<input type="hidden" name="services_serial" value="'.htmlentities(base64_encode(serialize($services))).'">
		<input type="hidden" name="serviceargs_serial" value="'.htmlentities(base64_encode(serialize($serviceargs))).'">

			';
			break;

		case CONFIGWIZARD_MODE_VALIDATESTAGE3DATA:

			break;

		case CONFIGWIZARD_MODE_GETFINALSTAGEHTML:


			$output='
			';
			break;

		case CONFIGWIZARD_MODE_GETOBJECTS:

			$hostname=grab_array_var($inargs,"hostname","");
			$address=grab_array_var($inargs,"address","");
			$hostaddress=$address;

			$model=grab_array_var($inargs,"model");

			$services_serial=grab_array_var($inargs,"services_serial","");
			$serviceargs_serial=grab_array_var($inargs,"serviceargs_serial","");

			$services=unserialize(base64_decode($services_serial));
			$serviceargs=unserialize(base64_decode($serviceargs_serial));

			/*
			echo "SERVICES<BR>";
			print_r($services);
			echo "<BR>";
			echo "SERVICEARGS<BR>";
			print_r($serviceargs);
			echo "<BR>";
			*/

			// save data for later use in re-entrance
			$meta_arr=array();
			$meta_arr["hostname"]=$hostname;
			$meta_arr["address"]=$address;
			$meta_arr["model"]=$model;
			$meta_arr["services"]=$services;
			$meta_arr["serivceargs"]=$serviceargs;
			save_configwizard_object_meta($wizard_name,$hostname,"",$meta_arr);

			$objs=array();

			if(!host_exists($hostname)){
				$objs[]=array(
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
			foreach($services as $svc => $svcstate){

				//echo "PROCESSING: $svc -> $svcstate<BR>\n";

				switch($svc){

					case "ping":
						$objs[]=array(
							"type" => OBJECTTYPE_SERVICE,
							"host_name" => $hostname,
							"service_description" => "Ping",
							"use" => "xiwizard_websensor_ping_service",
							"_xiwizard" => $wizard_name,
							);
						break;

					case "temp":

						$wl=grab_array_var($serviceargs,"temp_warning_low");
						if($wl=="")
							$wl="x";
						$wh=grab_array_var($serviceargs,"temp_warning_high");
						if($wh=="")
							$wh="x";
						$cl=grab_array_var($serviceargs,"temp_critical_low");
						if($cl=="")
							$cl="x";
						$ch=grab_array_var($serviceargs,"temp_critical_high");
						if($ch=="")
							$ch="x";

						switch($model){						
						case "EM32":
						case "AQUO":
						case "MOS":
						case "SM1":
							$warn=$wl." ".$wh;
							$crit=$cl." ".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Temperature",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_temp!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
						default:
							$warn=$wl."/".$wh;
							$crit=$cl."/".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Temperature",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em01_temp!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
							}
						break;

					case "humidity":

						$wl=grab_array_var($serviceargs,"humidity_warning_low");
						if($wl=="")
							$wl="x";
						$wh=grab_array_var($serviceargs,"humidity_warning_high");
						if($wh=="")
							$wh="x";
						$cl=grab_array_var($serviceargs,"humidity_critical_low");
						if($cl=="")
							$cl="x";
						$ch=grab_array_var($serviceargs,"humidity_critical_high");
						if($ch=="")
							$ch="x";

						switch($model){
						case "EM32":
						case "AQUO":
						case "MOS":
						case "SM1":
							$warn=$wl." ".$wh;
							$crit=$cl." ".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Humidity",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_humidity!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
						default:
							$warn=$wl."/".$wh;
							$crit=$cl."/".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Humidity",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em01_humidity!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
							}
						break;

					case "illumination":

						$wl=grab_array_var($serviceargs,"illumination_warning_low");
						if($wl=="")
							$wl="x";
						$wh=grab_array_var($serviceargs,"illumination_warning_high");
						if($wh=="")
							$wh="x";
						$cl=grab_array_var($serviceargs,"illumination_critical_low");
						if($cl=="")
							$cl="x";
						$ch=grab_array_var($serviceargs,"illumination_critical_high");
						if($ch=="")
							$ch="x";

						switch($model){
						case "EM32":
						case "AQUO":
						case "MOS":
						case "SM1":
							$warn=$wl." ".$wh;
							$crit=$cl." ".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Illumination",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_light!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
						default:
							$warn=$wl."/".$wh;
							$crit=$cl."/".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Illumination",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em01_light!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
							}
						break;

					case "thermistor":

						$wl=grab_array_var($serviceargs,"thm_warning_low");
						if($wl=="")
							$wl="x";
						$wh=grab_array_var($serviceargs,"thm_warning_high");
						if($wh=="")
							$wh="x";
						$cl=grab_array_var($serviceargs,"thm_critical_low");
						if($cl=="")
							$cl="x";
						$ch=grab_array_var($serviceargs,"thm_critical_high");
						if($ch=="")
							$ch="x";

						switch($model){
						case "EM32":
						case "AQUO":
						case "MOS":
						case "SM1":
							$warn=$wl." ".$wh;
							$crit=$cl." ".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Thermistor",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_thermistor!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
						default:
							$warn=$wl."/".$wh;
							$crit=$cl."/".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Thermistor",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_thermistor!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
							}
						break;

					case "voltage":

						$wl=grab_array_var($serviceargs,"volt_warning_low");
						if($wl=="")
							$wl="x";
						$wh=grab_array_var($serviceargs,"volt_warning_high");
						if($wh=="")
							$wh="x";
						$cl=grab_array_var($serviceargs,"volt_critical_low");
						if($cl=="")
							$cl="x";
						$ch=grab_array_var($serviceargs,"volt_critical_high");
						if($ch=="")
							$ch="x";

						switch($model){
						case "EM32":
						case "AQUO":
						case "MOS":
						case "SM1":
							$warn=$wl." ".$wh;
							$crit=$cl." ".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Voltage",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_voltage!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
						default:
							$warn=$wl."/".$wh;
							$crit=$cl."/".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Voltage",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_voltage!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
							}
						break;

					case "doorSwitch":
			
						$aw=grab_array_var($serviceargs,"doorSwitch");
											

						switch($model){
						case "EM32":
						case "AQUO":
						case "MOS":
						case "SM1":
							$warn=$aw." ".$aw;
							$crit=$aw." ".$aw;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Door Switch",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_doorSwitch!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
						default:
							$warn=$aw."/".$aw;
							$crit=$aw."/".$aw;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Door Switch",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_doorSwitch!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
							}
						break;

					case "floodSensor":

						switch($model){
						case "EM32":
						case "AQUO":
						case "MOS":
						case "SM1":
							$warn="";
							$crit="";
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Flood/ Water Leaks",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_flood!".$warn."!".$crit."",
								"_xiwizard" => $wizard_name,
								);
							break;
						default:
							$warn=$wl."/".$wh;
							$crit=$cl."/".$ch;
							$objs[]=array(
								"type" => OBJECTTYPE_SERVICE,
								"host_name" => $hostname,
								"service_description" => "Flood/ Water Leaks",
								"use" => "xiwizard_websensor_service",
								"check_command" => "check_em08_flood!".$warn."!".$crit."",
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
			$outargs[CONFIGWIZARD_NAGIOS_OBJECTS]=$objs;

			break;

		default:
			break;
			}

	return $output;
	}

?>
