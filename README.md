# Escáner de Vulnerabilidades IoT con ESP32

Prototipo de escáner de vulnerabilidades IoT para redes domésticas/educativas, desarrollado sobre el microcontrolador **ESP32 DevKit V1**, como parte de la macroactividad académica de la Unidad Educativa Particular Edward Joseph Flanagan (período 2025-2026).

El sistema descubre dispositivos activos en la red local, verifica un conjunto de puertos TCP comunes potencialmente inseguros en cada uno, y presenta los resultados en un **dashboard web embebido** servido directamente desde el propio ESP32, con una clasificación básica de nivel de riesgo (Bajo / Medio / Alto).

> Este repositorio corresponde a la **Fase 1** del proyecto. El diseño completo (incluyendo detección UPnP/SSDP, verificación de credenciales por defecto, almacenamiento en microSD y análisis CVSS v3.1) se describe en el documento de macroactividad; esos módulos están definidos como trabajo futuro (ver sección 7.5 del documento y el apartado [Trabajo futuro](#trabajo-futuro) más abajo).

## Índice

- [Funcionalidad implementada](#funcionalidad-implementada)
- [Requisitos](#requisitos)
- [Manual de instalación y uso](#manual-de-instalación-y-uso)
- [Estructura del repositorio](#estructura-del-repositorio)
- [Solución de problemas comunes](#solución-de-problemas-comunes)
- [Trabajo futuro](#trabajo-futuro)

## Funcionalidad implementada

- **Descubrimiento de hosts**: barrido de la subred local mediante ping ICMP (librería `ESP32Ping`) para identificar dispositivos activos.
- **Escaneo de puertos**: intento de conexión TCP sobre los puertos 21 (FTP), 22 (SSH), 23 (Telnet), 80 (HTTP), 443 (HTTPS) y 8080 (HTTP-alt) de cada host activo.
- **Dashboard web embebido**: el propio ESP32 sirve una página HTML (servidor `WebServer.h`) con una tabla de resultados: IP, puertos abiertos y nivel de riesgo, accesible desde cualquier navegador conectado a la misma red WiFi.
- **Clasificación de riesgo**: regla heurística simple — un dispositivo con puertos sensibles (Telnet/FTP) abiertos se marca en riesgo Alto; con otros puertos abiertos, riesgo Medio; sin puertos comunes abiertos, riesgo Bajo.

## Requisitos

### Hardware

- 1 placa ESP32 DevKit V1 (chip USB-serial CP2102 o CH340).
- Cable USB con transmisión de datos (no solo de carga).
- Red WiFi 2.4 GHz, con acceso para el ESP32 y para el dispositivo desde el cual se visualizará el dashboard.

### Software

- [Arduino IDE 2.x](https://www.arduino.cc/en/software)
- Paquete de placas **esp32** (Espressif Systems), instalado desde el Gestor de Tarjetas.
- Driver USB-serial correspondiente al chip de tu placa:
  - [CP210x (Silicon Labs)](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers)
  - CH340 (buscar "CH340 driver" según tu sistema operativo)
- Librería **ESP32Ping** ([marian-craciunescu/ESP32Ping](https://github.com/marian-craciunescu/ESP32Ping))

## Manual de instalación y uso

### 1. Preparar el entorno (una sola vez)

1. Instala Arduino IDE 2.x.
2. Ve a **Archivo > Preferencias** y en "URLs Adicionales de Gestor de Tarjetas" agrega:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Ve a **Herramientas > Placa > Gestor de Tarjetas**, busca `esp32` e instala el paquete de **Espressif Systems** (se recomienda la versión 2.0.17 si tienes problemas de descarga con versiones más nuevas).
4. Instala el driver CP210x o CH340 correspondiente a tu placa.
5. Instala la librería `ESP32Ping`:
   - Intenta primero desde **Sketch > Include Library > Manage Libraries**, buscando `ESP32Ping`.
   - Si no aparece, descárgala manualmente desde [github.com/marian-craciunescu/ESP32Ping](https://github.com/marian-craciunescu/ESP32Ping/archive/refs/heads/master.zip) y en Arduino IDE ve a **Sketch > Include Library > Add .ZIP Library...** y selecciona el archivo descargado. Reinicia Arduino IDE después de instalarla.

### 2. Verificar el hardware

1. Abre `firmware/paso1_test_blink/paso1_test_blink.ino`.
2. Conecta el ESP32 por USB.
3. En **Herramientas**: Placa = `ESP32 Dev Module`, Puerto = el puerto COM/ttyUSB donde aparece tu placa.
4. Sube el código (botón de flecha →).
5. El LED integrado de la placa debe parpadear cada segundo. Si esto ocurre, tu entorno y hardware están listos.

### 3. Ejecutar el escáner completo (dashboard web)

1. Abre `firmware/paso4_dashboard_web/paso4_dashboard_web.ino` — este es el firmware final e integrado (incluye descubrimiento de hosts + escaneo de puertos + dashboard web).
2. Edita las siguientes líneas con los datos de tu red WiFi:
   ```cpp
   const char* ssid     = "TU_RED_WIFI";
   const char* password = "TU_CLAVE_WIFI";
   ```
3. Verifica que el puerto seleccionado en **Herramientas > Puerto** sea el correcto (el correspondiente al chip CP210x/CH340, no un puerto Bluetooth).
4. Compila y sube el código.
5. Abre el **Monitor Serie** a **115200 baudios**. Verás:
   - La conexión a la red WiFi.
   - El descubrimiento de hosts activos en la subred.
   - El escaneo de puertos por cada host.
   - Al finalizar, el mensaje: `Dashboard listo en: http://<IP-del-ESP32>`
6. Abre esa dirección IP en el navegador de cualquier dispositivo (celular, laptop) conectado a la **misma red WiFi**.
7. Verás el dashboard con la tabla de dispositivos, puertos abiertos y nivel de riesgo.

> Los sketches `paso2_escaner_hosts` y `paso3_escaner_puertos` se incluyen por completitud didáctica: muestran el desarrollo incremental del proyecto (primero solo descubrimiento de hosts, luego descubrimiento + puertos vía Monitor Serie) antes de llegar al firmware final con dashboard web (`paso4_dashboard_web`).

## Estructura del repositorio

```
firmware/
├── paso1_test_blink/          Prueba básica de hardware (LED parpadeante)
├── paso2_escaner_hosts/       Descubrimiento de hosts activos (salida por Monitor Serie)
├── paso3_escaner_puertos/     Descubrimiento de hosts + escaneo de puertos (Monitor Serie)
└── paso4_dashboard_web/       Firmware final: hosts + puertos + dashboard web embebido
```

## Solución de problemas comunes

| Problema | Solución |
|---|---|
| `LED_BUILTIN was not declared in this scope` | Agregar `#define LED_BUILTIN 2` al inicio del sketch (bug conocido de algunos cores de placa ESP32). |
| `A serial exception error occurred: Write timeout` | Mantener presionado el botón físico **BOOT** de la placa mientras se sube el código; o bajar la velocidad en Herramientas > Upload Speed a 115200. |
| El puerto no aparece, o aparece como puerto Bluetooth | Instalar el driver CP210x o CH340 correspondiente al chip de tu placa (revisar en el Administrador de Dispositivos si aparece con advertencia). |
| `ESP32Ping.h: No such file or directory` tras instalar por ZIP | Cerrar y volver a abrir Arduino IDE por completo; si persiste, verificar que la carpeta `ESP32Ping` (sin sufijo `-master`) exista dentro de `Documentos/Arduino/libraries/`. |
| Error de compilación `'Dispositivo' was not declared in this scope` en `paso4_dashboard_web` | Ya corregido en este repositorio: las funciones que usan el `struct Dispositivo` reciben su índice numérico en lugar del struct directamente, evitando un bug del generador automático de prototipos de Arduino IDE. |

## Trabajo futuro

Conforme al diseño original del proyecto (documento de macroactividad, secciones 1-5), las siguientes fases incorporarán:

- Descubrimiento de servicios **UPnP/SSDP** mediante mensajes M-SEARCH (multicast UDP 239.255.255.250:1900).
- Verificación controlada de **credenciales por defecto**, sujeta a autorización institucional formal previa.
- Almacenamiento persistente de resultados en **tarjeta microSD** (formato JSON).
- Backend de análisis en **Python** con cálculo formal de severidad **CVSS v3.1** y generación automática de reportes en PDF/DOCX.
- Ampliación del alcance desde un segmento de red controlado hacia la totalidad de las subredes institucionales, previa autorización escrita de las autoridades de la Unidad Educativa Particular Edward Joseph Flanagan.

---

**Autor:** Giovanni William Paucar Guamán
**Proyecto académico** — Unidad Educativa Particular Edward Joseph Flanagan, período 2025-2026
