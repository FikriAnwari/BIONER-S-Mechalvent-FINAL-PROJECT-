#include <FirebaseESP32.h>
#include <PZEM004Tv30.h>
#include <max6675.h>
#include <HardwareSerial.h>
#include <WiFi.h>






//--------------------------------------------INITIALIZATION & VARIABLES----------------------------------------------
HardwareSerial Senddata(0);
//sensor tekanan     
float hasil, hasil2;

//sensor suhu
int sck = 25;
int cs = 26;
int so = 27;
float suhu;
MAX6675 thermocouple(sck,cs,so);

int sck2 = 14;
int cs2 = 12;
int so2 = 13;
float suhu2;
MAX6675 thermocouple2(sck2,cs2,so2);

//sensor ultrasonic
const int trigPin = 5;
const int echoPin = 18;
long duration;
int distance;












//inisialisasi wattmeter
float ampere, volt, watt; 
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif


#if defined(ESP32)
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
#elif defined(ESP8266)
#else
PZEM004Tv30 pzem(PZEM_SERIAL);
#endif

//FIREBASE
const char* SSID = "HUAWEI_E5577_3E89";
const char* PASSWORD = "Q363N44QLBE";
#define FIREBASE_HOST "bioner-s-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "Z0X0O8siCPVzBfKYiZDjqmvw22RcEBPlLfO03544"
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;
String Tekanan_uapPATH = "/Tekanan_uap";
String SuhuAirPATH = "/SuhuAir";
String SuhuApiPATH = "/SuhuApi";
String VoltPATH = "/Volt";
String AmperePATH = "/Ampere";
String WattPATH = "/Watt";
String BlowerPATH = "/Blower";

//RELAY
const int pinT = 2;
const int pinRelay = 4;
bool RelayState = false;















//--------------------------------------------FUNCTIONS---------------------------------------------------

//sensor tekanan
float sensorTekanan(int pin){
  float read, value;
  read = analogRead(pin);

  value = (read * 12) / 5204.5;
  return value;
}

//sensor ultrasonic
double fungsi_menghitung_jarak(){
// Clears the trigPin
digitalWrite(trigPin, LOW);
delayMicroseconds(2);

// Sets the trigPin on HIGH state for 10 micro seconds
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);

// Reads the echoPin, returns the sound wave travel time in microseconds
duration = pulseIn(echoPin, HIGH);

// Calculating the distance
double distance= duration*0.0343/2;

//mengembalikan nilai
return distance;
  
}













void setup(){
  Serial.begin(115200);
  Senddata.begin(115200, SERIAL_8N1, 1, 3); 
  Serial.println("Pengirim siap...");
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  pinMode(pinT, INPUT_PULLUP);
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay, HIGH); 



  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

// Assign Firebase credentials to config
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  // Initialize Firebase
  Serial.println("Initializing Firebase...");
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase initialized.");


  
  
}

  

void loop(){
  //sensor tekanan
  hasil = sensorTekanan(33);
  hasil2 = sensorTekanan(32);
  // Serial.println("Bioners");
  // Serial.print(hasil);
  // Serial.print(" ");
  // Serial.print(analogRead(33));
  // Serial.println(" pin33");
  // Serial.print(hasil2);
  // Serial.print(" ");
  // Serial.print(analogRead(32));
  // Serial.println(" pin32");
  // Serial.println("");

  //sensor suhu
  suhu = thermocouple.readCelsius();  
  // Serial.print(suhu);
  // Serial.println(" C");
  suhu2 = thermocouple2.readCelsius();  
  // Serial.print(suhu2);
  // Serial.println(" C");
  // Serial.println("");

  //sensor ultrasonic
  double jarak = fungsi_menghitung_jarak();
  // Serial.println(jarak);
  // Serial.println("");
  
//DUMMY DATA/////////////////////////////
hasil = random(8,9);
suhu2 = random(134, 140);
suhu = random(234, 240);
volt = random(80, 200);
ampere = random(0.30, 2);
watt = volt*ampere;
/////////////////////////////////////////


  //mengirim data ke layar tft esp32s3  
      // Serial.print("Tekanan uap: ");
Senddata.print(hasil); 
Senddata.print(",");

      // Serial.print("Suhu Air: ");
Senddata.print(suhu); 
Senddata.print(",");

  char voltBFR[8];
  snprintf(voltBFR, sizeof(voltBFR), "%06.2f", volt);

      // Serial.print("Teganganan: ");
Senddata.print(voltBFR);
Senddata.print(",");

      // Serial.print("Suhu Api: ");
Senddata.print(suhu2); 
Senddata.print(",");

      // Serial.print("Arus: ");
Senddata.print(random(0,10));
Senddata.println();


  
  // Send pressure data to Firebase
  Serial.println("Sending pressure data to Firebase...");
  if ( 
  Firebase.setFloat(firebaseData, Tekanan_uapPATH.c_str(), hasil) 
  && Firebase.setFloat(firebaseData, SuhuAirPATH.c_str(), suhu2) 
  && Firebase.setFloat(firebaseData, SuhuApiPATH.c_str(), suhu) 
  && Firebase.setFloat(firebaseData, VoltPATH.c_str(), volt)
  && Firebase.setFloat(firebaseData, AmperePATH.c_str(), ampere) 
  && Firebase.setFloat(firebaseData, WattPATH.c_str(), watt) 
  )
   {
    Serial.println("Pressure data sent successfully");
  } else {
    Serial.println("Failed to send pressure data");
    Serial.println("Reason: " + firebaseData.errorReason());
  }


//Relay
	if(suhu2 < 250 || digitalRead(pinT) == 1){
    digitalWrite(pinRelay, LOW);
    Firebase.setFloat(firebaseData, BlowerPATH.c_str(), true);
  }else{
    digitalWrite(pinRelay, HIGH);
    Firebase.setFloat(firebaseData, BlowerPATH.c_str(), false);
  }

delay(50); // Menunggu 1 detik sebelum membaca kembali
}


