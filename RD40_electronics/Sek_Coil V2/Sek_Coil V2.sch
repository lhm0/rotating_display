EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L my_symbols:Coil L**1
U 1 1 62821A0A
P 5350 3200
F 0 "L**1" H 5494 3201 50  0000 L CNN
F 1 "Coil" H 5494 3110 50  0000 L CNN
F 2 "my_footprints:Sek_Coil_2" H 5350 3200 50  0001 C CNN
F 3 "" H 5350 3200 50  0001 C CNN
	1    5350 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5250 3300 5450 3300
$Comp
L my_symbols:Pin-Connector U1
U 1 1 628A21F6
P 5150 3450
F 0 "U1" H 5150 3450 50  0001 C CNN
F 1 "Pin-Connector" H 5295 3483 50  0000 C CNN
F 2 "my_footprints:Pin_2mm" H 5150 3450 50  0001 C CNN
F 3 "" H 5150 3450 50  0001 C CNN
	1    5150 3450
	1    0    0    -1  
$EndComp
$Comp
L my_symbols:Pin-Connector U2
U 1 1 628A28E5
P 5150 3650
F 0 "U2" H 5150 3650 50  0001 C CNN
F 1 "Pin-Connector" H 5295 3683 50  0000 C CNN
F 2 "my_footprints:Pin_2mm" H 5150 3650 50  0001 C CNN
F 3 "" H 5150 3650 50  0001 C CNN
	1    5150 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 3550 5250 3550
Wire Wire Line
	5250 3550 5250 3300
Connection ~ 5250 3300
$EndSCHEMATC
