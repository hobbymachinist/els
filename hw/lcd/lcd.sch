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
Text GLabel 3400 2500 2    50   Input ~ 0
CS
Wire Wire Line
	3400 2500 3300 2500
Text GLabel 3400 2400 2    50   Input ~ 0
RS
Wire Wire Line
	3400 2400 3300 2400
Text GLabel 3400 2300 2    50   Input ~ 0
WR
Wire Wire Line
	3400 2300 3300 2300
Text GLabel 3400 2200 2    50   Input ~ 0
RD
Wire Wire Line
	3400 2200 3300 2200
Text GLabel 3400 2600 2    50   Input ~ 0
RST
Wire Wire Line
	3400 2600 3300 2600
Text GLabel 2200 2400 0    50   Input ~ 0
D0
Wire Wire Line
	2200 2400 2300 2400
Text GLabel 2200 2500 0    50   Input ~ 0
D1
Wire Wire Line
	2200 2500 2300 2500
Text GLabel 2200 2000 0    50   Input ~ 0
D4
Wire Wire Line
	2200 2000 2300 2000
Text GLabel 2200 2100 0    50   Input ~ 0
D5
Wire Wire Line
	2200 2100 2300 2100
Text GLabel 2200 2200 0    50   Input ~ 0
D6
Wire Wire Line
	2200 2200 2300 2200
Text GLabel 2200 2300 0    50   Input ~ 0
D7
Wire Wire Line
	2200 2300 2300 2300
Text GLabel 2200 1800 0    50   Input ~ 0
D2
Wire Wire Line
	2200 1800 2300 1800
Text GLabel 2200 1900 0    50   Input ~ 0
D3
Wire Wire Line
	2200 1900 2300 1900
Text GLabel 2400 4300 0    50   Input ~ 0
D0
Wire Wire Line
	2400 4300 2500 4300
Text GLabel 2400 4400 0    50   Input ~ 0
D1
Wire Wire Line
	2400 4400 2500 4400
Text GLabel 2400 4700 0    50   Input ~ 0
D4
Wire Wire Line
	2400 4700 2500 4700
Text GLabel 2400 4800 0    50   Input ~ 0
D5
Wire Wire Line
	2400 4800 2500 4800
Text GLabel 2400 4900 0    50   Input ~ 0
D6
Wire Wire Line
	2400 4900 2500 4900
Text GLabel 2400 5000 0    50   Input ~ 0
D7
Wire Wire Line
	2400 5000 2500 5000
Text GLabel 2400 4500 0    50   Input ~ 0
D2
Wire Wire Line
	2400 4500 2500 4500
Text GLabel 2400 4600 0    50   Input ~ 0
D3
Wire Wire Line
	2400 4600 2500 4600
$Comp
L Connector_Generic:Conn_02x08_Top_Bottom J1
U 1 1 611CDF80
P 2700 4600
F 0 "J1" H 2750 5117 50  0000 C CNN
F 1 "Conn_02x08_Top_Bottom" H 2750 5026 50  0000 C CNN
F 2 "Connector_IDC:IDC-Header_2x08_P2.54mm_Vertical" H 2700 4600 50  0001 C CNN
F 3 "~" H 2700 4600 50  0001 C CNN
	1    2700 4600
	1    0    0    -1  
$EndComp
Text GLabel 3100 4600 2    50   Input ~ 0
CS
Wire Wire Line
	3100 4600 3000 4600
Text GLabel 3100 4500 2    50   Input ~ 0
RS
Wire Wire Line
	3100 4500 3000 4500
Text GLabel 3100 4400 2    50   Input ~ 0
WR
Wire Wire Line
	3100 4400 3000 4400
Text GLabel 3100 4300 2    50   Input ~ 0
RD
Wire Wire Line
	3100 4300 3000 4300
Text GLabel 3100 4700 2    50   Input ~ 0
RST
Wire Wire Line
	3100 4700 3000 4700
Text GLabel 3100 4800 2    50   Input ~ 0
GND
Text GLabel 3100 4900 2    50   Input ~ 0
GND
Text GLabel 3100 5000 2    50   Input ~ 0
5V
Wire Wire Line
	3000 4800 3100 4800
Wire Wire Line
	3000 4900 3100 4900
Wire Wire Line
	3000 5000 3100 5000
$Comp
L MCU_Module:Arduino_UNO_R3 A1
U 1 1 611E16A6
P 2800 2200
F 0 "A1" H 2000 3200 50  0000 C CNN
F 1 "Arduino_UNO_R3" H 2000 3100 50  0000 C CNN
F 2 "Module:Arduino_UNO_R3" H 2950 1150 50  0001 L CNN
F 3 "https://www.arduino.cc/en/Main/arduinoBoardUno" H 2600 3250 50  0001 C CNN
	1    2800 2200
	1    0    0    -1  
$EndComp
Text GLabel 3000 900  1    50   Input ~ 0
5V
Wire Wire Line
	3000 900  3000 1200
Text GLabel 2800 3500 3    50   Input ~ 0
GND
Wire Wire Line
	2700 3300 2700 3400
Wire Wire Line
	2700 3400 2800 3400
Wire Wire Line
	2800 3400 2800 3500
Wire Wire Line
	2800 3400 2800 3300
Connection ~ 2800 3400
Wire Wire Line
	2800 3400 2900 3400
Wire Wire Line
	2900 3400 2900 3300
Text GLabel 2900 900  1    50   Input ~ 0
3V3
Wire Wire Line
	2900 900  2900 1200
$EndSCHEMATC
