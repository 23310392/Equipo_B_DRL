#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Credenciales Wi-Fi
const char* ssid = "NERV-Protocol";
const char* password = "sonotakus"; 

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
Adafruit_MPU6050 mpu;

// --- ASIGNACIÓN DE MOTORES (QUAD X) ---
const int motor1 = 2; // Trasero Derecho (CW)
const int motor2 = 3; // Frontal Derecho (CCW)
const int motor3 = 4; // Trasero Izquierdo (CCW)
const int motor4 = 5; // Frontal Izquierdo (CW)

// --- VARIABLES DE LA MÁQUINA DE ESTADOS ---
enum DroneState { SUELO, DESPEGANDO, VOLANDO, ATERRIZANDO };
DroneState estadoDron = SUELO;

int pwmBase = 0;                 
const int PWM_HOVER = 150;       
const int VELOCIDAD_RAMPA = 1;   

// --- VARIABLES DE CONTROL (Joysticks) ---
int throttleManual = 0, yawManual = 0, pitchManual = 0, rollManual = 0;

// --- VARIABLES DEL FILTRO COMPLEMENTARIO Y PID ---
unsigned long tiempoAnterior = 0;
float dt;

float pitchReal = 0, rollReal = 0;
float pitchTarget = 0, rollTarget = 0, yawTarget = 0;

// Constantes PID (¡Deberás afinar estos valores empíricamente!)
float Kp = 1.2, Ki = 0.04, Kd = 15.0; 
float Kp_yaw = 2.0, Ki_yaw = 0.02, Kd_yaw = 0.0;

// Errores PID
float errorPitch = 0, errorPitchPrev = 0, sumErrorPitch = 0;
float errorRoll = 0, errorRollPrev = 0, sumErrorRoll = 0;
float errorYaw = 0, errorYawPrev = 0, sumErrorYaw = 0;

// Interfaz Web incrustada
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
        <span id="status" style="color: red;">Desconectado</span> | 
        CMD: <span id="data-out">T:0 Y:0 P:0 R:0</span>
    </div>
    <div id="joystick-container">
        <div class="joystick-zone" id="zone-left"><div class="knob" id="knob-left"></div></div>
        <div id="center-control">
            <button class="btn-auto" id="btn-flight"></button>
            <div class="btn-label">Despegue/<br>Aterrizaje</div>
        </div>
        <div class="joystick-zone" id="zone-right"><div class="knob" id="knob-right"></div></div>
    </div>
    <script>
        let controlData = { T: 0, Y: 0, P: 0, R: 0 };
        let gateway = `ws://${window.location.hostname}:81/`;
        let websocket = new WebSocket(gateway);

        websocket.onopen = () => { document.getElementById('status').style.color = "lime"; document.getElementById('status').innerText = "Conectado"; };
        websocket.onclose = () => { document.getElementById('status').style.color = "red"; document.getElementById('status').innerText = "Desconectado"; };

        document.getElementById('btn-flight').addEventListener('touchstart', function(e) {
            e.preventDefault(); if (websocket.readyState === WebSocket.OPEN) { websocket.send("CMD:FLIGHT"); }
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
                if (isLeft) { controlData.Y = Math.round((x / center) * 100); controlData.T = Math.round((-y / center) * 100); } 
                else { controlData.R = Math.round((x / center) * 100); controlData.P = Math.round((-y / center) * 100); }
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
            if (websocket.readyState === WebSocket.OPEN) { websocket.send(payload); }
        }, 50);
    </script>
</body>
</html>
)rawliteral";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    if (strncmp((const char *)payload, "CMD:FLIGHT", 10) == 0) {
      if (estadoDron == SUELO) estadoDron = DESPEGANDO;
      else if (estadoDron == VOLANDO || estadoDron == DESPEGANDO) estadoDron = ATERRIZANDO;
      return; 
    }
    int t, y, p, r;
    if (sscanf((const char *)payload, "T:%d Y:%d P:%d R:%d", &t, &y, &p, &r) == 4) {
      throttleManual = t; yawManual = y; pitchManual = p; rollManual = r;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(motor1, OUTPUT); pinMode(motor2, OUTPUT); 
  pinMode(motor3, OUTPUT); pinMode(motor4, OUTPUT);

  // Inicializar MPU6050
  if (!mpu.begin()) {
    Serial.println("¡ERROR FÍSICO! MPU6050 no detectado.");
    while (1) { delay(10); } // Detener todo si falla el sensor
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ); // Filtro paso bajo para vibraciones de motores

  WiFi.softAP(ssid, password);
  server.on("/", []() { server.send(200, "text/html", index_html); });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  tiempoAnterior = micros();
}

