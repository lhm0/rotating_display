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
L Device:R R0
U 1 1 62EF8CCF
P 4600 3250
F 0 "R0" H 4670 3296 50  0000 L CNN
F 1 " " H 4670 3205 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 4530 3250 50  0001 C CNN
F 3 "~" H 4600 3250 50  0001 C CNN
	1    4600 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 3100 4450 3100
Wire Wire Line
	4450 3100 4450 3400
Wire Wire Line
	4450 3400 4600 3400
$Comp
L power:GND #PWR?
U 1 1 62EF9D7D
P 4600 3400
F 0 "#PWR?" H 4600 3150 50  0001 C CNN
F 1 "GND" H 4605 3227 50  0000 C CNN
F 2 "" H 4600 3400 50  0001 C CNN
F 3 "" H 4600 3400 50  0001 C CNN
	1    4600 3400
	1    0    0    -1  
$EndComp
Connection ~ 4600 3400
$EndSCHEMATC
