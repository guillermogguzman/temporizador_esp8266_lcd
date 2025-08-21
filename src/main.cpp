#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1

// ================== CONFIGURACIÓN ==================
#define BOTON_INC   D5
#define BOTON_DEC   D4
#define BOTON_EMERG D6
#define LED_SALIDA  D7

#define TIEMPO_MIN  600   // 10 min en segundos
#define TIEMPO_MAX  5400  // 90 min en segundos
#define TIEMPO_INC  300   // 5 min en segundos

// ================== VARIABLES ==================
volatile uint32_t tiempo_restante = TIEMPO_MIN; 
volatile uint8_t estado = 0;   // 0 detenido, 1 en marcha, 2 pausado
volatile bool reset_flag = false;
volatile bool flag_actualizar = true;

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección 0x27, puede variar a 0x3F

// ================== FUNCIONES ==================
void actualizar_display() {
  lcd.clear();
  uint8_t minutos = tiempo_restante / 60;
  uint8_t segundos = tiempo_restante % 60;

  // Línea 0 → Tiempo
  lcd.setCursor(0, 0);
  lcd.print("Tiempo: ");
  if (minutos < 10) lcd.print("0");
  lcd.print(minutos);
  lcd.print(":");
  if (segundos < 10) lcd.print("0");
  lcd.print(segundos);

  // Línea 1 → Estado
  lcd.setCursor(0, 1);
  if (estado == 1) {
    lcd.print("    ENCENDIDO    ");
  } else if (estado == 2) {
    lcd.print("    PAUSADO     ");
  } else {
    lcd.print("   DETENIDO     ");
  }
}


void enviar_estado_uart() {
  const char *estado_str = (estado == 0) ? "DETENIDO" : 
                           (estado == 1) ? "EN MARCHA" : "PAUSADO";
  Serial.printf("Estado: %s | Tiempo: %02d:%02d\r\n",
                estado_str, tiempo_restante / 60, tiempo_restante % 60);
}

void manejar_botones() {
static unsigned long ultima_lectura = 0;
  static bool inc_estado_anterior = HIGH;
  static bool emerg_estado_anterior = HIGH;

  unsigned long ahora = millis();

  // === Botón INC ===
  bool inc_lectura = digitalRead(BOTON_INC);
  if (inc_lectura != inc_estado_anterior && (ahora - ultima_lectura) > 80) {
    ultima_lectura = ahora;
    if (inc_lectura == LOW && estado == 0) { // flanco de bajada
      if (tiempo_restante + TIEMPO_INC <= TIEMPO_MAX) {
        tiempo_restante += TIEMPO_INC;
      } else {
        tiempo_restante = TIEMPO_MIN;
      }
      flag_actualizar = true;
    }
  }
  inc_estado_anterior = inc_lectura;

  // === Botón EMERG ===
  bool emerg_lectura = digitalRead(BOTON_EMERG);
  if (emerg_lectura != emerg_estado_anterior && (ahora - ultima_lectura) > 80) {
    ultima_lectura = ahora;
    if (emerg_lectura == LOW) { // flanco de bajada
      if (estado == 1) {        // En marcha → pausa
        digitalWrite(LED_SALIDA, HIGH);
        estado = 2;
      } else if (estado == 2) { // Pausado → reanuda
        digitalWrite(LED_SALIDA, LOW);
        estado = 1;
      } else {                  // Detenido → inicia
        digitalWrite(LED_SALIDA, LOW);
        estado = 1;
      }
      flag_actualizar = true;
    }
  }
  emerg_estado_anterior = emerg_lectura;
}

void contar_tiempo() {
  static unsigned long ultima_actualizacion = 0;
  if (millis() - ultima_actualizacion >= 1000) {
    ultima_actualizacion = millis();
    if (estado == 1 && tiempo_restante > 0) {
      tiempo_restante--;
      flag_actualizar = true;
      if (tiempo_restante == 0) {
        estado = 0;
        digitalWrite(LED_SALIDA, LOW);
        reset_flag = true;
      }
    }
  }
}

// ================== SETUP ==================
void setup() {
  pinMode(BOTON_INC, INPUT_PULLUP);
  pinMode(BOTON_DEC, INPUT_PULLUP);
  pinMode(BOTON_EMERG, INPUT_PULLUP);
  pinMode(LED_SALIDA, OUTPUT);
  digitalWrite(LED_SALIDA, HIGH);

  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.print("Bienvenido!");
  delay(2000);
  digitalWrite(LED_SALIDA, HIGH);
  actualizar_display();
}

// ================== LOOP ==================
void loop() {
  manejar_botones();
  contar_tiempo();

  if (flag_actualizar) {
    actualizar_display();
    flag_actualizar = false;
  }
}
