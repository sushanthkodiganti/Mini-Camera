//  Seeedstudio XIAO-ESP32-S3-Sense + Waveshare Pico 240x135 ST7789 display

//------------------------------------------------
// ******** User Setup file for TFT_eSPI *********

// #define ST7789_DRIVER

// #define TFT_WIDTH  135
// #define TFT_HEIGHT 240

// #define TFT_MISO -1
// #define TFT_MOSI  9     
// #define TFT_SCLK  7    
// #define TFT_CS    2     
// #define TFT_DC    4     
// #define TFT_RST   1        
// #define TFT_BACKLIGHT_ON HIGH

// #define SPI_FREQUENCY  40000000

// #define USE_HSPI_PORT

// #define LOAD_GLCD
// #define LOAD_FONT2
// #define LOAD_FONT4
// #define LOAD_FONT6
// #define LOAD_FONT7
// #define LOAD_FONT8
// #define LOAD_GFXFF
// #define SMOOTH_FONT
//-------------------------------------------



#include <esp_camera.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <img_converters.h>


//-----------------------------------------------------------------------
//defining camera pins for CAMERA_MODEL_XIAO_ESP32S3
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39

#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

#define LED_GPIO_NUM      21


//-----------------------------------------------------------------------
//defining SD chip select pin for SD card
#define SD_CS_PIN 21


//-----------------------------------------------------------------------
//  code to initialize camera
void initCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  // config.pixel_format = PIXFORMAT_JPEG;
  config.pixel_format = PIXFORMAT_RGB565;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // camera initialization and error handling
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //getting sensor details
  sensor_t *s = esp_camera_sensor_get();
  Serial.println("Camera Started. PID is");
  Serial.println(s->id.PID, HEX);
}


//-----------------------------------------------------------------------
//  code to initialize SD card
int photoIndex = 0;

void initSD() {
  if (SD.begin(SD_CS_PIN)) {
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD mounted. Size: %lluMB\n", cardSize);
  } else {
    Serial.println("SD not found");
  }

  if (!SD.exists("/photos")) {
    SD.mkdir("/photos");
    Serial.println("/photos created.");
  }

  // Find next available index
  while (true) {
    char path[32];
    sprintf(path, "/photos/img_%04d.jpg", photoIndex);
    if (!SD.exists(path)) break;
    photoIndex++;
  }
  Serial.printf("Next photo index: %d\n", photoIndex);

}


//-----------------------------------------------------------------------
//  code to initialize TFT display
TFT_eSPI tft = TFT_eSPI();

void initDisplay() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(65, 50);
  tft.print("Display OK");
  delay(2000);
}


//-----------------------------------------------------------------------
//  code to scale the QVGA frame and send to TFT display
#define DISPLAY_W  240
#define DISPLAY_H  135
#define SRC_W      320 //QVGA frame width
#define SRC_H      240 //QVGA frame height
#define SCALED_W   180
#define SCALED_H   135
#define OFFSET_X   30
#define OFFSET_Y   0
static uint16_t lineBuf[SCALED_W];

void pushScaledFrame(uint16_t *buf) {
  tft.startWrite();
  for (int y = 0; y < SCALED_H; y++) {
    int srcY = (y * SRC_H) / SCALED_H;
    for (int x = 0; x < SCALED_W; x++) {
      int srcX = (x * SRC_W) / SCALED_W;
      lineBuf[x] = buf[srcY * SRC_W + srcX];
    }
    tft.pushImage(OFFSET_X, y, SCALED_W, 1, lineBuf);
  }
  tft.endWrite();
}


//-----------------------------------------------------------------------
//  code to convert RGB565 frame to JPG and save to SD card
#define SHUTTER_PIN 44

void captureAndSave(camera_fb_t *fb) {
  uint8_t *jpegBuf = NULL;
  size_t   jpegLen = 0;

  // Convert RGB565 → JPEG
  bool ok = frame2jpg(fb, 90, &jpegBuf, &jpegLen);
  esp_camera_fb_return(fb);

  if (!ok || !jpegBuf) {
    Serial.println("JPEG conversion failed.");
    return;
  }

  char path[32];
  sprintf(path, "/photos/img_%04d.jpg", photoIndex);
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    free(jpegBuf);
    return;
  }
  file.write(jpegBuf, jpegLen);
  file.close();
  free(jpegBuf);

  Serial.printf("Saved: %s (%d bytes)\n", path, jpegLen);
  photoIndex++;
}


//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  initCamera();

  initSD();

  initDisplay();

  pinMode(SHUTTER_PIN, INPUT_PULLUP);

}



//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
void loop() {


  camera_fb_t *fb = esp_camera_fb_get();  //get frame from camera buffer

  if (!fb) {
    Serial.println("Frame capture failed");
    delay(100);
    return;

  }else {
    pushScaledFrame((uint16_t *)fb->buf); //display the frame on screen
  
    if(digitalRead(SHUTTER_PIN) == LOW){ //capture when shutter button is pressed
      Serial.println("Shutter Pressed");
      captureAndSave(fb);

      while(digitalRead(SHUTTER_PIN) == LOW); //while loop ends when button is released
    }

    esp_camera_fb_return(fb); //return frame to camera buffer and continue the loop
  }

}
