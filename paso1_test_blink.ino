/*
  PASO 1 - PRUEBA DE HARDWARE (Blink)
  Proyecto: Escáner de vulnerabilidades IoT con ESP32

  Sketch mínimo para confirmar que el ESP32 y el entorno Arduino IDE
  están correctamente configurados antes de continuar con el firmware
  del escáner. Si el LED integrado de la placa parpadea cada segundo,
  el hardware y los drivers están funcionando correctamente.

  Nota: en la mayoría de placas ESP32 DevKit V1 el LED integrado está
  en el pin 2. Si no compila por "LED_BUILTIN was not declared in this
  scope", esta definición manual soluciona el problema.
*/

#define LED_BUILTIN 2

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
