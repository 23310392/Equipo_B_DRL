#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Crear el objeto del sensor
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // Esperar a que se abra el Monitor Serie

  Serial.println("Inicializando MPU6050...");

  // Iniciar la comunicación con el sensor
  if (!mpu.begin()) {
    Serial.println("¡ERROR! No se encontro un chip MPU6050.");
    Serial.println("Revisa tus conexiones: VCC a 3.3V, GND a GND, SDA a A4 y SCL a A5.");
    while (1) {
      delay(10); // Detener el código aquí si falla
    }
  }
  
  Serial.println("MPU6050 Encontrado!");

  // Configuración del rango de sensibilidad del sensor (Ideal para drones)
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  delay(100);
}

void loop() {
  // Crear variables para almacenar los eventos del sensor
  sensors_event_t a, g, temp;
  
  // Leer los datos del MPU6050
  mpu.getEvent(&a, &g, &temp);

  // Imprimir los datos en el Monitor Serie
  Serial.print("Aceleracion (m/s^2) [ X: ");
  Serial.print(a.acceleration.x);
  Serial.print(" | Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(" | Z: ");
  Serial.print(a.acceleration.z);
  Serial.print(" ]  ---  Giroscopio (rad/s) [ X: ");
  
  Serial.print(g.gyro.x);
  Serial.print(" | Y: ");
  Serial.print(g.gyro.y);
  Serial.print(" | Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" ]");

  delay(100);
}