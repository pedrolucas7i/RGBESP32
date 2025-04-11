#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#define PIN_NEO_PIXEL 23
#define NUM_PIXELS 1

const char *ssid = "MEO-830900";
const char *password = "d0cc9ef64c";
const char *ESP32hostname = "RGBESP32";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
  <meta charset="UTF-8">
  <title>NeoPixel RGB</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background-color: #111;
      color: #fff;
      padding: 40px;
    }

    .neopixel {
      width: 250px;
      height: 250px;
      margin: 30px auto;
      border-radius: 50%;
      border-radius: 100vw;
      background: radial-gradient(circle at center, #000 0%, #222 100%);
      box-shadow: 0 0 30px rgba(0, 255, 0, 0.2);
      transition: background 0.3s, box-shadow 0.3s;
    }

    .inputs {
      display: flex;
      justify-content: center;
      gap: 15px;
      margin-top: 20px;
    }

    input[type=number] {
      width: 70px;
      padding: 5px;
      text-align: center;
      font-size: 16px;
      font-weight: 600;
      border-radius: 8px;
      border: none;
      outline: none;
    }

    button {
      margin-top: 25px;
      padding: 12px 24px;
      font-size: 16px;
      border: none;
      border-radius: 10px;
      cursor: pointer;
      background-color: #333;
      color: white;
      transition: background-color 0.3s;
    }

    button:hover {
      background-color: #555;
    }
  </style>
</head>
<body>

  <h1>Control of NeoPixel RGB</h1>
  <div class="neopixel" id="neopixel"></div>

  <div class="inputs">
    <div>
      <label>R:</label><br>
      <input type="number" id="red" min="0" max="255" value="0">
    </div>
    <div>
      <label>G:</label><br>
      <input type="number" id="green" min="0" max="255" value="0">
    </div>
    <div>
      <label>B:</label><br>
      <input type="number" id="blue" min="0" max="255" value="0">
    </div>
  </div>

  <button onclick="changeColor()">Change Color</button>

  <script>
    function clamp(value) {
      return Math.max(0, Math.min(255, isNaN(value) ? 0 : value));
    }
  
    function changeColor() {
      const r = clamp(parseInt(document.getElementById('red').value));
      const g = clamp(parseInt(document.getElementById('green').value));
      const b = clamp(parseInt(document.getElementById('blue').value));
  
      const neopixel = document.getElementById('neopixel');
      const color = `rgb(${r}, ${g}, ${b})`;
  
      // Atualiza visualmente o neopixel
      neopixel.style.background = `radial-gradient(circle at center, rgba(${r},${g},${b},1) 0%, rgba(${r},${g},${b},0.1) 70%)`;
      neopixel.style.boxShadow = `0 0 40px rgba(${r},${g},${b}, 0.7), 0 0 100px rgba(${r},${g},${b}, 0.3)`;
  
      // Enviar valores para o servidor
      const ip = window.location.hostname;
      const url = `http://${ip}/LEDSTRIP?r=${r}&g=${g}&b=${b}`;
      fetch(url)
        .then(response => {
          if (!response.ok) throw new Error('Erro ao enviar cor para o servidor');
          console.log("Sucess sending color!");
        })
        .catch(err => console.error(err));
    }
  </script>
  

</body>
</html>
)rawliteral";


WebServer server(80);

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);


void LEDSTRIP() {
  char neopixel[3000];
  
  snprintf(neopixel, 3000, index_html, server.arg(0).toInt(), server.arg(1).toInt(), server.arg(2).toInt());
  
  server.send(200, "text/html", neopixel);
  if (server.argName(0) == "r" && server.argName(1) == "g" && server.argName(2) == "b") {
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
  WiFi.setHostname(ESP32hostname);      // Define ESP32 Hostname
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

  server.on("/", LEDSTRIP);

  server.on("/LEDSTRIP", LEDSTRIP);

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
