#!/usr/bin/perl
#
# License to use per the terms of the GNU Public License (GPL)
#
# CHANGES:
# Modified by Nagios Enterprises January 2010 to return performance data
#
#$Id$
#
# EM01B http://eesensors.com/products/websensors/server-room-temperature-monitoring.html
# The EM01B websensor comes with factory calibrated digital temperature, humidity
# and illumination sensors. It provides a simple string output with sensor data.
# Temperature Range -40 to +123.8C. Temperature accurate to +/- 0.5C. Humidity Sensor
# accurate to +/-3%RH with range from 1% to 99% relative humidity.
# Fahrenheit or Celsius units available for temperature 
#
# EM32-Le http://eesensors.com/products/websensors/environmental-monitor-sensor.html
# Monitor Temperature, Humidity and light levels. Drop-in replacement for EM08.
# Supports API calls with sensor data output in XML format. 
#
# EM32-Xe http://eesensors.com/products/websensors/advanced-environmental-monitor.html
#
# EM08 http://eesensors.com/products/websensors/websensor-basic.html
# Our Second generation Websensor with SNMP/ Email options, 100 base T support,
# and temperature, humidity and illumination sensors.
#
# PM01 http://eesensors.com/products/web-power-meters/pm01.html
# Monitors 100-240V AC Line Voltage 50/ 60Hz.Standard XML /HTTP data outputs.
#
# PM01-Xe http://eesensors.com/products/web-power-meters/advanced-line-voltage-monitor-with-relay-pm01-xe.html
# Monitors 100-240V AC Line Voltage 50/ 60Hz.Standard XML /HTTP data outputs.
#
# AQUO Le http://eesensors.com/products/internet-wireless-water-detector-sensors/internet-water-sensor-detector-with-email-text-alerts.html
# AQUO LE Detects presence of water or other liquids in your basement, factory floors,
# labs or any locations. Supports API calls with sensor data output in JSON format. 
#
# AQUO-XE http://eesensors.com/products/internet-wireless-water-detector-sensors/advanced-internet-water-detector-sensor-with-email-text-snmp-cloud-alerts.html
# Supports API calls with sensor data output in XML format.
#
# APEX-LE http://eesensors.com/products/level-sensors-with-email-text-alerts/level-sensor-with-ethernet-email-text-alerts.html
# Monitor spot and continuous levels. Supports API calls
# with sensor data output in XML format. 
#
# APEX-XE http://eesensors.com/products/level-sensors-with-email-text-alerts/ethernet-level-sensor-with-email-text-alerts-relay-control.html
# Apex-Xe Level Sensors provide advanced level sensing, monitoring, reporting
# and control capabilities to your industrial and scientific installations.
# Supports API calls with sensor data output in XML format.
#
# SM1-Le http://eesensors.com/products/soil-humidity-moisture-water-content-sensor/soil-temperature-water-content-moisture-sensor.html
# SM1-Le offers continuous monitoring of soil moisture for your green house,
# farm, lab needs. Supports API calls with sensor data output in JSON format.
#
# SM1-Xe http://eesensors.com/products/soil-humidity-moisture-water-content-sensor/internet-soil-temperature-soil-moisture-water-content-sensor.html
# SM1-Xe is a highly reliable Soil Temperature and water content sensor
# in a rugged metallic package for continuous monitoring for your garden,
# green house, farm, lab needs. Supports API calls with sensor data output
# in XML format. 


# configure nagios utils
use lib "/usr/local/nagios/libexec";
use utils qw($TIMEOUT %ERRORS);

use strict;
use warnings;
use Getopt::Long;
use Data::Dumper;
use IO::Socket;
use Carp;

$| = 1;
$Data::Dumper::Sortkeys = 1;
                                                                  
Getopt::Long::Configure('bundling');

