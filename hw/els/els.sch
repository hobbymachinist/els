EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "ELS Board"
Date "2021-08-14"
Rev "v0.1"
Comp ""
Comment1 "Electronic Leadscrew Breakout for Nucleo64 446RE"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_02x08_Top_Bottom J12
U 1 1 61170526
P 1600 5975
F 0 "J12" H 1650 6492 50  0000 C CNN
F 1 "LCD" H 1650 6401 50  0000 C CNN
F 2 "Connector_IDC:IDC-Header_2x08_P2.54mm_Vertical" H 1600 5975 50  0001 C CNN
F 3 "~" H 1600 5975 50  0001 C CNN
	1    1600 5975
	1    0    0    -1  
$EndComp
Text GLabel 3800 2575 0    50   Input ~ 0
LCD-RD
Wire Wire Line
	4000 2575 3800 2575
Text GLabel 1050 2775 0    50   Input ~ 0
LCD-WR
Text GLabel 3800 2675 0    50   Input ~ 0
LCD-RS
Text GLabel 1050 2875 0    50   Input ~ 0
LCD-CS
Text GLabel 1050 2975 0    50   Input ~ 0
LCD-RST
Wire Wire Line
	1275 2775 1050 2775
Wire Wire Line
	4000 2675 3800 2675
Wire Wire Line
	1275 2875 1050 2875
Wire Wire Line
	1275 2975 1050 2975
Text GLabel 2100 5675 2    50   Input ~ 0
LCD-RD
Wire Wire Line
	1900 5675 2100 5675
Text GLabel 2100 5775 2    50   Input ~ 0
LCD-WR
Text GLabel 2100 5875 2    50   Input ~ 0
LCD-RS
Text GLabel 2100 5975 2    50   Input ~ 0
LCD-CS
Text GLabel 2100 6075 2    50   Input ~ 0
LCD-RST
Wire Wire Line
	1900 5775 2100 5775
Wire Wire Line
	1900 5875 2100 5875
Wire Wire Line
	1900 5975 2100 5975
Wire Wire Line
	1900 6075 2100 6075
Text GLabel 6375 975  2    50   Input ~ 0
+5V
Text GLabel 2100 6375 2    50   Input ~ 0
+5V
Text GLabel 2100 6175 2    50   Input ~ 0
GND
Text GLabel 2100 6275 2    50   Input ~ 0
GND
Wire Wire Line
	1900 6175 2100 6175
Wire Wire Line
	1900 6275 2100 6275
Wire Wire Line
	1900 6375 2100 6375
Text GLabel 5800 4175 3    50   Input ~ 0
GND
Wire Wire Line
	5800 3975 5800 4175
Text GLabel 1075 3275 0    50   Input ~ 0
Z-ENA
Text GLabel 1075 3375 0    50   Input ~ 0
Z-DIR
Text GLabel 1075 3475 0    50   Input ~ 0
Z-PUL
Wire Wire Line
	1275 3275 1075 3275
Wire Wire Line
	1275 3375 1075 3375
Wire Wire Line
	1275 3475 1075 3475
$Comp
L Connector_Generic:Conn_01x04 J16
U 1 1 61196276
P 7750 4550
F 0 "J16" H 7830 4542 50  0000 L CNN
F 1 "Z STEPPER" H 7830 4451 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 7750 4550 50  0001 C CNN
F 3 "~" H 7750 4550 50  0001 C CNN
	1    7750 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 4550 7550 4550
Text GLabel 7450 4650 0    50   Input ~ 0
Z-DIR
Wire Wire Line
	7450 4650 7550 4650
Text GLabel 7450 4750 0    50   Input ~ 0
Z-PUL
Wire Wire Line
	7450 4750 7550 4750
Text GLabel 7450 4450 0    50   Input ~ 0
GND
Wire Wire Line
	7450 4450 7550 4450
Text GLabel 7450 4550 0    50   Input ~ 0
Z-ENA
Text GLabel 2800 1375 2    50   Input ~ 0
S-ENC-A
Wire Wire Line
	2800 1375 2675 1375
Text GLabel 2800 1475 2    50   Input ~ 0
S-ENC-B
Wire Wire Line
	2800 1475 2675 1475
Text GLabel 1050 2575 0    50   Input ~ 0
S-ENC-A
Text GLabel 1050 2475 0    50   Input ~ 0
S-ENC-B
Wire Wire Line
	1275 2575 1050 2575
Wire Wire Line
	1275 2475 1050 2475
$Comp
L Connector_Generic:Conn_01x04 J15
U 1 1 611A035E
P 9275 4525
F 0 "J15" H 9355 4517 50  0000 L CNN
F 1 "SPINDLE ENCODER" H 8775 4775 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 9275 4525 50  0001 C CNN
F 3 "~" H 9275 4525 50  0001 C CNN
	1    9275 4525
	1    0    0    -1  
$EndComp
Text GLabel 8975 4525 0    50   Input ~ 0
GND
Wire Wire Line
	8975 4525 9075 4525
Text GLabel 8975 4425 0    50   Input ~ 0
+5V
Wire Wire Line
	8975 4425 9075 4425
Text GLabel 8975 4625 0    50   Input ~ 0
S-ENC-A
Wire Wire Line
	8975 4625 9075 4625
Text GLabel 8975 4725 0    50   Input ~ 0
S-ENC-B
Wire Wire Line
	8975 4725 9075 4725
Text GLabel 3800 3075 0    50   Input ~ 0
I-ENC-A
Text GLabel 3800 3175 0    50   Input ~ 0
I-ENC-B
Wire Wire Line
	3800 3075 4000 3075
