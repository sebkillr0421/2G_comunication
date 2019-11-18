#include "config.h"
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>


#define MQTT_SERVER "mqtt.h2h.name"
#define MQTT_USER "mqtt-clubvip-001"
#define MQTT_PASSWORD "NXPUx_w_mrqX5kfhD-sIosu6F7HqZMUM9Vpt_DhDpZg"
#define TOPIC "messages/mqtt-clubvip-001"
#define SUB_TOPIC "commands/mqtt-clubvip-001"

int ignition = 16;
int buzzer = 10;
#define PANIC D8

#define SS_PIN D4
#define RST_PIN D3

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

WiFiClient espClient;

const char* ssid ="ClubVIP";
const char* password = "Cathy9173";

const char* broker = "mqtt.h2h.name";
PubSubClient mqtt_hostpot(espClient); //mqtt connect with hospot WiFi


TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(4, 5); // The serial connection to the GPS device

void callback(char* topic, byte* payload, unsigned int length){
 Serial.print("message arrive: ");
 Serial.println(" ");
 if((char)payload[1]=='o'){
   digitalWrite(ignition, LOW);
   Serial.println("car unlock");
 }else if((char)payload[1]=='f'){
   digitalWrite(ignition, HIGH);
   Serial.println("car lock");
 }
}


void pollSerial();

bool status_connect_wifi;

int panic_status = 0;
int panic_status1 = 0;

void setup()
{
  ss.begin(9600);
  pinMode(ignition,OUTPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(PANIC,INPUT_PULLUP);
  digitalWrite(ignition, LOW);
  digitalWrite(buzzer, LOW);
  Serial.begin(115200);
  delay(10);
  Serial.println("Wait...");

  WiFi.begin(ssid, password);

  int count = 0;

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    count++;
    if (count > 20){
      break;
    }
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Ok connect to hospot...");
    status_connect_wifi = true;
  }else{
    Serial.println("failed connect to hospot...");
    status_connect_wifi = false;
  }

  mqtt_hostpot.setServer(broker, 1883);
  mqtt_hostpot.setCallback(callback);

  /*mqtt.setServer(broker, 17872);
  mqtt.setCallback(callback);
  mqtt_hostpot.setServer(broker, 17872);
  mqtt_hostpot.setCallback(callback);
  */
  SPI.begin();
  mfrc522.PCD_Init();


}


long timer_count = 0,timer_lock = 0;
byte ActualUID[4]; //save code of tag read
byte User[4] = {0x79, 0x0E, 0x9F, 0xD3};// code user1
byte User2[4] = {0x7B, 0xBB, 0x3D, 0x0A};// code user2
byte User3[4] = {0x4D, 0x5C, 0x6A, 0x45};// code user3

boolean compareArray(byte array1[],byte array2[])
{
  if(array1[0] != array2[0])return(false);
  if(array1[1] != array2[1])return(false);
  if(array1[2] != array2[2])return(false);
  if(array1[3] != array2[3])return(false);
  return(true);
}

bool login = false;

