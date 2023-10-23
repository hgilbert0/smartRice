#define TINY_GSM_MODEM_SIM800
#define SerialMon Serial // Set serial for debug console (to the Serial Monitor,digital pin 1(tx0) and 0(rx0)
#define SerialAT Serial1 // Use Hardware Serial1 on Mega for GSM/GPRS, digital pin 18(tx2) and 19(rx2)
#define Serial_NPK Serial2 // Use Hardware Serial2 on Mega for NPK semsor digital pin 16(tx2) and 17(rx2)


#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 650
#endif
#define TINY_GSM_YIELD() { delay(2); }

char apn[]  = "internet.mtn"; // Change this to your Provider details
char gprsUser[] = "";
char gprsPass[] = "";
char server[]   = "hatangaiot.atwebpages.com";
char resource[] = "/insert.php";
char key[]="a@4K%3";
int  port = 80;
unsigned long timeout;




#include <TinyGsmClient.h>
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
//#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// R0 to RX2 & DI to TX2
#define RE 3 //Receiver enable
#define DE 2 //Drive enale

const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xE4, 0x0C};
const byte phos[] =  {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xB5, 0xCC};
const byte pota[] =  {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xC0};
 
byte values[11];
byte nitrogen, phosphorous, potassium;

// Ultrasonic sensor connections
const int ultrasonicTrigPin = 4;
const int ultrasonicEchoPin = 5;
const int gsmPowerOn = 8;
const int buzzer_pin = 10;
const int SENSOR_PIN = 6; // Arduino pin connected to DS18B20 sensor's DQ pin(Temperature Sensor)
const int pir_pin = 9;
const int led_pin = 12;
const int moisture_pin = A2;

const int ph_pin = A4;
float calibration_value = 32.34 - 0.7;
int phval = 0;
unsigned long int avgval;
int buffer_arr[10],temp;

int pir_flag ,dist_flag,tempe_flag,mois_flag,pota_flag,phos_flag,nitro_flag = 1;
int pirVal = 0;
String message,post_data;

float moisture,tempcelius,pH_act,pH,temperature,Didtance,distance;

OneWire temperat(SENSOR_PIN);         // setup a oneWire instance
DallasTemperature tempSensor(&temperat); // pass oneWire to DallasTemperature library


void setup() 
{
  SerialMon.begin(115200);
  SerialMon.println("System Initialising..........");
  SerialAT.begin(9600);
  Serial_NPK.begin(9600);
  //---------------------------------------- put RS-485 into receive mod-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  //-------------------------------------------turn on GSM automatically-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  pinMode(gsmPowerOn, OUTPUT);
  digitalWrite(gsmPowerOn,HIGH );
  delay(1000);
  digitalWrite(gsmPowerOn, LOW);
  delay(2000);
  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  pinMode(buzzer_pin,OUTPUT);
  pinMode(pir_pin,INPUT);
  pinMode(led_pin,OUTPUT);
  pinMode(ultrasonicTrigPin,OUTPUT);
  pinMode(ultrasonicEchoPin,INPUT);
  //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  tempSensor.begin();    // initialize the sensor
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(1000);
    return;
  }
  SerialMon.println(" success");
  if (modem.isNetworkConnected()){
    SerialMon.println("Network connected");
  }
 digitalWrite(buzzer_pin,HIGH);
 delay(1000);
 digitalWrite(buzzer_pin,LOW);
 delay(1000);
}

void loop() 
{ 
  distance = getDistance();//read the 
  SerialMon.println("distance: "+String(distance)+String(" cm"));
  
  moisture = read_moisture();
  SerialMon.println("moisture: "+String(moisture)+String("%"));
  
  temperature = getTempe();//read the temperature from the sensor in the soil
  SerialMon.println("temperature: "+String(temperature)+String(" °C"));
                                           
  
  pH = get_pH();//read the PH level from the sensor in the soil
  SerialMon.println("pH: "+String(pH));
  
  nitrogen = get_Nitrogen();//read nitrogen from the sensor in the soil
  SerialMon.println("nitrogen: "+String(nitrogen)+String(" mg/kg"));
  delay(250);
  phosphorous = get_Phosphorous();//read phosphorus from the sensor in the soil
  SerialMon.println("Phosphorous: "+String(phosphorous)+String(" mg/kg"));
  delay(250);
  potassium = get_Potassium();//read potassium from the sensor in the soil
  SerialMon.println("Potassium: "+String(potassium)+String(" mg/kg"));

  post_data = String(nitrogen)+","+String(phosphorous)+","+String(potassium)+","+String(pH)+","+String(temperature)+","+String(moisture);
  SerialMon.println(post_data);
  SerialMon.println("--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"); 
  //----------------------------------------------------------Connect the GSM(sim900)to the internet(GPRS)------------------------------------------------------------------------------------------------------------------------
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(1000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");
  }
  else
  {
    modem.gprsConnect(apn, gprsUser, gprsPass);
    SerialMon.println("GPRS Reconnected");
  }
  SerialMon.println(F("connecting to Server...."));
  if (!client.connect(server, port))
  {
    SerialMon.println(" fail");
    client.connect(server, port);
  }
  else
  {
    SerialMon.println(F("Server connected"));
    HTTPpost();
  }
  check_motion();  //read the PIR sensor to check weather there is birds in the farm
  check_distance();//check weather there is an unthorised entrance in the farm
  check_nitrogen();//check weather the nitrogen level is within the normal level for the plant growth
  check_phosphorus();
  check_potassium();
  check_moisture();
  check_temperature();
}