Wire Wire Line
	3800 3175 4000 3175
$Comp
L Connector_Generic:Conn_01x04 J14
U 1 1 611AB995
P 10550 4525
F 0 "J14" H 10630 4517 50  0000 L CNN
F 1 "INPUT ENCODER" H 10275 4800 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 10550 4525 50  0001 C CNN
F 3 "~" H 10550 4525 50  0001 C CNN
	1    10550 4525
	1    0    0    -1  
$EndComp
Text GLabel 10250 4525 0    50   Input ~ 0
GND
Wire Wire Line
	10250 4525 10350 4525
Text GLabel 10250 4425 0    50   Input ~ 0
+5V
Wire Wire Line
	10250 4425 10350 4425
Text GLabel 10250 4625 0    50   Input ~ 0
I-ENC-A
Wire Wire Line
	10250 4625 10350 4625
Text GLabel 10250 4725 0    50   Input ~ 0
I-ENC-B
Wire Wire Line
	10250 4725 10350 4725
Text GLabel 2800 1175 2    50   Input ~ 0
KP-CLK
Text GLabel 2800 1275 2    50   Input ~ 0
KP-DATA
$Comp
L Connector_Generic:Conn_01x04 J13
U 1 1 611BDF86
P 7750 5950
F 0 "J13" H 7830 5942 50  0000 L CNN
F 1 "KEYPAD" H 7830 5851 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 7750 5950 50  0001 C CNN
F 3 "~" H 7750 5950 50  0001 C CNN
	1    7750 5950
	1    0    0    -1  
$EndComp
Text GLabel 7450 5950 0    50   Input ~ 0
GND
Wire Wire Line
	7450 5950 7550 5950
Text GLabel 7450 5850 0    50   Input ~ 0
+5V
Wire Wire Line
	7450 5850 7550 5850
Text GLabel 7450 6050 0    50   Input ~ 0
KP-CLK
Wire Wire Line
	7450 6050 7550 6050
Text GLabel 7450 6150 0    50   Input ~ 0
KP-DATA
Wire Wire Line
	7450 6150 7550 6150
Wire Wire Line
	2800 1175 2675 1175
Wire Wire Line
	2800 1275 2675 1275
$Comp
L ch340:CH340C U3
U 1 1 61167316
P 4900 5525
F 0 "U3" H 4900 6112 60  0000 C CNN
F 1 "CH340C" H 4900 6006 60  0000 C CNN
F 2 "Package_SO:SOIC-16_3.9x9.9mm_P1.27mm" H 4900 5525 60  0001 C CNN
F 3 "" H 4900 5525 60  0001 C CNN
	1    4900 5525
	1    0    0    -1  
$EndComp
Text GLabel 1050 1775 0    50   Input ~ 0
USART1-TX
Text GLabel 1050 1875 0    50   Input ~ 0
USART1-RX
Wire Wire Line
	1050 1775 1275 1775
Wire Wire Line
	1050 1875 1275 1875
Text GLabel 4100 5325 0    50   Input ~ 0
USART1-TX
Wire Wire Line
	4100 5325 4400 5325
Text GLabel 4100 5225 0    50   Input ~ 0
USART1-RX
Wire Wire Line
	4100 5225 4400 5225
Text GLabel 3500 5425 0    50   Input ~ 0
GND
Wire Wire Line
	3500 5425 4000 5425
$Comp
L Device:C_Small C1
U 1 1 611743B6
P 4200 5525
F 0 "C1" V 4160 5615 39  0000 C CNN
F 1 "0.1uF" V 4160 5415 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 4200 5525 50  0001 C CNN
F 3 "~" H 4200 5525 50  0001 C CNN
	1    4200 5525
	0    1    1    0   
$EndComp
Wire Wire Line
	4300 5525 4400 5525
Wire Wire Line
	4100 5525 4000 5525
Wire Wire Line
	4000 5525 4000 5425
Connection ~ 4000 5425
Wire Wire Line
	4000 5425 4400 5425
NoConn ~ 4400 5825
NoConn ~ 4400 5925
NoConn ~ 5400 5925
NoConn ~ 5400 5825
NoConn ~ 5400 5725
NoConn ~ 5400 5625
NoConn ~ 5400 5525
NoConn ~ 5400 5425
NoConn ~ 5400 5325
Text GLabel 5800 5025 1    50   Input ~ 0
+5V
Wire Wire Line
	5800 5025 5800 5225
Wire Wire Line
	5800 5225 5400 5225
$Comp
L Device:C_Small C4
U 1 1 6118E701
P 5800 5425
F 0 "C4" H 5900 5500 50  0000 C CNN
F 1 "1uF" H 5925 5350 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5800 5425 50  0001 C CNN
F 3 "~" H 5800 5425 50  0001 C CNN
	1    5800 5425
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 5225 5800 5325
Connection ~ 5800 5225
Text GLabel 5800 5825 3    50   Input ~ 0
GND
Wire Wire Line
	5800 5525 5800 5825
$Comp
L GL850G:GL850G U4
U 1 1 6119781D
P 9760 1770
F 0 "U4" H 9735 2767 60  0000 C CNN
F 1 "GL850G" H 9735 2661 60  0000 C CNN
F 2 "Package_SO:SSOP-28_5.3x10.2mm_P0.65mm" H 9760 1770 60  0001 C CNN
F 3 "" H 9760 1770 60  0001 C CNN
	1    9760 1770
	1    0    0    -1  
$EndComp
Text GLabel 8710 1020 0    50   Input ~ 0
AVDD
Wire Wire Line
	8710 1020 8960 1020
