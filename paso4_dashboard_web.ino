/*
  PASO 4 - DASHBOARD WEB
  Proyecto: Escáner de vulnerabilidades IoT con ESP32

  Qué hace este código:
  1. Conecta el ESP32 a tu red WiFi.
  2. Escanea la subred (hosts) y luego los puertos comunes de cada host,
     igual que el Paso 3, pero AHORA guarda los resultados en memoria
     para mostrarlos en una página web.
  3. Levanta un servidor web en el propio ESP32 (usando WebServer.h,
     que ya viene incluida con el paquete ESP32 — no necesitas instalar
     nada nuevo).
  4. Desde cualquier celular o laptop conectado a la MISMA red WiFi,
     entras a la IP del ESP32 en el navegador y ves una tabla con:
       - IP del dispositivo
       - Puertos abiertos encontrados
       - Nivel de riesgo básico (según cuántos puertos sensibles tiene)

  Cómo usarlo:
  1. Sube este código (con tu SSID/clave ya configurados).
  2. Abre el Monitor Serie a 115200 baudios.
  3. Espera a que termine el escaneo (verás "Dashboard listo en: http://IP").
  4. Copia esa IP y ábrela en el navegador de tu celular o laptop
     (deben estar conectados a la misma red WiFi que el ESP32).
  5. Verás la tabla del dashboard. Puedes tomar captura de pantalla
     de esa página como evidencia para tu entrega.
*/

#include <WiFi.h>
#include <ESP32Ping.h>
#include <WiFiClient.h>
#include <WebServer.h>

// ---------- CONFIGURA AQUÍ TUS DATOS DE RED ----------
const char* ssid     = "TU_RED_WIFI";
const char* password = "TU_CLAVE_WIFI";
// ------------------------------------------------------

// Puertos comunes a revisar y qué servicio suelen representar
const int puertosComunes[] = {21, 22, 23, 80, 443, 8080};
const char* nombrePuerto[] = {"FTP", "SSH", "Telnet", "HTTP", "HTTPS", "HTTP-alt"};
const int totalPuertos = 6;

// Puertos considerados más sensibles para el nivel de riesgo
// (Telnet y FTP sin cifrar son históricamente los más explotados en IoT)
bool esPuertoSensible(int puerto) {
  return (puerto == 23 || puerto == 21);
}

// Estructura para guardar el resultado de cada dispositivo
struct Dispositivo {
  String ip;
  bool puertoAbierto[6];   // paralelo a puertosComunes[]
  int totalAbiertos;
  bool tieneSensible;
};

Dispositivo dispositivos[254];
int totalDispositivos = 0;

WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== Escáner IoT - Paso 4 (Dashboard Web) ===");
  conectarWiFi();

  IPAddress ipLocal = WiFi.localIP();
  Serial.print("IP del ESP32 en la red: ");
  Serial.println(ipLocal);

  Serial.println();
  Serial.println(">>> Escaneando red (hosts + puertos) <<<");
  escanearRedCompleta(ipLocal);

  Serial.println();
  Serial.println("=== Escaneo finalizado. Iniciando servidor web... ===");

  server.on("/", handleRoot);
  server.begin();

  Serial.println();
  Serial.print("Dashboard listo en: http://");
  Serial.println(ipLocal);
  Serial.println("Abre esa dirección en un navegador conectado a la misma red WiFi.");
}

