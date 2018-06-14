#pragma mark - Depend ESP8266Audio and ESP8266_Spiram libraries
/*
cd ~/Arduino/libraries
git clone https://github.com/earlephilhower/ESP8266Audio
git clone https://github.com/Gianbacchio/ESP8266_Spiram
Use the "Tools->ESP32 Sketch Data Upload" menu to write the MP3 to SPIFFS
Then upload the sketch normally.
https://github.com/me-no-dev/arduino-esp32fs-plugin
*/
#include <Arduino.h>
#include <M5Stack.h>
#include <IRremote.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>
#include <WiFi.h>
#include <WiFiMulti.h>

/*Pin*/
int switch_PIN = 2; //SHOT switch
int IR_receptorPin = 17;//Pin used to read IR values

/*Game status*/
int switchOut = 0;
int last_switchOut = 0;
int shootCount = 0;
int lifeCount = 5;
int recoverFlag = 0; //If recoverFlag = 0, player can recover 1 muscle. After use recover item, this flag = 1.

/*Other*/
int i = 0;
int j = 10;

AudioGeneratorWAV *wav;
AudioFileSourceSD *file_shoot;
AudioOutputI2S *out;
/*IR*/
IRsend irsend; //Pin number 3 is IR
IRrecv irrecv(IR_receptorPin);//Create an object
decode_results results;
// Wi-Fi objects
WiFiMulti WiFiMulti;
WiFiClient client_M5Stack;

//Function
void watch_functions();
void IR_Receptor();
void IR_Transmitter();
void Game_over();
void wifi_connection_esp();//function for Wi-Fi

// This port and IP are the ones that we will use to connect to ESP32
const uint16_t port = 80;
const char * host = "10.31.1.35"; // ip or dns
int stack_ip;


void IR_Receptor() {
  if (irrecv.decode(&results)) {
    if (results.decode_type == SONY) {
      delay(1000);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.fillRect(20, 70+((5-lifeCount) * 25), 60, 20, BLACK); //Remove a life
      lifeCount--;
      // send one character H (Hit) every time the players was shooted
      client_M5Stack.print("HIT");
    }else if (results.decode_type == NEC){
      
        /*Make recover sound*/
        file_shoot->close();
        file_shoot = new AudioFileSourceSD("/se_maoudamashii_magical25.wav");
        wav->begin(file_shoot, out);
        dacWrite(25, 0);
        lifeCount++;
        M5.Lcd.fillRect(20, 70+((5-lifeCount) * 25), 60, 20, RED);
        recoverFlag = 1;
        delay(500);
      
    }
    irrecv.resume(); // Receive the next value
  }
}

void IR_Transmitter() {
  /*Shoot switch was pressed*/
  /*Read switch*/
  switchOut = digitalRead(switch_PIN);
  if(last_switchOut != switchOut){
    if (switchOut == HIGH){
      irsend.sendSony(0xa90, 12);

      /*Make shooting sound*/
      file_shoot->close();
      file_shoot = new AudioFileSourceSD("/se_maoudamashii_battle_gun05.wav");
      wav->begin(file_shoot, out);
      dacWrite(25, 0);

      //Showing the SHOOT!!!!
      M5.Lcd.setTextColor(RED);
      M5.Lcd.setCursor(0, 220);
      M5.Lcd.print("SHOOT!!!");

      shootCount = shootCount + 1;

      client_M5Stack.println("SHOT");

      if (shootCount % 5 == 0 ){
        M5.Lcd.fillRect(130, 70+((shootCount / 5 - 1) * 25), 60, 20, BLACK); //Remove a shot
      }

      //Delete the SHOOT!!!!
      delay(100);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setCursor(0, 220);
      M5.Lcd.print("SHOOT!!!");
      M5.Lcd.setTextColor(WHITE);
    }
  }
  last_switchOut = switchOut;
}
void Game_over(){
  if(lifeCount == 0){
    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setTextFont(4);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(70, 120);
    M5.Lcd.print("GAME OVER");
    delay(100000);
    exit(0);
  }
}