void target_read(){
   if ( mfrc522.PICC_IsNewCardPresent())
       {
           if ( mfrc522.PICC_ReadCardSerial())
           {
              Serial.print("Tag UID:");
              for (byte i = 0; i < mfrc522.uid.size; i++) {
                     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                     Serial.print(mfrc522.uid.uidByte[i], HEX);
                     ActualUID[i]=mfrc522.uid.uidByte[i];
               }

               Serial.println();
               if(compareArray(ActualUID,User)){
                    Serial.println("Acceso concedido...");
                    digitalWrite(ignition, LOW);
                    digitalWrite(buzzer, HIGH);
                    delay(200);
                    digitalWrite(buzzer, LOW);
                    login = true;

                  }
                  else if(compareArray(ActualUID,User2)){
                    Serial.println("Acceso concedido...");
                    digitalWrite(ignition, LOW);
                    digitalWrite(buzzer, HIGH);
                    delay(200);
                    digitalWrite(buzzer, LOW);
                    login = true;
                  }
                  else if(compareArray(ActualUID,User3)){
                    Serial.println("Acceso concedido...");
                    digitalWrite(ignition, LOW);
                    digitalWrite(buzzer, HIGH);
                    delay(200);
                    digitalWrite(buzzer, LOW);
                    login = true;
                  }
                  else{
                    Serial.println("Acceso denegado...");
                  }

               mfrc522.PICC_HaltA();
           }
         }
       }

       int alarm_set = 1;
       String status_ignition = "false";
       int status = 0;
       char date_gps[100];
       int gps_count=0;


       void mqttHostpot(){
         if(!mqtt_hostpot.connected()){
          if(mqtt_hostpot.connect("id_car",MQTT_USER,MQTT_PASSWORD)){
            Serial.println("Cloud mqtt_hostpot connect pass!!!");
            mqtt_hostpot.publish(TOPIC,"go:\"hospot\"");
            mqtt_hostpot.subscribe(SUB_TOPIC);
          }
        }else{
         mqtt_hostpot.loop();
         if(panic_status == 1){
           panic_status = 0;
           mqtt_hostpot.publish(TOPIC, "{\"panic\":\"active\"}");
         }
         if(ss.available()>0)
         {
           gps.encode(ss.read());
           if(gps.location.isUpdated()){
             if (gps_count<350){
               delay(100);
               gps_count ++;
             }else{
             Serial.print("{\"lat\":");
             Serial.print(gps.location.lat(),6);
             float lat = (gps.location.lat());
             Serial.print(",");
             Serial.print("\"lng\":");
             Serial.print(gps.location.lng(),6);
             float lng = (gps.location.lng());
             Serial.print(",");+
             Serial.print("\"speed\":");
             Serial.print(gps.speed.kmph());
             float speed =  gps.speed.kmph();
             Serial.print("}");
             Serial.println();
             sprintf(date_gps,"{\"lat\":%.6f,\"lng\":%.6f,\"speed\":%.2f}",lat,lng,speed);
             mqtt_hostpot.publish(TOPIC, date_gps);
             gps_count = 0;
           }
           }
        }
       }
     }


       void reconnected_WiFi(){
           delay(200);
           //Serial.print(".");
          target_read();
          if(login == false){
           Serial.println(timer_count);
           Serial.println(timer_lock);
           if(timer_count == 14){
            Serial.println("phone disconnect: lock car");
          }
           if(timer_count<60){
            timer_count++;
          }else{
             if(timer_lock <75){
               digitalWrite(buzzer,!digitalRead(buzzer));
               timer_lock++;
             }else{
               target_read();
               Serial.println("lock car: ok");
               digitalWrite(buzzer, LOW);
               digitalWrite(ignition, HIGH);
             }
            }
          }else{
            digitalWrite(ignition, LOW);
          }
       }


       void sound(){
         digitalWrite(buzzer, HIGH);
         delay(400);
         digitalWrite(buzzer, LOW);
         delay(400);
         digitalWrite(buzzer, HIGH);
         delay(400);
         digitalWrite(buzzer, LOW);
       }


void loop()
{
  if(WiFi.status() == 1){
    status_connect_wifi = false;
    alarm_set = 1;
    reconnected_WiFi();
  }else if(WiFi.status() == 3){
    status_connect_wifi = true;
    timer_count = 0;
    timer_lock = 0;
    if(alarm_set == 1){
     alarm_set = 0;
     login = false;
     Serial.println("phone connect: unlock car");
     sound();
     digitalWrite(ignition, LOW);
    }
    mqttHostpot();
  }

  if(digitalRead(PANIC) == HIGH){
    panic_status1 = 1;
    delay(5000);
    Serial.println(digitalRead(PANIC));
    Serial.println("press buttom panic detect..");
  }else{
    if(panic_status1 == 1){
      panic_status1 = 0;
      panic_status= 1;
    }
  }
}

































