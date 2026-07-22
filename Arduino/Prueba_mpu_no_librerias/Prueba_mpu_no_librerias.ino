#include <Wire.h>

// Dirección I2C por defecto del MPU-6050
const int MPU_ADDR = 0x68; 

// Variables para almacenar los datos en crudo (16 bits)
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, Tmp;

void setup() {
  Serial.begin(115200);
  
  // Iniciar el bus I2C (En el Nano ESP32 usará A4 para SDA y A5 para SCL)
  Wire.begin(); 
  
  // Despertar el MPU-6050 (por defecto inicia en modo 'sleep')
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // Registro PWR_MGMT_1
  Wire.write(0);     // Escribir 0 para despertar el sensor
  Wire.endTransmission(true);
  
  Serial.println("MPU-6050 Iniciado. Leyendo datos...");
  delay(1000);
}

void loop() {
  // Solicitar lectura de datos al sensor
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // Empezar a leer desde el registro 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true); // Solicitar 14 registros en total

  // Leer y combinar los bytes altos y bajos de cada eje
  AcX = Wire.read() << 8 | Wire.read(); // Acelerómetro X
  AcY = Wire.read() << 8 | Wire.read(); // Acelerómetro Y
  AcZ = Wire.read() << 8 | Wire.read(); // Acelerómetro Z
  
  Tmp = Wire.read() << 8 | Wire.read(); // Temperatura
  
  GyX = Wire.read() << 8 | Wire.read(); // Giroscopio X
  GyY = Wire.read() << 8 | Wire.read(); // Giroscopio Y
  GyZ = Wire.read() << 8 | Wire.read(); // Giroscopio Z

  // Imprimir los datos en el Monitor Serie
  Serial.print("Aceleracion [ ");
  Serial.print("X: "); Serial.print(AcX);
  Serial.print(" | Y: "); Serial.print(AcY);
  Serial.print(" | Z: "); Serial.print(AcZ);
  Serial.print(" ]  ---  Giroscopio [ ");
  Serial.print("X: "); Serial.print(GyX);
  Serial.print(" | Y: "); Serial.print(GyY);
  Serial.print(" | Z: "); Serial.print(GyZ);
  Serial.println(" ]");

  // Pequeña pausa para no saturar el monitor
  delay(100);
}