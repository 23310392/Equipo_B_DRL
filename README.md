# N.E.R.V. Protocol

# Proyecto: Dron FPV Modificado (Ingeniería Inversa E88)

Este repositorio documenta todo el proceso de diseño, análisis, configuración y ensamblaje de un dron FPV personalizado. Este proyecto nace a partir de la aplicación de ingeniería inversa a un dron comercial modelo E88, con el objetivo de mejorar sus características de vuelo, adaptarlo para FPV (First Person View) y optimizar su rendimiento general.

---

## 📐 Diseño y Decisiones Estructurales

En esta sección se documentan las decisiones de diseño tomadas para llegar a la versión final del chasis y la distribución de componentes. 

MARK I
Reducción de peso y distribución de masa: El chasis fue diseñado con dimensiones inferiores a 5 pulgadas, optimizando la geometría para reducir el peso sin comprometer la rigidez estructural. La distribución de la masa se concentró en la parte central del dron para favorecer la estabilidad y mejorar la respuesta durante el vuelo.
Materiales: El diseño está pensado para ser fabricado mediante impresión 3D en ABS, material que ofrece una buena relación entre resistencia mecánica, durabilidad y facilidad de fabricación. Esto permite obtener un prototipo ligero, resistente y de bajo costo para pruebas y validación.
Integración de la electrónica: Este primer diseño se enfocó principalmente en el desarrollo de la estructura mecánica. No se definieron alojamientos ni puntos de montaje específicos para los componentes electrónicos, por lo que durante el ensamblaje se identificó que elementos como la batería, el Arduino Nano ESP32, el MPU6050, los ESC, el cableado y otros componentes no contaban con el espacio necesario para una integración adecuada. Estas limitaciones sirvieron como base para el rediseño de versiones posteriores.


MARK II
Reducción de peso y distribución de masa: Para esta segunda iteración se realizó un rediseño completo del chasis tomando como referencia la arquitectura empleada en drones FPV de menos de 5 pulgadas. Se adoptó una configuración compuesta por una placa inferior, una cubierta superior y separadores, lo que permitió obtener una estructura más rígida y modular. Además, se buscó concentrar la mayor parte de la masa en la zona central del dron con el objetivo de mejorar la estabilidad, disminuir el momento de inercia y facilitar futuras tareas de mantenimiento y sustitución de componentes.
Materiales: Al igual que en el MARK I, el prototipo fue diseñado para ser fabricado mediante impresión 3D en ABS. La elección de este material respondió a su buena resistencia mecánica, su capacidad para soportar impactos moderados y las temperaturas generadas durante la operación de los componentes electrónicos. Asimismo, el uso de impresión 3D permitió fabricar y evaluar rápidamente nuevas iteraciones del diseño, reduciendo los tiempos de desarrollo y el costo de fabricación de los prototipos.
Diseño estructural: Durante esta iteración se incorporaron características propias de los chasis utilizados en drones FPV comerciales. Entre ellas destacan el soporte frontal para el montaje de la cámara, una cubierta superior desmontable para facilitar el acceso a la electrónica y una estructura interna pensada para organizar los diferentes módulos del sistema. Estas modificaciones representaron un avance significativo respecto al MARK I, ya que permitieron obtener un diseño más compacto, estético y cercano a una solución funcional.
Integración de la electrónica: Una de las principales mejoras del MARK II fue la incorporación de espacios destinados al montaje de la batería y otros componentes electrónicos. Sin embargo, durante el proceso de ensamblaje se identificó que las dimensiones utilizadas durante el modelado no coincidían con las medidas reales de varios de estos componentes. Como consecuencia, algunos elementos no podían instalarse correctamente o requerían modificaciones adicionales para su montaje. Esta situación evidenció la importancia de realizar un levantamiento dimensional más preciso antes de la etapa de diseño, así como de considerar tolerancias de fabricación y ensamblaje. Las observaciones obtenidas durante esta fase sirvieron como base para el desarrollo del MARK III, donde se redimensionaron los compartimientos internos y se optimizó la distribución de los componentes para garantizar una integración adecuada.