Wire Wire Line
	8960 1020 8960 1120
Wire Wire Line
	8960 1120 9160 1120
Connection ~ 8960 1020
Wire Wire Line
	8960 1020 9160 1020
Wire Wire Line
	8960 1120 8960 1220
Wire Wire Line
	8960 1220 9160 1220
Connection ~ 8960 1120
Text GLabel 8710 1370 0    50   Input ~ 0
DVDD
Wire Wire Line
	8710 1370 8760 1370
Text GLabel 8410 1470 0    50   Input ~ 0
V33
Wire Wire Line
	8410 1470 8610 1470
Text GLabel 7960 1570 0    50   Input ~ 0
+5V
Wire Wire Line
	7960 1570 8080 1570
Text GLabel 8260 2670 3    50   Input ~ 0
GND
$Comp
L Device:C_Small C6
U 1 1 611B14E3
P 8260 1770
F 0 "C6" V 8230 1700 39  0000 C CNN
F 1 "10uF" V 8220 1880 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8260 1770 50  0001 C CNN
F 3 "~" H 8260 1770 50  0001 C CNN
	1    8260 1770
	1    0    0    -1  
$EndComp
Wire Wire Line
	8260 1870 8260 1920
Connection ~ 8260 1570
Wire Wire Line
	8260 1570 8410 1570
Wire Wire Line
	8260 1670 8260 1570
Wire Wire Line
	9160 1670 9120 1670
Connection ~ 8260 1920
$Comp
L Device:C_Small C8
U 1 1 611BE611
P 8760 1820
F 0 "C8" V 8720 1720 39  0000 C CNN
F 1 "10uF" V 8720 1930 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8760 1820 50  0001 C CNN
F 3 "~" H 8760 1820 50  0001 C CNN
	1    8760 1820
	1    0    0    -1  
$EndComp
Wire Wire Line
	8760 1720 8760 1370
Connection ~ 8760 1370
Wire Wire Line
	8760 1370 8860 1370
Wire Wire Line
	8860 1370 8860 1470
Connection ~ 8860 1370
Wire Wire Line
	8860 1370 9160 1370
Connection ~ 8860 1470
Wire Wire Line
	8860 1470 9160 1470
$Comp
L Device:R_Small R4
U 1 1 611C7655
P 8960 2120
F 0 "R4" V 8920 1990 39  0000 C CNN
F 1 "10K" V 8960 2120 39  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8960 2120 50  0001 C CNN
F 3 "~" H 8960 2120 50  0001 C CNN
	1    8960 2120
	0    1    1    0   
$EndComp
Wire Wire Line
	9060 2120 9160 2120
$Comp
L Device:R_Small R3
U 1 1 611CB9C5
P 8960 2020
F 0 "R3" V 8920 1890 39  0000 C CNN
F 1 "680" V 8960 2020 39  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8960 2020 50  0001 C CNN
F 3 "~" H 8960 2020 50  0001 C CNN
	1    8960 2020
	0    1    1    0   
$EndComp
Wire Wire Line
	9060 2020 9160 2020
Wire Wire Line
	8760 1920 8760 2020
Wire Wire Line
	8260 1920 8260 2020
Wire Wire Line
	8760 2020 8860 2020
Wire Wire Line
	8760 2020 8260 2020
Connection ~ 8760 2020
Connection ~ 8260 2020
Wire Wire Line
	8860 2120 8610 2120
Wire Wire Line
	8610 2120 8610 1470
Connection ~ 8610 1470
Wire Wire Line
	8610 1470 8860 1470
$Comp
L Device:R_Small R5
U 1 1 611DF72F
P 8960 2220
F 0 "R5" V 8920 2090 39  0000 C CNN
F 1 "10K" V 8960 2220 39  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8960 2220 50  0001 C CNN
F 3 "~" H 8960 2220 50  0001 C CNN
	1    8960 2220
	0    1    1    0   
$EndComp
Wire Wire Line
	9060 2220 9160 2220
Wire Wire Line
	8860 2220 8610 2220
Wire Wire Line
	8610 2220 8610 2120
Connection ~ 8610 2120
$Comp
L Device:C_Small C7
U 1 1 611E5D35
P 8410 1770
F 0 "C7" V 8370 1700 39  0000 C CNN
F 1 "0.1uF" V 8360 1880 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8410 1770 50  0001 C CNN
F 3 "~" H 8410 1770 50  0001 C CNN
	1    8410 1770
	1    0    0    -1  
$EndComp
Wire Wire Line
	8410 1670 8410 1570
Connection ~ 8410 1570
Wire Wire Line
	8410 1870 8410 1920
Wire Wire Line
	8410 1920 8260 1920
Wire Wire Line
	9160 1820 9120 1820
Wire Wire Line
	9120 1820 9120 1670
Wire Wire Line
	8410 1570 9160 1570
$Comp
L Device:R_Small R2
U 1 1 611EE365
P 8080 2090
F 0 "R2" V 8000 2090 39  0000 C CNN
F 1 "1K" V 8075 2095 39  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8080 2090 50  0001 C CNN
F 3 "~" H 8080 2090 50  0001 C CNN
	1    8080 2090
	-1   0    0    1   
$EndComp
Wire Wire Line
	8260 2020 8260 2580
Wire Wire Line
	8080 2580 8260 2580
Connection ~ 8260 2580
Wire Wire Line
	8260 2580 8260 2670
Wire Wire Line
	8080 1990 8080 1570
Connection ~ 8080 1570
Wire Wire Line
	8080 1570 8260 1570
