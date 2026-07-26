<div align="justify">

# 📄 Análisis Post-Mortem: Proyecto N.E.R.V. Protocol

> **Nota:** Este documento detalla las causas técnicas, limitaciones operativas y fallos de integración por los cuales la plataforma física del dron N.E.R.V. Protocol no logró cumplir con los requisitos mínimos de desempeño para la demostración en vivo.

---

<div align="justify">

# 📄 Análisis Post-Mortem y Reporte Técnico de Integración: Proyecto N.E.R.V. Protocol

> **Estado del Documento:** Registro formal de fallas de integración, análisis de la dinámica de vuelo, limitaciones de manufactura aditiva y diagnóstico del sistema embebido. Este documento justifica por qué la plataforma física **MARK III** no logró cumplir los requisitos mínimos de despegue y estabilización en las pruebas finales.

---

## 📌 1. Resumen Ejecutivo y Marco Contextual

El proyecto **N.E.R.V. Protocol** tenía como objetivo la transformación e ingeniería inversa de un dron comercial de bajo costo (E88) hacia una plataforma FPV totalmente personalizada, controlada mediante una estación terrestre alojada en un microcontrolador **Arduino Nano ESP32** y un sensor IMU **MPU6050**. 

A pesar de haber desarrollado un modelo matemático congruente en Simulink —con control en cascada para la dinámica de 6 grados de libertad (6-DOF)— y de haber completado el ensamblaje físico del chasis **MARK III**, la plataforma no logró efectuar un despegue ni un vuelo estacionario controlado. 

Los factores determinantes de este fallo no fueron aislados; correspondieron a una **reacción en cadena de problemas multidominio**:
1. Desfases dimensionales por flexión y alabeo (*warping*) en la impresión 3D del chasis.
2. Incremento severo del momento de inercia que saturó los lazos de control PID.
3. Desincronización y asincronía en los controladores electrónicos de velocidad (ESCs).
4. Caídas de voltaje drásticas (*voltage sags*) bajo demanda de corriente de pico.
5. Ruido electromagnético que corrompió las lecturas del bus de datos I²C y la estabilidad de la red WebSockets.

---

## 🔍 2. Análisis Detallado de Causas Raíz

### 2.1. Desincronización y Respuesta Asíncrona del Sistema de Propulsión
Durante las pruebas de armado de motores y barridos de aceleración (*throttle ramps*), los cuatro motores *brushless* 2212 1000KV mostraron diferencias marcadas en el umbral de arranque y en la curva de aceleración lineal.
* **Mecanismo de Falla:** Las señales PWM generadas desde los pines del ESP32 presentaron variaciones temporales en el ancho de pulso debido a interrupciones internas del sistema operativo en tiempo real (FreeRTOS) al gestionar la pila del protocolo Wi-Fi. A esto se sumó la ausencia de un procedimiento de calibración simétrica individual para el rango de gas en los ESCs de 30A.
* **Efecto en la Dinámica:** En el momento en que se superaba el 40% del *throttle*, la fuerza de empuje ($T = C_T \cdot \rho \cdot n^2 \cdot D^4$) no crecía de forma uniforme en los cuatro vértices. Esta asimetría generaba un par de guiñada (*yaw torque*) y un momento de alabeo/cabeceo no deseado que volteaba la aeronave sobre su propio eje antes de romper la capa límite de suelo.

### 2.2. Penalización del Momento de Inercia por la Geometría MARK III
Las versiones previas (MARK I y MARK II) demostraron que intentar alojar la electrónica en un chasis de menos de 5 pulgadas generaba un empaquetamiento crítico e interferencias mecánicas. Para el MARK III se decidió romper la restricción dimensional y expandir los brazos para una mejor distribución.
* **Mecanismo de Falla:** El momento de inercia $I$ de un cuerpo rígido escala con el cuadrado de la distancia ($I = \int r^2 \, dm$). Al alejar los motores de 337 g (más los ESCs y cableado) del centro de masa, la inercia angular respecto a los ejes de Roll ($\phi$) y Pitch ($\theta$) creció exponencialmente.
* **Efecto en la Dinámica:** El controlador PID configurado en el firmware fue ajustado teóricamente para una respuesta inercial más rápida. Al encontrarse con una masa rotacional mucho mayor, el término Proporcional ($K_p$) generaba correcciones insuficientes, mientras que el término Derivativo ($K_d$) amplificaba el ruido de alta frecuencia, resultando en oscilaciones violentas de baja frecuencia (*prop wash* simulado) que volvían inestable el bucle de control.

