#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>

// --- CREDENCIALES WI-FI ---
const char* ssid = "NERV-Protocol";
const char* password = "sonotakus"; 

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// --- DIRECCIÓN Y VARIABLES DEL MPU-6050 ---
const int MPU_ADDR = 0x68; 
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, Tmp;

// --- ASIGNACIÓN DE MOTORES (QUAD X) ---
const int motor1 = 2; // Trasero Derecho (CW)
const int motor2 = 3; // Frontal Derecho (CCW)
const int motor3 = 4; // Trasero Izquierdo (CCW)
const int motor4 = 5; // Frontal Izquierdo (CW)

// --- VARIABLES DE ESTADO Y SEGURIDAD ---
enum DroneState { SUELO, DESPEGANDO, VOLANDO, ATERRIZANDO };
DroneState estadoDron = SUELO;
bool mpuConectado = false; // Bandera de seguridad

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

// Constantes PID (Punto de partida empírico)
float Kp = 1.2, Ki = 0.04, Kd = 15.0; 
float Kp_yaw = 2.0, Ki_yaw = 0.02, Kd_yaw = 0.0;

// Errores PID
float errorPitch = 0, errorPitchPrev = 0, sumErrorPitch = 0;
float errorRoll = 0, errorRollPrev = 0, sumErrorRoll = 0;
float errorYaw = 0, errorYawPrev = 0, sumErrorYaw = 0;

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
      if (estadoDron == SUELO && mpuConectado) estadoDron = DESPEGANDO;
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

  // 1. INICIAR WI-FI PRIMERO
  Serial.println("Iniciando Dron AP...");
  WiFi.softAP(ssid, password);
  server.on("/", []() { server.send(200, "text/html", index_html); });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("Wi-Fi y Servidor Web iniciados correctamente.");

  // 2. INICIAR SENSOR MPU6050 (I2C Raw)
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  byte error = Wire.endTransmission(); // Verifica si el dispositivo responde

  if (error != 0) {
    Serial.println("¡ADVERTENCIA! MPU6050 no responde en I2C.");
    Serial.println("El Wi-Fi funciona, pero el vuelo ha sido bloqueado por seguridad.");
    mpuConectado = false;
  } else {
    // Despertar MPU-6050
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B); // PWR_MGMT_1
    Wire.write(0);    // 0 = Despertar
    Wire.endTransmission(true);

    // Configurar Filtro Paso Bajo (DLPF) a 42Hz para mitigar vibración de motores
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1A); // CONFIG
    Wire.write(0x03); // Valor 3 = 42Hz
    Wire.endTransmission(true);

    Serial.println("Sensor MPU6050 inicializado correctamente.");
    mpuConectado = true;
  }
  
  tiempoAnterior = micros();
}