//---------------------------------------------Functions--------------------------------------------------------------------------------------------------------------------
//--------------------------------------Get Temperature Function--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------  
float getTempe()
{
  tempSensor.requestTemperatures();             // send the command to get temperatures
  tempcelius = tempSensor.getTempCByIndex(0);  // read temperature in Celsius
  return tempcelius;
  //delay(500);
} 
//---------------------------------------Get moisture Function---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float read_moisture() 
{
  int sensor_analog = analogRead(moisture_pin);
  float Moisture =(100-((sensor_analog/1023.00)*100));
  return Moisture; 
}
//---------------------------------------Get Distance Function---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float getDistance() 
{
  digitalWrite(ultrasonicTrigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(ultrasonicTrigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonicTrigPin,LOW);
  unsigned long duration = pulseIn(ultrasonicEchoPin, HIGH);
  float Distance = (duration * 0.0343)/2;//in cm
  if(Distance >=100){Distance=19;}//sorting back to the minimum distance to minimize noise
  return Distance;  // Calculate remaining water level
}
//--------------------------------Send SMS Function----------------------------------------------------------------------------------------------------------------
void sendSMS(String message)
{
  SerialAT.println("AT"); //Handshaking with SIM900
  delay(2000);
  updateSerial();
  SerialAT.println("AT+CMGF=1"); // Configuring TEXT mode(Format)
  updateSerial();
  delay(2000);
  SerialAT.println("AT+CMGS=\"+250785991560\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();delay(2000);
  SerialAT.print(message); //text content
  updateSerial();
  delay(2000);
  SerialAT.write(26);//ASCII codes for CTR Z, this is to indicate the end of the text message
}
//--------------------------------updateSerial Function------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void updateSerial()
{
  delay(500);
  while (SerialMon.available()) 
  {
    SerialAT.write(SerialMon.read());//Forward what Serial received to Serial1 Port
  }
  while(SerialAT.available()) 
  {
    SerialMon.write(SerialAT.read());//Forward what Serial1 received to Serial Port
  }
}
//----------------------------------------Get NPK Functions----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------Get Nitrogen Function--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
byte get_Nitrogen()
{
  // switch RS-485 to transmit Serial_NPKe
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(1);
 
  // write out the message
  for (uint8_t i = 0; i < sizeof(nitro); i++ ) Serial_NPK.write( nitro[i] );
 
  // wait for the transmission to complete
  Serial_NPK.flush();
  
  // switching RS485 to receive Serial_NPKe
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
 
  // delay to allow response bytes to be received!
  delay(200);
 
  // read in the received bytes
  for (byte i = 0; i < 7; i++) {
    values[i] = Serial_NPK.read();
    //Serial.print(values[i], HEX);
    //Serial.print(' ');
  }
  return values[4];
}
//--------------------------------------Get Phosphorous Function-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
byte get_Phosphorous()
{
  // switch RS-485 to transmit Serial_NPKe
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(1);

  // write out the message
  for (uint8_t i = 0; i < sizeof(phos); i++ ) Serial_NPK.write( phos[i] );
  
  // wait for the transmission to complete
  Serial_NPK.flush();

  // switching RS485 to receive Serial_NPKe
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  
// delay to allow response bytes to be received!
  delay(200);
  for (byte i = 0; i < 7; i++) {
    values[i] = Serial_NPK.read();
    //Serial.print(values[i], HEX);
    //Serial.print(' ');
  }
  return values[4];
}
//--------------------------------------Get Potassium Function--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
byte get_Potassium()
{
  // switch RS-485 to transmit Serial_NPKe
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(1);

  // write out the message
  for (uint8_t i = 0; i < sizeof(pota); i++ ) Serial_NPK.write( pota[i] );

   // wait for the transmission to complete
  Serial_NPK.flush();
  
  // switching RS485 to receive Serial_NPKe
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  
  // delay to allow response bytes to be received!
  delay(200);//wait for 200 milliseconds
  for (byte i = 0; i < 7; i++) {
    values[i] = Serial_NPK.read();
    //Serial.print(values[i], HEX);
    //Serial.print(' ');
  }
  return values[4];
}
//--------------------------------------Get PH Function--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float get_pH()
{
  for(int i=0;i<10;i++){
    buffer_arr[i]=analogRead(ph_pin);
    delay(30);
  }
 for(int i=0;i<9;i++){
  for(int j=i+1;j<10;j++){
    if(buffer_arr[i]>buffer_arr[j]){
      temp=buffer_arr[i];
      buffer_arr[i]=buffer_arr[j];
      buffer_arr[j]=temp;
      }
    }
  }
  avgval=0;
  for(int i=2;i<8;i++)
  avgval+=buffer_arr[i];
  float volt=(float)avgval*5.0/1024/6;//convert the analog into millivolt
  pH_act = -5.70 * volt + calibration_value;//convert the millivolt into pH value
  return pH_act;
  delay(500);
}
//------------------------------------------------------HTTPpost Function---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void HTTPpost()
{
  SerialMon.println("Performing HTTP POST request...");// Send the HTTP POST request
  //Serial.println(post_data);
  String httpRequestData = "key="+String(key)+"&post_data="+String(post_data)+"";
  client.print(String("POST ") + resource + " HTTP/1.1\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.println("Connection: close");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(httpRequestData.length());
  client.println();
  client.println(httpRequestData);
  SerialMon.println(httpRequestData);
  timeout = millis();

  while (client.connected() && millis() - timeout < 10000L) {
    while (client.available()) {
      char c = client.read();
      SerialMon.print(c);
      timeout = millis();
    }
  }
  SerialMon.println();
  client.stop();
  SerialMon.println(F("Server disconnected"));
  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));
}
//--------------------------------------------check_nitrogen Function---------------------------------------------------------------------------------------------------------
 void check_nitrogen()
 {
  if(nitrogen <= 85 )
  {
    if(nitro_flag == 1)
    {
      message = "Insufficient Nitrogen";
      SerialMon.println(message);
      sendSMS(message);
      nitro_flag = 0;
    }
    //digitalWrite(buzzer_pin,HIGH);
  }
  else
  {
    //digitalWrite(buzzer_pin,LOW);
    nitro_flag = 1; 
  }
 }
//--------------------------------------------check_phosphorus Function---------------------------------------------------------------------------------------------------------
 void check_phosphorus()  
 {
  if(phosphorous <= 55 )
  {
    if(phos_flag == 1)
    {
      message = "Insuffient phosphorus";
      SerialMon.println(message);
      sendSMS(message);
      dist_flag = 0;
    }
  }
  else
  {
    phos_flag = 1; 
  }
 }
//--------------------------------------------check_potassium Function---------------------------------------------------------------------------------------------------------
 void check_potassium()  
 {
  if(potassium <= 85 )
  {
    if(pota_flag == 1)
    {
      message = "Insuffient potassium";
      SerialMon.println(message);
      sendSMS(message);
      pota_flag = 0;
    }
  }
  else
  {
    pota_flag = 1; 
  }
 }
//--------------------------------------------check_moisture Function---------------------------------------------------------------------------------------------------------
 void check_moisture() 
 {
  if(moisture <= 90 )
  {
    if(mois_flag == 1)
    {
      message = "Insuffient Water,please water your rice";
      SerialMon.println(message);
      sendSMS(message);
      mois_flag = 0;
    }
  }
  else
  {
    mois_flag = 1; 
  }
 }
 //--------------------------------------------check_temperature Function---------------------------------------------------------------------------------------------------------
 void check_temperature()
 {   
  if(temperature >= 45 )
  {
    if(tempe_flag == 1)
    {
      message = "too hot!!!!! the soil must be dry";
      SerialMon.println(message);
      sendSMS(message);
      tempe_flag = 0;
    }
  }
  else
  {
    tempe_flag = 1;
  }
 }
//------------------------------------------- check_distance Function------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 void check_distance()
 {    
  if(distance <= 100 && distance >= 40)
  {
    digitalWrite(buzzer_pin,HIGH);
    digitalWrite(led_pin,HIGH);
    if(dist_flag == 1)
    {
      message = "Umujura,please do Something";
      SerialMon.println(message);
      sendSMS(message);
      dist_flag = 0;
    }
  }
  else
  {
    digitalWrite(buzzer_pin,LOW);
    digitalWrite(led_pin,LOW);
    dist_flag = 1;
  }
 }
//-------------------------------------------Get getMotion function------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void check_motion()
{ 
  pirVal = digitalRead(pir_pin);
  //Serial.println(pirVal);
  
  if(pirVal == HIGH)
  {
    digitalWrite(buzzer_pin,HIGH);
    digitalWrite(led_pin,HIGH);
    if(pir_flag == 1)
    {
      message = "Inyoni zirikona, do Something";
      SerialMon.println(message);
      sendSMS(message);
      pir_flag= 0;
    }
  }
  else
  {
    digitalWrite(buzzer_pin,LOW);
    digitalWrite(led_pin,LOW);
    pir_flag = 1;
  }
}
