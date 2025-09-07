#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================== CONFIGURACIÓN DE PINES POR PLATAFORMA ==================
#if defined(ESP8266)
  // NodeMCU (ESP8266)
  #define BOTON_INC   14    // D5
  #define BOTON_START 2     // D4  
  #define ACTIVAR_MOTOR  13 // D7
  #define ESTADO_MOTOR   5  // D1
  #define DESACTIVAR_MOTOR 12 // D6

#elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
  // Arduino UNO/Nano
  #define BOTON_INC   5
  #define BOTON_START 4
  #define ACTIVAR_MOTOR  7
  #define ESTADO_MOTOR   2
  #define DESACTIVAR_MOTOR 8

#else
  #error "Plataforma no compatible. Usa ESP8266 o Arduino UNO/Nano"
#endif


#define TIEMPO_MIN  300   // 5 min en segundos
#define TIEMPO_MAX  5400  // 90 min en segundos
#define TIEMPO_INC  300   // 5 min en segundos

// ================== VARIABLES ==================
volatile uint32_t tiempo_restante = TIEMPO_MIN; 
volatile uint8_t estado = 0;   // 0 detenido, 1 en marcha, 2 pausado
volatile bool reset_flag = false; // Lectura del estado del motor
volatile bool flag_actualizar = true;

// Variables para el debounce
int estadoActual;              // Estado actual después del debounce
int estadoAnterior;           // Estado anterior para comparación

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

void manejar_botones() {
static unsigned long ultima_lectura = 0;
  static bool inc_estado_anterior = HIGH;
  static bool start_estado_anterior = HIGH;

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

  // === Botón START ===
  bool start_lectura = digitalRead(BOTON_START);
  if (start_lectura != start_estado_anterior && (ahora - ultima_lectura) > 80) {
    ultima_lectura = ahora;
    if (start_lectura == LOW) { // flanco de bajada
      if (estado == 1) {        // En marcha → pausa
        //digitalWrite(DESACTIVAR_MOTOR, LOW);
        //delay(500);
        //digitalWrite(DESACTIVAR_MOTOR, HIGH);
        
        //estado = 2;
      } else if (estado == 2) { // Pausado → reanuda
        //digitalWrite(ACTIVAR_MOTOR, LOW);
        //delay(500);
        //digitalWrite(ACTIVAR_MOTOR, HIGH);
        estado = 1;
      } else {                  // Detenido → inicia
        digitalWrite(ACTIVAR_MOTOR, LOW);
        delay(500);
        digitalWrite(ACTIVAR_MOTOR, HIGH);
        estado = 1;
      }
      flag_actualizar = true;
    }
  }
  start_estado_anterior = start_lectura;
}

void contar_tiempo() {
  static unsigned long ultima_actualizacion = 0;
  if (millis() - ultima_actualizacion >= 1000) {
    ultima_actualizacion = millis();
    if (estado == 1 && tiempo_restante > 0) {
      tiempo_restante--;
      flag_actualizar = true;
      reset_flag = true;
      if (tiempo_restante == 0) {
        estado = 0;
        digitalWrite(DESACTIVAR_MOTOR, LOW);
        delay(500);
        digitalWrite(DESACTIVAR_MOTOR, HIGH);
        tiempo_restante = TIEMPO_MIN;
      }
    }
  }
}

void leer_estado_motor(){
  // Leer el pin con debounce
  int lectura = digitalRead(ESTADO_MOTOR);
  if (lectura != estadoActual) {
    estadoActual = lectura;
  }
  if (estadoActual == LOW and estado != 0) {
    estado = 0;
    digitalWrite(DESACTIVAR_MOTOR, LOW);
    delay(500);
    digitalWrite(DESACTIVAR_MOTOR, HIGH);
    tiempo_restante = TIEMPO_MIN;
    flag_actualizar = true;
  }
}

// ================== SETUP ==================
void setup() {
  pinMode(BOTON_INC, INPUT_PULLUP);
  pinMode(BOTON_START, INPUT_PULLUP);
  pinMode(ACTIVAR_MOTOR, OUTPUT);
  pinMode(ESTADO_MOTOR, INPUT_PULLUP);
  pinMode(DESACTIVAR_MOTOR, OUTPUT);

  digitalWrite(ACTIVAR_MOTOR, HIGH);
  digitalWrite(DESACTIVAR_MOTOR, HIGH);

  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.print("Bienvenido!");
  delay(2000);
  
  // Leer estado inicial motor
  estadoActual = digitalRead(ESTADO_MOTOR);
  estadoAnterior = estadoActual;

  actualizar_display();
}

// ================== LOOP ==================
void loop() {
  manejar_botones();
  contar_tiempo();
  if (reset_flag == true) {
    leer_estado_motor();
    reset_flag = false;
  }
  if (flag_actualizar) {
    actualizar_display();
    flag_actualizar = false;
  }
}