### 2.3. Deformación por Manufactura Aditiva (Efecto de Alabeo en ABS)
El chasis del MARK III fue fabricado íntegramente en acrilonitrilo butadieno estireno (ABS) mediante modelado por deposición fundida (FDM). 
* **Mecanismo de Falla:** Durante la impresión de uno de los brazos principales, las corrientes de aire térmicas de la impresora generaron una contracción desigual del polímero en las capas inferiores (*warping*). Esto introdujo una elevación del plano del motor de aproximadamente 2.3° respecto al plano horizontal del chasis.
* **Efecto en la Dinámica:** La inclinación del eje de empuje ($T$) introdujo una componente vectorial parásita en el plano horizontal ($T_{xy} = T \cdot \sin(\theta_{error})$). Este vector descompuesto actuaba como una fuerza lateral constante que impulsaba al dron a realizar trayectorias circulares e inclinaciones laterales involuntarias. El filtro complementario intentaba nivelar el chasis basándose en la gravedad, pero al hacerlo, desviaba aún más el vector de empuje total de la vertical.

### 2.4. Colapso del Manejo de Potencia (*Voltage Sag*) y Alimentación
El dron requería elevar una masa total en orden de vuelo cercana a los 950 g, lo que exigía un empuje total mínimo de ~1900 g para mantener una relación empuje-peso ($T/W$) viable de 2:1.
* **Mecanismo de Falla:** Al acelerar los cuatro motores simultáneamente, el consumo de corriente escalaba rápidamente por encima de los 45 Amperios totales. La batería lipo utilizada no poseía la tasa de descarga constante ($C$-rating) suficiente para sostener esa demanda instantánea, ni se contaba con una Power Distribution Board (PDB) con reguladores aislados.
* **Efecto en la Dinámica:** La resistencia interna de la batería generaba una caída severa en la tensión de la línea principal (bajando de 12.6V a menos de 9.8V en picos). Esto provocaba que el regulador embebido del Arduino Nano ESP32 operara al límite de su umbral de caída (*drop-out*), causando reinicios aleatorios del sistema (*brownouts*), pérdida transitoria de paquetes en los WebSockets y caídas de la red Wi-Fi.

### 2.5. Acoplamiento Electromagnético y Ruido en Bus I²C
El microcontrolador y el módulo IMU MPU6050 se montaron en una placa perforada directamente sobre la ruta por donde circulaban los cables de alta corriente hacia los variadores ESC.
* **Mecanismo de Falla:** Las líneas del bus I²C (SDA y SCL) carecían de apantallamiento magnético y de resistencias de *pull-up* ajustadas para entornos ruidosos. Las conmutaciones rápidas de los transistores MOSFET de los ESCs inducían picos de fuerza electromotriz (FEM) de alta frecuencia en el cableado de señal.
* **Efecto en la Dinámica:** El bus I²C experimentaba bloqueos de comunicación (*bus freeze*) o inyección de valores basura en las lecturas del giroscopio y acelerómetro. El filtro complementario procesaba lecturas con aceleraciones espurias, interpretando que el dron estaba en una inclinación crítica cuando se encontraba nivelado, lo que hacía que el PID aplicara correcciones bruscas a los motores a través de la matriz de mezcla (*mixer*).

---

## 🛠️ 3. Matriz Auditiva de Diagnóstico y Puntos Críticos

Para documentar formalmente la auditoría ejecutada sobre la plataforma, se presenta la siguiente tabla de evaluación de subsistemas:

| Subsistema / Punto Crítico | Estado Detectado | Causa Técnica Primaria | Severidad | Acción Correctora Requerida |
| :--- | :--- | :--- | :--- | :--- |
| **Calibración IMU (MPU6050)** | Desviación de cero en los ejes Roll/Pitch | Calibración realizada sobre chasis deformado por impresión | 🔴 Alta | Calibrar IMU mediante plantilla metálica independiente del chasis. |
| **Matriz de Mezcla (Mixer Quad-X)** | Asignación de potencia asimétrica | Desalineación de los puntos muertos (*deadbands*) de PWM | 🔴 Alta | Implementar calibración eeprom de ESCs en paralelo con señal maestra. |
| **Centro de Gravedad (CG)** | Desplazado 14mm hacia el cuadrante posterior | Batería ubicada sin riel de ajuste longitudinal | 🟡 Media | Rediseñar bandeja de batería en el eje central de simetría $X$-$Y$. |
| **Filtro de Fusión de Datos** | Deriva rápida durante la aceleración | Ruido mecánico de las hélices transmitido al giroscopio | 🔴 Alta | Implementar soporte antivibración (*gimbal dampeners*) y Filtro Kalman. |
| **Aislamiento Electromagnético** | Picos de ruido en datos de sensores | Cables de potencia paralelos a líneas SDA/SCL de la IMU | 🟡 Media | Trenzar cables de potencia y aislar bus I²C con malla trenzada a tierra. |
| **Resistencia Estructural** | Flexión torsional en los brazos | Relleno de impresión (*infill*) por debajo del 40% en ABS | 🔴 Alta | Incrementar relleno al 70% o migrar estructura a placas de Fibra de Carbono. |

