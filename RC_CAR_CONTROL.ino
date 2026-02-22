#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

const char* ssid = "oppok13";
const char* password = "wifi1234";

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// --- Your L298N Pin Mapping ---
const int ENA = 5;   // D1 (Speed Right)
const int ENB = 4;   // D2 (Speed Left)
const int IN1 = 14;  // D5
const int IN2 = 12;  // D6
const int IN3 = 13;  // D7
const int IN4 = 15;  // D8

void setup() {
  Serial.begin(115200);

  // Set all motor pins as outputs
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

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
      "margin:0;"
      "padding:0;"
      "overflow:hidden;"
      "}"

      //Make video box fill the whole screen
      ".video-box{"
      "position:fixed;"
      "top:0; left:0;"
      "width:100vw;"
      "height:100vh;"
      "border:none;"
      "background:#000;"
      "z-index:0;"
      "}"

      // Heading overlay at top center
      ".heading{"
      "position:fixed;"
      "top:15px;"
      "width:100%;"
      "text-align:center;"
      "z-index:10;"
      "font-size:32px;"
      "font-weight:bold;"
      "color:white;"
      "text-shadow:0 0 8px #000;"
      "}"

      // Buttons overlay at bottom right
      ".controls{"
      "position:fixed;"
      "bottom:80px;"
      "right:20px;"
      "z-index:10;"
      "text-align:center;"
      "}"

      // Slider overlay at bottom center
      ".speed-box{"
      "position:fixed;"
      "bottom:25px;"
      "left:50%;"
      "transform:translateX(-50%);"
      "z-index:10;"
      "text-align:center;"
      "color:white;"
      "text-shadow:0 0 6px #000;"
      "}" 

      //button style
      ".btn{"
      "width:85px;"
      "height:85px;"
      "margin:4px;"
      "border-radius:10px;"
      "border:none;"
      "background:rgba(255,255,255,0.15);"  // semi-transparent
      "color:white;"
      "font-weight:bold;"
      "cursor:pointer;"
      "backdrop-filter:blur(4px);"
      "font-size:25px;"
      "}"

      ".bottom-overlay{"
        "position:fixed;"
        "bottom:0;"
        "left:0;"
        "width:100%;"
        "height:220px;"
        "background:linear-gradient(to top, rgba(50,50,50,1.0), transparent);"
        "z-index:5;"
        "pointer-events:none;"
      "}"

      ".slider{"
      "width:400px;"
      "}"
      "</style>"
      "</head>"

      "<body>"

        "<img class='video-box' src='http://192.168.46.131:8080/video'>"  // use img
        "<div class='bottom-overlay'></div>"
        "<div class='heading'>RC CAR CONTROLLER</div>"
        "<div class='controls'>"
          "<div>"
            "<button class='btn' onmousedown='send(\"F\")' onmouseup='send(\"S\")'>F</button>"
          "</div>"
          "<div>"
            "<button class='btn' onmousedown='send(\"L\")' onmouseup='send(\"S\")'>L</button>"
            "<button class='btn' onclick='send(\"S\")'>S</button>"
            "<button class='btn' onmousedown='send(\"R\")' onmouseup='send(\"S\")'>R</button>"
          "</div>"
          "<div>"
            "<button class='btn' onmousedown='send(\"B\")' onmouseup='send(\"S\")'>B</button>"
          "</div>"
        "</div>"

        "<div class='speed-box'>"
          "<label>SPEED: <span id='spd'>800</span></label><br>"
          "<input type='range' min='0' max='1023' value='800' class='slider' "
            "oninput='document.getElementById(\"spd\").innerText=this.value' "
            "onchange='send(\"SPD:\"+this.value)'>"
        "</div>"
      "<script>"

      "var ws = new WebSocket('ws://'+location.hostname+':81/');"
      "ws.onclose = function() {"
        "setTimeout(function() {"
          "ws = new WebSocket('ws://'+location.hostname+':81/');"
        "}, 2000);"
      "};"

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

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
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
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void moveBackward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