Text GLabel 9160 1920 0    39   Input ~ 0
~RST
Text GLabel 9050 1670 0    39   Input ~ 0
GND
Wire Wire Line
	9050 1670 9120 1670
Connection ~ 9120 1670
Text GLabel 7990 2250 0    39   Input ~ 0
~RST
Wire Wire Line
	8080 2510 8080 2580
$Comp
L Device:C_Small C5
U 1 1 61201D88
P 8080 2410
F 0 "C5" V 8130 2480 39  0000 C CNN
F 1 "0.1uF" V 8040 2290 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8080 2410 50  0001 C CNN
F 3 "~" H 8080 2410 50  0001 C CNN
	1    8080 2410
	1    0    0    -1  
$EndComp
Wire Wire Line
	8080 2190 8080 2250
Wire Wire Line
	7990 2250 8080 2250
Connection ~ 8080 2250
Wire Wire Line
	8080 2250 8080 2310
$Comp
L Device:Crystal_Small Y1
U 1 1 61272360
P 10680 2990
F 0 "Y1" H 10680 3215 50  0000 C CNN
F 1 "12MHz" H 10680 3124 50  0000 C CNN
F 2 "Crystal:Crystal_HC49-4H_Vertical" H 10680 2990 50  0001 C CNN
F 3 "~" H 10680 2990 50  0001 C CNN
	1    10680 2990
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C11
U 1 1 61273274
P 10480 3190
F 0 "C11" V 10440 3090 39  0000 C CNN
F 1 "28pF" V 10440 3300 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10480 3190 50  0001 C CNN
F 3 "~" H 10480 3190 50  0001 C CNN
	1    10480 3190
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C12
U 1 1 61273A83
P 10900 3190
F 0 "C12" V 10860 3090 39  0000 C CNN
F 1 "28pF" V 10860 3300 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10900 3190 50  0001 C CNN
F 3 "~" H 10900 3190 50  0001 C CNN
	1    10900 3190
	1    0    0    -1  
$EndComp
Wire Wire Line
	10480 3090 10480 2990
Wire Wire Line
	10480 2990 10580 2990
Wire Wire Line
	10780 2990 10900 2990
Wire Wire Line
	10900 2990 10900 3090
Text GLabel 10480 3430 3    50   Input ~ 0
GND
Wire Wire Line
	10480 3290 10480 3430
Text GLabel 10900 3430 3    50   Input ~ 0
GND
Wire Wire Line
	10900 3290 10900 3430
Wire Wire Line
	10310 2620 10480 2620
Wire Wire Line
	10480 2620 10480 2990
Connection ~ 10480 2990
Wire Wire Line
	10310 2520 10900 2520
Wire Wire Line
	10900 2520 10900 2990
Connection ~ 10900 2990
Text GLabel 8910 2930 0    50   Input ~ 0
AVDD
Text GLabel 9270 2930 2    50   Input ~ 0
V33
Wire Wire Line
	8910 2930 9020 2930
$Comp
L Device:C_Small C10
U 1 1 61294426
P 9190 3150
F 0 "C10" V 9150 3080 39  0000 C CNN
F 1 "0.1uF" V 9140 3260 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 9190 3150 50  0001 C CNN
F 3 "~" H 9190 3150 50  0001 C CNN
	1    9190 3150
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C9
U 1 1 61294F11
P 9020 3150
F 0 "C9" V 8980 3080 39  0000 C CNN
F 1 "1uF" V 8970 3260 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 9020 3150 50  0001 C CNN
F 3 "~" H 9020 3150 50  0001 C CNN
	1    9020 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	9020 2930 9020 3050
Connection ~ 9020 2930
Wire Wire Line
	9020 2930 9190 2930
Wire Wire Line
	9190 2930 9190 3050
Connection ~ 9190 2930
Wire Wire Line
	9190 2930 9270 2930
Text GLabel 9020 3380 3    50   Input ~ 0
GND
Wire Wire Line
	9020 3250 9020 3320
Wire Wire Line
	9020 3320 9190 3320
Wire Wire Line
	9190 3320 9190 3250
Connection ~ 9020 3320
Wire Wire Line
	9020 3320 9020 3380
Text GLabel 10420 1020 2    50   Input ~ 0
DM1
Text GLabel 10420 1120 2    50   Input ~ 0
DP1
Wire Wire Line
	10310 1020 10420 1020
Wire Wire Line
	10310 1120 10420 1120
Text GLabel 4340 5725 0    50   Input ~ 0
DM2
Wire Wire Line
	4340 5725 4400 5725
Text GLabel 4030 5625 0    50   Input ~ 0
DP2
Wire Wire Line
	4030 5625 4400 5625
Text GLabel 10420 1270 2    50   Input ~ 0
DM2
Wire Wire Line
	10420 1270 10310 1270
Text GLabel 10420 1370 2    50   Input ~ 0
DP2
Wire Wire Line
	10420 1370 10310 1370
$Comp
L Connector:USB_B J18
U 1 1 612D5A0E
P 9075 5650
F 0 "J18" H 9132 6117 50  0000 C CNN
F 1 "USB_B" H 9132 6026 50  0000 C CNN
F 2 "Connector_USB:USB_B_OST_USB-B1HSxx_Horizontal" H 9225 5600 50  0001 C CNN
F 3 " ~" H 9225 5600 50  0001 C CNN
	1    9075 5650
	1    0    0    -1  
$EndComp
Text GLabel 9575 5230 1    50   Input ~ 0
+5V
Wire Wire Line
	9375 5450 9575 5450
Wire Wire Line
	9575 5450 9575 5230