MARK III
Reducción de peso y distribución de masa: A diferencia de las iteraciones anteriores, en este diseño se decidió abandonar la restricción de construir un dron con dimensiones inferiores a 5 pulgadas. Durante el desarrollo se comprobó que los componentes electrónicos seleccionados requerían un mayor espacio para su integración, por lo que mantener ese límite comprometía tanto el ensamblaje como la distribución del peso. Como resultado, se diseñó un chasis completamente nuevo, con dimensiones personalizadas que permitieran alojar todos los componentes sin afectar la estabilidad ni la accesibilidad. La nueva distribución de masa se centró en mantener el centro de gravedad lo más cercano posible al eje del dron, favoreciendo un comportamiento estable durante el vuelo.
Materiales: El chasis continúa siendo fabricado mediante impresión 3D en ABS, material seleccionado por su resistencia mecánica, durabilidad y facilidad de fabricación. El uso de este material permitió mantener un peso estructural relativamente bajo, además de facilitar la fabricación de piezas personalizadas y futuras modificaciones sin incrementar significativamente los costos del proyecto.
Diseño estructural: El MARK III representa un rediseño completo respecto a los prototipos anteriores. Tanto la geometría como las dimensiones fueron desarrolladas específicamente para satisfacer las necesidades del proyecto, sin seguir estrictamente las medidas convencionales de los drones FPV comerciales. Se priorizó la funcionalidad sobre el tamaño, obteniendo un chasis más amplio que proporciona una mejor distribución interna, mayor facilidad de ensamblaje y acceso a los componentes para tareas de mantenimiento. Aunque sus dimensiones son mayores que las de los prototipos MARK I y MARK II, el diseño mantiene una estructura robusta, práctica y cómoda para su manipulación.
Integración de la electrónica: Todas las dimensiones del chasis fueron ajustadas tomando como referencia las medidas reales de los componentes electrónicos. Esto permitió integrar correctamente la batería, el Arduino Nano ESP32, el MPU6050, los ESC, los motores, el sistema de distribución del cableado y los demás elementos necesarios para el funcionamiento del dron. A diferencia de las versiones anteriores, todos los componentes pueden instalarse sin interferencias mecánicas, facilitando el ensamblaje y el mantenimiento del sistema.
Peso del prototipo: El chasis impreso y completamente ensamblado presenta un peso aproximado de 337 g. Una vez instalados todos los componentes electrónicos, el peso total del dron permanece por debajo de los 950 g, cumpliendo con el objetivo de mantener una plataforma suficientemente ligera para operar sin comprometer la resistencia estructural. Este equilibrio entre peso, espacio disponible y robustez convierte al MARK III en el diseño final del proyecto y en la versión que satisface completamente los requerimientos establecidos por el equipo.
---

## 📈 Modelado Matemático y Simulación en MATLAB

Para comprender el comportamiento dinámico del dron y predecir su respuesta a los comandos de control, se desarrolló un modelo matemático en MATLAB. A continuación se presenta la evidencia fotográfica y gráfica de las ondas de comportamiento de nuestro modelo (por ejemplo, respuesta al escalón, ajuste de PIDs teórico, estabilización de altitud).

### Diagrama de control
<img width="962" height="712" alt="Diagrama de control" src="https://github.com/user-attachments/assets/84e2185d-0bf3-4349-8312-b6d280ddee65" />

Este diagrama implementa un **sistema de control en cascada** para la estabilización y seguimiento de trayectoria de un modelo no lineal de 6 grados de libertad (`Modelo_NERV`) en Simulink.

### 📐 Arquitectura del Sistema
* **Control de Posición (Lazos Externos):** Regula las coordenadas de translación en los ejes $X$, $Y$ y la altura ($Z$), generando las referencias de actitud correspondientes.
* **Control de Actitud (Lazos Internos):** Estabiliza la orientación del vehículo gestionando los ángulos de **Roll** ($\phi$), **Pitch** ($\theta$) y **Yaw** ($\psi$).
* **Planta No Lineal:** Integra la dinámica de vuelo de 6-DOF (`Modelo_NERV`) junto con bloques de integración temporal y realimentación de estados.
* **Visualización:** Monitoreo en tiempo real de la posición espacial y el comportamiento de los ángulos de Euler mediante bloques *Scope*.

### Gráficas de movimiento (X, Y, Z, Roll, Pitch y Yaw)
**Movimiento en X**: Representa el desplazamiento lineal en el eje X. Se observa una transición limpia y rápida que alcanza la posición objetivo de 2m sin sobreimpulsos agresivos.
<img width="2184" height="1166" alt="Gráfica de movimiento en X" src="https://github.com/user-attachments/assets/ab90fce3-ff6e-4937-9438-a0f3afded2bc" />


**Movimiento en Y**: Muestra la respuesta en el eje Y alcanzando de forma estable y asintótica el valor de referencia configurado en 1m.
<img width="2184" height="1165" alt="Gráfica de movimiento en Y" src="https://github.com/user-attachments/assets/ea59f573-c938-4c1e-aef7-9efca886711a" />

