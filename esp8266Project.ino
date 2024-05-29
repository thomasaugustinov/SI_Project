#include <Wire.h>
#include <ESP8266WiFi.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";

WiFiServer server(80);
String data = "No data received";

void receiveEvent(int howMany) {
  data = "";
  while (Wire.available()) {
    char c = Wire.read();
    data += c;
  }
  Serial.print("Received data: ");
  Serial.println(data);
}

void setup() {
  Serial.begin(115200); // Start the serial communication
  Wire.begin(8); // Join I2C bus with address #8
  Wire.onReceive(receiveEvent); // Register receive event

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<html><body><h1>Environmental Sensor Data</h1>");
            client.println("<pre>" + data + "</pre>");
            client.println("</body></html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }
}