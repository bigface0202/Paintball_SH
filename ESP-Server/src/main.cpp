#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;
WiFiClient client_M5Stack;
WiFiClient client_ESP;

//PGM Valve
int PGM_valve1 = 25;
int PGM_valve2 = 26;
int PGM_valve3 = 32;
int PGM_valve4 = 33;

//Player life
int lifeCount = 5;

// Functions
void server_raspberry();
void watch();
void valves_actuation();

// Server objects for ESP
WiFiServer server(80);
//Server variables
const char* ssid     = "Human-A1-721-2G_EXT";
const char* password = "bsys12bsys34";
char c;
// Client variables to send to raspberry
const uint16_t port = 5204;
const char * host = "10.31.1.108"; // ip or dns
// flag to print just one time the IP of the connected server
char flag_activated = 'N';

void setup() {
    Serial.begin(115200);
    delay(10);
    // PIN setup
    pinMode(PGM_valve1,INPUT);
    pinMode(PGM_valve2,INPUT);
    pinMode(PGM_valve3,INPUT);
    pinMode(PGM_valve4,INPUT);

    // ESP Server
    WiFi.begin(ssid, password);
    // Connecting to the network
    //assigned when connected to a WiFi network
    Serial.print("Wait for WiFi... ");
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(500);
    // This is to start the Server (for the M5Stack)
    server.begin();
    // This part is to make a client for the Raspberry
    // WiFiMulti.addAP(ssid, password);
    // while(WiFiMulti.run() != WL_CONNECTED) {
    //   Serial.print(".");
    //   delay(100);
    // }
}

void loop() {
    // This function connect the watch
    watch();
    delay(10);
}

void watch() {
  // listen for incoming clients
  client_M5Stack = server.available();
  if (client_M5Stack) {
    // if client connected
    Serial.println("New Client.");
    while (client_M5Stack.connected()) {
      // loop while the client's connected
      if (client_M5Stack.available()) {
        // if there's bytes to read from the client
        c = client_M5Stack.read();
        Serial.println(c);
        if (c == 'H'){
          lifeCount--;
        }else if (c == 'R'){
          if(lifeCount < 5){
            lifeCount++;
          }
        }
        // read a byte
        valves_actuation();
        // This function connect to the Raspberry
        server_raspberry();
      }
    }
  }
}

void valves_actuation() {
  if(lifeCount == 4){
    digitalWrite(PGM_valve1,HIGH);
    digitalWrite(PGM_valve2,LOW);
    digitalWrite(PGM_valve3,LOW);
    digitalWrite(PGM_valve4,LOW);
  }else if(lifeCount == 3){
    digitalWrite(PGM_valve1,HIGH);
    digitalWrite(PGM_valve2,HIGH);
    digitalWrite(PGM_valve3,LOW);
    digitalWrite(PGM_valve4,LOW);
  }else if(lifeCount == 2){
    digitalWrite(PGM_valve1,HIGH);
    digitalWrite(PGM_valve2,HIGH);
    digitalWrite(PGM_valve3,HIGH);
    digitalWrite(PGM_valve4,LOW);
  }else if(lifeCount == 1){
    digitalWrite(PGM_valve1,HIGH);
    digitalWrite(PGM_valve2,HIGH);
    digitalWrite(PGM_valve3,HIGH);
    digitalWrite(PGM_valve4,HIGH);
  }else{
    digitalWrite(PGM_valve1,LOW);
    digitalWrite(PGM_valve2,LOW);
    digitalWrite(PGM_valve3,LOW);
    digitalWrite(PGM_valve4,LOW);
  }
}

void server_raspberry() {
  // Client connecting to raspberry
  // Print just one time the IP of the server
  if (flag_activated == 'N') {
    Serial.print("connecting to ");
    Serial.println(host);
    flag_activated = 'Y';
  }
  if (!client_ESP.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(10);
    return;
  }
  // This will send the request to the server
  client_ESP.print("T");
  delay(10);
}
