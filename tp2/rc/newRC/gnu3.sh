#!/bin/bash

/etc/init.d/networking restart

# Y é num da bancada
ifconfig eth0 down
ifconfig eth0 up 172.16.30.1/24

route add -net 172.16.30.0/24 gw 172.16.30.254 eth0
route add default gw 172.16.30.254 eth0