void loop() {
  server.handleClient();
  webSocket.loop();

  unsigned long tiempoActual = micros();
  dt = (tiempoActual - tiempoAnterior) / 1000000.0; // Tiempo en segundos

  // Ejecutamos el lazo de control PID a alta velocidad (aprox 100Hz = cada 10ms)
  if (dt >= 0.01) {
    tiempoAnterior = tiempoActual;

    // 1. LEER SENSOR
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // 2. CALCULAR ÁNGULOS (Filtro Complementario)
    // Convertir aceleraciones a grados
    float accPitch = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
    float accRoll  = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;

    // Integrar giroscopio (convertido de rad/s a grados/s)
    pitchReal = 0.98 * (pitchReal + (g.gyro.x * 180/PI) * dt) + 0.02 * accPitch;
    rollReal  = 0.98 * (rollReal  + (g.gyro.y * 180/PI) * dt) + 0.02 * accRoll;
    float yawRate = g.gyro.z * 180/PI; // Para el yaw usamos velocidad angular directamente

    // 3. MÁQUINA DE ESTADOS DE POTENCIA BÁSICA
    if (estadoDron == DESPEGANDO) {
      pwmBase += VELOCIDAD_RAMPA;
      if (pwmBase >= PWM_HOVER) { pwmBase = PWM_HOVER; estadoDron = VOLANDO; }
    } else if (estadoDron == ATERRIZANDO) {
      pwmBase -= VELOCIDAD_RAMPA;
      if (pwmBase <= 0) { pwmBase = 0; estadoDron = SUELO; }
    }

    int pwmFinalM1 = 0, pwmFinalM2 = 0, pwmFinalM3 = 0, pwmFinalM4 = 0;

    // Solo calculamos PID y activamos motores si no estamos apagados en el suelo
    if (estadoDron != SUELO) {
      
      // Mapear joysticks a ángulos deseados (Máximo inclinación de 20 grados)
      pitchTarget = map(pitchManual, -100, 100, -20, 20);
      rollTarget  = map(rollManual, -100, 100, -20, 20);
      yawTarget   = map(yawManual, -100, 100, -45, 45); // Grados por segundo deseados
      int t_pwm   = map(throttleManual, -100, 100, -50, 50);

      // --- CÁLCULO PID PARA PITCH ---
      errorPitch = pitchTarget - pitchReal;
      sumErrorPitch += errorPitch * dt;
      sumErrorPitch = constrain(sumErrorPitch, -50, 50); // Anti-windup
      float dErrorPitch = (errorPitch - errorPitchPrev) / dt;
      float pidPitch = (Kp * errorPitch) + (Ki * sumErrorPitch) + (Kd * dErrorPitch);
      errorPitchPrev = errorPitch;

      // --- CÁLCULO PID PARA ROLL ---
      errorRoll = rollTarget - rollReal;
      sumErrorRoll += errorRoll * dt;
      sumErrorRoll = constrain(sumErrorRoll, -50, 50); // Anti-windup
      float dErrorRoll = (errorRoll - errorRollPrev) / dt;
      float pidRoll = (Kp * errorRoll) + (Ki * sumErrorRoll) + (Kd * dErrorRoll);
      errorRollPrev = errorRoll;

      // --- CÁLCULO PID PARA YAW ---
      errorYaw = yawTarget - yawRate; // El yaw compara velocidades, no ángulos absolutos
      sumErrorYaw += errorYaw * dt;
      sumErrorYaw = constrain(sumErrorYaw, -50, 50);
      float dErrorYaw = (errorYaw - errorYawPrev) / dt;
      float pidYaw = (Kp_yaw * errorYaw) + (Ki_yaw * sumErrorYaw) + (Kd_yaw * dErrorYaw);
      errorYawPrev = errorYaw;

      // 4. MEZCLA DE MOTORES CON SALIDAS PID
      int base = pwmBase + t_pwm;

      // A la potencia base, le sumamos/restamos las correcciones calculadas
      pwmFinalM1 = base + pidPitch - pidRoll - pidYaw; // Trasero Derecho
      pwmFinalM2 = base - pidPitch - pidRoll + pidYaw; // Frontal Derecho
      pwmFinalM3 = base + pidPitch + pidRoll + pidYaw; // Trasero Izquierdo
      pwmFinalM4 = base - pidPitch + pidRoll - pidYaw; // Frontal Izquierdo

      // Límite estricto PWM
      pwmFinalM1 = constrain(pwmFinalM1, 0, 255);
      pwmFinalM2 = constrain(pwmFinalM2, 0, 255);
      pwmFinalM3 = constrain(pwmFinalM3, 0, 255);
      pwmFinalM4 = constrain(pwmFinalM4, 0, 255);
    } else {
      // Si está en el suelo, resetear integrales para evitar arranques bruscos
      sumErrorPitch = 0; sumErrorRoll = 0; sumErrorYaw = 0;
    }

    analogWrite(motor1, pwmFinalM1);
    analogWrite(motor2, pwmFinalM2);
    analogWrite(motor3, pwmFinalM3);
    analogWrite(motor4, pwmFinalM4);
  }
}