my $sensors = {
    'temperature'  => { 'name' => 'Temperature',  'flag' => 'sht',  'value' => 'tm0', 'uom' => 'tun0', },
    'humidity'     => { 'name' => 'Humidity',     'flag' => 'sht',  'value' => 'hu0', },
    'illumination' => { 'name' => 'Illumination', 'flag' => 'ilum', 'value' => 'il0', },
    'voltage'      => { 'name' => 'Voltage',      'flag' => 'evin', 'value' => 'vin', },
    'thermistor'   => { 'name' => 'Thermistor',   'flag' => 'ethm', 'value' => 'thm', },
    'contact'      => { 'name' => 'Contact',      'flag' => 'ecin', 'value' => 'cin', 'type' => 'boolean' },
    'flood'        => { 'name' => 'Flood',        'flag' => 'efld', 'value' => 'fin', 'type' => 'boolean' },
    };
$sensors->{'temp'} = $sensors->{'temperature'};
$sensors->{'hum'} = $sensors->{'humidity'};
$sensors->{'illum'} = $sensors->{'illumination'};
$sensors->{'light'} = $sensors->{'illumination'};

my $opt_debug = 0;
my $opt_ver = 0;
my $opt_help = 0;
my $opt_typ = '';
my $opt_temp = 'x/x,x/x';
my $opt_hum = 'x/x,x/x';
my $opt_illum = 'x/x,x/x';
my $opt_host;
my $opt_port = 80;
my $opt_timeout = $TIMEOUT;
my $opt_dev = 0;
my $opt_test;
my $opt_device = "xml";
my $opt_limits = 'x/x,x/x';
my $opt_url = "";

GetOptions(
    "version|v" => \$opt_ver,
    "debug|d" => \$opt_debug,
    "help|h" => \$opt_help,
    
    # tell web server to close HTTP/1.1 session explicitely
    # which actual device does without the flag (not sure
    # how it will react if the flag is present in request,
    # so not touching this as currently I don't have
    # actual device to test on)
    "dev" => \$opt_dev,

    # run internal tests
    "test" => \$opt_test,

    "host=s" => \$opt_host,
    "timeout=s" => \$opt_timeout,
    "port=s" => \$opt_port,

    "device=s" => \$opt_device,
    "sensor|type=s" => \$opt_typ,
    "limits|lim=s" => \$opt_limits,

    "url=s" => \$opt_url,

    # backward compatibility with old check_em01.pl
    "temp|temperature=s" => \$opt_temp,
    "hum|humidity=s" => \$opt_hum,
    "illum|illumination=s" => \$opt_illum,
);

if ($opt_help) {
    &syntax();
}
if ($opt_test) {
    &run_tests();
}

my $sensor_host = $opt_host || $ARGV[0] || &syntax();
if ($opt_timeout eq 'default') {
    $opt_timeout = $TIMEOUT;
}

if ($opt_device ne 'em01' && !$opt_typ) {
    &syntax();
}
if (! defined($sensors->{$opt_typ})) {
    &syntax();
}
if (!$opt_url) {
    if ($opt_device eq 'em01') {
        $opt_url = "/index.html?em345678";
    }
    else {
        $opt_url = "/ssetings.xml";
    }
}

#############################################
my $vals = &read_sensor($sensor_host, $opt_timeout);
&debug(Data::Dumper::Dumper($vals));

my @msgs = ();
my $condition = $ERRORS{'OK'};

