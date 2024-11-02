// Importing necessary libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h> 
// Setting network credentials
const char* ssid = "test";
const char* password = "lgl040703";

int channel_PWM = 3;  //使用3号通道 定时器1  总共16个通道 
// 舵机频率，那么周期也就是1/50，也就是20ms ，PWM⼀共有16个通道，0-7位⾼速通道由80Mhz时钟驱动，后⾯8个为低速通道由1Mhz
int freq_PWM = 50;  //50HZ pwm波
// PWM分辨率，取值为 0-20 之间  ，这⾥填写为10，那么后⾯的ledcWrite 这个⾥⾯填写的pwm值就在 0 - 2的10次⽅ 之间 也就是 0-1024，如果是要求不⾼的东西你可以直接拿1000去算了
int res_PWM = 10;  //分辨率  0-1024  共1025
const int  PWM_PIN = 15; //使用15号引脚
// const char* input_parameter1 = "output";
// const char* input_parameter2 = "state";
 
// Creating a AsyncWebServer object 
AsyncWebServer server(80);
 
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  ledcSetup(channel_PWM,freq_PWM,res_PWM);  //设置通道
  ledcAttachPin(PWM_PIN,channel_PWM);  //将引脚绑定到通道上
 
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi");
  }
//  http://ip:80/turning
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  server.on("/turning",HTTP_GET,[](AsyncWebServerRequest *request){
    Serial.println("success");
    ledcWrite(channel_PWM, 27);
    delay(2700);
    // delay(1000);
    ledcWrite(channel_PWM, 75);
    delay(1000);
    request->send(200, "text/plain", "OK");
  });
 
  // Start server
  server.begin();
}
 
void loop() {
 
}
