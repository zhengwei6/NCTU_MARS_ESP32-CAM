#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "FS.h" //sd card esp32
#include "SD_MMC.h" //sd card esp32
#include "soc/soc.h" //disable brownour problems
#include "soc/rtc_cntl_reg.h"  //disable brownour problems
#include <WiFi.h> //used for internet time
#include "dl_lib.h"
#include "esp_http_server.h"
#include "WiFiClientSecure.h"
#include <HTTPClient.h>

#define CAMERA_MODEL_AI_THINKER
#define BOUNDARY     "WebKitFormBoundary7MA4YWxkTrZu0gW"  
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define SERVER     "westus2.api.cognitive.microsoft.com"
#define PORT 443
#define TIMEOUT      20000

static mtmn_config_t mtmn_config = {0};
static int8_t recognition_enabled = 0;
static int8_t is_enrolling = 0;
static face_id_list id_list = {0};

const char* ssid = "cba"; //remember change your SSID 
const char* password = "11111111";// remember change your SSID PW
String host="westus2.api.cognitive.microsoft.com";
String url="https://westus2.api.cognitive.microsoft.com/customvision/v3.0/Prediction/58a8698a-abfb-4732-bfbb-ef5199b4ed0b/detect/iterations/Iteration1/image";
String azureKEY="5d6f82f21bc84218b5e1cf90ef808cc7";
String azureURL="/customvision/v3.0/Prediction/58a8698a-abfb-4732-bfbb-ef5199b4ed0b/detect/iterations/Iteration1/image";

//-----get Pic
void getPic(String filename){
  Serial.println("Camera capturing:"+ filename);
  camera_fb_t * fb = NULL;
  //esp_err_t res = ESP_OK;
  fb = esp_camera_fb_get(); //get picture from cam 
  if (!fb) {
        Serial.println("Camera capture failed");        
  }      
  Serial.println ("fb lengte=");
  Serial.println ( fb->len );//jpg filesize     
  fs::FS &fs = SD_MMC; 
  Serial.printf("Writing file: %s\n", filename);
  File file = fs.open(filename, FILE_WRITE);
  
  if(!file){
      Serial.println("Failed to open file for SD");
  }   else  {
      file.write(fb->buf , fb->len); //payload , lengte vd payload
      Serial.println("succes to open file for SD");
  }
  return;
}

void setup() {
  //1. init sd card
  //2. setup camera param 
  //3. Wifi connect

  //1. init sd card
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);  
  Serial.println();
  //3. init sd card
  if(!SD_MMC.begin()){
      Serial.println("Card Mount Failed");
      return;
  }else {
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
        Serial.println("No SD_MMC card attached");
        return;
    }  
      Serial.print("SD_MMC Card Type: ");
      if(cardType == CARD_MMC){
          Serial.println("MMC");
      } else if(cardType == CARD_SD){
          Serial.println("SDSC");
      } else if(cardType == CARD_SDHC){
          Serial.println("SDHC");
      } else {
          Serial.println("UNKNOWN");
      }    
      uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
      Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);     
  }

  
  //2. setup camera param 
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  //Format of the pixel data: PIXFORMAT_ + YUV422|GRAYSCALE|RGB565|JPEG 
  config.pixel_format = PIXFORMAT_JPEG;   
  // frame_size set as FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.frame_size = FRAMESIZE_SVGA; 
  //jpeg_quality set 10-63 
  config.jpeg_quality = 10;
  // fb_count set 2 for jpg
  config.fb_count = 2;
  
  esp_err_t err = esp_camera_init(&config);
  if (err == ESP_OK) {
    Serial.println("Camera init passed");
    // setup stream ------------------------
    sensor_t * s = esp_camera_sensor_get();
    int res = 0;
    //res=s->set_brightness(s, 0);//-2 to 2
    //res=s->set_contrast(s, 0);//-2 to 2
    //res=s->set_saturation(s, 0);//-2 to 2
    //res=s->set_special_effect(s, 0);//0=no effect,1 to 6=negative to sepia
    //res=s->set_whitebal(s, 1);//disabled=0,enabled=1
    //res=s->set_awb_gain(s, 1);//disabled=0,enabled=1
    //res=s->set_wb_mode(s, 0);//0=default,1 to 4=sunny to home
    //res=s->set_lenc(s, 1);//disabled=0,enabled=1
    //res=s->set_hmirror(s, 0);//disabled=0,enabled=1
    res=s->set_vflip(s, 1);//disabled=0,enabled=1
  }  else {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;   
  }
  
  //3. Wifi connect
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
}

int i=0; //pic count as filename
void loop() {
  i++;
  getPic("/pic" + String(i) +".jpg");
  delay(10000);
  String serverRes = custom_vision_send("/pic" + String(i) +".jpg", host ,url, azureKEY);
}