if ($opt_device eq 'xml') {
    if (! defined($vals->{$sensors->{$opt_typ}->{'flag'}})) {
        return_nagios_status('UNKNOWN', "UNKNOWN: sensor [$sensors->{$opt_typ}->{'name'} ($opt_typ)] " .
            "(field $sensors->{$opt_typ}->{'flag'}) is not present on the device");
    }
    if ($vals->{$sensors->{$opt_typ}->{'flag'}} ne 'inline') {
        return_nagios_status('UNKNOWN', "UNKNOWN: sensor [$sensors->{$opt_typ}->{'name'} ($opt_typ)]: ".
            "field $sensors->{$opt_typ}->{'flag'} is [$vals->{$sensors->{$opt_typ}->{'flag'}}];" . 
            "sensor is not present on the device");
    }
    if (! defined($vals->{$sensors->{$opt_typ}->{'value'}})) {
        return_nagios_status('UNKNOWN', "UNKNOWN: sensor [$sensors->{$opt_typ}->{'name'} ($opt_typ)] " .
            "(field $sensors->{$opt_typ}->{'value'}) is not present on the device");
    }

    my @limits = split(/[\/\,\ ]/, $opt_limits);
    if (defined($sensors->{$opt_typ}->{'type'}) &&
        $sensors->{$opt_typ}->{'type'} eq 'boolean') {

        my $uctype = ucfirst($sensors->{$opt_typ}->{'value'});
        if ($limits[1] ne 'x' && $vals->{$sensors->{$opt_typ}->{'value'}} eq $limits[1]) {
            push(@msgs, "CRITICAL $uctype (=" . $vals->{$sensors->{$opt_typ}->{'value'}} . ") - ");
            $condition = 'CRITICAL';
        }
        elsif ($limits[0] ne 'x' && $vals->{$sensors->{$opt_typ}->{'value'}} eq $limits[0]) {
            push(@msgs, "WARNING $uctype (=" . $vals->{$sensors->{$opt_typ}->{'value'}} . ") - ");
            $condition = 'WARNING';
        }
    }
    else {
        $condition = check_value($sensors->{$opt_typ}->{'value'}, $vals, @limits);
    }

    my $msg = "";
    if ($#msgs > -1) {
      $msg .= join("; ", @msgs);
    }
    $msg .=
        $sensors->{$opt_typ}->{'name'} . ": " .
        $vals->{$sensors->{$opt_typ}->{'value'}} .
        (defined($sensors->{$opt_typ}->{'uom'}) && $vals->{$sensors->{$opt_typ}->{'uom'}} ?
         " " . $vals->{$sensors->{$opt_typ}->{'uom'}} : "")
        ;
    $msg .= "|";
    $msg .=
        $sensors->{$opt_typ}->{'name'} . "=" .
        $vals->{$sensors->{$opt_typ}->{'value'}} .
        (defined($sensors->{$opt_typ}->{'uom'}) && $vals->{$sensors->{$opt_typ}->{'uom'}} ?
         $vals->{$sensors->{$opt_typ}->{'uom'}} : "") .
        ";;;;\n";

    return_nagios_status($condition, $msg);
}
elsif ($opt_device eq 'em01') {
    if ($opt_typ eq '') {
      my @temp = split(/[\/,]/, $opt_temp);
      my @hum = split(/[\/,]/, $opt_hum);
      my @illum = split(/[\/,]/, $opt_illum);

      $condition = check_value('temperature', $vals, @temp);
      $condition = check_value('humidity', $vals, @hum);
      $condition = check_value('illumination', $vals, @illum);

      if ($#msgs > -1) {
        print join("; ", @msgs);
      }

      print "Temp: $vals->{'temperature'} $vals->{'temp-unit'}, ";
      print "Humidity: $vals->{'humidity'}%, ";
      print "Illum: $vals->{'illumination'}\n";
    } else {
      if ($opt_typ eq "temp") {
        my @temp = split(/[\/,]/, $opt_temp || $opt_limits);
        $condition = check_value('temperature', $vals, @temp);

        if ($#msgs > -1) {
          print join("; ", @msgs);
        }

        print "Temp: $vals->{'temperature'} $vals->{'temp-unit'}";
        print "|";
        print "temperature=$vals->{'temperature'}$vals->{'temp-unit'};;;; \n";
      } elsif ($opt_typ eq "hum") {
        my @hum = split(/[\/,]/, $opt_hum || $opt_limits);
        $condition = check_value('humidity', $vals, @hum);

        if ($#msgs > -1) {
          print join("; ", @msgs);
        }

        print "Humidity: $vals->{'humidity'}%";
        print "|";
        print "humidity=$vals->{'humidity'}%;;;; \n";
      } elsif ($opt_typ eq "illum") {
        my @illum = split(/[\/,]/, $opt_illum || $opt_limits);
        $condition = check_value('illumination', $vals, @illum);

        if ($#msgs > -1) {
          print join("; ", @msgs);
        }

        print "Illum: $vals->{'illumination'}";
        print "|";
        print "illumination=$vals->{'illumination'};;;; \n";
      }
      else {
        return_nagios_status('UNKNOWN', "UNKNOWN: sensor [$sensors->{$opt_typ}->{'name'} ($opt_typ)] is not present on $opt_device device");
        print "";
      }
    }
}
else {
    &syntax();
}

