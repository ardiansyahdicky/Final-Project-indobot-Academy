#include <Servo.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager

// Sensor ultrasonik 1
const int trigPin1 = D4; // Trigger pin untuk sensor 1
const int echoPin1 = D8; // Echo pin untuk sensor 1

// Sensor ultrasonik 2
const int trigPin2 = D5; // Trigger pin untuk sensor 2
const int echoPin2 = D6; // Echo pin untuk sensor 2

// DFPlayer setup
SoftwareSerial mySoftwareSerial(D2, D1); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27

long duration1;
int US1;
long duration2;
int US2;

#define MAX_DISTANCE 50
Servo myservo;

// Isikan sesuai pada Firebase
#define FIREBASE_HOST "proyek-akhir-7d1ac-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "8HIRc32AbTvQ4mfryPL4cLvcUlzfMGR4gSK4DuCt"

// Mendeklarasikan objek data dari FirebaseESP8266
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

void setup_wifi() {
    // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
    //Serial.begin(115200);
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("WISEBIN.CO","indobotacademy"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

}

void setup() {
  Serial.begin(115200);
  //********** CHANGE PIN FUNCTION  TO GPIO **********
  //GPIO 1 (TX) swap the pin to a GPIO.
  pinMode(1, FUNCTION_3); 
  //GPIO 3 (RX) swap the pin to a GPIO.
  pinMode(3, FUNCTION_3); 
  //***
  Wire.begin(3,1);
  lcd.init();
  lcd.backlight();
  // Set up ultrasonic sensor pins
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  myservo.attach(D7);
  myservo.write(0);
  mySoftwareSerial.begin(9600);
    if (!myDFPlayer.begin(mySoftwareSerial)) {
      Serial.println("Unable to begin:");
      Serial.println("1. Please recheck the connection!");
      Serial.println("2. Please insert the SD card!");
      while (true); // Stop execution
    }
    myDFPlayer.volume(30);
  
  Serial.println("Setup Complete");
  setup_wifi();

  // Konfigurasi Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);
}

void loop() {
  // Read ultrasonic sensor 1
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  US1 = duration1 * 0.034 / 2;

  // Read ultrasonic sensor 2
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  US2 = duration2 * 0.034 / 2;

  // Cap US2 at MAX_DISTANCE
  if (US2 > MAX_DISTANCE) {
    US2 = MAX_DISTANCE;
  }

  // Calculate percentage
  int persentase = (US2 / (float)MAX_DISTANCE) * 100;
  
  // Print readings to Serial Monitor for debugging
  /*Serial.print("Jarak S/ensor 1: ");
  Serial.print(US1);
  Serial.println(" cm");
  Serial.print("Jarak Sensor 2: ");
  Serial.print(US2);
  Serial.println(" cm");
  Serial.print("Sisa Kapasitas: ");
  Serial.print(persentase);
  Serial.println("%");*/

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sisa Kapasitas");
  lcd.setCursor(0, 1);
  lcd.print("Tong Sampah: ");
  lcd.print(persentase);
  lcd.print("%");

  // Create buffer to store percentage
  char percentageBuffer[7];
  snprintf(percentageBuffer, sizeof(percentageBuffer), "%3d%%", persentase);
  lcd.print(percentageBuffer);   // Print percentage on the LCD

  // Kontrol servo berdasarkan pembacaan sensor US1 dan kirim data Firebase
  if (Firebase.setInt(firebaseData, "/Data/Sisa Kapasitas Tong Sampah", persentase) &&
      Firebase.setInt(firebaseData, "/Data/Jarak Ultrasonik 1", US1)) {
  
      Serial.print("Sampah: ");
      Serial.println(US1);
  
      Serial.print("Kapasitas: ");
      Serial.print(persentase);
      Serial.println("%");
  
      // Memposisikan servo berdasarkan nilai sensor US1 jika persentase > 10
      if (persentase > 10) {
          if (US1 <= 10) {
              myservo.write(180);
              myDFPlayer.play(4);
              delay(5000);
              myDFPlayer.play(1);
              delay(5000);
          } else {
              myservo.write(0);
          }
      } else if (US1 <= 10 && persentase < 10) {
          myservo.write(0); 
          myDFPlayer.play(3);
          delay(5000);
      } else {
          myservo.write(0);
      }
  
      Serial.println("Jarak ultrasonik 1 dan Kapasitas Tong Sampah terkirim");
  } else {
      Serial.println("Data tidak terkirim");
      Serial.print("Alasan: ");
      Serial.println(firebaseData.errorReason());
  }

  // Delay before next loop iteration
  delay(100); // Increase delay to 1 second for readability
}
