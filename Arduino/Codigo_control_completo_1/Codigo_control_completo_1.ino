#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// Credenciales de la red Wi-Fi que creará el Dron
const char* ssid = "NERV-Protocol";
const char* password = "sonotakus"; 

// Servidor Web en el puerto 80 y WebSockets en el puerto 81
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// --- ASIGNACIÓN ESTÁNDAR BETAFLIGHT (QUAD X) ---
const int motor1 = 2; // Trasero Derecho (CW)
const int motor2 = 3; // Frontal Derecho (CCW)
const int motor3 = 4; // Trasero Izquierdo (CCW)
const int motor4 = 5; // Frontal Izquierdo (CW)

// --- VARIABLES DE LA MÁQUINA DE ESTADOS ---
enum DroneState { SUELO, DESPEGANDO, VOLANDO, ATERRIZANDO };
DroneState estadoDron = SUELO;

unsigned long tiempoAnterior = 0;
int pwmBase = 0;                 // Potencia base para mantener el dron flotando
const int PWM_HOVER = 150;       // Valor simulado necesario para flotar a 1 metro
const int VELOCIDAD_RAMPA = 2;   // Qué tan rápido sube/baja en modo automático

// Variables globales para leer los Joysticks (-100 a 100)
int throttleManual = 0; 
int yawManual = 0;
int pitchManual = 0;
int rollManual = 0;