void loop() {
  server.handleClient();
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

void escanearRedCompleta(IPAddress ipLocal) {
  IPAddress ipPrueba = ipLocal;

  Serial.println("Buscando dispositivos activos (esto puede tardar varios minutos)...");

  for (int host = 1; host <= 254; host++) {
    ipPrueba[3] = host;

    if (ipPrueba == ipLocal) continue;

    bool responde = Ping.ping(ipPrueba, 1);

    if (responde) {
      String ipTexto = ipPrueba.toString();
      Serial.println("Dispositivo activo: " + ipTexto + " -> escaneando puertos...");

      Dispositivo d;
      d.ip = ipTexto;
      d.totalAbiertos = 0;
      d.tieneSensible = false;

      for (int p = 0; p < totalPuertos; p++) {
        WiFiClient cliente;
        int puerto = puertosComunes[p];
        cliente.setTimeout(300);
        bool conectado = cliente.connect(ipPrueba, puerto);

        d.puertoAbierto[p] = conectado;

        if (conectado) {
          d.totalAbiertos++;
          if (esPuertoSensible(puerto)) {
            d.tieneSensible = true;
          }
          cliente.stop();
        }
        delay(10);
      }

      dispositivos[totalDispositivos] = d;
      totalDispositivos++;
    }

    delay(20);
  }
}

// Devuelve el texto y color del nivel de riesgo según lo encontrado.
// Recibe el ÍNDICE del dispositivo (no el struct directamente) para
// evitar un bug conocido del Arduino IDE al generar prototipos
// automáticos de funciones que usan tipos definidos por el usuario.
String nivelRiesgo(int indice, String &colorFondo) {
  Dispositivo &d = dispositivos[indice];
  if (d.tieneSensible) {
    colorFondo = "#f8d7da"; // rojo claro
    return "ALTO";
  } else if (d.totalAbiertos > 0) {
    colorFondo = "#fff3cd"; // amarillo claro
    return "MEDIO";
  } else {
    colorFondo = "#d4edda"; // verde claro
    return "BAJO";
  }
}

void handleRoot() {
  String html = "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Escaner IoT - Dashboard</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;background:#f4f6f8;margin:0;padding:20px;color:#222;}";
  html += "h1{font-size:22px;margin-bottom:4px;}";
  html += "p.sub{color:#555;margin-top:0;margin-bottom:20px;}";
  html += "table{border-collapse:collapse;width:100%;background:#fff;box-shadow:0 1px 3px rgba(0,0,0,0.1);}";
  html += "th,td{padding:10px 12px;text-align:left;border-bottom:1px solid #e0e0e0;font-size:14px;}";
  html += "th{background:#2c3e50;color:#fff;}";
  html += ".badge{padding:3px 10px;border-radius:12px;font-weight:bold;font-size:12px;}";
  html += ".resumen{display:flex;gap:16px;margin-bottom:20px;flex-wrap:wrap;}";
  html += ".tarjeta{background:#fff;padding:14px 20px;border-radius:8px;box-shadow:0 1px 3px rgba(0,0,0,0.1);}";
  html += ".tarjeta b{display:block;font-size:20px;}";
  html += "</style></head><body>";

  html += "<h1>Escaner de Vulnerabilidades IoT</h1>";
  html += "<p class='sub'>Proyecto academico - ESP32 | Red analizada en tiempo real</p>";

  // Tarjetas resumen
  int alto = 0, medio = 0, bajo = 0;
  for (int i = 0; i < totalDispositivos; i++) {
    String cf;
    String nivel = nivelRiesgo(i, cf);
    if (nivel == "ALTO") alto++;
    else if (nivel == "MEDIO") medio++;
    else bajo++;
  }

  html += "<div class='resumen'>";
  html += "<div class='tarjeta'>Dispositivos escaneados<b>" + String(totalDispositivos) + "</b></div>";
  html += "<div class='tarjeta'>Riesgo alto<b style='color:#c0392b;'>" + String(alto) + "</b></div>";
  html += "<div class='tarjeta'>Riesgo medio<b style='color:#b7950b;'>" + String(medio) + "</b></div>";
  html += "<div class='tarjeta'>Riesgo bajo<b style='color:#27ae60;'>" + String(bajo) + "</b></div>";
  html += "</div>";

  // Tabla de resultados
  html += "<table><tr><th>IP</th><th>Puertos abiertos</th><th>Nivel de riesgo</th></tr>";

  for (int i = 0; i < totalDispositivos; i++) {
    Dispositivo &d = dispositivos[i];
    String cf;
    String nivel = nivelRiesgo(i, cf);

    String listaPuertos = "";
    for (int p = 0; p < totalPuertos; p++) {
      if (d.puertoAbierto[p]) {
        if (listaPuertos.length() > 0) listaPuertos += ", ";
        listaPuertos += String(puertosComunes[p]) + " (" + String(nombrePuerto[p]) + ")";
      }
    }
    if (listaPuertos.length() == 0) listaPuertos = "Ninguno detectado";

    html += "<tr style='background:" + cf + "'>";
    html += "<td>" + d.ip + "</td>";
    html += "<td>" + listaPuertos + "</td>";
    html += "<td><span class='badge'>" + nivel + "</span></td>";
    html += "</tr>";
  }

  html += "</table>";
  html += "<p class='sub' style='margin-top:20px;'>Generado por ESP32 DevKit V1 - Escaner de vulnerabilidades IoT</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}
