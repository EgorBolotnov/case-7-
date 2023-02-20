#include <ESP8266WiFi.h> 
#include <espnow.h>
#include <Wire.h>
#include <AHT20.h>
AHT20 aht20;

uint8_t broadcastAddress[] = {0xEC,0xFA,0xBC,0xA5,0xF2,0xD4};

#define BOARD_ID 1

typedef struct struct_message {
    int id;
    int temp;
    int hum;
    int humg;
} struct_message;
struct_message myData;
 
unsigned long lastTime = 0;
unsigned long timerDelay = 100;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void setup(){
  Serial.begin(115200);

  Wire.begin();
  aht20.begin();

  pinMode(A0, INPUT);

  float hum = 0;
  float temp = 0;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
  Serial.println("Error initializing ESP-NOW");
  return;
}
  // Указываем роль платы в ESP-NOW
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  // После запуска протокола получаем обратную связь о состоянии отправки
  esp_now_register_send_cb(OnDataSent);
  
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}
 
void loop(){
  if ((millis() - lastTime) > timerDelay) {
    float temp = aht20.getTemperature();
    Serial.println(temp);
    float hum = aht20.getHumidity();
    Serial.println(hum);
    
    // Выставляем значения, которые будем отправлять
    myData.id = BOARD_ID;
    myData.hum = hum;
    myData.humg = analogRead(A0)/10.24;
    myData.temp = temp;
 
    // Отправляем сообщение
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
    lastTime = millis();
  }
}