exit($ERRORS{$condition});

#############################################

sub return_nagios_status {
    my ($state, $msg) = @_;
    print $msg . ($msg =~ /\n$/s ? "" : "\n");
    exit($ERRORS{$state});
}

sub check_value {
    my ($type, $vals, $w_lo, $w_hi, $c_lo, $c_hi) = @_;
    my $current = $vals->{$type};

    my $uctype = ucfirst $type;
    if ($c_lo ne 'x' && $current < $c_lo) {
        push(@msgs, "CRITICAL LOW $uctype (<$c_lo) - ");
        return 'CRITICAL';
    } elsif ($c_hi ne 'x' && $current > $c_hi) {
        push(@msgs, "CRITICAL HIGH $uctype (>$c_hi) - ");
        return 'CRITICAL';
    } elsif ($w_lo ne 'x' && $current < $w_lo) {
        push(@msgs, "WARNING LOW $uctype (<$w_lo) - ");
        return 'WARNING';
    } elsif ($w_hi ne 'x' && $current > $w_hi) {
        push(@msgs, "WARNING HIGH $uctype (>$w_hi) - ");
        return 'WARNING';
    }
    
    return 'OK';
};

sub syntax_sensors_list {
    my ($alignment) = @_;
    my $list = "";
    my $line = "";
    foreach my $sensor (sort keys %{$sensors}) {
        $line .= $sensor . ", ";
        if (length($line) > 60) {
            $list .= $line;
            $line = "\n" . $alignment;
        }
    }
    if ($line =~ /[^\s]+/s) {
        $list .= $line;
    }
    chop($list);
    chop($list);
    return $list;
}

sub syntax {
  print qq{
Syntax:
    $0 --host <NAME> --sensor <NAME> --limits <LIMITS> [options]

Mandatory parameters:
  --host <NAME>
    address of device on network (name or IP).
  --sensor <NAME>
    name of the sensor; set of available sensors depends on device model;
    should be one of (note few aliases which are interchangable):
    } . syntax_sensors_list("    ") . qq{
  --limits <WARN_LOW/WARN_HIGH,CRIT_LOW/CRIT_HIGH>
    two ranges of allowed sensor values in the form of LOW,HIGH;
    if sensor value is outside of the range, alert is raised.
    use 'x' instead of value to specify infinity.

Optional parameters:
  --port=x
    port of the device. Default=$opt_port.
  --device <DEVICE>
    specifies model of device, for backwards compatibility with em01;
    should be one of:
      em01 (old devices which return non-xml formatted output)
      xml (default)
  --timeout=x
    how long to wait before failing. Default=$opt_timeout.

  --debug
    print debug messages to STDERR
  --dev
    developer's mode allowing to test plugin on a standard http server
    (older devices http server did not conform to http standard)

Example:
    $0 --host sensor0 --sensor temperature --limits 65/75,50/90

};
  exit($ERRORS{'UNKNOWN'});
}