Text GLabel 9075 6180 3    50   Input ~ 0
GND
Wire Wire Line
	9075 6050 9075 6120
Wire Wire Line
	9075 6120 8975 6120
Wire Wire Line
	8975 6120 8975 6050
Connection ~ 9075 6120
Wire Wire Line
	9075 6120 9075 6180
Wire Wire Line
	10425 6120 10425 6180
Connection ~ 10425 6120
Wire Wire Line
	10325 6120 10325 6050
Wire Wire Line
	10425 6120 10325 6120
Wire Wire Line
	10425 6050 10425 6120
Text GLabel 10425 6180 3    50   Input ~ 0
GND
Wire Wire Line
	10925 5450 10925 5230
Wire Wire Line
	10725 5450 10925 5450
Text GLabel 10925 5230 1    50   Input ~ 0
+5V
$Comp
L Connector:USB_A J19
U 1 1 612D3550
P 10425 5650
F 0 "J19" H 10482 6117 50  0000 C CNN
F 1 "USB_A" H 10482 6026 50  0000 C CNN
F 2 "Connector_USB_Extra:USB_A_CONNFLY_DS1095-WNR0" H 10575 5600 50  0001 C CNN
F 3 " ~" H 10575 5600 50  0001 C CNN
	1    10425 5650
	1    0    0    -1  
$EndComp
Text GLabel 10815 5750 2    50   Input ~ 0
DM1
Wire Wire Line
	10725 5750 10815 5750
Text GLabel 10815 5650 2    50   Input ~ 0
DP1
Wire Wire Line
	10815 5650 10725 5650
Text GLabel 9425 5650 2    50   Input ~ 0
DP0
Wire Wire Line
	9425 5650 9375 5650
Text GLabel 9425 5750 2    50   Input ~ 0
DM0
Wire Wire Line
	9425 5750 9375 5750
Text GLabel 9050 2370 0    50   Input ~ 0
DM0
Wire Wire Line
	9050 2370 9160 2370
Text GLabel 9050 2470 0    50   Input ~ 0
DP0
Wire Wire Line
	9050 2470 9160 2470
Wire Wire Line
	1275 3075 1050 3075
$Comp
L Memory_EEPROM:24LC64 U2
U 1 1 61190248
P 4905 6965
F 0 "U2" H 4675 7345 50  0000 C CNN
F 1 "FT24C64A" H 4665 7235 39  0000 C CNN
F 2 "Package_SO:SO-8_5.3x6.2mm_P1.27mm" H 4905 6965 50  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/21189f.pdf" H 4905 6965 50  0001 C CNN
	1    4905 6965
	1    0    0    -1  
$EndComp
Text GLabel 6050 875  2    50   Input ~ 0
+3V3
Text GLabel 4905 6405 1    50   Input ~ 0
+3V3
Text GLabel 4905 7515 3    50   Input ~ 0
GND
Wire Wire Line
	4905 7265 4905 7365
$Comp
L Device:C_Small C3
U 1 1 611ADD9B
P 6135 7005
F 0 "C3" V 6095 6935 39  0000 C CNN
F 1 "0.1uF" V 6085 7115 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 6135 7005 50  0001 C CNN
F 3 "~" H 6135 7005 50  0001 C CNN
	1    6135 7005
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C2
U 1 1 611ADE77
P 5965 7005
F 0 "C2" V 5925 6935 39  0000 C CNN
F 1 "1uF" V 5915 7115 39  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5965 7005 50  0001 C CNN
F 3 "~" H 5965 7005 50  0001 C CNN
	1    5965 7005
	1    0    0    -1  
$EndComp
Wire Wire Line
	4905 6545 5385 6545
Wire Wire Line
	5965 6545 5965 6905
Connection ~ 4905 6545
Wire Wire Line
	4905 6545 4905 6665
Wire Wire Line
	5965 6545 6135 6545
Wire Wire Line
	6135 6545 6135 6905
Connection ~ 5965 6545
Wire Wire Line
	5965 7105 5965 7365
Wire Wire Line
	5965 7365 5555 7365
Connection ~ 4905 7365
Wire Wire Line
	6135 7105 6135 7365
Wire Wire Line
	6135 7365 5965 7365
Connection ~ 5965 7365
Text GLabel 5640 6865 2    50   Input ~ 0
SDA2
Text GLabel 5640 6965 2    50   Input ~ 0
SCL2
Wire Wire Line
	5305 6965 5530 6965
Text GLabel 3800 3475 0    50   Input ~ 0
SDA2
Text GLabel 1050 3075 0    50   Input ~ 0
SCL2
Wire Wire Line
	3800 3475 4000 3475
Wire Wire Line
	5305 7065 5555 7065
Wire Wire Line
	5555 7065 5555 7365
Connection ~ 5555 7365
Wire Wire Line
	5555 7365 4905 7365
Text GLabel 3800 3575 0    50   Input ~ 0
X-ENA
Text GLabel 3800 3675 0    50   Input ~ 0
X-DIR
Text GLabel 3800 3775 0    50   Input ~ 0
X-PUL
Wire Wire Line
	3800 3575 4000 3575
Wire Wire Line
	3800 3675 4000 3675
Wire Wire Line
	3800 3775 4000 3775
$Comp
L Connector_Generic:Conn_01x04 J17
U 1 1 61322321
P 7750 5250
F 0 "J17" H 7830 5242 50  0000 L CNN
F 1 "X STEPPER" H 7830 5151 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 7750 5250 50  0001 C CNN
F 3 "~" H 7750 5250 50  0001 C CNN
	1    7750 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 5250 7550 5250
