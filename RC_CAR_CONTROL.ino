#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

const char* ssid = "oppok13";
const char* password = "wifi1234";

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// --- Your L298N Pin Mapping ---
const int ENA = 5;  // D1 (Speed Right)
const int ENB = 4;  // D2 (Speed Left)
const int IN1 = 14; // D5
const int IN2 = 12; // D6
const int IN3 = 13; // D7
const int IN4 = 15; // D8

void setup() {
  Serial.begin(115200);

  // Set all motor pins as outputs
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // Default speed (0-1023)
  analogWrite(ENA, 800);
  analogWrite(ENB, 800);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); 

// Web Interface
server.on("/", []() {

  String html =
    "<html>"
    "<head>"
      "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>"

      "<style>"
        "body{"
          "background:#1a1a1a;"
          "color:white;"
          "text-align:center;"
          "font-family:sans-serif;"
          "touch-action:none;"
        "}"

        ".btn{"
          "width:70px;"
          "height:70px;"
          "margin:5px;"
          "border-radius:10px;"
          "border:none;"
          "background:#444;"
          "color:white;"
          "font-weight:bold;"
          "cursor:pointer;"
        "}"

        ".video-box{"
          "width:100%;"
          "max-width:600px;"
          "height:400px;"
          "border:2px solid #555;"
          "margin-bottom:10px;"
          "background:#000;"
        "}"

        ".slider{"
          "width:80%;"
          "margin:20px;"
        "}"
      "</style>"
    "</head>"

    "<body>"

      "<h1>RC CONTROL</h1>"

      // Video Stream Window
      "<iframe class='video-box' src='http://192.168.46.131:8080/video'></iframe>"

      "<div>"
        "<button class='btn' "
          "onmousedown='send(\"F\")' "
          "onmouseup='send(\"S\")'>"
          "UP"
        "</button>"
      "</div>"

      "<div>"
        "<button class='btn' onmousedown='send(\"L\")' onmouseup='send(\"S\")'>LEFT</button>"
        "<button class='btn' onclick='send(\"S\")'>STOP</button>"
        "<button class='btn' onmousedown='send(\"R\")' onmouseup='send(\"S\")'>RIGHT</button>"
      "</div>"

      "<div>"
        "<button class='btn' onmousedown='send(\"B\")' onmouseup='send(\"S\")'>DOWN</button>"
      "</div>"

      "<h3>SPEED</h3>"

      "<input type='range' "
        "min='0' "
        "max='1023' "
        "value='800' "
        "class='slider' "
        "onchange='send(\"SPD:\"+this.value)'>"

      "<script>"

        "var ws = new WebSocket('ws://'+location.hostname+':81/');"

        "function send(m){"
          "ws.send(m);"
        "}"

        "window.addEventListener('keydown', function(e) {"
          "if(e.repeat) return;"
          "if(e.key == 'ArrowUp') send('F');"
          "if(e.key == 'ArrowDown') send('B');"
          "if(e.key == 'ArrowLeft') send('L');"
          "if(e.key == 'ArrowRight') send('R');"
        "});"

        "window.addEventListener('keyup', function(e) {"
          "if(e.key.includes('Arrow')) send('S');"
        "});"

      "</script>"

    "</body>"
    "</html>";

  server.send(200, "text/html", html);
});

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String command = (char*)payload;
    Serial.println("Received: " + command);

    if (command == "F") moveForward();
    else if (command == "B") moveBackward();
    else if (command == "L") turnLeft();
    else if (command == "R") turnRight();
    else if (command == "S") stopMotors();
    else if (command.startsWith("SPD:")) {
      int speed = command.substring(4).toInt();
      analogWrite(ENA, speed);
      analogWrite(ENB, speed);
    }
  }
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
