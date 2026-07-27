#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <ESP32Servo.h>

// --- CREDENCIALES WI-FI ---
const char* ssid = "NERV-Protocol";
const char* password = "sonotakus"; 

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// --- ASIGNACIÓN DE MOTORES (QUAD X) ---
// Nota: Dejé tus pines originales (2, 3, 4, 5). Si notas que los motores 2, 3 y 4 
// siguen sin funcionar por conflicto de hardware, recuerda cambiarlos a 13, 12, 14, 27.
const int pinMotor1 = 2; // Trasero Derecho (CW)
const int pinMotor2 = 3; // Frontal Derecho (CCW)
const int pinMotor3 = 4; // Trasero Izquierdo (CCW)
const int pinMotor4 = 5; // Frontal Izquierdo (CW)

Servo esc1, esc2, esc3, esc4;

// Valores en microsegundos para los ESC (1000 = Apagado, 2000 = Máximo)
const int MIN_PULSE = 1000;
const int MAX_PULSE = 2000;
const int PWM_GIRO = 2000; // Velocidad de prueba para la secuencia (giro suave)

// --- VARIABLES DE LA SECUENCIA DE ARRANQUE ---
bool secuenciaActiva = false;
int pasoSecuencia = 0; 
unsigned long tiempoSecuencia = 0;