Text GLabel 7450 5350 0    50   Input ~ 0
X-DIR
Wire Wire Line
	7450 5350 7550 5350
Text GLabel 7450 5450 0    50   Input ~ 0
X-PUL
Wire Wire Line
	7450 5450 7550 5450
Text GLabel 7450 5150 0    50   Input ~ 0
GND
Wire Wire Line
	7450 5150 7550 5150
Text GLabel 7450 5250 0    50   Input ~ 0
X-ENA
Text GLabel 1000 6900 1    50   Input ~ 0
PH0
Text GLabel 1100 6900 1    50   Input ~ 0
PH1
Text GLabel 1200 6900 1    50   Input ~ 0
PD2
Text GLabel 1300 6900 1    50   Input ~ 0
PC0
Text GLabel 1400 6900 1    50   Input ~ 0
PC1
Text GLabel 1500 6900 1    50   Input ~ 0
PC8
Text GLabel 1600 6900 1    50   Input ~ 0
PC9
Text GLabel 1700 6900 1    50   Input ~ 0
PC10
Text GLabel 1800 6900 1    50   Input ~ 0
PC11
Text GLabel 1900 6900 1    50   Input ~ 0
PA8
Text GLabel 2000 6900 1    50   Input ~ 0
PA12
Text GLabel 2100 6900 1    50   Input ~ 0
PA11
Text GLabel 2200 6900 1    50   Input ~ 0
PA13
Text GLabel 2300 6900 1    50   Input ~ 0
PA14
Text GLabel 2400 6900 1    50   Input ~ 0
PA15
Text GLabel 2500 6900 1    50   Input ~ 0
PB2
Text GLabel 2600 6900 1    50   Input ~ 0
PB1
Text GLabel 2700 6900 1    50   Input ~ 0
PB5
Text GLabel 2800 6900 1    50   Input ~ 0
PB12
Text GLabel 2900 6900 1    50   Input ~ 0
GND
Text GLabel 3000 6900 1    50   Input ~ 0
+3V3
Text GLabel 3100 6900 1    50   Input ~ 0
+5V
Wire Wire Line
	1000 6900 1000 7000
Wire Wire Line
	1100 6900 1100 7000
Wire Wire Line
	1200 6900 1200 7000
Wire Wire Line
	1300 7000 1300 6900
Wire Wire Line
	1400 6900 1400 7000
Wire Wire Line
	1500 6900 1500 7000
Wire Wire Line
	1600 6900 1600 7000
Wire Wire Line
	1700 7000 1700 6900
Wire Wire Line
	1800 6900 1800 7000
Wire Wire Line
	1900 6900 1900 7000
Wire Wire Line
	2000 6900 2000 7000
Wire Wire Line
	2100 7000 2100 6900
Wire Wire Line
	2200 6900 2200 7000
Wire Wire Line
	2300 6900 2300 7000
Wire Wire Line
	2400 6900 2400 7000
Wire Wire Line
	2500 7000 2500 6900
Wire Wire Line
	2600 6900 2600 7000
Wire Wire Line
	2700 7000 2700 6900
Wire Wire Line
	2800 6900 2800 7000
Wire Wire Line
	2900 6900 2900 7000
Wire Wire Line
	3000 6900 3000 7000
Wire Wire Line
	3100 7000 3100 6900
Text GLabel 5925 1875 2    50   Input ~ 0
PH0
Wire Wire Line
	5925 1875 5800 1875
Text GLabel 5925 1975 2    50   Input ~ 0
PH1
Wire Wire Line
	5925 1975 5800 1975
Text GLabel 5925 1675 2    50   Input ~ 0
PD2
Wire Wire Line
	5925 1675 5800 1675
Text GLabel 3800 2875 0    50   Input ~ 0
PC0
Wire Wire Line
	3800 2875 4000 2875
Text GLabel 3800 2975 0    50   Input ~ 0
PC1
Wire Wire Line
	3800 2975 4000 2975
Text GLabel 2800 1575 2    50   Input ~ 0
PC8
Wire Wire Line
	2800 1575 2675 1575
Text GLabel 2800 1675 2    50   Input ~ 0
PC9
Wire Wire Line
	2800 1675 2675 1675
Text GLabel 3800 3375 0    50   Input ~ 0
PC10
Wire Wire Line
	3800 3375 4000 3375
Text GLabel 3800 3275 0    50   Input ~ 0
PC11
Wire Wire Line
	3800 3275 4000 3275
Text GLabel 1050 1675 0    50   Input ~ 0
PA8
Wire Wire Line
	1050 1675 1275 1675
Text GLabel 1050 1975 0    50   Input ~ 0
PA11
Wire Wire Line
	1050 1975 1275 1975
Text GLabel 1050 2075 0    50   Input ~ 0
PA12
Wire Wire Line
	1050 2075 1275 2075
Text GLabel 3800 2175 0    50   Input ~ 0
PA13
Wire Wire Line
	3800 2175 4000 2175
Text GLabel 3800 2275 0    50   Input ~ 0
PA14
Wire Wire Line
	3800 2275 4000 2275
Text GLabel 3800 2375 0    50   Input ~ 0
PA15
Wire Wire Line
	3800 2375 4000 2375
Text GLabel 1050 2275 0    50   Input ~ 0
PB1
Wire Wire Line
	1050 2275 1275 2275
Text GLabel 1050 2375 0    50   Input ~ 0
PB2
Wire Wire Line
	1050 2375 1275 2375
Text GLabel 1050 2675 0    50   Input ~ 0
PB5
Wire Wire Line
	1050 2675 1275 2675
