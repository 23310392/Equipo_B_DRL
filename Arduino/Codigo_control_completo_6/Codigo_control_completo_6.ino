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

// --- DIRECCIÓN Y VARIABLES DEL MPU-6050 ---
const int MPU_ADDR = 0x68; 
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, Tmp;

// --- ASIGNACIÓN DE MOTORES (QUAD X) ---
const int pinMotor1 = 2; // Trasero Derecho (CW) - ESC 40A
const int pinMotor2 = 3; // Frontal Derecho (CCW) - ESC 30A
const int pinMotor3 = 4; // Trasero Izquierdo (CCW) - ESC 30A
const int pinMotor4 = 5; // Frontal Izquierdo (CW) - ESC 30A

Servo esc1, esc2, esc3, esc4;

// --- VARIABLES DE ESTADO Y SEGURIDAD ---
enum DroneState { SUELO, DESPEGANDO, VOLANDO, ATERRIZANDO };
DroneState estadoDron = SUELO;
bool mpuConectado = false; 

// Valores en microsegundos para los ESC
const int MIN_PULSE = 1000;
const int MAX_PULSE = 2000;
const int ARMING_PULSE = 900; // Señal extra baja para forzar el armado de los ESC 30A

int pwmBase = MIN_PULSE;                 
const int PWM_HOVER = 1450; 
const int VELOCIDAD_RAMPA = 2; 

// --- VARIABLES DE CONTROL (Joysticks) ---
int throttleManual = 0, yawManual = 0, pitchManual = 0, rollManual = 0;

// --- VARIABLES DEL FILTRO COMPLEMENTARIO Y PID ---
unsigned long tiempoAnterior = 0;
float dt;

float pitchReal = 0, rollReal = 0;
float pitchTarget = 0, rollTarget = 0, yawTarget = 0;

// Constantes PID
float Kp = 2.5, Ki = 0.05, Kd = 18.0; 
float Kp_yaw = 3.0, Ki_yaw = 0.05, Kd_yaw = 0.0;

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

  // Ampliamos el rango de attach para permitir el pulso de 900us
  esc1.attach(pinMotor1, ARMING_PULSE, MAX_PULSE);
  esc2.attach(pinMotor2, ARMING_PULSE, MAX_PULSE);
  esc3.attach(pinMotor3, ARMING_PULSE, MAX_PULSE);
  esc4.attach(pinMotor4, ARMING_PULSE, MAX_PULSE);

  Serial.println(">>> ARMANDO ESCs... ESPERA 5 SEGUNDOS <<<");
  
  // Enviamos la señal extra baja para forzar el reconocimiento de "cero"
  esc1.writeMicroseconds(ARMING_PULSE);
  esc2.writeMicroseconds(ARMING_PULSE);
  esc3.writeMicroseconds(ARMING_PULSE);
  esc4.writeMicroseconds(ARMING_PULSE);
  
  // Damos 5 segundos al inicio para que todos los ESC procesen la señal
  delay(5000); 
  
  // Subimos al pulso mínimo estándar de vuelo
  esc1.writeMicroseconds(MIN_PULSE);
  esc2.writeMicroseconds(MIN_PULSE);
  esc3.writeMicroseconds(MIN_PULSE);
  esc4.writeMicroseconds(MIN_PULSE);
  
  Serial.println("ESCs Armados y Listos.");

  // 3. INICIAR SENSOR MPU6050
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("¡ERROR! MPU6050 no responde.");
    mpuConectado = false;
  } else {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B); Wire.write(0);
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1A); Wire.write(0x03); 
    Wire.endTransmission(true);

    mpuConectado = true;
  }
  
  tiempoAnterior = micros();
}

