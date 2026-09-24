/*
  PASO 3 - ESCÁNER DE PUERTOS
  Proyecto: Escáner de vulnerabilidades IoT con ESP32

  Qué hace este código:
  1. Conecta el ESP32 a tu red WiFi (igual que el Paso 2).
  2. Escanea la subred para encontrar dispositivos activos (igual que el Paso 2).
  3. NUEVO: por cada dispositivo activo, intenta abrir una conexión TCP
     a una lista de puertos comunes (21, 22, 23, 80, 443, 8080).
  4. Si logra conectar, ese puerto está ABIERTO en ese dispositivo,
     lo cual puede indicar un servicio expuesto (web, FTP, SSH, Telnet).
  5. Imprime en el Monitor Serie un resumen: IP y qué puertos tiene abiertos.

  No requiere librerías nuevas además de ESP32Ping (usa WiFi.h y
  WiFiClient.h, que ya vienen incluidas con el paquete ESP32 que
  instalaste).

  Nota educativa: esto es exactamente lo que hace Nmap cuando escaneas
  puertos TCP con "connect scan" (-sT), solo que aquí lo hace el propio
  ESP32 en vez de tu computador.
*/

#include <WiFi.h>
#include <ESP32Ping.h>
#include <WiFiClient.h>

// ---------- CONFIGURA AQUÍ TUS DATOS DE RED ----------
const char* ssid     = "TU_RED_WIFI";
const char* password = "TU_CLAVE_WIFI";
// ------------------------------------------------------

// Puertos comunes a revisar y qué servicio suelen representar
const int puertosComunes[] = {21, 22, 23, 80, 443, 8080};
const char* nombrePuerto[] = {"FTP", "SSH", "Telnet", "HTTP", "HTTPS", "HTTP-alt"};
const int totalPuertos = 6;

// Guardamos aquí las IPs activas que encontremos
String dispositivosActivos[254];
int totalActivos = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== Escáner IoT - Paso 3 (Hosts + Puertos) ===");
  conectarWiFi();

  IPAddress ipLocal = WiFi.localIP();
  Serial.print("IP del ESP32 en la red: ");
  Serial.println(ipLocal);

  Serial.println();
  Serial.println(">>> FASE 1: Descubrimiento de hosts <<<");
  escanearRed(ipLocal);

  Serial.println();
  Serial.print("Dispositivos activos encontrados: ");
  Serial.println(totalActivos);

  Serial.println();
  Serial.println(">>> FASE 2: Escaneo de puertos por dispositivo <<<");
  for (int i = 0; i < totalActivos; i++) {
    escanearPuertos(dispositivosActivos[i]);
  }

  Serial.println();
  Serial.println("=== Escaneo completo finalizado ===");
}

void loop() {
  // El escaneo corre una sola vez en setup().
  // Presiona el botón EN/RESET del ESP32 para volver a ejecutarlo.
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
    while (true) { delay(1000); }
  }
}

void escanearRed(IPAddress ipLocal) {
  IPAddress ipPrueba = ipLocal;

  Serial.println("Buscando dispositivos activos (esto puede tardar 1-2 min)...");

  for (int host = 1; host <= 254; host++) {
    ipPrueba[3] = host;

    if (ipPrueba == ipLocal) continue;

    bool responde = Ping.ping(ipPrueba, 1);

    if (responde) {
      String ipTexto = ipPrueba.toString();
      Serial.println("Dispositivo activo: " + ipTexto);
      dispositivosActivos[totalActivos] = ipTexto;
      totalActivos++;
    }

    delay(20);
  }
}

void escanearPuertos(String ipTexto) {
  IPAddress ip;
  ip.fromString(ipTexto);

  Serial.println();
  Serial.println("Escaneando puertos en: " + ipTexto);

  bool algunoAbierto = false;

  for (int p = 0; p < totalPuertos; p++) {
    WiFiClient cliente;
    int puerto = puertosComunes[p];

    // Intentamos conectar con un timeout corto para no demorar demasiado
    cliente.setTimeout(300); // milisegundos de espera por intento
    bool conectado = cliente.connect(ip, puerto);

    if (conectado) {
      Serial.println("  [ABIERTO] Puerto " + String(puerto) +
                      " (" + String(nombrePuerto[p]) + ")");
      algunoAbierto = true;
      cliente.stop();
    }

    delay(10);
  }

  if (!algunoAbierto) {
    Serial.println("  Ningún puerto común abierto detectado.");
  }
}
