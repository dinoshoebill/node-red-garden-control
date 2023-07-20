#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// WiFi connection settings
const char* SSID = "<WiFI_SSID>";
const char* PASSWORD = "<WiFi_PASSWORD>";

// MQTT connection settings
const char* MQTT_USERNAME = "<MQTT_USERNAME>";
const char* MQTT_PASSWORD = "<MQTT_PASSWORD>";

const char* MQTT_SERVER = "<MQTT_SERVER_IP>";

WiFiClient esp32Client;
PubSubClient client(esp32Client);

// SCL22, SDA21
// 27, 26, 25 LED
#define TEMP_LED GPIO_NUM_27 // esp32 API notation for pinMode()
#define AIR_HUM_LED GPIO_NUM_26
#define SOIL_HUM_LED GPIO_NUM_25

// 32 SOIL, 35 LIGHT, 33 DHT11, I2C BME280, I2C 16x02 LCD

// treshold values
#define CONNECTION_ATTEMPT_TRESHOLD 60000       // time before esp32 tries to connect if disconnected
#define READ_VALUE_TRESHOLD 5000                // interval between each value reading and mqtt publishing
#define STOP_CONNECTION_ATTEMPT_TRESHOLD 10000  // how long until connection attempt is aborted

// DHT11 sensor settings
#define DHT_SENSOR_PIN 33  // GPIO signal pin
#define DHT_TYPE DHT11     // DHT sensory type
DHT dht(DHT_SENSOR_PIN, DHT_TYPE);

// soil humidity sensor
#define SOIL_SENSOR_PIN 32
#define AIR_VALUE 2570 // air calibration value
#define WATER_VALUE 970 // soil calibration value

// light sensor
#define LIGHT_SENSOR_PIN 35

// I2C BMP280 sensor
Adafruit_BMP280 bme;

// 16x02 LCD display
#define LCD_COLUMNS 16
#define LCD_ROWS 2
LiquidCrystal_I2C lcd(0x27, LCD_COLUMNS, LCD_ROWS);

// reading values
float temperature = 0;
float airHumidity = 0;
float soilHumidity = 0;
int soilHumidityValue = 0;
float light = 0;

// treshold default values
float tempOptimal = 22;
float tempRadius = 1;
float airHumOptimal = 50;
float airHumRadius = 5;
float soilHumOptimal = 50;
float soilHumRadius = 5;

// flow control variables
unsigned long sinceLastReading = 0;
unsigned long sinceLastConnectionAttemptWifi = 0;
unsigned long sinceLastConnectionAttemptMQTT = 0;

// attempt WiFi connection
void connect_to_wifi() {

  // attempt connection only if enough time has passed
  if (sinceLastConnectionAttemptWifi != 0) {
    if (millis() - sinceLastConnectionAttemptWifi < CONNECTION_ATTEMPT_TRESHOLD) {
      return;
    }
  }

  sinceLastConnectionAttemptWifi = millis();

  Serial.print("WiFi");

  long started = millis();

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - started > STOP_CONNECTION_ATTEMPT_TRESHOLD) {
      Serial.println(" Connection timeout");
      return;
    }
  }

  Serial.println(" Connected");
}

// attempt MQTT connection
void connect_to_mqtt() {

  // attempt connection only if enough time has passed
  if (sinceLastConnectionAttemptMQTT != 0) {
    if (millis() - sinceLastConnectionAttemptMQTT < CONNECTION_ATTEMPT_TRESHOLD) {
      return;
    }
  }

  sinceLastConnectionAttemptMQTT = millis();

  Serial.print("MQTT");

  long started = millis();

  while (!client.connected()) {
    if (client.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD)) {
      
      // şubscribe to topics
      client.subscribe("esp32/tempRadius");
      client.subscribe("esp32/tempOptimal");
      client.subscribe("esp32/airHumOptimal");
      client.subscribe("esp32/airHumRadius");
      client.subscribe("esp32/soilHumOptimal");
      client.subscribe("esp32/soilHumRadius");
      client.subscribe("esp32/readVariables");
      client.subscribe("esp32/syncToDevice");
      client.subscribe("esp32/challenge");
      client.subscribe("esp32/status");
    }

    if (millis() - started > STOP_CONNECTION_ATTEMPT_TRESHOLD) {
      Serial.println(" Connection timeout");
      return;
    }
  }

  Serial.println(" Connected");
  client.publish("esp32/forceResponse", "true");
  sendVariables(false);
}

void sendVariables(bool sync) {

  // put all variables in array
  float variables[6] = {
    tempOptimal,
    tempRadius,
    airHumOptimal,
    airHumRadius,
    soilHumOptimal,
    soilHumRadius,
  };

  // array for output message
  char values[100];

  // put array variables to output message
  int len = 0;
  for (int i = 0; i < 6; i++) {
    char str[15];

    sprintf(str, "%0.0f", variables[i]);
    strcpy(values + len, str);

    len += strlen(str);
    values[len] = ' ';
    len++;
  }

  // terminate output message
  values[len] = '\0';

  // publish values
  if (sync) {
    client.publish("esp32/syncDevice", values);
  } else {
    client.publish("esp32/deviceVariables", values);
  }
}