// --- INTERFAZ WEB ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>GCS Dron ESP32</title>
    <style>
        body { margin: 0; padding: 0; background-color: #222; color: white; font-family: Arial, sans-serif; display: flex; flex-direction: column; height: 100vh; overflow: hidden; }
        #header { text-align: center; padding: 10px; background: #333; font-size: 14px; }
        #joystick-container { display: flex; justify-content: space-between; align-items: center; flex: 1; padding: 10px 30px; }
        .joystick-zone { width: 150px; height: 150px; background: rgba(255, 255, 255, 0.1); border-radius: 50%; position: relative; touch-action: none; border: 2px solid #555; }
        .knob { width: 50px; height: 50px; background: #00ff88; border-radius: 50%; position: absolute; top: 50px; left: 50px; pointer-events: none; }
        #knob-left { background: #ff4444; } 
        #center-control { display: flex; flex-direction: column; align-items: center; justify-content: center; }
        .btn-auto { width: 80px; height: 80px; border-radius: 50%; background: #007bff; border: 2px solid #0056b3; cursor: pointer; box-shadow: 0 4px 10px rgba(0,0,0,0.5); }
        .btn-auto:active { background: #0056b3; transform: scale(0.95); }
        .btn-label { margin-top: 10px; font-size: 14px; font-weight: bold; text-align: center; color: #ccc; }
    </style>
</head>
<body>
    <div id="header">
        <span id="status" style="color: red;">Desconectado</span>
    </div>
    <div id="joystick-container">
        <div class="joystick-zone" id="zone-left"><div class="knob" id="knob-left"></div></div>
        <div id="center-control">
            <button class="btn-auto" id="btn-flight"></button>
            <div class="btn-label">Secuencia<br>On / Off</div>
        </div>
        <div class="joystick-zone" id="zone-right"><div class="knob" id="knob-right"></div></div>
    </div>
    <script>
        let gateway = `ws://${window.location.hostname}:81/`;
        let websocket = new WebSocket(gateway);

        websocket.onopen = () => { document.getElementById('status').style.color = "lime"; document.getElementById('status').innerText = "Conectado"; };
        websocket.onclose = () => { document.getElementById('status').style.color = "red"; document.getElementById('status').innerText = "Desconectado"; };

        document.getElementById('btn-flight').addEventListener('touchstart', function(e) {
            e.preventDefault(); 
            if (websocket.readyState === WebSocket.OPEN) { 
                websocket.send("CMD:FLIGHT"); 
            }
        });
    </script>
</body>
</html>
)rawliteral";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    if (strncmp((const char *)payload, "CMD:FLIGHT", 10) == 0) {
      // Alternar el estado de la secuencia
      if (!secuenciaActiva) {
        secuenciaActiva = true;
        pasoSecuencia = 1;             // Empezar en el paso 1 (Motor 1)
        tiempoSecuencia = millis();    // Guardar el tiempo de inicio
        Serial.println("Secuencia INICIADA");
      } else {
        secuenciaActiva = false;
        pasoSecuencia = 0;             // Apagar todo
        Serial.println("Secuencia DETENIDA/APAGADA");
      }
      return; 
    }
  }
}

void setup() {
  Serial.begin(115200);

  // 1. INICIAR COMUNICACIONES
  Serial.println("Iniciando Dron AP...");
  WiFi.softAP(ssid, password);
  Serial.print("Direccion IP del Dron: ");
  Serial.println(WiFi.softAPIP());
  
  server.on("/", []() { server.send(200, "text/html", index_html); });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  // 2. CONFIGURAR Y ARMAR LOS ESCs
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  esc1.setPeriodHertz(50);
  esc2.setPeriodHertz(50);
  esc3.setPeriodHertz(50);
  esc4.setPeriodHertz(50);

  esc1.attach(pinMotor1, MIN_PULSE, MAX_PULSE);
  esc2.attach(pinMotor2, MIN_PULSE, MAX_PULSE);
  esc3.attach(pinMotor3, MIN_PULSE, MAX_PULSE);
  esc4.attach(pinMotor4, MIN_PULSE, MAX_PULSE);

  Serial.println(">>> ARMANDO ESCs... ESCUCHA LOS PITIDOS <<<");
  
  // Mandar señal de "Acelerador en cero" (1000us)
  esc1.writeMicroseconds(MIN_PULSE);
  esc2.writeMicroseconds(MIN_PULSE);
  esc3.writeMicroseconds(MIN_PULSE);
  esc4.writeMicroseconds(MIN_PULSE);
  
  // Damos 4 segundos al inicio para que los ESCs se armen
  delay(4000); 
  Serial.println("ESCs Armados y Listos para la secuencia.");
}

void loop() {
  server.handleClient();
  webSocket.loop();

  // --- LÓGICA DE SECUENCIA DE MOTORES NO BLOQUEANTE ---
  
  if (secuenciaActiva) {
    unsigned long tiempoActual = millis();

    // Transiciones de estado cada 2000 ms (2 segundos)
    if (pasoSecuencia == 1 && (tiempoActual - tiempoSecuencia >= 2000)) {
      pasoSecuencia = 2;
      tiempoSecuencia = tiempoActual; // Reiniciar cronómetro
    } 
    else if (pasoSecuencia == 2 && (tiempoActual - tiempoSecuencia >= 2000)) {
      pasoSecuencia = 3;
      tiempoSecuencia = tiempoActual;
    } 
    else if (pasoSecuencia == 3 && (tiempoActual - tiempoSecuencia >= 2000)) {
      pasoSecuencia = 4;
      // Ya no actualizamos el tiempo, se queda en el paso 4 infinitamente hasta que se presione el botón.
    }
  } else {
    // Si la secuencia está apagada, nos aseguramos de que el paso sea 0
    pasoSecuencia = 0; 
  }

  // --- ASIGNACIÓN DE PWM SEGÚN EL PASO ---
  int pwmFinalM1 = MIN_PULSE;
  int pwmFinalM2 = MIN_PULSE;
  int pwmFinalM3 = MIN_PULSE;
  int pwmFinalM4 = MIN_PULSE;

  // Lógica en cascada: si estamos en un paso superior, los anteriores siguen encendidos.
  if (pasoSecuencia >= 1) pwmFinalM1 = PWM_GIRO;
  if (pasoSecuencia >= 2) pwmFinalM2 = PWM_GIRO;
  if (pasoSecuencia >= 3) pwmFinalM3 = PWM_GIRO;
  if (pasoSecuencia >= 4) pwmFinalM4 = PWM_GIRO;

  // MANDAR SEÑAL A ESCs EN MICROSEGUNDOS
  esc1.writeMicroseconds(pwmFinalM1);
  esc2.writeMicroseconds(pwmFinalM2);
  esc3.writeMicroseconds(pwmFinalM3);
  esc4.writeMicroseconds(pwmFinalM4);
}