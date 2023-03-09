# cat-feeder

A cat feeder with a web interface

## Setup

- Upload to ESP8266 with WiFi credentials in `include/secrets.h`
- Set DHCP entry for cat-feeder MAC and IP
- Add DNS entry `catfeeder.agartha`
- Power on, wait for ready LED to turn on (while feeder connects to WiFi)
- Saturate the feeder by running a few long feeds until food falls out of the chute
- Use the web interface at http://catfeeder.agartha to feed the cat

## Parts

TODO:

## References

- https://www.youtube.com/watch?v=ICCWvEK6_tg
- https://www.youtube.com/watch?v=7lvErV4l5wQ
- https://microcontrollerslab.com/l298n-dc-motor-driver-module-esp8266-nodemcu/
- https://github.com/jdunmire/kicad-ESP8266
