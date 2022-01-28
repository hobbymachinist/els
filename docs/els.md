# ELSv2

[TOC]

## Demos

Some short demos available on YouTube

https://www.youtube.com/channel/UCVE05OvPm0yVd1yQSVZ0YSA/videos

## Hardware

![](images/els.jpg)

**IMPORTANT NOTE**

1. Ensure all signal cables are shielded.
2. Ensure the shielding is connected to ground at one end, close to the enclosure.
3. Ensure the enclosure is connected to power ground.
4. Always jog both axes to origin position (0, 0) before starting turning operations
    - This ensures any leadscrew backlash is taken care of in the lathe.
    - Allows the backlash compensation to kick in, ready for the turning steps.

### Kicad assets

All hw designs are done in Kicad.

- hw/els: The main board.
- hw/lcd: The LCD adapter for ILI9481 module

Optional extras:

- hw/buffer: Buffer and pulse shaper, helps keep the PUL signals going to the stepper drivers clean.
- hw/demo-driver: A standalone demo board with drv8825 for testing
- hw/power-clamp: Voltage clamp / suppression board.

#### Main Board

Kicad files in hw/els

##### Active components

* Nucleo-F446RE https://www.st.com/en/evaluation-tools/nucleo-f446re.html
* GL850G USB HUB Controller
* CH340C USB UART
* FT24C64A EEPROM

The Nucleo 64 is connected to the main board USB hub with a small cable. Nucleo 446RE will need a fair few
jumper changes and soldering to permit the firmware to use the required GPIO pins.

Refer to the GPIO pins listed below, the solder bridges for these pins should be setup correctly. See Nucleo 446RE
documentation UM1724, Section 6.9 on Solder Bridges.

##### Optional design guidance

The main board is primarily a breakout board for the Nucleo F446RE with a USB hub, USB USART controller
and additional terminal connectors. If you don't care about the USB UART serial, you can just use the
Nucleo F446RE without populating the board with GL850G and CH340C.

**NOTE**
You will not be able to use UART through the onboard programmer since USART2 peripheral cannot be used. The corresponding
GPIOs (PA2 & PA3) are used for the LCD.

### ILI9481 Breakout

Kicad files in hw/lcd

This primarily a carrier / breakout board with an IDC ribbon cable connector to main board.

* ILI9481 TFT LCD, https://www.aliexpress.com/item/1005001288341298.html
* The main board is connected to the LCD module using an adapter board that breaks out the pins to a 16 pin IDC socket.

### Output buffer / pulse shaper

Kicad files in hw/buffer

Optional add-on for cleaning up the high frequency pulses from the main board to the stepper drivers. This needs to be powered
separately and co-located with the stepper drivers. You may not need this depending on the length of the signal cables from
the main board to the stepper drivers.

* CD4050BE, non-inverting hex buffer.
* CRE1S0505SC, isolated dc-dc converter 5v.

### Keypad

I used this because I had an unused one left over from a different project that did not work out. They're not cheap but they're
very reliable and resistant to liquids in the workshop. The seller can also laser etch the art for keypads if you're interested.

- IP65 18 Key, PS2 Keypad, https://www.aliexpress.com/item/1005002340320709.html

### Encoder

**IMPORTANT NOTE**

The spindle encoder PPR, along with the gear ratio and pulses per MM for Z axis determines the maximum spindle RPM and
thread pitch you can turn in threading operations. There is currently no warning displayed if the spindle PPR is less
than the minimum required PPR.

* Spindle Encoder: Optical 600PPR, https://www.aliexpress.com/item/4000392086291.html
* Hand wheel Encoder / pulser 100PPR, https://www.aliexpress.com/item/32949618549.html

### Stepper motor & controller

* Z Stepper: 24HS40-5004D-E1000
  - https://www.omc-stepperonline.com/nema-24-closed-loop-stepper-motor-4-0-nm-566-56oz-in-encoder-1000cpr.html

* Z Controller: CL57Y
  - https://www.omc-stepperonline.com/y-series-closed-loop-stepper-driver-0-7-0a-24-50vdc-for-nema-17-23-24-stepper-motor.html

