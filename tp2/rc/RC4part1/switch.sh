#!/bin/bash

#depois dos tuxe iniciados, correer no q está ligado ao switch
#bancada 3 da sala de lá tem o eth0 trocado com eth1 no tux4
enable

configure terminal
vlan Y0
end

configure terminal
interface fastethernet 0/porta onde eth0 de 3 esta ligada
switchport mode access
switchport access vlan Y0
end

configure terminal
interface fastethernet 0/porta onde eth0 de 4 esta ligada
switchport mode access
switchport access vlan Y0
end

configure terminal
vlan Y1
end

configure terminal
interface fastethernet 0/porta onde eth0 de 2 esta ligada
switchport mode access
switchport access vlan Y1
end

#experiencia 4 parte 1
configure terminal
interface fastethernet 0/porta onde eth1 de 4 esta ligada
switchport mode access
switchport access vlan Y1
end

#experiencia 4 parte 2
configure terminal
interface fastethernet 0/porta onde GE1 esta ligada
switchport mode access
switchport access vlan Y1
end