// #define TINY_GSM_MODEM_SIM800
//
// #include "config.h"
// #include <Arduino.h>
// #include <SoftwareSerial.h>
// #include <ESP8266WiFi.h>
// #include <SPI.h>
// #include <MFRC522.h>
// #include <TinyGsmClient.h>
// #include <PubSubClient.h>
//
// #define MQTT_SERVER "mqtt.h2h.name"
// #define MQTT_USER "mqtt-clubvip-001"
// #define MQTT_PASSWORD "NXPUx_w_mrqX5kfhD-sIosu6F7HqZMUM9Vpt_DhDpZg"
// #define TOPIC "messages/mqtt-clubvip-001"
// #define SUB_TOPIC "commands/mqtt-clubvip-001"
//
// /*
// #define MQTT_SERVER "m16.cloudmqtt.com"
// #define MQTT_USER "bjqlvjrp"
// #define MQTT_PASSWORD "FXAW5K6WJH8G"
// #define TOPIC "messages"
// #define SUB_TOPIC "commands"
// */
// #define SS_PIN D4
// #define RST_PIN D3
//
// MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
//
//
// WiFiClient espClient;
//
// const char* ssid ="ClubVIP";
// const char* password = "Kvip.2020";
// /*
// const char* ssid ="Familia imt";
// const char* password = "DejH.2025";*/
// const char apn[] = "ba.amx";
// const char gprsUser[] = "comcel";
// const char gprsPass[] = "comcel";
//
// SoftwareSerial SerialAT(4,5);
//
// //SoftwareSerial SerialGps(9,10);
//
// TinyGsm modem(SerialAT);
// TinyGsmClient client(modem);
//
// PubSubClient mqtt(client); // mqtt connect with GSM
// PubSubClient mqtt_hostpot(espClient); //mqtt connect with hospot WiFi
//
// int ignition = 16;
// int buzzer = 10;
// #define PANIC D8
//
// const char* broker = "mqtt.h2h.name";
// //const char* broker = "m16.cloudmqtt.com";
// long lastReconnectAttempt = 0;
//
// void callback(char* topic, byte* payload, unsigned int length){
//  Serial.print("message arrive: ");
//  Serial.println(" ");
//  if((char)payload[1]=='o'){
//    digitalWrite(ignition, LOW);
//    Serial.println("car unlock");
//  }else if((char)payload[1]=='f'){
//    digitalWrite(ignition, HIGH);
//    Serial.println("car lock");
//  }
// }
//
// void pollSerial();
//
// bool status_connect_wifi;
//
// int panic_status = 0;
//
//
// void setup(){
//
//   SerialAT.begin(9600);
//   delay(5000);
//
//   pinMode(ignition,OUTPUT);
//   pinMode(buzzer,OUTPUT);
//   pinMode(PANIC,INPUT_PULLUP);
//   digitalWrite(ignition, LOW);
//   digitalWrite(buzzer, LOW);
//   Serial.begin(9600);
//   delay(10);
//   Serial.println("Wait...");
//
//   WiFi.begin(ssid, password);
//
//   int count = 0;
//
//   while (WiFi.status() != WL_CONNECTED){
//     delay(500);
//     Serial.print(".");
//     count++;
//     if (count > 20){
//       break;
//     }
//   }
//   if(WiFi.status() == WL_CONNECTED){
//     Serial.println("Ok connect to hospot...");
//     status_connect_wifi = true;
//   }else{
//     Serial.println("failed connect to hospot...");
//     status_connect_wifi = false;
//   }
//
//   mqtt.setServer(broker, 1883);
//   mqtt.setCallback(callback);
//   mqtt_hostpot.setServer(broker, 1883);
//   mqtt_hostpot.setCallback(callback);
//
//   /*mqtt.setServer(broker, 17872);
//   mqtt.setCallback(callback);
//   mqtt_hostpot.setServer(broker, 17872);
//   mqtt_hostpot.setCallback(callback);
//   */
//   SPI.begin();
//   mfrc522.PCD_Init();
// }
//
//
// long timer_count = 0,timer_lock = 0;
// byte ActualUID[4]; //save code of tag read
// byte User[4] = {0x79, 0x0E, 0x9F, 0xD3};// code user1
// byte User2[4] = {0x7B, 0xBB, 0x3D, 0x0A};// code user2
// byte User3[4] = {0x4D, 0x5C, 0x6A, 0x45};// code user3
//
// boolean compareArray(byte array1[],byte array2[])
// {
//   if(array1[0] != array2[0])return(false);
//   if(array1[1] != array2[1])return(false);
//   if(array1[2] != array2[2])return(false);
//   if(array1[3] != array2[3])return(false);
//   return(true);
// }
//
// bool login = false;
//
// void target_read(){
//    if ( mfrc522.PICC_IsNewCardPresent())
//        {
//            if ( mfrc522.PICC_ReadCardSerial())
//            {
//               Serial.print("Tag UID:");
//               for (byte i = 0; i < mfrc522.uid.size; i++) {
//                      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
//                      Serial.print(mfrc522.uid.uidByte[i], HEX);
//                      ActualUID[i]=mfrc522.uid.uidByte[i];
//                }
//
//                Serial.println();
//                if(compareArray(ActualUID,User)){
//                     Serial.println("Acceso concedido...");
//                     digitalWrite(ignition, LOW);
//                     digitalWrite(buzzer, HIGH);
//                     delay(200);
//                     digitalWrite(buzzer, LOW);
//                     login = true;
//
//                   }
//                   else if(compareArray(ActualUID,User2)){
//                     Serial.println("Acceso concedido...");
//                     digitalWrite(ignition, LOW);
//                     digitalWrite(buzzer, HIGH);
//                     delay(200);
//                     digitalWrite(buzzer, LOW);
//                     login = true;
//                   }
//                   else if(compareArray(ActualUID,User3)){
//                     Serial.println("Acceso concedido...");
//                     digitalWrite(ignition, LOW);
//                     digitalWrite(buzzer, HIGH);
//                     delay(200);
//                     digitalWrite(buzzer, LOW);
//                     login = true;
//                   }
//                   else{
//                     Serial.println("Acceso denegado...");
//                   }
//
//                mfrc522.PICC_HaltA();
//            }
//          }
//        }
//
//
// String gps = "";
//
//
// void reconnect_GSM(){
//   Serial.println("Initializing modem...");
//   modem.restart();
//
//   String modemInfo = modem.getModemInfo();
//   Serial.print("Modem Info: ");
//   Serial.println(modemInfo);
//
//
//   Serial.print("Waiting for network...");
//   if (!modem.waitForNetwork()) {
//     Serial.println(" fail");
//     delay(5000);
//   }
//   Serial.println(" success");
//
//   if (modem.isNetworkConnected()) {
//     Serial.println("Network connected");
//   }
//
//   // GPRS connection parameters are usually set after network registration
//     Serial.print(F("Connecting to "));
//     Serial.print(apn);
//     modem.gprsConnect(apn, gprsUser, gprsPass);
//     if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
//       Serial.println(" fail");
//       delay(5000);
//     }
//     Serial.println(" success");
//
//   if (modem.isGprsConnected()) {
//     Serial.println("GPRS connected");
//   }
// }
//
// int mark_gsm = 0;
// int alarm_set = 1;
// String status_ignition = "false";
// int status = 0;
// char date_gps[100];
// int gps_count=0;
//
//
// void mqttHostpot(){
//   if(!mqtt_hostpot.connected()){
//    if(mqtt_hostpot.connect("id_car",MQTT_USER,MQTT_PASSWORD)){
//      Serial.println("Cloud mqtt_hostpot connect pass!!!");
//      mqtt_hostpot.publish(TOPIC,"go:\"hospot\"");
//      mqtt_hostpot.subscribe(SUB_TOPIC);
//    }
//  }else{
//   mqtt_hostpot.loop();
//   if(panic_status == 1){
//     panic_status = 0;
//     mqtt_hostpot.publish(TOPIC, "{\"panic\":\"active\"}");
//   }
//
//   if (Serial.available()>0){
//      String arrive=Serial.readString();
//      Serial.println(arrive);
//      float lat = arrive.substring(6,arrive.indexOf(",\"lng\":")).toFloat();
//      float lng = arrive.substring(arrive.indexOf(",\"lng\":")+7,arrive.indexOf(",\"speed\":")).toFloat();
//      float speed = arrive.substring(arrive.indexOf(",\"speed\":")+9).toFloat();
//
//      Serial.print("lat:");
//      Serial.print(lat,6);
//      Serial.print(",lng:");
//      Serial.println(lng,6);
//      Serial.print(",speed:");
//      Serial.println(speed,2);
//      sprintf(date_gps,"{\"lat\":%.6f,\"lng\":%.6f,\"speed\":%.2f}",lat,lng,speed);
//      mqtt_hostpot.publish(TOPIC, date_gps);
//    }
//  }
// }
//
//
//
// void connect_mqtt(){
//   if(!mqtt.connected()){
//     mark_gsm++;
//     if (mark_gsm > 2){
//       reconnect_GSM();
//       mark_gsm = 0;
//     }
//     delay(100);
//
//     if(mqtt.connect("id_car",MQTT_USER,MQTT_PASSWORD)){
//       Serial.println("Cloud mqtt connect pass!!!");
//        mark_gsm = 0;
//        mqtt.subscribe(SUB_TOPIC);
//        mqtt.publish(TOPIC,"go:\"GSM\"");
//     }
//    }
//    mqtt.loop();
//    if(panic_status == 1){
//      panic_status = 0;
//      mqtt.publish(TOPIC, "{\"panic\":\"active\"}");
//    }
//
//    if (Serial.available()>0){
//       String arrive=Serial.readString();
//       Serial.println(arrive);
//       float lat = arrive.substring(6,arrive.indexOf(",\"lng\":")).toFloat();
//       float lng = arrive.substring(arrive.indexOf(",\"lng\":")+7,arrive.indexOf(",\"speed\":")).toFloat();
//       float speed = arrive.substring(arrive.indexOf(",\"speed\":")+9).toFloat();
//
//       Serial.print("lat:");
//       Serial.print(lat,6);
//       Serial.print(",lng:");
//       Serial.println(lng,6);
//       Serial.print(",speed:");
//       Serial.println(speed,2);
//       sprintf(date_gps,"{\"lat\":%.6f,\"lng\":%.6f,\"speed\":%.2f}",lat,lng,speed);
//       mqtt.publish(TOPIC,date_gps);
//     }
//
// }
//
// void reconnected_WiFi(){
//     delay(200);
//     //Serial.print(".");
//     target_read();
//    if(login == false){
//     Serial.println(timer_count);
//     Serial.println(timer_lock);
//     if(timer_count == 238){
//      Serial.println("phone disconnect: lock car");
//    }
//     if(timer_count<239){
//      timer_count++;
//    }else{
//       if(timer_lock <239){
//         digitalWrite(buzzer,!digitalRead(buzzer));
//         timer_lock++;
//       }else{
//         Serial.println("lock car: ok");
//         digitalWrite(buzzer, LOW);
//         digitalWrite(ignition, HIGH);
//         connect_mqtt();
//       }
//      }
//      }
//      else{
//        connect_mqtt();
//        timer_count = 0;
//        timer_lock = 0;
//      }
// }
//
//
// void sound(){
//   digitalWrite(buzzer, HIGH);
//   delay(400);
//   digitalWrite(buzzer, LOW);
//   delay(400);
//   digitalWrite(buzzer, HIGH);
//   delay(400);
//   digitalWrite(buzzer, LOW);
// }
//
//
// void loop(){
//
//
//
//   if(WiFi.status() == 1){
//     status_connect_wifi = false;
//     alarm_set = 1;
//     reconnected_WiFi();
//   }else if(WiFi.status() == 3){
//     status_connect_wifi = true;
//     timer_count = 0;
//     timer_lock = 0;
//     if(alarm_set == 1){
//      alarm_set = 0;
//      login = false;
//      Serial.println("phone connect: unlock car");
//      sound();
//      digitalWrite(ignition, LOW);
//     }
//     mqttHostpot();
//   }
//
//   if(digitalRead(PANIC) == HIGH){
//     panic_status = 1;
//     delay(500);
//     Serial.println(digitalRead(PANIC));
//     Serial.println("press buttom panic detect..");
//   }
//
// }