* X Stepper: NEMA23 1.2Nm 3A
  - https://www.aliexpress.com/item/1005002871654388.html

* X Controller: DM556
  - https://www.aliexpress.com/item/1005002628784693.html


## Wiring / GPIO Map

```
PA0  - TFT Data
PA1  - TFT Data
PA2  - TFT Data
PA3  - TFT Data
PA4  - TFT Data
PA5  - TFT Data
PA6  - TFT Data
PA7  - TFT Data
PA8  - NOT USED
PA9  - USART TX
PA10 - USART RX
PA11 - DRO X-A (INTERRUPT)
PA12 - DRO X-B (INTERRUPT)
PA13 - NOT USED
PA14 - NOT USED
PA15 - NOT USED

PB0  - TFT RD
PB1  - DRO Z-A (INTERRUPT)
PB2  - DRO Z-B
PB3  - Spindle Enc A (INTERRUPT)
PB4  - Spindle Enc B
PB5  - NOT USED
PB6  - TFT WR
PB7  - TFT RS
PB8  - TFT CS
PB9  - TFT RST
PB10 - EEPROM SCL
PB11 - N/A
PB12 - NOT USED
PB13 - Z ENA
PB14 - Z DIR
PB15 - Z PUL

PC0  - NOT USED
PC1  - NOT USED
PC2  - Input Enc A (INTERRUPT)
PC3  - Input Enc B
PC4  - PS/2 Keypad CLK (INTERRUPT)
PC5  - PS/2 Keypad DAT
PC6  - Spindle Enc A (Timer in Encoder mode)
PC7  - Spindle Enc B (Timer in Encoder mode)
PC8  - NOT USED
PC9  - NOT USED
PC10 - NOT USED
PC11 - NOT USED
PC12 - EEPROM SDA
PC13 - X ENA
PC14 - X DIR
PC15 - X PUL

PH0  - NOT USED
PH1  - NOT USED
```

## Keypad Layout

![](images/keypad.png)

Keymap, top to bottom and left to right

1. Settings
2. Lathe Functions
3. Special Function Key - F1
4. Special Function Key - F2
5. Keypad Lock
6. Keypad Unlock
7. Feed / Pitch
8. Reverse feed direction (only available in manual operations)
9. Turning
10. Threading External
11. Stop Operation / Exit setting
12. Ready to start / Accept setting
13. Z/X axis setting
14. Zero Z / X axis
15. Set position as minimum limit (only for manual turning)
16. Set position as maximum limit (only for manual turning)
17. Jog to minimum (only available in manual operations)
17. Jog to maximum (only available in manual operations)

## DRO

Supports TTL optical DROs. Defaults to the commonly available 5uM resolution versions. The DRO module only supports half the
resolution along Z axis 10uM (0.01mm) to avoid the overhead of processing an additional interrupt. This seems to be
enough for my needs.

## Lathe Functions

The manual turning and threading functions do not control the X axis / cross slide but allows turning
and threading operations at a given feed / pitch to a given length.

The remaining functions are stepper / servo driven on both Z (longitudinal along bed) and X (cross slide) axis.

All function screens display the spindle status at the top

1. Clockwise / Counter clockwise direction of spindle and RPM. Chuck icon is shown when RPM = 0
2. Angular position of the spindle in degrees. This can be used for any indexing functions.
3. Pulse counter, this is a function of the encoder PPR and wraps around. Threading operations synchronise to this value.

**NOTE**
1. Some lathe functions display diagrams where the stock to be removed is displayed in orange.
2. The origin (0,0) required for turning operations is displayed using a yellow dot.

### Settings

![](images/settings.png)

#### X pulses per MM

X axis stepper / servo pulses per MM. This should include any gear ratios in transmission.

#### Z pulses per MM

Z axis stepper / servo pulses per MM. This should include any gear ratios in tranmission, especially if you're using
the gearbox.

#### X backlash uM

X backlash in microns. You can measure this by using a dial gauge against the cross slide and driving the stepper in
one direction for a full rotation and reversing to measure the amount of distance the stepper has to drive the axis before the
dial gauge indicates movement.

