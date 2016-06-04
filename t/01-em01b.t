#!/usr/bin/env perl

use strict;
use warnings;
use FindBin;

$| = 1;

use Carp;
use Data::Dumper;
use IO::Socket;
use Test::More;
use Time::HiRes;


my $cases = [
    {
        'cmd' => 'historic/nagios-current/perl/check_em01.pl',
        'params' => '--type=temp --temp=65/75,60/80 127.0.0.1',
        'expect' => 'CRITICAL HIGH Temperature (>80)[Temp: 80.6 F]',
    },
    {
        'cmd' => 'historic/nagios-current/perl/check_em01.pl',
        'params' => '--type hum --hum=10/40,10/50 127.0.0.1',
        'expect' => '[Humidity: 19.7%]',
    },
    {
        'cmd' => 'historic/nagios-current/perl/check_em01.pl',
        'params' => '--type illum --illum=10/70,0/80 127.0.0.1',
        'expect' => 'WARNING LOW Illumination (<10)[Illum: 4.1]',
    },
    {
        'cmd' => 'historic/nagios-current/perl/check_em01.pl',
        'params' => '--temp=65/75,60/80 --hum=30/40,25/50 --illum=40/70,30/80 127.0.0.1',
        'expect' => 'CRITICAL HIGH Temperature (>80); CRITICAL LOW Humidity (<25); CRITICAL LOW Illumination (<30)[Temp: 80.6 F, Humidity: 19.7%, Illum: 4.1]',
    },

    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--type=temp --temp=65/75,60/80 --host 127.0.0.1 --port 12345 --device em01',
        'expect' => 'CRITICAL HIGH Temperature (>80) - Temp: 80.6 F|temperature=80.6F;;;; ',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--type hum --hum=10/40,10/50 --host 127.0.0.1 --port 12345 --device em01',
        'expect' => 'Humidity: 19.7%|humidity=19.7%;;;; ',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--type illum --illum=10/70,0/80 --host 127.0.0.1 --port 12345 --device em01',
        'expect' => 'WARNING LOW Illumination (<10) - Illum: 4.1|illumination=4.1;;;; ',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--type all --temp=65/75,60/80 --hum=30/40,25/50 --illum=40/70,30/80 --host 127.0.0.1 --port 12345 --device em01',
        'expect' => 'CRITICAL HIGH Temperature (>80) - ; CRITICAL LOW Humidity (<25) - ; CRITICAL LOW Illumination (<30) - Temp: 80.6 F, Humidity: 19.7%, Illum: 4.1',
    },


    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor temperature --limits=65/75,60/80 --host 127.0.0.1 --port 12345 --url',
        'expect' => 'Temperature: 73.5 F|Temperature=73.5F;;;;',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor light --limits=65/75,60/80 --host 127.0.0.1 --port 12345',
        'expect' => 'CRITICAL HIGH Il0 (>80) - Illumination: 124.3|Illumination=124.3;;;;',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor humidity --limits=65/75,10/80 --host 127.0.0.1 --port 12345',
        'expect' => 'WARNING LOW Hu0 (<65) - Humidity: 32.7|Humidity=32.7;;;;',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor voltage --limits=65/75,60/80 --host 127.0.0.1 --port 12345',
        'expect' => 'UNKNOWN: sensor [Voltage (voltage)]: field evin is [none];sensor is not present on the device',
    },


    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor temperature --limits=65/75,60/80 --host 127.0.0.1 --port 12345 --url status.xml',
        'expect' => 'CRITICAL HIGH Tm0 (>80) - Temperature: 80.78 F|Temperature=80.78F;;;;',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor light --limits=65/75,60/80 --host 127.0.0.1 --port 12345 --url status.xml',
        'expect' => 'WARNING HIGH Il0 (>75) - Illumination: 76.61|Illumination=76.61;;;;',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor humidity --limits=65/75,10/80 --host 127.0.0.1 --port 12345 --url status.xml',
        'expect' => 'WARNING LOW Hu0 (<65) - Humidity: 40.84|Humidity=40.84;;;;',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor voltage --limits=65/75,60/80 --host 127.0.0.1 --port 12345 --url status.xml',
        'expect' => 'CRITICAL LOW Vin (<60) - Voltage: 0.00|Voltage=0.00;;;;',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor thermistor --limits=65/75,60/80 --host 127.0.0.1 --port 12345 --url status.xml',
        'expect' => 'WARNING HIGH Thm (>75) - Thermistor: 77.08|Thermistor=77.08;;;;',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor contact --limits=0,0 --host 127.0.0.1 --port 12345 --url status.xml',
        'expect' => 'Contact: 1|Contact=1;;;;',
    },
    {
        'cmd' => 'wizard/plugins/check_esensor.pl',
        'params' => '--sensor flood --limits=1,0 --host 127.0.0.1 --port 12345 --url status.xml',
        'expect' => 'WARNING Fin (=1) - Flood: 1|Flood=1;;;;',
    },
];


