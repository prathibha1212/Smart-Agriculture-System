#define DHTTYPE DHT22      // Use DHT22 if you're using that model
#define SOIL_MOISTURE_PIN 34
#define RELAY_PIN 26

// WiFi credentials
const char* ssid = "vivo T2x 5G";
const char* password = "prathibha123";

// MQTT broker
const char* mqtt_server = "192.168.101.252"; // Replace with your MQTT broker IP
const int mqtt_port = 1884;
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {
  delay(100);
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("SmartAgriClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Turn OFF pump initially
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1884);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoistureValue);
  
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" °C  Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  String pumpStatus;
  if (soilMoistureValue > 3000) {
    Serial.println("Soil is dry. Turning pump ON.");
    digitalWrite(RELAY_PIN, LOW);
    pumpStatus = "ON";
  } else {
    Serial.println("Soil is wet. Turning pump OFF.");
    digitalWrite(RELAY_PIN, HIGH);
    pumpStatus = "OFF";
  }

  // Publish MQTT data
  client.publish("smartagri/soil", String(soilMoistureValue).c_str());
  client.publish("smartagri/temp", String(temp).c_str());
  client.publish("smartagri/humidity", String(hum).c_str());
  client.publish("smartagri/pump", pumpStatus.c_str());

  delay(5000);
}






