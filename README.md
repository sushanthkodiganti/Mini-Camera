
# Mini-Camera
ESP32 based mini camera to capture and save photos

Hardware:
1. Seeedstudio XIAO-ESP32S3-Sense
2. Waveshare Pico 1.14 inch 240x135 ST7789 TFT display
3. 6x6x5 push button as a shutter for the camera

Pin connections are as follows:
| XIAO PIN | GPIO Number | Connected to                     |
|----------|-------------|----------------------------------|
| D0       | 1           | TFT_RST                          |
| D1       | 2           | TFT_CS                           |
| D2       | -           | -                                |
| D3       | 4           | TFT_DC                           |
| D4       | 5           | -                                |
| D5       | 6           | -                                |
| D6       | 43          | -                                |
| D7       | 44          | SHUTTER_PIN                      |
| D8       | 7           | TFT_SCLK                         |
| D9       | 8           | -                                |
| D10      | 9           | TFT_MOSI                         |
| 3V3      | -           | VSYS & TFT_BL                    |
| GND      | -           | GND (display & push button)      |
| 5V5      | -           | -                                |