// function is called upon receiving MQTT message
void callback(char* topic, byte* message, unsigned int length) {

  // array to save input message
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }

  msg.setCharAt(length, '\0');

  // do something for specific topic recieved
  if (String(topic) == "esp32/tempOptimal") {
    tempOptimal = msg.toFloat();
    sendVariables(false);

  } else if (String(topic) == "esp32/tempRadius") {
    tempRadius = msg.toFloat();
    sendVariables(false);

  } else if (String(topic) == "esp32/airHumOptimal") {
    airHumOptimal = msg.toFloat();
    sendVariables(false);

  } else if (String(topic) == "esp32/airHumRadius") {
    airHumRadius = msg.toFloat();
    sendVariables(false);

  } else if (String(topic) == "esp32/soilHumOptimal") {
    soilHumOptimal = msg.toFloat();
    sendVariables(false);

  } else if (String(topic) == "esp32/soilHumRadius") {
    soilHumRadius = msg.toFloat();
    sendVariables(false);

  } else if (String(topic) == "esp32/readVariables") {
    sendVariables(false);

  } else if (String(topic) == "esp32/syncToDevice") {
    sendVariables(true);

  } else if (String(topic) == "esp32/challenge") {
    client.publish("esp32/response", "true");

  } else if (String(topic) == "esp32/status") {
    client.publish("esp32/online", "true");
  }
}

void work() {
  if (isnan(temperature) || (temperature > tempOptimal + tempRadius)) {
    gpio_set_level(TEMP_LED, LOW);  // turn off
  } else if (temperature < tempOptimal - tempRadius) {
    gpio_set_level(TEMP_LED, HIGH); // turn on
  }

  if (isnan(airHumidity) || (airHumidity > airHumOptimal + airHumRadius)) {
    gpio_set_level(AIR_HUM_LED, LOW); // turn off
  } else if (airHumidity < airHumOptimal - airHumRadius) {
    gpio_set_level(AIR_HUM_LED, HIGH);  // turn on
  }

  if (isnan(soilHumidity) || (soilHumidity > soilHumOptimal + soilHumRadius)) {
    gpio_set_level(SOIL_HUM_LED, LOW);  // turn off
  } else if (soilHumidity < soilHumOptimal - soilHumRadius) {
    gpio_set_level(SOIL_HUM_LED, HIGH); // turn on
  }
}

void connectionStatus() {
  // connect to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    // connect to MQTT
    if (client.connected()) {
      client.loop();
    } else {
      connect_to_mqtt();
    }
  } else {
    connect_to_wifi();
  }
}

void display_values() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(String("T:") + String((int) temperature) + String("C ")+ String("A:") + String((int) airHumidity) + String("%"));
  lcd.setCursor(0, 1);
  lcd.print(String("S:") + String((int) soilHumidity) + String("% ")+ String("L:") + String((int) light));
}

void read_values() {

  // Convert the value to a char array
  char tempString[10];
  temperature = bme.readTemperature();
  dtostrf(temperature, 3, 0, tempString);
  client.publish("esp32/temperature", tempString);

  char airHumString[10];
  airHumidity = dht.readHumidity();
  dtostrf(airHumidity, 3, 0, airHumString);
  client.publish("esp32/airHumidity", airHumString);

  char soilHumString[10];
  soilHumidityValue = analogRead(SOIL_SENSOR_PIN);
  soilHumidity = map(soilHumidityValue, AIR_VALUE, WATER_VALUE, 0, 100);

  if (soilHumidity < 0) {
    soilHumidity = 0;
  } else if (soilHumidity > 100) {
    soilHumidity = 100;
  }

  dtostrf(soilHumidity, 3, 0, soilHumString);
  client.publish("esp32/soilHumidity", soilHumString);

  char lightString[10];
  light = analogRead(LIGHT_SENSOR_PIN);
  dtostrf(light, 4, 0, lightString);
  client.publish("esp32/light", lightString);
}

void setup() {
  Serial.begin(115200); // begin serial monitor conenction

  // set pin modes
  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  
  // LEDs
  gpio_set_direction(TEMP_LED, GPIO_MODE_OUTPUT);
  gpio_set_direction(AIR_HUM_LED, GPIO_MODE_OUTPUT);
  gpio_set_direction(SOIL_HUM_LED, GPIO_MODE_OUTPUT);
  bme.begin();
  dht.begin();

  // 1602 LCD initialization
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // set client server and callback funtion
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);

  // try connecting before anything
  connect_to_wifi();
}

void loop() {
  // connect to WiFi
  connectionStatus();

  // read values if enough time has passed
  if (millis() - sinceLastReading > READ_VALUE_TRESHOLD) {
    sinceLastReading = millis();
    read_values();
    work(); // do work
    display_values();
  }
}