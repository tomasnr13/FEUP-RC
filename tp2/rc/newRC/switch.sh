#!/bin/bash

#depois dos tuxe iniciados, correer no q está ligado ao switch

enable

configure terminal
vlan Y0
end

configure terminal
interface fastethernet 0/porta em cima a q se liga o tux3
switchport mode access
switchport access vlan Y0
end

configure terminal
interface fastethernet 0/porta em cima a q se liga o tux4
switchport mode access
switchport access vlan Y0
end

configure terminal
vlan Y1
end

configure terminal
interface fastethernet 0/porta em cima a q se liga o tux2
switchport mode access
switchport access vlan Y1
end
