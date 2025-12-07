## First Week Sprint Review #1

This week we inspected and tested the existing equipments in the lab, checking their datasheets to ensure that the equipment was applicable. We also completed the basic functional implementation and individual testing of the Temperature & Humidity Sensor Breakout Board and music maker with speaker.

### last week's progress

#### 1. Examine part from lab works

To save costs, reduce unnecessary waiting time for device shipments, and make full use of existing laboratory resources, we identified several devices suitable for our project during our experiments and verified their applicability with datasheets.

##### SHT20 - Temperature & Humidity Sensor:

We found a SHT20 from labs, which integrates a capacitive humidity sensor and a precision temperature sensor with on-chip signal processing and factory calibration, and provides fully digital output over an I²C interface. According to the AHT20 datasheet, it supports a 2.0–5.5 V supply voltage, a 0–100 %RH humidity range, a –40 to +85 °C temperature range, and typical accuracies of ±2 %RH and ±0.3 °C. These characteristics make it significantly more capable than low-cost sensors such as DHT11, and very well matched for our ATmega32PB-based system: it is accurate, low-power, and easy to interface. Functionally, the HT20 is used to detect ambient temperature and humidity, enabling interactive functions such as reminding users to dress or drink water. It also works in conjunction with soil moisture detection to determine when to water plants.

![w1_i1](./image/ht_front.jpg)
![w1_i2](./image/ht_back.jpg)

(test code: SHT_humiAndTemp_test.c)

Video:
[humidity and temperature sensor](https://drive.google.com/file/d/1q-K4fy1wZYKdtjqaLBNcZiMGyouqcFei/view?usp=sharing)

##### Adafruit Audio FX Sound Board + 2x2W Amp (16 MB)

The Adafruit Audio FX Sound Board + 2x2W Amp (16 MB) is suitable to be used as the main audio module to play voice prompts and sound effects for the “electronic plant plot”. According to the Adafruit documentation, this board is a stand-alone audio effects trigger with on-board flash storage, up to 11 trigger inputs, built-in 2×2 W class-D amplifier, and support for WAV/OGG audio at up to 44.1 kHz / 16-bit stereo. This makes it very suitable for our system, because we only need simple, reliable sound playback instead of a complex audio codec + MCU software stack. The board also supports different trigger behaviours simply by changing the file name pattern. so we can implement rich sound interactions without extra firmware complexity. Functionally, we can use it in conjunction with a speaker to play various interactive sound effects.

![w1_i3](./image/sound_front.jpg)
![w1_i4](./image/sound_back.jpg)

(test code: Speaker_test.c)

Video:
[music maker with speaker](https://drive.google.com/file/d/1XzJ54Gmg9IRC7lquxUHrpikLp9J229z9/view?usp=sharing)

##### Soil Moisture Sensor
After testing, we also found a ST0160 Soil Moisture Sensor Platform Evaluation Expansion Board to measure the soil moisture level in the plant pot. According to the product datasheet, ST0160 is a capacitive soil moisture sensor with an on-board interface board that provides an analog voltage output proportional to soil moisture, powered from 3.3 V or 5 V and using a simple 3-pin PH2.54 connector (VCC, GND, AO). This makes it very easy for our ATmega32PB-based system, which already has multiple ADC channels for analog sensing. This sensor is used to detect soil moisture to determine when to water, and can also visualize the soil condition on a screen.


### Next weeks's plan

Minimal reuqirement:
Since we have already tested out soundboard and humidity & temerature sensor, we are good to start writing the skleton code for out system. Thus, if we cannot get our OLED driver and screen, we will write the program that output the sensor readings to uart.

Desired result:
If we can get our ordered stuffs by next week, we should work on more individual testing and assigned ports. After doing that, we also need to start working on the main program. This week we have tested funtionality of three parts, we also need to finalize and write the individual functions as part of main program. 
Also we need to start thinking of the housing problem and do we need 3d printing for our final product.

### 5. Personal task
Zhihui: Design the mood system and interactive sentences & 3D printing model.

Houjie: Integrate Temperature & Humidity Sensor and Sound Board codes.

Yisen:  OLED driver and display.
