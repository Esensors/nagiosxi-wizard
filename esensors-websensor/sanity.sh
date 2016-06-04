#!/bin/bash

#esensors-websensors configwizard sanity check

function zipit() {
  :
}

#~ Include general library (should go in all sanity scripts.)
if [ ! -f /usr/local/nagiosxi/html/includes/components/sanitychecks/sanitylib.sh ];then
    echo "Sanity Checks Component not installed"
    exit 1
else 
    . /usr/local/nagiosxi/html/includes/components/sanitychecks/sanitylib.sh
fi

do_these_files_exist $LIBEXEC/check_esensor.pl $WIZARDS/esensors-websensor/esensors-websensor.inc.php

is_wizard $WIZARDS/esensors-websensor/esensors-websensor.inc.php

can_nagios_execute $LIBEXEC/check_esensor.pl
 
commands_exist check_esensor_temp_txt check_esensor_humidity_txt check_esensor_light_txt check_esensor_temp check_esensor_humidity check_esensor_light check_esensor_thermistor check_esensor_voltage check_esensor_contact check_esensor_flood
templates_exist xiwizard_websensor_host xiwizard_generic_host xiwizard_websensor_ping_service xiwizard_websensor_service

print_results