// Interfaz Web (html) incrustada en la memoria del ESP32
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
        #joystick-container { display: flex; justify-content: space-between; align-items: center; flex: 1; padding: 10px 30px; }
        .joystick-zone { width: 150px; height: 150px; background: rgba(255, 255, 255, 0.1); border-radius: 50%; position: relative; touch-action: none; border: 2px solid #555; }
        .knob { width: 50px; height: 50px; background: #00ff88; border-radius: 50%; position: absolute; top: 50px; left: 50px; pointer-events: none; }
        #knob-left { background: #ff4444; } 
        
        /* Estilos del botón central (Flechas removidas) */
        #center-control { display: flex; flex-direction: column; align-items: center; justify-content: center; }
        .btn-auto { width: 80px; height: 80px; border-radius: 50%; background: #007bff; border: 2px solid #0056b3; cursor: pointer; box-shadow: 0 4px 10px rgba(0,0,0,0.5); }
        .btn-auto:active { background: #0056b3; transform: scale(0.95); }
        .btn-label { margin-top: 10px; font-size: 14px; font-weight: bold; text-align: center; color: #ccc; }
    </style>
</head>
<body>
    <div id="header">
        Estado: <span id="status" style="color: red;">Desconectado</span> | 
        Datos: <span id="data-out">T:0 Y:0 P:0 R:0</span>
    </div>
    
    <div id="joystick-container">
        <!-- Joystick Izquierdo (Acelerador y Guiñada) -->
        <div class="joystick-zone" id="zone-left"><div class="knob" id="knob-left"></div></div>
        
        <!-- Botón Central de Despegue/Aterrizaje -->
        <div id="center-control">
            <button class="btn-auto" id="btn-flight"></button>
            <div class="btn-label">Despegue/<br>Aterrizaje</div>
        </div>

        <!-- Joystick Derecho (Cabeceo y Alabeo) -->
        <div class="joystick-zone" id="zone-right"><div class="knob" id="knob-right"></div></div>
    </div>

    <script>
        let controlData = { T: 0, Y: 0, P: 0, R: 0 };
        let gateway = `ws://${window.location.hostname}:81/`;
        let websocket = new WebSocket(gateway);

        websocket.onopen = () => { document.getElementById('status').style.color = "lime"; document.getElementById('status').innerText = "Conectado"; };
        websocket.onclose = () => { document.getElementById('status').style.color = "red"; document.getElementById('status').innerText = "Desconectado"; };

        // Evento del botón de despegue/aterrizaje
        document.getElementById('btn-flight').addEventListener('touchstart', function(e) {
            e.preventDefault(); 
            if (websocket.readyState === WebSocket.OPEN) { websocket.send("CMD:FLIGHT"); }
        });
        document.getElementById('btn-flight').addEventListener('click', function(e) {
            if (websocket.readyState === WebSocket.OPEN) { websocket.send("CMD:FLIGHT"); }
        });

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
    
    // 1. EVALUAR COMANDO DEL BOTÓN
    if (strncmp((const char *)payload, "CMD:FLIGHT", 10) == 0) {
      if (estadoDron == SUELO) {
        estadoDron = DESPEGANDO;
        Serial.println("Comando: INICIANDO DESPEGUE...");
      } 
      else if (estadoDron == VOLANDO || estadoDron == DESPEGANDO) {
        estadoDron = ATERRIZANDO;
        Serial.println("Comando: INICIANDO ATERRIZAJE...");
      }
      return; 
    }

    // 2. ACTUALIZAR VARIABLES DE JOYSTICK
    int t = 0, y = 0, p = 0, r = 0;
    if (sscanf((const char *)payload, "T:%d Y:%d P:%d R:%d", &t, &y, &p, &r) == 4) {
      throttleManual = t; 
      yawManual = y;
      pitchManual = p;
      rollManual = r;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(motor3, OUTPUT);
  pinMode(motor4, OUTPUT);

  Serial.println("Iniciando Dron AP...");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Red creada. Conéctate a 'NERV-Protocol'. IP: ");
  Serial.println(IP);

  server.on("/", []() { server.send(200, "text/html", index_html); });
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();

  unsigned long tiempoActual = millis();
  
  // Ciclo de control a ~33Hz (cada 30ms)
  if (tiempoActual - tiempoAnterior >= 30) {
    tiempoAnterior = tiempoActual;

    // --- MÁQUINA DE ESTADOS AUTÓNOMA ---
    if (estadoDron == DESPEGANDO) {
      pwmBase += VELOCIDAD_RAMPA;
      if (pwmBase >= PWM_HOVER) {
        pwmBase = PWM_HOVER;
        estadoDron = VOLANDO;
        Serial.println("ESTABILIZADO.");
      }
    } 
    else if (estadoDron == ATERRIZANDO) {
      pwmBase -= VELOCIDAD_RAMPA;
      if (pwmBase <= 0) {
        pwmBase = 0;
        estadoDron = SUELO;
        Serial.println("EN TIERRA.");
      }
    }

    // --- MEZCLA BETAFLIGHT (MOTOR MIXING) ---
    int pwmFinalM1 = 0, pwmFinalM2 = 0, pwmFinalM3 = 0, pwmFinalM4 = 0;

    // Solo mezclamos si el dron no está apagado en el suelo
    if (estadoDron != SUELO) {
      
      // Mapeamos los valores del joystick (-100 a 100) a un impacto real en PWM.
      // Limitamos la fuerza máxima que los joysticks pueden inyectar para no saturar los motores.
      int t_pwm = map(throttleManual, -100, 100, -40, 40); // Sube o baja respecto al HOVER
      int y_pwm = map(yawManual, -100, 100, -30, 30);      // Fuerza de rotación
      int p_pwm = map(pitchManual, -100, 100, -40, 40);    // Fuerza para ir adelante/atrás
      int r_pwm = map(rollManual, -100, 100, -40, 40);     // Fuerza para ir izq/der

      // El acelerador base es el punto de flotación automático más lo que dictamine el piloto
      int base = pwmBase + t_pwm;

      // Ecuaciones cinemáticas de Betaflight para Configuración Quad X
      pwmFinalM1 = base + p_pwm - r_pwm - y_pwm; // Trasero Derecho
      pwmFinalM2 = base - p_pwm - r_pwm + y_pwm; // Frontal Derecho
      pwmFinalM3 = base + p_pwm + r_pwm + y_pwm; // Trasero Izquierdo
      pwmFinalM4 = base - p_pwm + r_pwm - y_pwm; // Frontal Izquierdo

      // Límite estricto de seguridad: El ESP32 solo puede mandar de 0 a 255 por PWM
      pwmFinalM1 = constrain(pwmFinalM1, 0, 255);
      pwmFinalM2 = constrain(pwmFinalM2, 0, 255);
      pwmFinalM3 = constrain(pwmFinalM3, 0, 255);
      pwmFinalM4 = constrain(pwmFinalM4, 0, 255);
    }

    // Ejecutar orden en los motores
    analogWrite(motor1, pwmFinalM1);
    analogWrite(motor2, pwmFinalM2);
    analogWrite(motor3, pwmFinalM3);
    analogWrite(motor4, pwmFinalM4);
  }
}