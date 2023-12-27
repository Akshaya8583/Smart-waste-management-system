#include <M5Stack.h>
#include <WiFi.h>
#include <IRremote.h>
#include <SD.h>

#define WIFI_NETWORK "Homeless"
#define WIFI_PASS "sherlock"
#define WIFI_TIMEOUT 20000

const int IR_PIN_Wet = 21;
const int IR_PIN_Metal = 26;
const int IR_PIN_Dry = 36;  // define the digital pin for the IR sensor


const char *serverIP = "192.168.168.124";
const int serverPort = 8080; // Use the port number you want

WiFiClient client;

void setup() {
  M5.begin();
  M5.Power.begin();
  M5.Lcd.setTextSize(3);

  pinMode(IR_PIN_Wet, INPUT);
  pinMode(IR_PIN_Dry, INPUT);
  pinMode(IR_PIN_Metal, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_NETWORK, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  int containerFullWet = digitalRead(IR_PIN_Wet);
  int containerFullDry = digitalRead(IR_PIN_Dry);
  int containerFullMetal = digitalRead(IR_PIN_Metal);
  String msg = messageForFullContainer(containerFullWet,containerFullDry,containerFullMetal);
  // Connect to the server
  if (!client.connected()) {
    Serial.println("Connecting to server...");
    if (client.connect(serverIP, serverPort)) {
      Serial.println("Connected to server");
      // Send data to the server
      client.println(msg);
      delay(1000); // Add a delay to avoid flooding the server
      client.stop(); // Disconnect from the server
    } else {
      Serial.println("Connection failed");
    }
  }

  delay(10000); // Repeat every 5 seconds
  msg = "";
}


String messageForFullContainer(int containerFullWet,int containerFullDry,int containerFullMetal ){
  if(!containerFullWet){
    return "Wet container is full";
  }
  
  else if(!containerFullDry){
    return "Dry container is full";
  }
  else if(!containerFullMetal){
    return "Metal container is full";
  }
  else{
    return "";
  }
}
