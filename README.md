# Controlador de Lavadoras Industriales con ESP8266

Este proyecto implementa un **controlador temporizado** para motores de lavadoras industriales.  
El sistema utiliza un **ESP8266 (NodeMCU V3)** para controlar contactores de potencia mediante un relé, con interfaz de usuario basada en un **LCD I2C** y **dos pulsadores físicos**.

---

## ⚙️ Características principales

- Microcontrolador: **ESP8266 (NodeMCU V3)**  
- Fuente: **220 VAC → 5 VDC** (fuente conmutada integrada)  
- Actuación: **Relé** para control de 220 VAC (manejo de contactores de los motores)  
- Interfaz de usuario:
  - **LCD 16x2 con módulo I2C**
  - **Botón de incremento de tiempo**
  - **Botón de inicio/pausa/emergencia**
- Lógica de control:
  - Temporizador configurable en pasos de 5 minutos
  - Rango: **10 min – 90 min**
  - Estados: **DETENIDO → ENCENDIDO → PAUSADO**
  - Visualización en LCD:
    - Línea 1 → Tiempo restante (`Tiempo: MM:SS`)
    - Línea 2 → Estado del sistema (`DETENIDO`, `ENCENDIDO`, `PAUSADO`)
- Comunicación serie (UART) para depuración y monitoreo

---

## 🛠️ Hardware utilizado

- **ESP8266 NodeMCU V3**  
- **Módulo LCD 16x2 con interfaz I2C** (dirección habitual `0x27`)  
- **Módulo relé 5V activo en LOW** para manejar contactores a 220 VAC  
- **Fuente de 220 VAC a 5 VDC** (para alimentar el NodeMCU + relé)  
- **2 pulsadores** con resistencias internas (`INPUT_PULLUP`)  

---

## 📲 Flujo de funcionamiento

1. Al encender, el controlador inicia en estado **DETENIDO**.  
2. El usuario puede:
   - Usar el botón **INC** para aumentar el tiempo en pasos de 5 minutos (máx. 90 min, vuelve a 10 min al superar el límite).
   - Usar el botón **EMERG** para:
     - Iniciar el ciclo → activa el relé y comienza la cuenta regresiva.
     - Pausar el ciclo → desactiva el relé pero conserva el tiempo.
     - Reanudar el ciclo → activa nuevamente el relé.
3. Cuando el tiempo llega a **0**, el sistema pasa automáticamente a **DETENIDO** y apaga el relé.  

---

## 💻 Desarrollo

El código fue escrito en **C++** y desarrollado con:

- **Visual Studio Code**
- **PlatformIO**
- Librerías utilizadas:
  - [`Arduino.h`](https://www.arduino.cc/reference/en/)
  - [`LiquidCrystal_I2C`](https://github.com/johnrickman/LiquidCrystal_I2C)
  - [`Wire.h`](https://www.arduino.cc/en/reference/wire)

---

## 🚀 Instalación y carga

1. Clonar este repositorio:
   ```bash
   git clone git@github.com:guillermogguzman/temporizador_esp8266_lcd.git
