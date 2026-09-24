/*
  PASO 2 - ESCÁNER DE HOSTS (Descubrimiento de dispositivos en la red)
  Proyecto: Escáner de vulnerabilidades IoT con ESP32

  Qué hace este código:
  1. Conecta el ESP32 a tu red WiFi.
  2. Detecta automáticamente el rango de tu subred (ej: 192.168.1.x).
  3. Hace "ping" a cada dirección IP posible (1 a 254) para ver cuáles
     responden, es decir, qué dispositivos están activos en la red.
  4. Imprime en el Monitor Serie la lista de dispositivos encontrados.

  Librería necesaria (instalar desde el Gestor de Librerías de Arduino IDE):
    - "ESP32Ping" de marian-craciunescu
      (Sketch > Include Library > Manage Libraries > buscar "ESP32Ping")

  Siguiente paso (Paso 3) usará esta misma lista de IPs activas para
  escanear puertos abiertos en cada una.
*/

#include <WiFi.h>
#include <ESP32Ping.h>

// ---------- CONFIGURA AQUÍ TUS DATOS DE RED ----------
const char* ssid     = "TU_RED_WIFI";
const char* password = "TU_CLAVE_WIFI";
// ------------------------------------------------------

// Guardaremos aquí las IPs activas que encontremos
String dispositivosActivos[254];
int totalActivos = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== Escáner de Hosts IoT - Paso 2 ===");
  conectarWiFi();

  IPAddress ipLocal = WiFi.localIP();
  Serial.print("IP del ESP32 en la red: ");
  Serial.println(ipLocal);

  escanearRed(ipLocal);

  Serial.println();
  Serial.println("=== Resultado del escaneo ===");
  Serial.print("Dispositivos activos encontrados: ");
  Serial.println(totalActivos);
  for (int i = 0; i < totalActivos; i++) {
    Serial.println(" - " + dispositivosActivos[i]);
  }
  Serial.println("Escaneo finalizado.");
}

void loop() {
  // No repetimos el escaneo automáticamente en esta etapa.
  // Cuando integremos el dashboard web, este ciclo podrá
  // repetirse cada cierto tiempo o al pulsar un botón en la web.
}

void conectarWiFi() {
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("¡Conectado!");
  } else {
    Serial.println();
    Serial.println("ERROR: No se pudo conectar al WiFi. Revisa SSID/clave.");
    while (true) { delay(1000); } // Detiene el programa
  }
}

void escanearRed(IPAddress ipLocal) {
  // Tomamos los primeros 3 octetos de la IP local (ej: 192.168.1)
  // y probamos con el último octeto de 1 a 254.
  IPAddress ipPrueba = ipLocal;

  Serial.println("Iniciando escaneo de la subred...");
  Serial.println("Esto puede tardar 1-2 minutos dependiendo de la red.");

  for (int host = 1; host <= 254; host++) {
    ipPrueba[3] = host;

    // No probamos hacer ping a nuestra propia IP
    if (ipPrueba == ipLocal) continue;

    bool responde = Ping.ping(ipPrueba, 1); // 1 intento por IP para ser rápidos

    if (responde) {
      String ipTexto = ipPrueba.toString();
      Serial.println("Dispositivo activo: " + ipTexto);
      dispositivosActivos[totalActivos] = ipTexto;
      totalActivos++;
    }

    // Pequeña pausa para no saturar la red ni el ESP32
    delay(20);
  }
}
