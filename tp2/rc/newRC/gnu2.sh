#!/bin/bash

/etc/init.d/networking restart

# Y é num da bancada
ifconfig eth0 down
ifconfig eth0 up 172.16.31.1/24

