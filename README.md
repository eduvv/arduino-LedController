[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
# LED Controller
A LED controller build on arduino and a programmable LEDstrip. 

![preview](https://media.giphy.com/media/fH3LI2GcdHOaiwGGxD/giphy.gif)
## Components used
- 1x 128x64 I2C OLED Display SSD1306
- 1x Arduino Nano ATmega328P
- 1x 10K Ohm potentiometer
- 1x 2pin momentary pushbutton
- 1x WS2812B ledstrip SMD5050
## Libraries used
This project used 2 libraries:
- [U8g2](https://github.com/olikraus/u8g2): used for drawing on the OLED display.
- [FastLED](https://github.com/FastLED/FastLED): used to control the ledstrip.
## Features
| feature    | example         | 
|:------------- |:-------------|
| Solid color | [example](https://streamable.com/qeslt)| 
| Fade |[example](https://streamable.com/lnfix)|
| Rainbow |[example](https://streamable.com/du4m3)|
| Bleed |[example](https://streamable.com/vonfq)|
| Speed control |[example](https://streamable.com/h6yyl)|
| Brightness control |[example](https://streamable.com/agday)|
## Diagram
![Fritzing diagram of the project.](https://i.imgur.com/KQnQSTK.png "LED Controller")
