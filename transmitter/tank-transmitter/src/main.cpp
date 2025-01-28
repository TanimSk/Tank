#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <RF24.h>

// Wi-Fi credentials
const char *ssid = "ghost-AP";
const char *password = "12345678";

// WebSocket server on port 81
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

// NRF24L01 pins and configuration
#define CE_PIN D4
#define CSN_PIN D2

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "12345";

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
    webSocket.sendTXT(num, "Welcome to ghost-AP WebSocket!");
  }
  break;
  case WStype_TEXT:
    Serial.printf("[%u] Received: %s\n", num, payload);

    // Send the payload via nRF24L01
    bool success = radio.write(payload, length);
    if (success)
    {
      Serial.println("Message sent to nRF24L01 successfully.");
      webSocket.sendTXT(num, "Message delivered to nRF24L01.");
    }
    else
    {
      Serial.println("Failed to send message to nRF24L01.");
      webSocket.sendTXT(num, "Failed to deliver message to nRF24L01.");
    }
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  // Initialize nRF24L01
  if (!radio.begin())
  {
    Serial.println("nRF24L01 initialization failed!");
    while (true)
      ; // Stop execution
  }
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();

  // Start Wi-Fi in AP mode
  WiFi.softAP(ssid, password);
  Serial.println("Wi-Fi hotspot started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Start HTTP server
  server.on("/", []()
            { server.send(200, "text/plain", "WebSocket server is running!"); });
  server.begin();

  Serial.println("WebSocket and HTTP servers are running.");
}

void loop()
{
  webSocket.loop();      // Handle WebSocket communication
  server.handleClient(); // Handle HTTP server requests
}