#### Z backlash uM

Z backlash in microns. Measure as above.

#### Spindle encoder PPR

Pulses per one revolution of the spindle. Note, this is usually the actual pulses generated by the encoder times any
gear ratios in drive mechanism. You can drive the encoder at 1.5-2x the speed of the spindle but pay attention to the
maximum RPM rating for the encoder.

### Manual Turning

**NOTE**

When soft limits are turned on, with conventional turning the tool is moved from right to left along the Z axis at the
set feed rate until the minimum position is reached.

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis optionally for DRO.
3. Set feed
4. Set min and max Z positions, turn on soft limits if needed
5. Turn on spindle
6. Press Ready to start
7. When the tool reaches the soft limit, it will stop.
8. Back off tool, jog to max (or min if reverse direction is chosen)
9. Feed in tool and press Ready again

![](images/manual-turning.png)

#### F/P

Set feed. The feed rate is independent of the spindle RPM at mm/s.

#### Reverse direction

Reverses feed direction. The usual direction is right to left towards spindle with counter clockwise rotation of spindle.

#### Z/X

1. Set Z axis, jogging and setting min / max positions for virtual stop.
2. Pressing the button again switches to X axis setting, only used for zeroing the DRO reading.

#### 0

Zero Z axis or X axis (DRO)

#### F1

Turns on / off soft limits. This will stop Z drive when the position reaches Z min.

#### F2

Manually set the Z axis min and max values.

#### Minimum Limit

You can jog Z using the input pulse encoder to a specific position and set the position as the minimum limit.

#### Maximum Limit

You can jog Z using the input pulse encoder to a specific position and set the position as the maximum limit.

#### Jog Z to minimum

Jog Z axis to minimum position. Only availble in the set Z axis mode or during turning when soft limit is reached.

#### Jog Z to maximum

Jog Z axis to maximum position. Only availble in the set Z axis mode or during turning when soft limit is reached.

### Manual Threading

**NOTE**

When soft limits are turned on, with conventional turning the tool is moved from right to left along the Z axis at the
set pitch ratio until the minimum position is reached.

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set pitch
4. Set min and max Z positions
5. Turn on spindle
6. Press Ready to start
7. When the tool reaches the soft limit, it will stop.
8. Back off tool, jog to max
9. Feed in tool and press Ready again

![](images/manual-threading.png)

#### F/P

Choose pitch - the pitch determines the feed rate and this is tied to the rotation of the spindle.

You can pick a standard metric pitch with the displayed thread crest dimension. Pressing the key again will allow you to set
an arbitrary pitch value. This can be used to turn approximate imperial threads.

#### Reverse direction

Reverses feed direction. The usual direction is right to left towards spindle with counter clockwise rotation of spindle.

#### Z/X

1. Set Z axis, jogging and setting min / max positions for virtual stop.
2. Pressing the button again switches to X axis setting, only used for zeroing the DRO reading.

#### 0

Zero Z axis or X axis (DRO)

#### F1

N/A

#### F2

Manually set the Z axis min and max values. This controls the length of the thread.

#### Minimum Limit

You can jog Z using the input pulse encoder to a specific position and set the position as the minimum limit.

#### Maximum Limit

You can jog Z using the input pulse encoder to a specific position and set the position as the maximum limit.

#### Jog Z to minimum

Jog Z axis to minimum position. Only availble in the set Z axis mode or during turning when soft limit is reached.

#### Jog Z to maximum

Jog Z axis to maximum position. Only availble in the set Z axis mode or during turning when soft limit is reached.

### Turning

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set feed and depth of cut
4. Set length and turning depth
5. Turn on spindle
6. Press Ready to start

![](images/turning.png)

#### F/P

1. Set feed. The feed rate is independent of the spindle RPM at mm/s.
2. Press key again to set depth of cut for turning and finishing pass.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set length
2. Press key again to set depth

#### F2

Display DRO or diagram.

![](images/turning-dro.png)

### Boring Pocket

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set feed and depth of cut
4. Set depth and turning radius
5. Turn on spindle
6. Press Ready to start

![](images/boring-pocket.png)

#### F/P

