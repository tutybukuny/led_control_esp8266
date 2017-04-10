//#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <SocketIOClient.h>
#include <ArduinoJson.h>
//#include <SerialCommand.h>

//include thư viện để kiểm tra free RAM trên con esp8266
extern "C" {
#include "user_interface.h"
}

bool up = false;
int percent = 0;

//const byte RX = 3;// Chân 3 được dùng làm chân RX
//const byte TX = 2;// Chân 2 được dùng làm chân TX

//SoftwareSerial mySerial = SoftwareSerial(RX, TX);
//SerialCommand sCmd(mySerial); // Khai báo biến sử dụng thư viện Serial Command

SocketIOClient client;
const char* ssid = "Anh Tuan";          //Tên mạng Wifi mà Socket server của bạn đang kết nối
const char* password = "anhtuan1601";  //Pass mạng wifi ahihi, anh em rãnh thì share pass cho mình với.

char host[] = "192.168.100.2";  //Địa chỉ IP dịch vụ, hãy thay đổi nó theo địa chỉ IP Socket server của bạn.
int port = 9695;                  //Cổng dịch vụ socket server do chúng ta tạo!

//từ khóa extern: dùng để #include các biến toàn cục ở một số thư viện khác. Trong thư viện SocketIOClient có hai biến toàn cục
// mà chúng ta cần quan tâm đó là
// RID: Tên hàm (tên sự kiện
// Rfull: Danh sách biến (được đóng gói lại là chuối JSON)
extern String RID;
extern String Rfull;

#define LED_PIN D7

void setup()
{
  //Bật baudrate ở mức 57600 để giao tiếp với máy tính qua Serial
  Serial.begin(57600);
  //  mySerial.begin(57600); //Bật software serial để giao tiếp với Arduino, nhớ để baudrate trùng với software serial trên mạch arduino
  delay(10);

  //Việc đầu tiên cần làm là kết nối vào mạng Wifi
  Serial.print("Kết nối tới mạng ");
  Serial.println(ssid);

  //Kết nối vào mạng Wifi
  WiFi.begin(ssid, password);

  //Chờ đến khi đã được kết nối
  while (WiFi.status() != WL_CONNECTED) { //Thoát ra khỏi vòng
    delay(500);
    Serial.print('.');
  }

  Serial.println();
  Serial.println(F("Đã kết nối WiFi"));
  Serial.println(F("Địa chỉ IP của ESP8266 (Socket Client ESP8266): "));
  Serial.println(WiFi.localIP());

  if (!client.connect(host, port)) {
    Serial.println(F("Kết nối tới socket server thất bại!"));
    return;
  } else {
    Serial.println(F("Đã kết nối tới socket server!"));
    client.send("clientGreeting", "{\"clientName\": \"esp8266\", \"message\": \"Hello there!\"}");
  }

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_PIN, OUTPUT);
  //  digitalWrite(LED_PIN, LOW);
  analogWrite(LED_PIN, percent);
}

void loop()
{

  //Khi bắt được bất kỳ sự kiện nào thì chúng ta có hai tham số:
  //  +RID: Tên sự kiện
  //  +RFull: Danh sách tham số được nén thành chuỗi JSON!
  if (client.monitor()) {
    if (RID == "reqLEDStatus") {
      reqLEDStatus();
    } else if (RID == "reqLEDBrn") {
      reqLEDBrn();
    }

    //Kiểm tra xem còn dư bao nhiêu RAM, để debug
    uint32_t free = system_get_free_heap_size();
    Serial.println(free);
  }

  //Kết nối lại!
  if (!client.connected()) {
    client.reconnect(host, port);
    Serial.println(F("Đã kết nối tới socket server!"));
    client.send("clientGreeting", "{\"clientName\": \"esp8266\", \"message\": \"Hello there!\"}");
  }

  delay(10);

  //  sCmd.readSerial();
}

void changeLED() {
  int brightness = percent * 255 / 100;
  analogWrite(LED_PIN, brightness);

  reqLEDStatus();

  Serial.println("Đã thay đổi trạng thái LED!");
}

void reqLEDStatus() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["status"] = up;
  root["percent"] = percent;
  char json[200];
  root.printTo(json, sizeof(json));
  client.send("resLEDStatus", json);
  Serial.println("Đã gửi status! json: ");
  Serial.println(json);
}

void reqLEDBrn() {
  Serial.println(RID);
  const char* json = Rfull.c_str();
  StaticJsonBuffer<200> jsonBuffer; //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root = jsonBuffer.parseObject(json);//đặt một biến root mang kiểu json

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  up = root["status"];
  percent = root["percent"];

  if (percent == 0) up = false;

  percent = ((up) ? percent : 0);

  changeLED();
}

