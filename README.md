# N.E.R.V. Protocol

# Proyecto: Dron FPV Modificado (Ingeniería Inversa E88)

Este repositorio documenta todo el proceso de diseño, análisis, configuración y ensamblaje de un dron FPV personalizado. Este proyecto nace a partir de la aplicación de ingeniería inversa a un dron comercial modelo E88, con el objetivo de mejorar sus características de vuelo, adaptarlo para FPV (First Person View) y optimizar su rendimiento general.

---

## 📐 Diseño y Decisiones Estructurales

En esta sección se documentan las decisiones de diseño tomadas para llegar a la versión final del chasis y la distribución de componentes. 

Al partir del modelo E88, se identificaron varias áreas de mejora estructural. Las decisiones clave de diseño incluyen:
*   **Reducción de peso y distribución de masa:** Se modificó la estructura central para centralizar la masa, mejorando así la respuesta en el eje de *roll* y *pitch*.
*   **Materiales:** [Explica aquí si cambiaste materiales, por ejemplo, de plástico inyectado a fibra de carbono o impresión 3D (PETG/TPU/PLA) para mayor resistencia a impactos].
*   **Montaje de la electrónica:** Se diseñó un espacio específico para aislar la controladora de vuelo de las vibraciones de los motores (soft-mounting).

---

## 📈 Modelado Matemático y Simulación en MATLAB

Para comprender el comportamiento dinámico del dron y predecir su respuesta a los comandos de control, se desarrolló un modelo matemático en MATLAB. A continuación se presenta la evidencia fotográfica y gráfica de las ondas de comportamiento de nuestro modelo (por ejemplo, respuesta al escalón, ajuste de PIDs teórico, estabilización de altitud).

### Diagrama de control
![Modelo dron NERV/Diagrama de control.png]

Este diagrama implementa un **sistema de control en cascada** para la estabilización y seguimiento de trayectoria de un modelo no lineal de 6 grados de libertad (`Modelo_NERV`) en Simulink.

### 📐 Arquitectura del Sistema
* **Control de Posición (Lazos Externos):** Regula las coordenadas de translación en los ejes $X$, $Y$ y la altura ($Z$), generando las referencias de actitud correspondientes.
* **Control de Actitud (Lazos Internos):** Estabiliza la orientación del vehículo gestionando los ángulos de **Roll** ($\phi$), **Pitch** ($\theta$) y **Yaw** ($\psi$).
* **Planta No Lineal:** Integra la dinámica de vuelo de 6-DOF (`Modelo_NERV`) junto con bloques de integración temporal y realimentación de estados.
* **Visualización:** Monitoreo en tiempo real de la posición espacial y el comportamiento de los ángulos de Euler mediante bloques *Scope*.

---

## ⚡ Electrónica y Protocolos de Comunicación

La transformación del E88 a un dron FPV funcional requirió una actualización completa de la electrónica y la implementación de protocolos de comunicación robustos.

### Componentes Principales
*   **Controladora de Vuelo (FC):** [Ingresar modelo de FC]
*   **Variadores de Velocidad (ESC):** [Ingresar modelo y amperaje de ESCs]
*   **Motores:** [Ingresar tamaño y KV, ej. 1103 8000KV]
*   **Sistema FPV:** Cámara [Modelo] y VTX de [Potencia, ej. 200mW].
*   **Receptor (RX):** [Modelo del receptor de radio]

### Protocolos de Comunicación Utilizados
*   **Receptor a Controladora (RX -> FC):** Se utiliza el protocolo **[Ej. CRSF / SBUS / IBUS]** vía UART, lo que garantiza una latencia ultra baja para el control manual del dron.
*   **Controladora a ESCs:** La comunicación con los motores se realiza mediante el protocolo digital **[Ej. DSHOT300 / DSHOT600]**, eliminando la necesidad de calibración analógica y ofreciendo mayor precisión en las RPM.
*   **Telemetría y Configuración de VTX:** Se implementó el protocolo **[Ej. SmartAudio / IRC Tramp]** para poder cambiar canales y potencia de transmisión de video directamente desde el control remoto (OSD).
*   **Sensores Internos (Giroscopio/Acelerómetro):** Comunicación interna vía **SPI / I2C** para una lectura de datos a alta frecuencia (ej. 8kHz).

---

## 💻 Programación y Lógica de Vuelo (Custom Firmware)

El "cerebro" de este dron no utiliza un firmware comercial prefabricado, sino que funciona con un controlador de vuelo personalizado programado desde cero para el microcontrolador ESP32.

*   **Estación de Control Terrestre (Interfaz Web y WebSockets):** Para este proyecto, decidimos prescindir de un radio control físico tradicional debido a limitaciones de tiempo y presupuesto. En su lugar, el ESP32 está configurado como un Punto de Acceso (AP) que genera su propia red Wi-Fi y aloja una interfaz web construida en HTML, CSS y JavaScript. Elegimos esta arquitectura web por su inmensa practicidad y facilidad de implementación: nos permite pilotar el dron desde cualquier smartphone directamente en el navegador, sin necesidad de instalar aplicaciones de terceros. Para garantizar que los movimientos de los joysticks virtuales lleguen al dron sin retrasos perceptibles, la comunicación bidireccional se realiza mediante el protocolo WebSockets.

*   **Fusión de Sensores (Filtro Complementario):** Para que el dron "sienta" su orientación en el espacio, el ESP32 se comunica mediante el protocolo I2C con un sensor IMU MPU-6050. El código implementa un Filtro Complementario matemático que fusiona las lecturas rápidas del giroscopio con la referencia absoluta de la gravedad del acelerómetro, eliminando el ruido mecánico y el drift (desviación) a lo largo del tiempo.

*   **Lazo de Control PID:** El corazón del vuelo autónomo es nuestro bucle PID (Proporcional, Integral, Derivativo), diseñado para ejecutarse a alta velocidad (aprox. 100Hz). El código compara constantemente la orientación real del dron (medida por el MPU-6050) con la orientación deseada por el piloto en la interfaz web. El PID calcula instantáneamente la compensación necesaria en los ejes de Cabeceo (Pitch), Alabeo (Roll) y Guiñada (Yaw) para corregir cualquier desviación.

*   **Matriz de Mezcla de Motores (Mixer Quad-X):** Basándonos en la cinemática estándar de los drones cuadricópteros, los resultados del cálculo PID se introducen en un algoritmo de mezcla de motores. Este sistema distribuye matemáticamente la potencia necesaria a cada uno de los cuatro motores de forma independiente mediante señales PWM. Esto permite que el dron contrarreste la gravedad, obedezca los comandos direccionales y se estabilice a sí mismo frente a perturbaciones externas.

---

## 📸 Galería del Modelo Físico

En esta sección se muestra el resultado final del ensamblaje del dron, detallando el empaquetado de los componentes, la gestión de cables (cable management) y la estética final de la aeronave.

> *<!-- Reemplaza las rutas de las imágenes con fotografías de tu dron armado -->*
> 
> ![Vista Superior del Dron](ruta/a/tu/foto_superior.jpg)
> *Vista superior del ensamblaje final.*
>
> ![Vista Lateral - Perfil Aerodinámico](ruta/a/tu/foto_lateral.jpg)
> *Vista lateral mostrando el montaje de la cámara FPV y la batería.*
>
> ![Detalle de la Electrónica](ruta/a/tu/foto_electronica.jpg)
> *Detalle del apilamiento (stack) de la controladora de vuelo y los ESCs.*
