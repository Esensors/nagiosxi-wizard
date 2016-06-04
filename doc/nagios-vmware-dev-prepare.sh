#!/bin/sh

yum -y install nginx mc telnet git tree
cat /etc/nginx/conf.d/default.conf | sed "s%listen.*%listen 8080 default_server;%" > /tmp/nginx.conf
mv -f /etc/nginx/conf.d/default.conf /etc/nginx/conf.d/default.conf.`date +%s`.orig
mv -f /tmp/nginx.conf /etc/nginx/conf.d/default.conf
chkconfig nginx on
service nginx start

cat /etc/sysconfig/iptables | sed '/--dport 80 -j/i-A INPUT -m state --state NEW -m tcp -p tcp --dport 8080 -j ACCEPT' > /tmp/iptables
mv -f /etc/sysconfig/iptables /etc/sysconfig/iptables,`date +%s`.orig
mv -f /tmp/iptables /etc/sysconfig/iptables
service iptables restart

echo '<sensorsSW>
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
' > /usr/share/nginx/html/ssetings.xml

echo 'T 1
V 2

T CEL 12 HU 34 IL 31
' > /usr/share/nginx/html/em01.html

echo '127.0.0.2 sensor0-em01
127.0.0.3 sensor1-em08
127.0.0.4 sensor2-em08
127.0.0.5 sensor3-em08
127.0.0.6 sensor4-em08
127.0.0.7 sensor5-em08
127.0.0.8 sensor6-em08
127.0.0.9 sensor7-em08
127.0.0.10 sensor8-em08
' >> /etc/hosts
