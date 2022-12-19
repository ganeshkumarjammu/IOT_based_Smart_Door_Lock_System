#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Ganesh";
const char* password = "ganesh2000";

// Initialize Telegram BOT
String BOTtoken = "5734241556:AAGPSO1L7H4N8L8ViO8Js91W1wqrxR_1AQo";  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
String CHAT_ID = "-1001686432193";
// "1260116757";

bool sendPhoto = false;
bool msgEnabled = true ;
WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 2
bool flashState = LOW;

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// Define GPIOs
#define BUTTON 12
#define LOCK 5
#define LOCK2 16
#define LOCK3 15
#define FLASH_LED 2

#define TRIG_PIN 23 // ESP32 pin GIOP23 connected to Ultrasonic Sensor's TRIG pin
#define ECHO_PIN 22 // ESP32 pin GIOP22 connected to Ultrasonic Sensor's ECHO pin
float duration_us, distance_cm;

int lockState = 0;
String r_msg = "";

String unlockDoor(){  
  if (lockState == 1) {
  digitalWrite(LOCK, LOW);
  lockState = 0;
  delay(100);
  return "Door Unlocked. /lock";
 }
 else{
  return "Door Already Unlocked. /lock";
 }
}
String lockDoor(){
 if (lockState == 0) {
  digitalWrite(LOCK, HIGH);
  lockState = 1;
  delay(100);
  return "Door Locked. /unlock";
 }
 else{
  return "Door Already locked. /unlock";
 }  
}


void handleNewMessages(int numNewMessages){
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++){
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    Serial.print("chat id: ");
    Serial.println(chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
//    CHAT_ID = chat_id;
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    String fromName = bot.messages[i].from_name;
    if ((text == "/photo") || (text == "/photo@Smart_Dora_bot" )) {
      sendPhoto = true;
      Serial.println("New photo request");
    }

    if ((text == "/lock") || (text == "/lock@Smart_Dora_bot" )){
      String r_msg = lockDoor();
      bot.sendMessage(CHAT_ID, r_msg, "");
    }
    if ((text == "/unlock") || (text == "/unlock@Smart_Dora_bot" )){
      String r_msg = unlockDoor();
      bot.sendMessage(CHAT_ID, r_msg, "");
    }
    if ((text == "/start") || (text == "/start@Smart_Dora_bot" )){
      String welcome = "Welcome to the ESP32 Telegram Smart Lock.\n";
      welcome += "/lock : Lock the Door\n";
      welcome += "/unlock : Unlock the Door\n\n";
      welcome += "/lockStatus : Takes a know Lock Status\n";
      welcome += "/notify : To turn on/off notificaton\n";
      welcome += "/Buzz : To turn on buzzer\n";
      welcome += "To get the photo please tap on /photo.\n";
      bot.sendMessage(CHAT_ID, welcome, "Markdown");
    }
    if ((text == "/flash") || (text == "/flash@Smart_Dora_bot" )) {
      flashState = !flashState;
      String r_msg = "Flash toggled";
      bot.sendMessage(CHAT_ID, r_msg, "");
      digitalWrite(FLASH_LED_PIN, flashState);
      Serial.println("Change flash LED state");
    }
    if ((text == "/notify") || (text == "/notify@Smart_Dora_bot" )){
      msgEnabled = !msgEnabled ;
      if(msgEnabled){
      String r_msg = "notification turned on";
      bot.sendMessage(CHAT_ID, r_msg, "");
      }
      else{
      String r_msg = "notification turned off";
      bot.sendMessage(CHAT_ID, r_msg, ""); 
      }
    }
  }
}


void setup(){
  pinMode(LOCK,OUTPUT);
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  // Init Serial Monitor
  Serial.begin(115200);

  // configure the trigger pin to output mode
  pinMode(TRIG_PIN, OUTPUT);
  // configure the echo pin to input mode
  pinMode(ECHO_PIN, INPUT);
  
  // Set LED Flash as output
  pinMode(FLASH_LED_PIN, OUTPUT); 
  digitalWrite(FLASH_LED_PIN, flashState);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  lockDoor();
  Serial.println("Succesfully Connected ");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP()); 
}

void loop() {
  if (sendPhoto) {
    Serial.println("Preparing photo");
    digitalWrite(FLASH_LED, HIGH);
    delay(200);
    digitalWrite(FLASH_LED, LOW);
    sendPhoto = false; 
  }
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//  Serial.println(numNewMessages);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  
  // generate 10-microsecond pulse to TRIG pin
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // measure duration of pulse from ECHO pin
  duration_us = pulseIn(ECHO_PIN, HIGH);
  delayMicroseconds(2);
  // calculate the distance
  distance_cm = 0.017 * duration_us;
  
  // print the value to Serial Monitor
  Serial.print("distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  delay(50);
  if (distance_cm < 30 && msgEnabled ){
      String r_msg = "Object detected , /photo"; //whether this text will be received by the esp32cam ??
      bot.sendMessage(CHAT_ID, r_msg, "");
  }
}
