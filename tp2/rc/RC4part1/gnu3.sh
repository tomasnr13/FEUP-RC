#!/bin/bash

/etc/init.d/networking restart

# Y é num da bancada
ifconfig eth0 down
ifconfig eth0 up 172.16.Y0.1/24

#experiencia 2
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

#experiencia 4 parte 1
route add -net 172.16.Y1.0/24 gw 172.16.Y0.254