**Movimiento en Z (Altura)**: Muestra la respuesta escalón de la altitud del cuadricóptero. Presenta un ligero sobreimpulso controlado durante los primeros segundos antes de estabilizarse suavemente en la referencia deseada de 2m.
<img width="2184" height="1166" alt="Gráfica de movimiento en Z" src="https://github.com/user-attachments/assets/11f11cfb-4df9-4a46-87cd-6ca594511471" />

**Ángulo Pitch**: Ilustra la evolución del ángulo de cabeceo. Exhibe un pequeño transitorio inicial debido al acoplamiento dinámico antes de asentarse de manera completamente horizontal en estado estacionario.
<img width="2184" height="1166" alt="Gráfica de ángulo Pitch" src="https://github.com/user-attachments/assets/8d356107-4641-4839-b96b-425f9b3bc812" />

**Ángulo Roll**: Muestra el comportamiento del ángulo de alabeo. Presenta la compensación inicial de la inclinación lateral correspondiente al acoplamiento de los ejes antes de mantenerse estable en 0°.
<img width="2184" height="1166" alt="Gráfica de ángulo Roll" src="https://github.com/user-attachments/assets/26562e65-5041-4e23-a67b-55fdc1eea755" />

**Ángulo Yaw**: Refleja la orientación del ángulo de guiñada, el cual converge de forma uniforme y estable hacia el valor de referencia establecido.
<img width="2184" height="1165" alt="Gráfica de movimiento en Y" src="https://github.com/user-attachments/assets/0b7ef997-bc66-4cbc-b189-3b9c86487593" />

---

## ⚡ Electrónica y Protocolos de Comunicación

La transformación del E88 a un dron FPV funcional requirió una actualización completa de la electrónica y la implementación de protocolos de comunicación robustos.

### Componentes Principales
*   **Controladora de Vuelo (FC):** [Sensor MPU6050]
*   **Variadores de Velocidad (ESC):** [ESCs 30A]
*   **Motores:** [2212 1000KV]
*   **Receptor :** [Arduino nano ESP32]

### Protocolos de Comunicación Utilizados
*   **Receptor a Controladora (Arduino nano ESP32 -> dispositivo conectado por wifi):** El protocolo más común para conectar un Arduino Nano ESP32 a un dispositivo vía Wi-Fi y controlar un dron es el uso de comunicación basada en UDP/TCP sobre la red inalámbrica, donde el ESP32 actúa como punto de acceso o cliente y recibe comandos de control (pitch, roll, yaw, throttle) enviados desde un smartphone o computadora. Esto permite transmitir órdenes en tiempo real con baja latencia.
*   **Controladora(Arduino nano ESP32) a ESCs:** La comunicación entre el Arduino Nano ESP32 y los ESCs de 30A se realiza de forma simple y eficiente mediante PWM, permitiendo controlar la velocidad de los motores y, con ello, la estabilidad y maniobrabilidad del dron.
*   **Sensores Internos (Acelerómetro):**
  *El **MPU6050** integra un acelerómetro de 3 ejes que mide las aceleraciones lineales en tiempo real.
  * Permite detectar cambios en la posición y movimiento del dron, fundamentales para calcular la orientación (roll, pitch).
  * Su comunicación mediante protocolo **I²C** (líneas SDA y SCL) simplifica la conexión con el **Arduino Nano ESP32**.
  * El acelerómetro trabaja junto con el giroscopio para reducir errores de deriva y mejorar la precisión de los cálculos de estabilidad.



## Reporte de materiales y justificación

## Sensor MPU6050
### Descripción
El MPU6050 es un módulo que integra un acelerómetro de 3 ejes y un giroscopio de 3 ejes, ofreciendo un sistema de medición de 6 grados de libertad (6DOF). Se utiliza ampliamente en proyectos de drones por su capacidad de medir aceleraciones lineales y velocidades angulares en tiempo real.

### Justificación de uso en el dron
- Permite obtener datos de orientación y movimiento (roll, pitch, yaw).
- Mejora la estabilidad del vuelo, corrigiendo oscilaciones y vibraciones.
- Facilita la implementación de controladores PID, ajustando la velocidad de los motores para mantener equilibrio.
- Incluye un Digital Motion Processor (DMP) que procesa cálculos de orientación dentro del chip, reduciendo la carga del microcontrolador.
- Se comunica mediante I²C, lo que simplifica la integración con placas como Arduino o ESP32.