Text GLabel 1050 3175 0    50   Input ~ 0
PB12
Wire Wire Line
	1050 3175 1275 3175
$Comp
L Connector_Generic:Conn_01x02 J2
U 1 1 61593450
P 1200 7200
F 0 "J2" V 1300 7200 50  0000 R CNN
F 1 "Conn_01x02" V 1163 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 1200 7200 50  0001 C CNN
F 3 "~" H 1200 7200 50  0001 C CNN
	1    1200 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 6159386F
P 1400 7200
F 0 "J3" V 1500 7200 50  0000 R CNN
F 1 "Conn_01x02" V 1363 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 1400 7200 50  0001 C CNN
F 3 "~" H 1400 7200 50  0001 C CNN
	1    1400 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J4
U 1 1 61593D7D
P 1600 7200
F 0 "J4" V 1700 7200 50  0000 R CNN
F 1 "Conn_01x02" V 1563 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 1600 7200 50  0001 C CNN
F 3 "~" H 1600 7200 50  0001 C CNN
	1    1600 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J5
U 1 1 61594861
P 1800 7200
F 0 "J5" V 1900 7200 50  0000 R CNN
F 1 "Conn_01x02" V 1763 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 1800 7200 50  0001 C CNN
F 3 "~" H 1800 7200 50  0001 C CNN
	1    1800 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J1
U 1 1 6157B15C
P 1000 7200
F 0 "J1" V 1100 7200 50  0000 R CNN
F 1 "Conn_01x02" V 963 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 1000 7200 50  0001 C CNN
F 3 "~" H 1000 7200 50  0001 C CNN
	1    1000 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J6
U 1 1 615BA140
P 2000 7200
F 0 "J6" V 2100 7200 50  0000 R CNN
F 1 "Conn_01x02" V 1963 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 2000 7200 50  0001 C CNN
F 3 "~" H 2000 7200 50  0001 C CNN
	1    2000 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J7
U 1 1 615BA567
P 2200 7200
F 0 "J7" V 2300 7200 50  0000 R CNN
F 1 "Conn_01x02" V 2163 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 2200 7200 50  0001 C CNN
F 3 "~" H 2200 7200 50  0001 C CNN
	1    2200 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J8
U 1 1 615BA957
P 2400 7200
F 0 "J8" V 2500 7200 50  0000 R CNN
F 1 "Conn_01x02" V 2363 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 2400 7200 50  0001 C CNN
F 3 "~" H 2400 7200 50  0001 C CNN
	1    2400 7200
	0    -1   1    0   
$EndComp
$Comp
L Nucleo64-446RE:NUCLEO-F446RE U1
U 1 1 6118E461
P 4900 2375
F 0 "U1" H 4900 4142 50  0000 C CNN
F 1 "NUCLEO-F446RE" H 4900 4051 50  0000 C CNN
F 2 "custom_modules:MODULE_NUCLEO-F446RE" H 4900 2375 50  0001 L BNN
F 3 "" H 4900 2375 50  0001 L BNN
F 4 "" H 4900 2375 50  0001 L BNN "MAXIMUM_PACKAGE_HEIGHT"
F 5 "STMicroelectronics" H 4900 2375 50  0001 L BNN "MANUFACTURER"
F 6 "13" H 4900 2375 50  0001 L BNN "PARTREV"
F 7 "Manufacturer Recommendations" H 4900 2375 50  0001 L BNN "STANDARD"
	1    4900 2375
	1    0    0    -1  
$EndComp
$Comp
L Nucleo64-446RE:NUCLEO-F446RE U1
U 2 1 61196842
P 1975 2275
F 0 "U1" H 1975 3942 50  0000 C CNN
F 1 "NUCLEO-F446RE" H 1975 3851 50  0000 C CNN
F 2 "custom_modules:MODULE_NUCLEO-F446RE" H 1975 2275 50  0001 L BNN
F 3 "" H 1975 2275 50  0001 L BNN
F 4 "" H 1975 2275 50  0001 L BNN "MAXIMUM_PACKAGE_HEIGHT"
F 5 "STMicroelectronics" H 1975 2275 50  0001 L BNN "MANUFACTURER"
F 6 "13" H 1975 2275 50  0001 L BNN "PARTREV"
F 7 "Manufacturer Recommendations" H 1975 2275 50  0001 L BNN "STANDARD"
	2    1975 2275
	1    0    0    -1  
$EndComp
Text GLabel 2675 3975 3    50   Input ~ 0
GND
Wire Wire Line
	2675 3775 2675 3975
Text GLabel 3800 1875 0    50   Input ~ 0
PA0
Text GLabel 3800 1975 0    50   Input ~ 0
PA1
Text GLabel 3800 2075 0    50   Input ~ 0
PA4
Wire Wire Line
	3800 1875 4000 1875
Wire Wire Line
	3800 1975 4000 1975
Wire Wire Line
	3800 2075 4000 2075
Text GLabel 1050 1175 0    50   Input ~ 0
PA2
Wire Wire Line
	1050 1175 1275 1175
Text GLabel 1050 1275 0    50   Input ~ 0
PA3
Wire Wire Line
	1050 1275 1275 1275
Text GLabel 1050 1375 0    50   Input ~ 0
PA5
Wire Wire Line
	1050 1375 1275 1375
Text GLabel 1050 1475 0    50   Input ~ 0
PA6
Wire Wire Line
	1050 1475 1275 1475
Text GLabel 1050 1575 0    50   Input ~ 0
PA7
Wire Wire Line
	1050 1575 1275 1575