1. Set feed. The feed rate is independent of the spindle RPM at mm/s.
2. Press key again to set depth of cut for turning pass.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set depth
2. Press key again to set radius

The diameter of the bore is twice the radius.

#### F2

Display DRO or diagram.

### Boring Hole

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set feed and depth of cut
4. Set depth and width for stock removal
5. Turn on spindle
6. Press Ready to start

![](images/boring-hole.png)

Use this if you have drilled a hole or performing boring operation with a through hole.

#### F/P

1. Set feed. The feed rate is independent of the spindle RPM at mm/s.
2. Press key again to set depth of cut for turning pass.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set depth
2. Press key again to set width of stock to remove.

#### F2

Display DRO or diagram.

### External Threading

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set pitch (and optionally thread height)
4. Set length and depth of cut
5. Turn on spindle
6. Press Ready to start

![](images/threading-ext.png)

#### F/P

1. Select pitch, cycles through standard metric pitches.
2. Press key again to set an arbitrary pitch and thread height / depth.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set length of thread
2. Press key again to set depth of cut.

#### F2

Display DRO or diagram.

### Internal Threading

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set pitch (and optionally thread depth)
4. Set length and depth of cut
5. Turn on spindle
6. Press Ready to start

![](images/threading-int.png)

#### F/P

1. Select pitch, cycles through standard metric pitches.
2. Press key again to set an arbitrary pitch and thread height / depth.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set length of thread
2. Press key again to set depth of cut.

#### F2

Display DRO or diagram.

### External Taper - Right

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set feed and depth of cut
4. Set length and depth of taper. Press F2 - The angle should be displayed in addition to DRO values.
5. Turn on spindle
6. Press Ready to start

![](images/taper-ext-r.png)

#### F/P

1. Select feed, mm/s.
2. Press key again to set depth of cut.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set length of taper
2. Press key again to set depth of taper.

#### F2

Display DRO or diagram. This also displays the taper angle.

### External Taper - Left

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set feed and depth of cut
4. Set length and depth of taper. Press F2 - The angle should be displayed in addition to DRO values.
5. Turn on spindle
6. Press Ready to start

![](images/taper-ext-l.png)

#### F/P

1. Select feed, mm/s.
2. Press key again to set depth of cut.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set length of taper
2. Press key again to set depth of taper.

#### F2

Display DRO or diagram. This also displays the taper angle.

### Internal Taper

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set feed and depth of cut
4. Set length and depth of taper. Press F2 - The angle should be displayed in addition to DRO values.
5. Turn on spindle
6. Press Ready to start

![](images/taper-int.png)

#### F/P

1. Select feed, mm/s.
2. Press key again to set depth of cut.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set length of taper
2. Press key again to set depth of taper.

#### F2

Display DRO or diagram. This also displays the taper angle.

### Convex External - Right

![](images/convex.png)

Glossary:

1. Radius: Radius of the circle intersecting the stock at the bottom.
2. Depth: Distance from stock to intersection of the circle to the face of stock.

Varying the radius and depth will allow a variety of convex curves to be turned.

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set feed and depth of cut
4. Set radius and depth of arc.
5. Turn on spindle
6. Press Ready to start

![](images/convex-ext-r.png)

#### F/P

1. Select feed, mm/s.
2. Press key again to set depth of cut.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set radius of circle
2. Press key again to set depth.

#### F2

Display DRO or diagram.

### Convex External - Left

Steps:

1. Move tool to Z/X origin as required
2. Zero out Z and X axis
3. Set feed and depth of cut
4. Set radius and depth of arc.
5. Turn on spindle
6. Press Ready to start

![](images/convex-ext-l.png)

#### F/P

1. Select feed, mm/s.
2. Press key again to set depth of cut.

#### Z/X

1. Z axis jogging.
2. Press key again for X axis jogging

#### 0

Zero Z axis or X axis (DRO)

#### F1

1. Set radius of circle
2. Press key again to set depth.

#### F2

Display DRO or diagram.

### Concave External - Right

TODO

### Concave External - Left

TODO

### Rounded Groove

TODO

### Straight Groove

TODO
