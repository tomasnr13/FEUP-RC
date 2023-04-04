ifconfig eth0 up
ifconfig eth0 172.16.11.1/24

ip route add 172.16.10.0/24 via 172.16.11.253

ip route add default via 172.16.11.254