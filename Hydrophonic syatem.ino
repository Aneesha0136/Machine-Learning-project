#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Ai"; // Your WiFi network SSID
const char* password = "Aneeesha#000"; // Your WiFi network password
const char* server = "http://api.thingspeak.com/update"; // ThingSpeak server URL
const char* api_key = "QZ6WDQYZRABGQH6G"; // ThingSpeak channel write API key

#define DHTPIN 4        // DHT11 connected to GPIO 4
#define DHTTYPE DHT11   // Define the sensor type
#define TDSPIN 34       // TDS meter connected to GPIO 34
#define LDRPIN 2        // LDR sensor connected to GPIO 2

DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT sensor

void setup() {
    Serial.begin(115200);
    dht.begin();  // Start the DHT sensor
    pinMode(TDSPIN, INPUT);
    pinMode(LDRPIN, INPUT);
    
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("Connected!");
}

void loop() {
    // Read temperature and humidity from DHT11
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Read raw TDS value
    int tdsRaw = analogRead(TDSPIN);
    
    // Convert raw TDS reading to a voltage (ESP32 ADC is 12-bit, i.e., 0-4095)
    float voltage = tdsRaw * (3.3 / 4095.0);

    // Convert voltage to TDS value (approximate conversion formula)
    float tdsValue = (133.42 * voltage * voltage * voltage - 255.86 * voltage * voltage + 857.39 * voltage) * 0.5;  

    // Read LDR sensor value
    int ldrValue = analogRead(LDRPIN);  // Read raw LDR value (0-4095)
    float lightIntensity = map(ldrValue, 0, 4095, 0, 100); // Convert to 0-100% scale

    // Print values to Serial Monitor
    Serial.println("===== Sensor Readings =====");
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" °C");

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
    }

    Serial.print("TDS Value: ");
    Serial.print(tdsValue);
    Serial.println(" ppm");

    Serial.print("Light Intensity: ");
    Serial.print(lightIntensity);
    Serial.println(" %");
    
    Serial.println("===========================");

    // Send data to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = server;
        url += "?api_key=" + String(api_key);
        url += "&field1=" + String(temperature);
        url += "&field2=" + String(humidity);
        url += "&field3=" + String(tdsValue);
        url += "&field4=" + String(lightIntensity); // Sending LDR data

        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            Serial.println("Data sent to ThingSpeak!");
        } else {
            Serial.println("Error sending request");
        }

        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }

    delay(30000); // Wait for 30 seconds before sending the next reading
}