my $debug;
if ($ARGV[0] && $ARGV[0] eq 'debug') {
    $debug = 1;
}

my $listener;

my $pid = fork();

if ($pid == 0) {
    $listener = IO::Socket::INET->new(
        Proto    => "tcp",
        LocalAddr => "0.0.0.0",
        LocalPort => 12345,
        Listen => 5,
        Blocking => 0,
        Reuse => 1,
        );
    Carp::confess("Unable to create server for tests: $!")
        unless $listener;

    my $i = 0;
    my $done;
    while (!$done) {
        print ++$i if $debug;
        my $client_socket;
        while (!$client_socket) {
            print "." if $debug;
            $client_socket = $listener->accept();
            Time::HiRes::usleep(100_000);
        }
        print "\n";

        my $peer_address = $client_socket->peerhost();
        my $peer_port = $client_socket->peerport();
#       print "connected $peer_address:$peer_port\n";

        
        my $close_session;
        while (!$close_session) {
            my $data = "";
            $client_socket->recv($data, 2048);

            if ($data =~ /GET \/index.html.* HTTP\/1\.1/) {
                $client_socket->send("HTTP/1.1 200 OK\nServer: " . $0 . "Host: \n\n");
                $client_socket->send("SM701679TF: 80.6HU:19.7%IL 4.1\n");
                $client_socket->close();
                $close_session = 1;
            }
            elsif ($data =~ /GET \/ssetings.xml.* HTTP\/1\.1/) {
                $client_socket->send("HTTP/1.1 200 OK\nServer: " . $0 . "Host: \n\n");
                $client_socket->send(qq{
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
                } . "\n");
                $client_socket->close();
                $close_session = 1;
            }
            elsif ($data =~ /GET \/status.xml.* HTTP\/1\.1/) {
                $client_socket->send("HTTP/1.1 200 OK\nServer: " . $0 . "Host: \n\n");
                $client_socket->send(qq{
<?xml version="1.0"?><sensorsSW><dvc>TBD</dvc><sht>inline</sht><ilum>inline</ilum><evin>inline</evin><ethm>inline</ethm><ecin>inline</ecin><efld>inline</efld><epir>none</epir><egas>none</egas><ght>1</ght><gsn>1</gsn><eDL>none</eDL><ecam>none</ecam><ephc>none</ephc><sid0>995805</sid0><stu0>Alarm</stu0><tm0>80.78</tm0><hu0>40.84</hu0><il0>76.61</il0><tun0>F</tun0><cin>1</cin><fin>1</fin><pin>0</pin><vin>0.00</vin><thm>77.08</thm><phcv>0.00</phcv></sensorsSW>
} . "\n");
                $client_socket->close();
                $close_session = 1;
            }
            else {
                print "unknown client sent: [$data]\n";
                $client_socket->close();
                $close_session = 1;
            }
        }
    }
}
else {
    if ($debug) {
        while(1) {
            print "_";
            sleep(1);
        }
    }


    Test::More::plan("tests" => scalar(@{$cases}));

    my $basedir = $FindBin::Bin;
    my $tmpdir = $basedir . "/../tmp";
    mkdir($tmpdir);

    foreach my $case (@{$cases}) {
        my $source = $basedir . "/../" . $case->{'cmd'};
        my $fn = $case->{'cmd'};
        $fn =~ s/^.+\///s;
        my $target = $tmpdir . "/" . $fn;

        my $cmd = 'cat ' . $source .
            ' | sed "s%use lib \"/usr/local/nagios/libexec\";%use lib \"' . $basedir . '\";%"' .
            ' | sed "s%PeerPort => 80%PeerPort => 12345%" ' .
            ' > ' . $target;
        my $r = qx/$cmd/;

        $cmd = "perl " . $target . " " . $case->{'params'};
        $r = qx/$cmd/;
        $r =~ s/\n$//s;
        if ($r eq $case->{'expect'}) {
            pass("command [" . $case->{'cmd'} . " " . $case->{'params'} . "] returned [" . $r . "]");
        }
        else {
            fail("command [" . $case->{'cmd'} . " " . $case->{'params'} . "] returned [" . $r . "], expected [" . $case->{'expect'} . "]");
        }
    }
}

END {
#    print "shutdown code\n";

    $listener->close()
        if ref($listener) eq 'IO::Socket::INET';

    kill 'TERM', $pid
        if $pid && $pid != 0;
}