void loop() {
  server.handleClient();
  webSocket.loop();

  if (!mpuConectado) {
    esc1.writeMicroseconds(MIN_PULSE); esc2.writeMicroseconds(MIN_PULSE);
    esc3.writeMicroseconds(MIN_PULSE); esc4.writeMicroseconds(MIN_PULSE);
    estadoDron = SUELO;
    return; 
  }

  unsigned long tiempoActual = micros();
  dt = (tiempoActual - tiempoAnterior) / 1000000.0; 

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

    float accPitch = atan2(AcY, AcZ) * 180 / PI;
    float accRoll  = atan2(-AcX, sqrt((long)AcY * AcY + (long)AcZ * AcZ)) * 180 / PI;

    float gyroXrate = GyX / 131.0;
    float gyroYrate = GyY / 131.0;
    float yawRate   = GyZ / 131.0;

    pitchReal = 0.98 * (pitchReal + gyroXrate * dt) + 0.02 * accPitch;
    rollReal  = 0.98 * (rollReal  + gyroYrate * dt) + 0.02 * accRoll;

    // 3. MÁQUINA DE ESTADOS
    if (estadoDron == DESPEGANDO) {
      pwmBase += VELOCIDAD_RAMPA;
      if (pwmBase >= PWM_HOVER) { pwmBase = PWM_HOVER; estadoDron = VOLANDO; }
    } else if (estadoDron == ATERRIZANDO) {
      pwmBase -= VELOCIDAD_RAMPA;
      if (pwmBase <= MIN_PULSE) { pwmBase = MIN_PULSE; estadoDron = SUELO; }
    }

    int pwmFinalM1 = MIN_PULSE, pwmFinalM2 = MIN_PULSE;
    int pwmFinalM3 = MIN_PULSE, pwmFinalM4 = MIN_PULSE;

    if (estadoDron != SUELO) {
      pitchTarget = map(pitchManual, -100, 100, -20, 20);
      rollTarget  = map(rollManual, -100, 100, -20, 20);
      yawTarget   = map(yawManual, -100, 100, -45, 45); 
      
      int t_pwm   = map(throttleManual, -100, 100, -200, 200); 

      // PID PITCH
      errorPitch = pitchTarget - pitchReal;
      sumErrorPitch += errorPitch * dt;
      sumErrorPitch = constrain(sumErrorPitch, -50, 50); 
      float dErrorPitch = (errorPitch - errorPitchPrev) / dt;
      float pidPitch = (Kp * errorPitch) + (Ki * sumErrorPitch) + (Kd * dErrorPitch);
      errorPitchPrev = errorPitch;

      // PID ROLL
      errorRoll = rollTarget - rollReal;
      sumErrorRoll += errorRoll * dt;
      sumErrorRoll = constrain(sumErrorRoll, -50, 50); 
      float dErrorRoll = (errorRoll - errorRollPrev) / dt;
      float pidRoll = (Kp * errorRoll) + (Ki * sumErrorRoll) + (Kd * dErrorRoll);
      errorRollPrev = errorRoll;

      // PID YAW
      errorYaw = yawTarget - yawRate; 
      sumErrorYaw += errorYaw * dt;
      sumErrorYaw = constrain(sumErrorYaw, -50, 50);
      float dErrorYaw = (errorYaw - errorYawPrev) / dt;
      float pidYaw = (Kp_yaw * errorYaw) + (Ki_yaw * sumErrorYaw) + (Kd_yaw * dErrorYaw);
      errorYawPrev = errorYaw;

      // MEZCLA DE MOTORES (Quad X)
      int base = pwmBase + t_pwm;

      pwmFinalM1 = base + pidPitch - pidRoll - pidYaw;
      pwmFinalM2 = base - pidPitch - pidRoll + pidYaw;
      pwmFinalM3 = base + pidPitch + pidRoll + pidYaw;
      pwmFinalM4 = base - pidPitch + pidRoll - pidYaw;

      pwmFinalM1 = constrain(pwmFinalM1, MIN_PULSE, MAX_PULSE);
      pwmFinalM2 = constrain(pwmFinalM2, MIN_PULSE, MAX_PULSE);
      pwmFinalM3 = constrain(pwmFinalM3, MIN_PULSE, MAX_PULSE);
      pwmFinalM4 = constrain(pwmFinalM4, MIN_PULSE, MAX_PULSE);
    } else {
      sumErrorPitch = 0; sumErrorRoll = 0; sumErrorYaw = 0;
      pwmBase = MIN_PULSE;
    }

    esc1.writeMicroseconds(pwmFinalM1);
    esc2.writeMicroseconds(pwmFinalM2);
    esc3.writeMicroseconds(pwmFinalM3);
    esc4.writeMicroseconds(pwmFinalM4);
  }
}