Text GLabel 1175 5675 0    50   Input ~ 0
PA0
Wire Wire Line
	1175 5675 1400 5675
Text GLabel 1175 5775 0    50   Input ~ 0
PA1
Wire Wire Line
	1175 5775 1400 5775
Text GLabel 1175 6175 0    50   Input ~ 0
PA5
Wire Wire Line
	1175 6175 1400 6175
Text GLabel 1175 6275 0    50   Input ~ 0
PA6
Wire Wire Line
	1175 6275 1400 6275
Text GLabel 1175 6375 0    50   Input ~ 0
PA7
Wire Wire Line
	1175 6375 1400 6375
Text GLabel 1175 5875 0    50   Input ~ 0
PA2
Wire Wire Line
	1175 5875 1400 5875
Text GLabel 1175 5975 0    50   Input ~ 0
PA3
Wire Wire Line
	1175 5975 1400 5975
Text GLabel 1175 6075 0    50   Input ~ 0
PA4
Wire Wire Line
	1175 6075 1400 6075
Wire Wire Line
	5800 975  6375 975 
Wire Wire Line
	5800 875  6050 875 
$Comp
L Connector_Generic:Conn_01x02 J11
U 1 1 615BB669
P 3000 7200
F 0 "J11" V 3100 7200 50  0000 R CNN
F 1 "Conn_01x02" V 2963 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 3000 7200 50  0001 C CNN
F 3 "~" H 3000 7200 50  0001 C CNN
	1    3000 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J10
U 1 1 615BB151
P 2800 7200
F 0 "J10" V 2900 7200 50  0000 R CNN
F 1 "Conn_01x02" V 2763 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 2800 7200 50  0001 C CNN
F 3 "~" H 2800 7200 50  0001 C CNN
	1    2800 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J9
U 1 1 615BAC64
P 2600 7200
F 0 "J9" V 2700 7200 50  0000 R CNN
F 1 "Conn_01x02" V 2563 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 2600 7200 50  0001 C CNN
F 3 "~" H 2600 7200 50  0001 C CNN
	1    2600 7200
	0    -1   1    0   
$EndComp
Text GLabel 3200 6900 1    50   Input ~ 0
GND
Text GLabel 3300 6900 1    50   Input ~ 0
GND
Text GLabel 3400 6900 1    50   Input ~ 0
+3V3
Text GLabel 3500 6900 1    50   Input ~ 0
+5V
Wire Wire Line
	3200 6900 3200 7000
Wire Wire Line
	3300 6900 3300 7000
Wire Wire Line
	3400 6900 3400 7000
Wire Wire Line
	3500 7000 3500 6900
$Comp
L Connector_Generic:Conn_01x02 J21
U 1 1 611CD5E3
P 3400 7200
F 0 "J21" V 3500 7200 50  0000 R CNN
F 1 "Conn_01x02" V 3363 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 3400 7200 50  0001 C CNN
F 3 "~" H 3400 7200 50  0001 C CNN
	1    3400 7200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J20
U 1 1 611CD5ED
P 3200 7200
F 0 "J20" V 3300 7200 50  0000 R CNN
F 1 "Conn_01x02" V 3163 7012 50  0001 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 3200 7200 50  0001 C CNN
F 3 "~" H 3200 7200 50  0001 C CNN
	1    3200 7200
	0    -1   1    0   
$EndComp
Wire Wire Line
	4905 7365 4905 7515
Wire Wire Line
	4905 6405 4905 6545
$Comp
L Connector_Generic:Conn_01x02 J22
U 1 1 611B9857
P 1075 5125
F 0 "J22" H 993 4800 50  0000 C CNN
F 1 "EXT POWER +5V / 1A" H 993 4891 50  0000 C CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 1075 5125 50  0001 C CNN
F 3 "~" H 1075 5125 50  0001 C CNN
	1    1075 5125
	-1   0    0    1   
$EndComp
Text GLabel 1425 5125 2    50   Input ~ 0
GND
Text GLabel 1425 5025 2    50   Input ~ 0
+5V
Wire Wire Line
	1275 5025 1425 5025
Wire Wire Line
	1275 5125 1425 5125
$Comp
L Device:R_Small R6
U 1 1 61D46B56
P 5530 6700
F 0 "R6" V 5450 6700 39  0000 C CNN
F 1 "4K7" V 5525 6705 39  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5530 6700 50  0001 C CNN
F 3 "~" H 5530 6700 50  0001 C CNN
	1    5530 6700
	-1   0    0    1   
$EndComp
$Comp
L Device:R_Small R1
U 1 1 61D4AC0B
P 5385 6700
F 0 "R1" V 5305 6700 39  0000 C CNN
F 1 "4K7" V 5380 6705 39  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5385 6700 50  0001 C CNN
F 3 "~" H 5385 6700 50  0001 C CNN
	1    5385 6700
	-1   0    0    1   
$EndComp
NoConn ~ 4505 6965
NoConn ~ 4505 7065
NoConn ~ 4505 6865
Wire Wire Line
	5305 6865 5385 6865
Wire Wire Line
	5385 6800 5385 6865
Connection ~ 5385 6865
Wire Wire Line
	5385 6865 5640 6865
Wire Wire Line
	5530 6800 5530 6965
Connection ~ 5530 6965
Wire Wire Line
	5530 6965 5640 6965
Wire Wire Line
	5385 6600 5385 6545
Connection ~ 5385 6545
Wire Wire Line
	5385 6545 5530 6545
Wire Wire Line
	5530 6600 5530 6545
Connection ~ 5530 6545
Wire Wire Line
	5530 6545 5965 6545
$EndSCHEMATC