void watch_functions() {
  /*LCD setup*/
  M5.Lcd.setTextFont(4);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print(lifeCount);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.print("IP :");
  M5.Lcd.setCursor(40, 30);
  M5.Lcd.print(host);

  M5.Lcd.drawRect(10,60,80,140,WHITE);
  M5.Lcd.setCursor(25, 215);
  M5.Lcd.print("LIFE");

  M5.Lcd.setTextFont(4);
  M5.Lcd.drawRect(120,60,80,140,WHITE);
  M5.Lcd.setCursor(115, 215);
  M5.Lcd.print("BULLET");

  M5.Lcd.setTextFont(4);
  M5.Lcd.drawRect(230,60,80,140,WHITE);
  M5.Lcd.setCursor(215, 215);
  M5.Lcd.print("CHARGE");

  /*IR Receptor*/
  IR_Receptor();

  /*IR Transmitter*/
  IR_Transmitter();

  /*Game over*/
  Game_over();

  /*??*/
  if(wav->isRunning()){
    if(!wav->loop()){
      wav->stop();
    }
  }

 
}

void setup() {
  Serial.begin(115200);
  M5.begin();

  // /*LCD setup*/
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Wait for WiFi... ");
  
  // WiFi setup
  // We start by connecting to a WiFi network
  WiFiMulti.addAP("Human-A1-721-2G_EXT", "bsys12bsys34");
  // Start the connection of the client and wait until connect to the lan
  while(WiFiMulti.run() != WL_CONNECTED) {
           M5.Lcd.setCursor(i, j);
           M5.Lcd.print(".");
           delay(1000);
           i = i + 5;
  }
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 25);
  M5.Lcd.print("WiFi connected");
  // //print the IP assigned to the device
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.print("IP address: ");
  M5.Lcd.setCursor(0, 55);
  M5.Lcd.print(WiFi.localIP());
  delay(500);
  M5.Lcd.setCursor(0, 70);
  M5.Lcd.print("connecting to ");
  M5.Lcd.setCursor(0, 85);
  M5.Lcd.print(host);
  // This will comprobe if Stack is connected to the ESP-Server
  
  /*
  if (!client_M5Stack.connect(host, port)) {
          M5.Lcd.setCursor(0, 100);
          M5.Lcd.print("connection failed");
          M5.Lcd.setCursor(0, 115);
          M5.Lcd.print("wait 5 sec...");
          delay(2000);
          return;
  }
  */

  /*Audio setup*/
  /*Please move music file(se_maoudamashii_battle_gun05.wav) into SD.
  This file put on the music folder*/
  file_shoot = new AudioFileSourceSD("/se_maoudamashii_battle_gun05.wav");
  out = new AudioOutputI2S(0, 1); // Output to builtInDAC
  out->SetOutputModeMono(true);
  wav = new AudioGeneratorWAV();
  //wav->begin(file, out);

  /*Game setup*/
  pinMode(switch_PIN, INPUT);
  delay(2000);
  //M5.Lcd.print("START!!");
  //delay(2000);
  M5.Lcd.fillScreen(BLACK);

  /*IRrecev setup*/
  irrecv.enableIRIn(); // Start the receiver

  /*Show your live*/
  M5.Lcd.fillRect(20,70,60,20,RED);
  M5.Lcd.fillRect(20,95,60,20,RED);
  M5.Lcd.fillRect(20,120,60,20,RED);
  M5.Lcd.fillRect(20,145,60,20,RED);
  M5.Lcd.fillRect(20,170,60,20,RED);

  /*Show your bullets*/
  M5.Lcd.fillRect(130,70,60,20,GREEN);
  M5.Lcd.fillRect(130,95,60,20,GREEN);
  M5.Lcd.fillRect(130,120,60,20,GREEN);
  M5.Lcd.fillRect(130,145,60,20,GREEN);
  M5.Lcd.fillRect(130,170,60,20,GREEN);

  /*Show your charge*/
  M5.Lcd.fillRect(240,70,60,20,YELLOW);
  M5.Lcd.fillRect(240,95,60,20,YELLOW);
  M5.Lcd.fillRect(240,120,60,20,YELLOW);
  M5.Lcd.fillRect(240,145,60,20,YELLOW);
  M5.Lcd.fillRect(240,170,60,20,YELLOW);

  //At the begining, IR turn on. I don't know why. This code turn off the IR.
  irsend.sendSony(0xa90, 12);
}

void loop() {
  watch_functions();
  M5.update();
}
