<div align="justify">

# 📄 Análisis Post-Mortem: Proyecto N.E.R.V. Protocol

> **Nota:** Este documento detalla las causas técnicas, limitaciones operativas y fallos de integración por los cuales la plataforma física del dron N.E.R.V. Protocol no logró cumplir con los requisitos mínimos de desempeño para la demostración en vivo.

---

## 📌 Resumen Ejecutivo

A pesar de haber alcanzado un modelado matemático funcional en simulaciones y un ensamblaje físico completo, el prototipo **MARK III** no logró efectuar un vuelo estable ni controlado durante las pruebas finales. Los factores determinantes abarcan desde limitaciones físicas del chasis hasta cuellos de botella en la distribución de potencia y sincronización de hardware.

---

## 🔍 Análisis de Cusas Raíz

### 1. Desincronización y Respuesta Asíncrona de los Motores
Durante la fase de armado de motores y pruebas de aceleración iniciales, se detectó que los cuatro motores *brushless* 2212 1000KV no respondían a la misma velocidad ni con la misma latencia.
* **Causa:** Variaciones en los tiempos de respuesta de las señales PWM generadas por la placa de desarrollo, sumado a la falta de un procedimiento de calibración individual y simétrica de los ESC de 30A.
* **Impacto:** Al aplicar aceleración (*throttle*), el dron generaba un empuje asimétrico instantáneo, provocando giros involuntarios sobre sus ejes antes de lograr despegar.

### 2. Distribución de Masa y Geometría del Chasis (Geometría del MARK III)
Para solucionar los problemas de espacio de las iteraciones anteriores, el chasis MARK III incrementó sus dimensiones sustancialmente.
* **Causa:** Al alejar los motores del centro de masa, el momento de inercia ($I$) de la aeronave aumentó de forma considerable.
* **Impacto:** Los brazos extendidos requirieron un par de torsión (*torque*) mucho mayor para corregir pequeñas desviaciones. El bucle de control PID no logró compensar la inercia adicional, generando oscilaciones severas e inestabilidad estructural.

### 3. Deficiencias en el Sistema de Alimentación y Potencia
El sistema de potencia presentó caídas de voltaje (*voltage sags*) drásticas durante los picos de demanda de corriente.
* **Causa:** La batería seleccionada no logró suministrar la tasa de descarga constante ($C$-rating) requerida por los cuatro ESCs de 30A operando en simultáneo.
* **Impacto:** Al exigir el empuje necesario para elevar los ~950 g del prototipo, el voltaje del sistema caía por debajo del umbral operativo, causando reinicios aleatorios o comportamiento errático en los controladores.

### 4. Limitaciones de Hardware en la Placa de Control (Arduino Nano ESP32)
Aunque la placa cuenta con conectividad Wi-Fi y buena capacidad de procesamiento, la arquitectura de integración física presentó inconvenientes severos.
* **Causa:** Sensibilidad del microcontrolador al ruidoso entorno electromagnético generado por los motores y la falta de aislamiento en las líneas de alimentación lógica/potencia.
* **Impacto:** Se presentaron caídas puntuales en la conexión de la interfaz web por WebSockets y lecturas ruidosas en el bus I²C del sensor IMU (MPU6050), lo que corrompía los datos del filtro complementario en momentos críticos.

---

## 🛠️ Lecciones Aprendidas y Trabajo Futuro

1. **Rediseño Geométrico:** Regresar a una geometría más compacta (True-X o Deadcat de < 5 pulgadas) utilizando materiales de menor densidad o corte en fibra de carbono para mantener el momento de inercia controlado.
2. **Arquitectura Eléctrica:** Implementar una PDB (Power Distribution Board) dedicada con reguladores de voltaje aislados (BEC) para separar la alimentación del microcontrolador de la línea de alta corriente de los motores.
3. **Firmware e Inercia:** Calibrar electrónicamente el rango del acelerador en todos los ESCs e integrar filtros digitales (ej. Filtro Notch / Kalman) para mitigar el ruido mecánico transferido a la IMU.

---

<p align="center">
  📂 <i>Este análisis queda registrado como base de conocimiento para futuras iteraciones del proyecto.</i><br>
  ⬅️ <a href="./README.md">Volver al README Principal</a>
</p>

</div>