### Beneficios en la mejora del dron
- Vuelo más estable y controlado.
- Maniobras más suaves y precisas.
- Reducción de errores por deriva del giroscopio mediante fusión de datos con el acelerómetro.
- Flexibilidad para integrarse con otros sensores (GPS, barómetro, magnetómetro).

---

## Placa de desarrollo Arduino Nano ESP32
### Descripción
El Arduino Nano ESP32 es una placa compacta que combina la facilidad de uso del ecosistema Arduino con la potencia del microcontrolador ESP32. Ofrece conectividad inalámbrica integrada (Wi-Fi y Bluetooth), múltiples entradas/salidas digitales y analógicas, y capacidad de procesamiento superior en comparación con placas tradicionales como el Arduino Nano clásico.

### Justificación de uso en el dron
- Actúa como unidad de control principal, gestionando la lectura de sensores y el envío de señales a los motores.
- Su procesador dual-core permite ejecutar algoritmos de control en tiempo real, como filtros de fusión de datos y controladores PID.
- La conectividad Wi-Fi/Bluetooth facilita la comunicación remota para telemetría, control desde aplicaciones móviles o transmisión de datos.
- Compatible con librerías y entornos de programación de Arduino, lo que simplifica el desarrollo y la integración con otros módulos.
- Su tamaño reducido y bajo consumo energético lo hacen ideal para aplicaciones en drones ligeros.

### Beneficios en la mejora del dron
- Mayor capacidad de procesamiento para cálculos de estabilidad y navegación.
- Posibilidad de implementar funciones avanzadas como control remoto inalámbrico o envío de datos en tiempo real.
- Integración sencilla con sensores como el MPU6050, GPS y módulos de comunicación.
- Optimización del espacio y peso gracias a su formato compacto.
- Es un componente de bajo costo y alta disponibilidad, ideal para proyectos académicos y prototipos.

---

## Controlador de velocidad electrónico (ESC) 30A
### Descripción
El ESC de 30 amperios (30A) es un controlador de velocidad electrónico diseñado para regular la potencia suministrada a los motores brushless del dron. Convierte la señal de control proveniente del microcontrolador (Arduino Nano ESP32) en pulsos eléctricos adecuados para ajustar la velocidad y dirección de los motores.

### Justificación de uso en el dron
- Permite un control preciso de la velocidad de los motores, esencial para la estabilidad y maniobrabilidad del dron.
- Soporta hasta 30 amperios de corriente, adecuado para motores de tamaño medio utilizados en drones académicos o recreativos.
- Incluye funciones de protección contra sobrecorriente y sobrecalentamiento, aumentando la seguridad del sistema.
- Compatible con señales PWM estándar, facilitando la integración con placas de control como el Arduino Nano ESP32.
- Su tamaño compacto y peso ligero lo hacen ideal para aplicaciones aéreas.

### Beneficios en la mejora del dron
- Mejora la respuesta dinámica del dron al ajustar rápidamente la velocidad de los motores.
- Contribuye a un vuelo más estable y seguro, evitando daños por sobrecarga eléctrica.
- Permite implementar algoritmos de control avanzados (PID) para mantener equilibrio y realizar maniobras suaves.
- Asegura una distribución eficiente de energía, optimizando el rendimiento de la batería.

---

## Motor Brushless 2212 1000KV
### Descripción
El motor brushless 2212 1000KV es un motor eléctrico sin escobillas diseñado para aplicaciones aéreas como drones y cuadricópteros. El valor 1000KV indica que el motor gira aproximadamente a 1000 revoluciones por minuto (RPM) por cada voltio aplicado, lo que lo hace adecuado para un equilibrio entre potencia y eficiencia.

### Justificación de uso en el dron
- Proporciona la fuerza de empuje necesaria para levantar y mantener el dron en vuelo.
- Su diseño sin escobillas (brushless) ofrece mayor durabilidad, menor mantenimiento y mejor eficiencia energética en comparación con motores con escobillas.
- El rango de 1000KV es ideal para drones de tamaño medio, ya que permite un buen control de velocidad y estabilidad sin sacrificar autonomía.
- Compatible con controladores ESC de 30A, asegurando una integración confiable en el sistema de propulsión.
- Su tamaño compacto y peso ligero lo hacen adecuado para estructuras de drones académicos y recreativos.

### Beneficios en la mejora del dron
- Vuelo más estable y eficiente, gracias a su respuesta rápida y precisa.
- Mayor tiempo de vuelo, al optimizar el consumo de energía de la batería.
- Capacidad de carga adecuada, permitiendo transportar sensores y componentes adicionales sin comprometer el rendimiento.
- Durabilidad y confiabilidad.

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