void loop() {
  server.handleClient();
  webSocket.loop();

  if (!mpuConectado) {
    analogWrite(motor1, 0); analogWrite(motor2, 0);
    analogWrite(motor3, 0); analogWrite(motor4, 0);
    estadoDron = SUELO;
    return; 
  }

  unsigned long tiempoActual = micros();
  dt = (tiempoActual - tiempoAnterior) / 1000000.0; // Segundos

  // Lazo de control a ~100Hz
  if (dt >= 0.01) {
    tiempoAnterior = tiempoActual;

    // 1. LEER SENSOR RAW I2C
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 14, true);

    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    Tmp = Wire.read() << 8 | Wire.read();
    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();

    // 2. CALCULAR ÁNGULOS
    // Se usa (long) para evitar desbordamiento matemático al multiplicar enteros de 16 bits
    float accPitch = atan2(AcY, AcZ) * 180 / PI;
    float accRoll  = atan2(-AcX, sqrt((long)AcY * AcY + (long)AcZ * AcZ)) * 180 / PI;

    // Escalar giroscopio a grados por segundo (Rango por defecto de +/- 250 dps = 131.0 LSB/dps)
    float gyroXrate = GyX / 131.0;
    float gyroYrate = GyY / 131.0;
    float yawRate   = GyZ / 131.0;

    pitchReal = 0.98 * (pitchReal + gyroXrate * dt) + 0.02 * accPitch;
    rollReal  = 0.98 * (rollReal  + gyroYrate * dt) + 0.02 * accRoll;

    // 3. MÁQUINA DE ESTADOS Y CONTROL PID
    if (estadoDron == DESPEGANDO) {
      pwmBase += VELOCIDAD_RAMPA;
      if (pwmBase >= PWM_HOVER) { pwmBase = PWM_HOVER; estadoDron = VOLANDO; }
    } else if (estadoDron == ATERRIZANDO) {
      pwmBase -= VELOCIDAD_RAMPA;
      if (pwmBase <= 0) { pwmBase = 0; estadoDron = SUELO; }
    }

    int pwmFinalM1 = 0, pwmFinalM2 = 0, pwmFinalM3 = 0, pwmFinalM4 = 0;

    if (estadoDron != SUELO) {
      pitchTarget = map(pitchManual, -100, 100, -20, 20);
      rollTarget  = map(rollManual, -100, 100, -20, 20);
      yawTarget   = map(yawManual, -100, 100, -45, 45); 
      int t_pwm   = map(throttleManual, -100, 100, -50, 50);

      // --- PID PITCH ---
      errorPitch = pitchTarget - pitchReal;
      sumErrorPitch += errorPitch * dt;
      sumErrorPitch = constrain(sumErrorPitch, -50, 50); 
      float dErrorPitch = (errorPitch - errorPitchPrev) / dt;
      float pidPitch = (Kp * errorPitch) + (Ki * sumErrorPitch) + (Kd * dErrorPitch);
      errorPitchPrev = errorPitch;

      // --- PID ROLL ---
      errorRoll = rollTarget - rollReal;
      sumErrorRoll += errorRoll * dt;
      sumErrorRoll = constrain(sumErrorRoll, -50, 50); 
      float dErrorRoll = (errorRoll - errorRollPrev) / dt;
      float pidRoll = (Kp * errorRoll) + (Ki * sumErrorRoll) + (Kd * dErrorRoll);
      errorRollPrev = errorRoll;

      // --- PID YAW ---
      errorYaw = yawTarget - yawRate; 
      sumErrorYaw += errorYaw * dt;
      sumErrorYaw = constrain(sumErrorYaw, -50, 50);
      float dErrorYaw = (errorYaw - errorYawPrev) / dt;
      float pidYaw = (Kp_yaw * errorYaw) + (Ki_yaw * sumErrorYaw) + (Kd_yaw * dErrorYaw);
      errorYawPrev = errorYaw;

      // 4. MEZCLA DE MOTORES (Quad X)
      int base = pwmBase + t_pwm;

      pwmFinalM1 = base + pidPitch - pidRoll - pidYaw; // Trasero Derecho
      pwmFinalM2 = base - pidPitch - pidRoll + pidYaw; // Frontal Derecho
      pwmFinalM3 = base + pidPitch + pidRoll + pidYaw; // Trasero Izquierdo
      pwmFinalM4 = base - pidPitch + pidRoll - pidYaw; // Frontal Izquierdo

      pwmFinalM1 = constrain(pwmFinalM1, 0, 255);
      pwmFinalM2 = constrain(pwmFinalM2, 0, 255);
      pwmFinalM3 = constrain(pwmFinalM3, 0, 255);
      pwmFinalM4 = constrain(pwmFinalM4, 0, 255);
    } else {
      sumErrorPitch = 0; sumErrorRoll = 0; sumErrorYaw = 0;
    }

    analogWrite(motor1, pwmFinalM1);
    analogWrite(motor2, pwmFinalM2);
    analogWrite(motor3, pwmFinalM3);
    analogWrite(motor4, pwmFinalM4);
  }
}