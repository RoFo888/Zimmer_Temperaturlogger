//Thingspeak : rofo90@trash-mail.com PW : Internet1

#define Version 0.26 //Umstellung auf BME680

//#define _DEBUG_
#include <Adafruit_BME680.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#define SEALEVELPRESSURE_HPA (1013.25)
#define SSID "FRITZ!Box 7490-1"
#define SSID_PASSWORD "07023708331231881462"

Adafruit_BME680 bme; // I2C

// ThingSpeak Settings
const int channelID = 1017554;
String writeAPIKey = "UV4I7X3G8KVW0K83"; // write API key for your ThingSpeak Channel
const char* server = "api.thingspeak.com";

WiFiClient client;
unsigned long delayTime;
String API = "SD401ZMAAGU5CO5N";
long previousMillis = 0;
long Twitterinterval = 60 * 60 * 1000;// jede Stunde einen Tweet
bool flag;

void setup() {
  delay(2500);
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.println(F("BME680 test"));

  bool status;
  Wire.begin(D6, D5);
  status = bme.begin();
  if (!status) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP8266-Temperaturlogger");
  WiFi.begin(SSID, SSID_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("Temperaturlogger");
  bme.readTemperature();

  flag = true;
  bme.gas_resistance;  //dummyread
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(200);
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  delay(200);
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  delay(200);
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  delay(200);
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  delay(200);
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
}




void loop() {
  Serial.println(bme.readTemperature());
  Serial.println(String(bme.readPressure() / 100, 3));
  if ((millis() - previousMillis > Twitterinterval) || (flag == true)) {
    previousMillis = millis();
    if (flag == true) {
      flag = false;
      SendTweet("ESP8266 restarts! Ver.:" + String(Version));
    }
    SendTweet("Temperatur :" + String(bme.readTemperature()) + "C    Luftfeuchte :" + String(bme.readHumidity()) +
              "  Luftdruck :" + String(bme.readPressure() / 100, 3) + "mBar" + "  Luftqualität :" + String(bme.gas_resistance / 1000, 1) + "kOhm");
  }
  long zeitalt = millis() + 60000;
  while (millis() < zeitalt)
  {
    ArduinoOTA.handle();
    yield();
  }
  Thinkspeak();
}

void Thinkspeak() {
  if (client.connect(server, 80)) {
    // Construct API request body
    String body = "field1=";
    body += String(bme.readTemperature());
    body += "&field2=";
    body += String(bme.readPressure() / 100, 3);
    body += "&field3=";
    body += String(bme.readHumidity());
    body += "&field4=";
    body += String(Version);
    body += "&field5=";
    body += String(bme.gas_resistance);
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(body.length());
    client.print("\n\n");
    client.print(body);
    client.print("\n\n");
    delay(1000);
    client.stop();
  }
}

void SendTweet(String Tweet) {
  Serial.println(Tweet);//Serielle Ausgabe
  Tweet.replace(" ", "%20");//Leerzeichen für Twitter eingügen
  if (client.connect("184.106.153.149", 80))
  {
    Serial.println("Sende zu Twitter");
    client.print("GET /apps/thingtweet/1/statuses/update?key=" + API + "&status=" + Tweet + " HTTP/1.1\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Accept: */*\r\n");
    client.print("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n");
    client.print("\r\n");
  }
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
}
