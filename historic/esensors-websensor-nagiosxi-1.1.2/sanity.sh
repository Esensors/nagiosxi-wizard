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

do_these_files_exist $LIBEXEC/check_em01.pl $WIZARDS/esensors-websensor/esensors-websensor.inc.php

is_wizard $WIZARDS/esensors-websensor/esensors-websensor.inc.php

can_nagios_execute $LIBEXEC/check_em01.pl
 
can_apache_execute $LIBEXEC/check_em01.pl

commands_exist check_emc_clariion check_em01_temp check_em01_humidity check_em01_light check_em08_temp check_em08_humidity check_em08_light check_em08_rtd check_em08_voltage check_em08_contacts
templates_exist xiwizard_websensor_host xiwizard_generic_host xiwizard_websensor_ping_service xiwizard_websensor_service

print_results

