#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// Credenciales de la red Wi-Fi que creará el Dron
const char* ssid = "NERV-Protocol";
const char* password = "sonotakus"; // Mínimo 8 caracteres

// Servidor Web en el puerto 80 y WebSockets en el puerto 81
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Definición de pines para los LEDs (Motores simulados)
const int motor1 = 2; // D2 en Nano ESP32
const int motor2 = 3; // D3 en Nano ESP32
const int motor3 = 4; // D4 en Nano ESP32
const int motor4 = 5; // D5 en Nano ESP32

// Interfaz Web incrustada en la memoria del ESP32
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>GCS Dron ESP32</title>
    <style>
        body { margin: 0; padding: 0; background-color: #222; color: white; font-family: Arial, sans-serif; display: flex; flex-direction: column; height: 100vh; overflow: hidden; }
        #header { text-align: center; padding: 10px; background: #333; }
        #joystick-container { display: flex; justify-content: space-between; align-items: center; flex: 1; padding: 20px; }
        .joystick-zone { width: 150px; height: 150px; background: rgba(255, 255, 255, 0.1); border-radius: 50%; position: relative; touch-action: none; border: 2px solid #555; }
        .knob { width: 50px; height: 50px; background: #00ff88; border-radius: 50%; position: absolute; top: 50px; left: 50px; pointer-events: none; }
        #knob-left { background: #ff4444; } 
    </style>
</head>
<body>
    <div id="header">
        Estado: <span id="status" style="color: red;">Desconectado</span> | 
        Datos: <span id="data-out">T:0 Y:0 P:0 R:0</span>
    </div>
    <div id="joystick-container">
        <div class="joystick-zone" id="zone-left"><div class="knob" id="knob-left"></div></div>
        <div class="joystick-zone" id="zone-right"><div class="knob" id="knob-right"></div></div>
    </div>

    <script>
        let controlData = { T: 0, Y: 0, P: 0, R: 0 };
        // Conectar al WebSocket en el puerto 81 de la IP del ESP32
        let gateway = `ws://${window.location.hostname}:81/`;
        let websocket = new WebSocket(gateway);

        websocket.onopen = () => { document.getElementById('status').style.color = "lime"; document.getElementById('status').innerText = "Conectado"; };
        websocket.onclose = () => { document.getElementById('status').style.color = "red"; document.getElementById('status').innerText = "Desconectado"; };

        function setupJoystick(zoneId, knobId, isLeft) {
            const zone = document.getElementById(zoneId);
            const knob = document.getElementById(knobId);
            const center = 75;
            
            zone.addEventListener('touchmove', function(e) {
                e.preventDefault();
                let touch = e.targetTouches[0];
                let rect = zone.getBoundingClientRect();
                let x = touch.clientX - rect.left - center;
                let y = touch.clientY - rect.top - center;

                let distance = Math.sqrt(x*x + y*y);
                if (distance > center) { x = x * (center / distance); y = y * (center / distance); }

                knob.style.transform = `translate(${x}px, ${y}px)`;
                let valX = Math.round((x / center) * 100);
                let valY = Math.round((-y / center) * 100);

                if (isLeft) { controlData.Y = valX; controlData.T = valY; } 
                else { controlData.R = valX; controlData.P = valY; }
            });

            zone.addEventListener('touchend', function(e) {
                knob.style.transform = `translate(0px, 0px)`;
                if(isLeft) { controlData.Y = 0; controlData.T = 0; } 
                else { controlData.R = 0; controlData.P = 0; }
            });
        }

        setupJoystick('zone-left', 'knob-left', true);
        setupJoystick('zone-right', 'knob-right', false);

        setInterval(() => {
            let payload = `T:${controlData.T} Y:${controlData.Y} P:${controlData.P} R:${controlData.R}`;
            document.getElementById('data-out').innerText = payload;
            if (websocket.readyState === WebSocket.OPEN) {
                websocket.send(payload);
            }
        }, 50);
    </script>
</body>
</html>
)rawliteral";

// Función que maneja los eventos de WebSocket entrantes
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    int t = 0, y = 0, p = 0, r = 0;
    
    // Parseamos el string que viene de la App (Ej: "T:50 Y:0 P:-10 R:0")
    if (sscanf((const char *)payload, "T:%d Y:%d P:%d R:%d", &t, &y, &p, &r) == 4) {
      
      // Imprimimos en el Monitor Serie para depurar
      Serial.printf("Throttle: %d | Yaw: %d | Pitch: %d | Roll: %d\n", t, y, p, r);

      // LÓGICA DE CONTROL BÁSICA (Solo Throttle por ahora para la prueba)
      // Mapeamos el valor de Throttle (0 a 100) a PWM (0 a 255)
      // Ignoramos valores negativos de Throttle por seguridad
      int pwmValue = 0;
      if (t > 0) {
        pwmValue = map(t, 0, 100, 0, 255);
      }

      // Enviamos el PWM a los 4 motores (LEDs)
      analogWrite(motor1, pwmValue);
      analogWrite(motor2, pwmValue);
      analogWrite(motor3, pwmValue);
      analogWrite(motor4, pwmValue);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Configurar pines de motores como salida
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(motor3, OUTPUT);
  pinMode(motor4, OUTPUT);

  // Iniciar Modo Access Point (El dron crea la red)
  Serial.println("Iniciando Dron AP...");
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Red creada. Conéctate a 'Dron_ESP32'. IP de la GCS: ");
  Serial.println(IP);

  // Configurar el servidor web para entregar la interfaz HTML
  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });
  server.begin();

  // Iniciar servidor WebSockets
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  // Mantener los servicios vivos
  server.handleClient();
  webSocket.loop();
}