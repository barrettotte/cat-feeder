# cat-feeder

A cat feeder with a web interface

<p align="middle">
  <img src="images/cat-feeder-front.jpg" alt="cat-feeder-front" width="50%" height="50%"/>
  <img src="images/web-interface.jpg" alt="web-interface" width="25%" height="25%"/>
</p>

## Summary

A cat feeder utilizing a DC motor and an archimedes screw to dispense food.
The feeder is configurable and interactable from a web page interface on the feeder.
The web interface also generates a corresponding `curl` command, so feed times can be scheduled via cron jobs on another machine.

This project was put on hold for a couple months due to other projects and 3D printer problems.
I also screwed up the enclosure design at just about every point I could have.
This cat feeder is way too big, hard to assemble, and barely functional.

But, oh well; Can't win 'em all. Time to move onto something else.
Maybe one day I'll redesign a better cat feeder from scratch when I've learned a lot more on design.

<img src="images/cat-feeder-back.jpg" alt="cat-feeder-back" width="50%" height="50%"/>

<img src="images/cat-feeder-bottom.jpg" alt="cat-feeder-bottom" width="50%" height="50%"/>

## Setup

- Configure WiFi credentials in `include/secrets.h`
- Upload to ESP8266 with `make upload`
- Set DHCP entry in router for cat-feeder MAC and IP
- Add DNS entry for `catfeeder.agartha`
- Power on, wait for ready LED to turn on (while feeder connects to WiFi)
- Saturate the feeder by running a few long feeds until food falls out of the chute
- Use the web interface at http://catfeeder.agartha to feed the cat

## Circuit

Kicad project files can be found in [kicad/](kicad/)

![images/schematic-cropped.png](images/schematic-cropped.png)

## CAD

The Fusion360 project and individual 3D-printed parts can be found in [fusion360/](fusion360/).

![images/fusion360-final.png](images/fusion360-final.png)

## Parts

- Electronics parts list is in [kicad/parts.md]
- Incomplete hardware/3D-printed parts list is in [fusion360/parts.md]

## References

- https://www.youtube.com/watch?v=ICCWvEK6_tg
- https://www.youtube.com/watch?v=7lvErV4l5wQ
- https://microcontrollerslab.com/l298n-dc-motor-driver-module-esp8266-nodemcu/
- https://github.com/jdunmire/kicad-ESP8266
