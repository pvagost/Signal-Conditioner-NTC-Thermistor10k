#include <ESP8266WiFi.h>                      // Include the ESP8266WiFi library for WiFi functionality
#include <ESP8266HTTPClient.h>                // Include the ESP8266HTTPClient library for making HTTP requests
#include <WiFiClientSecure.h>                 // Include the WiFiClientSecure library for HTTPS connections
// WiFi Credentials
const char* ssid = "Example";           // Set the WiFi network name (SSID) to connect to
const char* password = "12345678";            // Set the WiFi password for authentication
// Google Apps Script Web App URL
const char* googleScriptURL = "https://....";  // URL endpoint where data will be sent
// Constants for Voltage-to-Temperature Conversion
#define VCC 3.3  // ESP8266 ADC reference voltage
#define T_MIN -55  // Temperature at 0V
#define T_MAX 125    // Temperature at 3.3V
void setup() {
  Serial.begin(115200);                       // Initialize serial communication at 115200 baud rate
  WiFi.begin(ssid, password);                 // Connect to WiFi network using the provided credentials
  Serial.print("Connecting to WiFi");         // Print message to serial monitor
  int timeout = 30;                           // Set a 30-second timeout for WiFi connection
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {  // Loop until connected or timeout
    delay(1000);                              // Wait 1 second between connection attempts
    Serial.print(".");                        // Print a dot to show connection attempt
    timeout--;                                // Decrement the timeout counter
  }
  if (WiFi.status() == WL_CONNECTED) {        // Check if successfully connected to WiFi
    Serial.println("\nConnected to WiFi!");   // Print success message to serial monitor
  } else {
    Serial.println("\nFailed to connect. Restarting...");  // Print failure message to serial monitor
    ESP.restart();                            // Restart the ESP8266 if WiFi connection failed
  }
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {        // Check if WiFi is still connected
    WiFiClientSecure client;                  // Create a secure WiFi client for HTTPS
    client.setInsecure();                     // Disable certificate verification (not secure but simpler)
    HTTPClient http;                          // Create an HTTP client object
    http.begin(client, googleScriptURL);      // Initialize the HTTP connection to the Google Script
    http.addHeader("Content-Type", "application/json");  // Set the content type header to JSON
    // Read voltage from op-amp output (A0 pin)
    int raw_data = analogRead(A0);            // Read analog value from A0 pin (0-1023)
    float voltage = (raw_data / 1023.0) * VCC;  // Convert ADC reading to voltage
    // Convert voltage to temperature
    float temperature = (voltage * (T_MAX - T_MIN)/VCC) + T_MIN; // Apply linear conversion formula
    // Assign temperature to sensor_value 
    float sensor_value = temperature;         // Store temperature in sensor_value variable
    // DEBUG: Print values in Serial Monitor
    Serial.print("Raw ADC: "); Serial.println(raw_data);  // Print raw ADC value (0-1023)
    Serial.print("Voltage: "); Serial.println(voltage, 2);  // Print voltage with 2 decimal places
    Serial.print("Temperature (°C): "); Serial.println(sensor_value, 2);  // Print temperature with 2 decimal places
    // Create JSON payload
    String jsonData = "{\"raw_data\":\"" + String(raw_data) + 
                      "\",\"sensor_value\":\"" + String(sensor_value, 2) + "\"}";  // Format data as JSON string
    Serial.print("Sending JSON: "); Serial.println(jsonData);  // Print the JSON payload to serial monitor
    // Send HTTP POST request
    int httpResponseCode = http.POST(jsonData);  // Send the JSON data to the server and get response code
    // Print response
    if (httpResponseCode > 0) {               // Check if the HTTP request was successful
      Serial.println("Data sent successfully!");  // Print success message
    } else {
      Serial.println("Error: " + String(httpResponseCode));  // Print error code if request failed
    }
    http.end();                               // Close the HTTP connection
  } else {
    Serial.println("WiFi disconnected!");     // Print message if WiFi is disconnected
  }
  delay(5000); // Wait 5 seconds before next reading
}