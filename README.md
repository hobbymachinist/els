# STM32 - Electronic Leadscrew

Electronic Leadscrew with STM32

## Demos

Some short demos available on YouTube

https://www.youtube.com/channel/UCVE05OvPm0yVd1yQSVZ0YSA/videos

## Building

```
git clone https://github.com/libopencm3/libopencm3
export OPENCM3_DIR=$PWD/libopencm3

cd libopencm3
make

cd ../
git clone <repo-url>
cd els

# edit Makefile to pick target and a TFT version
make TFT=ILI9481

# or
make TFT=ILI9486

# or
make TFT=ILI9488
```

Some ILITEK ICs use a non-standard 556 16-bit colorspace, use `COLORS=TFT_RGB_556` option in that case.

```
make TFT=ILI9488 COLORS=TFT_RGB_556
```

## Technical documentation

See [Technical Documentation](docs/els.md)

## Design ideas & Reference

- https://github.com/clough42/electronic-leadscrew
- https://github.com/prototypicall/Didge

## Goals / Features

1. The initial software is going to focus on being functional.
2. There is huge scope for optimisation, which can be something I might look into down the track.
3. Eventually, I'd like to add a G-code parser and motion controller to enable optional CNC control.

### Working with limited testing

* Z axis control for manual turning and threading operations
* Soft limits on Z & X
* Hard limits on Z & X
* Backlash compensation for leadscrews
* Semi automated cycles:
    - Turning
    - Threading
    - External Tapers
    - Internal Tapers
    - External Arcs
* Optional DRO support for:
    - TTL optical scales

### Wishlist / TO DO

* Semi automated cycles:
    - Internal Arc
    - External groove
    - Internal groove
* Tool radius compensation

## Pull Requests

Please reach out to discuss any major rewrites or changes if you wish to merge the changes upstream.

A lot of the firmware was developed over a few years on and off when I could find time to code and think about how I'd use it.
I don't want to disappoint or stop anyone from improving the software or hardware design but the primary focus of this work is
for my own personal use and don't want anything to distract me from that in a significant fashion. I don't have plenty of spare
time to review design changes or major software changes.

## IMPORTANT - PLEASE READ

Lathes are not toys, you can get injured or hurt someone else if you're not careful. Use this software at your
own risk.

This software is not provided with any warranty or support. It has only had limited testing in a home / hobby use setting
and may result in you getting injured or your equipment getting damaged due to errors in programming.

The author does not take any responsibility for the use of this software by others and cannot be held liable for any damages
that may result from its use.

## LICENSE

https://opensource.org/licenses/GPL-3.0

Free to be used in any fashion without royalty but any changes to hardware or software should be attributed and publicly shared
for the benefit of others.

Some of the code in this project uses snippets of code from [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library/)
