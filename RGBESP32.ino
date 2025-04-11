#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#define PIN_NEO_PIXEL 23
#define NUM_PIXELS 1

const char *ssid = "MEO-830900";
const char *password = "d0cc9ef64c";

WebServer server(80);

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);


void handleRoot() {
  server.send(200, "text/plain", "hello from esp32!");
  if (server.argName(0) == "led" && server.argName(1) == "led" && server.argName(2) == "led") {
    NeoPixel.clear();
    NeoPixel.show();
    delay(1000);
    NeoPixel.setPixelColor(0, NeoPixel.Color(server.arg(0).toInt(), server.arg(1).toInt(), server.arg(2).toInt()));
    NeoPixel.show();
    delay(1000);
  }
}

void handleNotFound() {
  String message = "Page Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
  NeoPixel.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(2);  //allow the cpu to switch to other tasks
}