sub read_sensor {
    my ($host, $timeout) = @_;

    my $remote = IO::Socket::INET->new(
         Proto    => "tcp",
         PeerAddr => $host,
         PeerPort => $opt_port,
         Timeout  => $timeout,
    );

    if (!$remote) {
        &debug("connect error: $!\n");
        print "failed to connect\n";
        exit($ERRORS{'UNKNOWN'});
    }
    &debug("connected to $host:$opt_port\n");

    my $didalarm = 0;
    my $hdrs = {};
    my $read = {};
    my $remote_line_number = 0;
    my $response = "";

    eval {
        local $SIG{'ALRM'} = sub { $didalarm=1; die "alarm\n"; };
        alarm($timeout);

        if ($opt_device eq 'xml') {
            if ($opt_dev) {
                print $remote "GET " . $opt_url . " HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: $host\r\nConnection: close\r\n\r\n";
            }
            else {
                print $remote "GET " . $opt_url . " HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: $host\r\n\r\n";
            }

            # read in all the response, waiting for the sensor data end
            READ_REMOTE: while (my $l = <$remote>) {
                $response .= $l;

                if ($response =~ /(<sensorsSW>.+<\/sensorsSW>)/s) {
                    $read = parse_ssetings_xml($response);
                    last READ_REMOTE;
                }
            }
        }
        else {
            if ($opt_dev) {
                print $remote "GET /em01.html?123 HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: $host\r\nConnection: close\r\n\r\n";
            }
            else {
                # HTTP/1.1 bug found by Fabrice Duley (Fabrice.Duley@alcatel.fr)
                print $remote "GET " . $opt_url . " HTTP/1.1 \r\n";
            }
                    
            my $inhdr = 1;
            while (my $l = <$remote>) {
                $remote_line_number++;
                $response .= $l;

                if ($inhdr) {
                    $l =~ s/[\n\r]+$//;
                    if (!length($l)) {
                        $inhdr = 0;
                    }
                    elsif ($l =~ /^([^ :]+)([ :])\s*(.*)$/) {
                        my $n = lc $1;
                        my $v = $3;
                        if (!exists($hdrs->{$n})) {
                            $hdrs->{$n} = [$v];
                        } else {
                            push(@{$hdrs->{$n}}, $v);
                        }
                    }
                    else {
                        &debug("Unexpected HTTP header at line $remote_line_number: $l\n");
                    }
                }
                # https://www.monitoring-plugins.org/archive/devel/2005-May/003660.html
                # Ben Clewett ben at clewett.org.uk
                # Thu May 12 05:17:31 CEST 2005
                elsif ($l =~ /T\D*?([A-Z])\D*?([0123456789.]+)\D*?
                          HU\D*?([0123456789.]+)\D*?
                          IL\D*?([0123456789.]+)/gx)
                # http://eesensors.com/products/websensors/server-room-temperature-monitoring.html
                # SM701679TF: 80.6HU:19.7%IL 4.1
                # àM701679RF 81.1 (not implemented)
                {
                    $read->{'temp-unit'} = $1;
                    $read->{'temperature'} = $2;
                    $read->{'humidity'} = $3;
                    $read->{'illumination'} = $4;
                }
            }
        }

        close($remote);
        alarm(0);
    };

    if (defined($read->{'temp-unit'}) &&
        defined($read->{'temperature'}) &&
        defined($read->{'humidity'}) &&
        defined($read->{'illumination'}) ||
        defined($read->{'sht'})
        ) {
    
        return $read;
    }
    else {
        if ($@) {
            die $@ if $didalarm != 1;
            &debug("timeout $timeout expired during sensor read\n");
            print "Unable to read sensor (timeout $timeout seconds expired)\n";
            exit($ERRORS{'UNKNOWN'});
        }
        else {
            # https://github.com/pashol/nagios-checks
            # our sensor retunred many times no values, which ended up as critical in nagios.
            # Which in return gave a bad SLA at the end of the month. Therefore it goes into unknown now.
            #
            # https://github.com/pashol/nagios-checks/commit/31cdba775a226672207f92d2e9e5e3365ac88a54#diff-262e64bf9a35e13ebbec3d1c8ab6856b
            print "No values received from sensor\n";
            exit($ERRORS{'UNKNOWN'});
        }
    }
};

