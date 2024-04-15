#include <WiFi.h>
#include <SocketIoClient.h>
#include <ArduinoJson.h>

const char* ssid     = "";
const char* password = "";

char host[] = "192.168.100.202";
int port = 3000;
char path[] = "/socket.io/?transport=websocket";

SocketIoClient webSocket;
WiFiClient client;

unsigned long startTime;
unsigned long currentTime;


int LED_PIN_CONTROL = 16;
int LED_PIN_UMBRAL = 5;

bool ledStatus = false;
int ledIntensity = 255;

int sensorTreshold = 50;



void socket_connected(const char * payload, size_t length) {
    Serial.println("Socket Connected!");
}

void socket_action(const char * payload, size_t length) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload); // Parse the JSON object in the string

  if (error) { // Check for errors in parsing
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  const char* type = doc["type"]; // Get the value of "type"
  int value = doc["value"]; // Get the value of "value"

  Serial.println(type);
  Serial.println(value);
  

  if(strcmp(type, "umbral-humedad") == 0) {
      sensorTreshold = value;
  }
  if(strcmp(type, "led1-apagar") == 0) {
      ledStatus = false;
  }
  if(strcmp(type, "led1-encender") == 0) {
      ledStatus = true;
  }
  if(strcmp(type, "led1-intensidad") == 0) {
      ledIntensity = value;
  }
}

void socket_send_update(int value) {
  String message = "{\"type\": \"humedad\", \"value\": " + String(value) + "}";
  webSocket.emit("event", message.c_str());
}

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  ledcSetup(0, 5000, 8);
  ledcAttachPin(LED_PIN_CONTROL, 0);
  
  pinMode(LED_PIN_UMBRAL, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  webSocket.on("connect", socket_connected);
  webSocket.on("action", socket_action);  

  webSocket.begin(host, port, path);

  startTime = millis(); 
}


void sensor_loop() {

  int sensor_value = 40;
  if(millis() - startTime > 4000) {
    startTime = millis();
    socket_send_update(sensor_value);
  }

  if(sensor_value > sensorTreshold) {
    digitalWrite(LED_PIN_UMBRAL, HIGH);
  } else {
    digitalWrite(LED_PIN_UMBRAL, LOW);
  }
  
  if(ledStatus == true) {
    ledcWrite(0, ledIntensity);
  }
  else {
    ledcWrite(0, 0);
  }
}

void loop() {
  webSocket.loop();
  sensor_loop();
}