---

## 💡 4. Plan de Rediseño y Hoja de Ruta (MARK IV)

Para corregir integralmente las fallas documentadas y lograr una plataforma 100% funcional en futuras iteraciones, se establece el siguiente protocolo de ingeniería:

### 4.1. Optimización Estructural y Metrología
1. **Reducción de Escala:** Volver a una arquitectura geométrica "True-X" compacta con una distancia diagonal de motor a motor no mayor a 220 mm (5 pulgadas), reduciendo el momento de inercia ($I$) al menos en un 35%.
2. **Control de Calidad Dimensional:**
   * Utilizar un calibrador Vernier digital para validar tolerancias mecánicas en un rango de $\pm 0.05 \text{ mm}$ en los puntos de montaje de los motores.
   * Modificar el diseño CAD introduciendo filamentos de refuerzo en los vértices del chasis y prever un factor de contracción del $1.5\%$ para impresiones en ABS/PETG.
   * Implementar placas de amortiguación elástica (TPU) para aislar mecánicamente el soporte del Arduino Nano ESP32 y el sensor MPU6050 del chasis principal.

### 4.2. Reorganización de la Arquitectura Eléctrica
1. **Red de Distribución Integrada (PDB):** Incorporar una placa de distribución con dos circuitos BEC (Battery Eliminator Circuit) totalmente aislados:
   * **Línea de Potencia (12V ilimitado):** Conexión directa desde la batería de alta tasa de descarga ($>75C$) hacia los 4 ESCs.
   * **Línea Lógica (5V / 3A Regulado):** Conexión independiente con filtrado LC para alimentar el microcontrolador ESP32 y la IMU, eliminando *brownouts*.
2. **Filtrado de Ruido:** Soldar un capacitor electrolítico de bajo ESR (1000 µF / 25V) directamente en la entrada principal de la batería para absorber los picos de tensión inductivos (*voltage spikes*) provocados por el frenado regenerativo o aceleraciones del motor.

### 4.3. Refactorización del Firmware y Protocolos
1. **Calibración Simultánea de ESCs:** Escribir una rutina en la fase de inicialización del microcontrolador que envíe el pulso de ancho máximo ($2000 \, \mu\text{s}$) y mínimo ($1000 \, \mu\text{s}$) a los 4 canales PWM de forma sincronizada al energizar el sistema.
2. **Procesamiento de Señal Avanzado:** Sustituir el filtro complementario por un **Filtro de Kalman Extendido (EKF)** o implementar filtros muesca digitales (*Notch Filters*) dinámicos fijados en la frecuencia fundamental de rotación de los motores (~150-200 Hz).
3. **Optimización de Comunicaciones:** Migrar el envío de datos de control de WebSockets sobre TCP a un socket puro sobre **UDP**, evitando que el reenvío de paquetes perdidos bloquee la ejecución del bucle PID principal.

---

## 📋 5. Conclusiones

El desarrollo de las tres iteraciones sucesivas del dron MARK I, MARK II y MARK III demostró que el diseño de una aeronave no depende de la abstracción del software o del modelado conceptual en simuladores, sino de la interacción física real entre manufactura, eléctrica, dinámica de estructuras y señales.

El MARK III resolvió con éxito las restricciones de espacio físico y accesibilidad para mantenimiento que hacían inviables a los prototipos anteriores. Sin embargo, este sobredimensionamiento trasladó el problema hacia el dominio cinemático, la manufactura y la integridad eléctrica. 

Las fallas documentadas en este reporte constituyen el activo técnico más valioso del proyecto. El análisis de las causas raíz proporciona la base metrológica, electromecánica y de control necesaria para garantizar la estabilidad y viabilidad operativa en la versión MARK IV.

---

<p align="center">
  📂 <i>Este documento concluye el análisis técnico post-mortem del Proyecto N.E.R.V. Protocol.</i><br>
  ⬅️ <a href="./README.md">Volver al Repositorio Principal (README.md)</a>
</p>

</div>
