enable

configure terminal
vlan 10
end

configure terminal
interface fastethernet 0/porta onde ligas tux3 e0
switchport mode access
switchport access vlan 10
end

configure terminal
interface fastethernet 0/porta onde ligas tux4 e0
switchport mode access
switchport access vlan 10
end

configure terminal
vlan 11
end

configure terminal
interface fastethernet 0/porta onde ligas tux2 e0
switchport mode access
switchport access vlan 11
end

configure terminal
interface fastethernet 0/porta onde ligas tux4 e1
switchport mode access
switchport access vlan 11
end

configure terminal
interface fastethernet 0/porta onde ligas router ge0
switchport mode access
switchport access vlan 11
end