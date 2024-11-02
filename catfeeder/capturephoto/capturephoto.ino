#include <ArduinoJson.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_client.h"

// AI THINKER camera init
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


// wifi名称与密码，可考虑做手动配网
const char* ssid = "test";
const char* password = "lgl040703";

const char* post_url = "http://192.168.43.126:5000/recv_img";        // 默认上传地址
static String httpResponseString;         //接收服务器返回信息
bool internet_connected = false;
long current_millis;
long last_capture_millis = 0;
unsigned long previousMillis = 0;  //用于计时断网重连
unsigned long interval = 10000;    //如果断网，重连时间间隔10秒
int capture_interval = 10 * 1000;  //拍照间隔

// 程序初始化
void setup(){
  // 开启串口通信，波特率115200，方便之后从串口获取ip地址
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  // 初始化相机的相关配置，针脚、格式……
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
  config.pixel_format = PIXFORMAT_JPEG;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  // esp32cam的PSRAM是8MB的
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // 判断上述camera初始化是否成功，不成功则跳出，并打印报错信息
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // 定义一个变量，地址为s，类型为sensor_t,将camera的sensor传入
  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

  // 配网
  WiFi.begin(ssid, password);
  // 等待WiFi连接成功
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // 提示WiFi连接成功
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

}

/********http请求处理函数*********/
esp_err_t _http_event_handler(esp_http_client_event_t* evt) {
  if (evt->event_id == HTTP_EVENT_ON_DATA) {
    httpResponseString.concat((char*)evt->data);
  }
  return ESP_OK;
}
// 拍照并上传
static esp_err_t take_send_photo() {
  Serial.println("take_send_photo...");
  // fb就是照片的信息
  camera_fb_t* fb = NULL;
  esp_err_t res = ESP_OK;
  fb = esp_camera_fb_get();
  // fb为空说明拍照失败，返回错误信息
  if (!fb) {
    Serial.println("Camera capture failed...");
    return ESP_FAIL;
  }
  // 拍照成功
  // 定义post请求
  
  httpResponseString = "";
  esp_http_client_handle_t http_client;
  esp_http_client_config_t config_client = { 0 };
  config_client.url = post_url;
  config_client.event_handler = _http_event_handler;
  config_client.method = HTTP_METHOD_POST;
  http_client = esp_http_client_init(&config_client);
  esp_http_client_set_post_field(http_client, (const char*)fb->buf, fb->len);  //设置http发送的内容和长度
  esp_http_client_set_header(http_client, "Content-Type", "image/jpg");        //设置http头部字段
  esp_err_t err = esp_http_client_perform(http_client);                        //发送http请求
  if (err == ESP_OK) {
    Serial.println(httpResponseString);  //打印获取的URL
    //json数据解析
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, httpResponseString);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    }
    String url = doc["url"];
    Serial.println(url);  //打印获取的URL
  }
  Serial.println("Taking picture END");
  esp_camera_fb_return(fb);
  esp_http_client_cleanup(http_client);
  return res;
}


void loop(){
  if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >= interval)) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = millis();
  }
  //定时发送
  current_millis = millis();
  if (current_millis - last_capture_millis > capture_interval) {  // Take another picture
    last_capture_millis = millis();
    take_send_photo();
  }
}