sub debug {
    my ($msg) = @_;

    if ($opt_debug) {
        print STDERR $msg;
    }
};

sub parse_ssetings_xml {
    my ($xml) = @_;
    my $struct = {};

    my $tmp = $xml;
    # get all the opening tags
    if ($tmp =~ /<sensorsSW>(.+)$/s) {
        $tmp = $1;
        while ($tmp =~ /[^\<]<([a-zA-Z0-9]+)>(.+)$/s) {
            $tmp = $2;
            $struct->{$1} = "";
        }
    }
    
    # fill in the values for all the found tags
    foreach my $k (keys %{$struct}) {
        if ($xml =~ /<$k>([^\<]+)<\/$k>/) {
            $struct->{$k} = $1;
        }
    }

    return $struct;
}

sub run_tests {
    my $struct;
    
    $struct = parse_ssetings_xml(qq{
<sensorsSW>
<dvc>563</dvc>//Ignore
<sht>inline</sht>//SHT71 is a temperature + Humidity sensor module. Inline means sensor is present, none means not present. 
<ilum>inline</ilum>//light sensor
<evin>none</evin>//voltage sensor
<ethm>none</ethm>//thermistor
<ecin>none</ecin>//Contact input.
<efld>none</efld>//flood sensor
<epir>none</epir>//motion sensor. not needed, ignore.
<egas>none</egas>// gas sensor



<tm0>73.5</tm0>// sht temperature sensor reading. If sht is inline, then this value is the temperature reading.
<hu0>32.7</hu0>//humidity reading
<il0>124.3</il0>//illumination
<pot0>0</pot0>//ignore
<tun0>F</tun0>//F or 0: Farenheit, C or 1: Celcius 
<thm>0.00</thm>// Thermistor temperature. If <ethm> is inline, then this value could be something like 72.34
<vin>0</vin>// voltage input. 
<cin>0</cin>// contact input
<fin>0</fin>//flood input


</sensorsSW>
    });
    print Data::Dumper::Dumper($struct)
        if $opt_debug;

    Carp::confess("Got undefined structure from parse_ssetings_xml")
        unless defined($struct);
    Carp::confess("Expected HASH reference, got [" . ref($struct) . "] from parse_ssetings_xml")
        unless ref($struct) eq 'HASH';
    Carp::confess("Expected il0 sensor, not found from parse_ssetings_xml")
        unless defined($struct->{'il0'});
    $struct = undef;

    $struct = parse_ssetings_xml(qq{
<sensorsSW>
<sid0>
<sht>inline</sht>
<tm0>36</tm0>
<hu0>12</hu0>
<tun0>C</tun0>
<ilum>inline</ilum>
<il0>2</il0>
<ecin>inline</ecin>
<cin>1</cin>
<ethm>inline</ethm>
<thm>15</thm>
<evin>inline</evin>
<vin>220</vin>
<efld>inline</efld>
<fin>12</fin>
</sensorsSW>
    });
    print Data::Dumper::Dumper($struct)
        if $opt_debug;
 
    Carp::confess("Got undefined structure from parse_ssetings_xml")
        unless defined($struct);
    Carp::confess("Expected HASH reference, got [" . ref($struct) . "] from parse_ssetings_xml")
        unless ref($struct) eq 'HASH';
    Carp::confess("Expected fin sensor, not found from parse_ssetings_xml")
        unless defined($struct->{'fin'});
    $struct = undef;

    print "parse_ssetings_xml: all ok\n";

    exit($ERRORS{'UNKNOWN'});
}
