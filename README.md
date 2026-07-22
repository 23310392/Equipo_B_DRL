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

> **Evidencia de Ondas de Comportamiento:**
> 
> *<!-- Reemplaza las rutas de las imágenes con tus propias capturas de MATLAB -->*
> 
> ![Gráfica de Respuesta de Roll/Pitch](ruta/a/tu/imagen_matlab_1.png)
> *Figura 1: Comportamiento del sistema ante una perturbación en el eje X.*
>
> ![Gráfica de Estabilización](ruta/a/tu/imagen_matlab_2.png)
> *Figura 2: Simulación de la estabilización del dron en vuelo estacionario.*

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

## 💻 Programación y Configuración (Betaflight)

El cerebro de este dron funciona gracias al firmware open-source **Betaflight**. 

**¿Cómo funciona el código/configuración?**
Betaflight se encarga de tomar las lecturas del giroscopio (movimiento real) y compararlas con las entradas del piloto en el control remoto (movimiento deseado). A través de un bucle de control PID (Proporcional, Integral, Derivativo) que se ejecuta miles de veces por segundo, el código calcula los ajustes necesarios y envía señales a los ESCs para variar la velocidad de cada motor de forma independiente. 

Para este modelo específico, la configuración se enfocó en:
1.  **Mapeo de Motores (Mixer):** Ajuste de la geometría personalizada basada en la estructura modificada del E88.
2.  **Filtros (Filtering):** Configuración de filtros (RPM Filter, Dynamic Notch) para lidiar con las frecuencias de resonancia específicas de nuestro chasis y evitar que el ruido llegue a los motores.
3.  **Tuning PID:** Ajustes de las ganancias basados en los resultados obtenidos previamente en la simulación de MATLAB para un vuelo *locked-in* y suave.

*(Opcional: Puedes incluir aquí un fragmento de tu configuración CLI de Betaflight o `diff all`)*
```text
# Ejemplo de configuración CLI clave
board_name = [TU_PLACA]
set motor_pwm_protocol = DSHOT600
set pid_process_denom = 1
# ...
```

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
