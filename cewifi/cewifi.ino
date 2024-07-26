#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <string>
#include <sstream>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuración del punto de acceso
const char* ssid = "Mi_Red_AP";
const char* password = "12345678";

// Definiciones de pantalla y SD
#define SD_CS_PIN D8
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
const int lineHeight = 8;
int yPos = 0;
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ESP8266WebServer server(80);

int contadorCaracterSerialRead = 0;
File myFile;
bool habilitacionComuni = false;
bool pedidoEventos = false;
bool grabandoEventos = false;
bool paquete = true;

void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background-color: #f0f0f0; }";
  html += "h1 { color: #333; }";
  html += "a { display: inline-block; margin: 10px; padding: 15px 25px; font-size: 20px; color: #fff; background-color: #007BFF; text-decoration: none; border-radius: 5px; }";
  html += "a:hover { background-color: #0056b3; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<h1>¡Hola Mundo!</h1>";
  html += "<p><a href=\"/ledOn\">Conectar a MCT </a></p>";
  html += "<p><a href=\"/ledOff\">Apagar LED</a></p>";
  html += "<p><a href=\"/eventos\">Pedir Eventos</a></p>";
  html += "<p><a href=\"/download\">Descargar Archivo</a></p>"; // Añadir un enlace para descargar el archivo
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleDownload() {
  if (SD.exists("datalog.txt")) {
    File downloadFile = SD.open("datalog.txt", FILE_READ);
    if (downloadFile) {
      server.streamFile(downloadFile, "text/plain");
      downloadFile.close();
    } else {
      server.send(500, "text/plain", "Error al abrir el archivo para la descarga.");
    }
  } else {
    server.send(404, "text/plain", "Archivo no encontrado.");
  }
}

void handleLedOnEventos() {
  pedidoEventos = true;
  server.send(200, "text/html", "<h1>Pedido de eventos enviado</h1><p><a href=\"/\">Volver</a></p>");
}

void handleLedOn() {
  habilitacionComuni = true;
  server.send(200, "text/html", "<h1>Conexión habilitada</h1><p><a href=\"/\">Volver</a></p>");
}

void handleLedOff() {
  habilitacionComuni = false;
  server.send(200, "text/html", "<h1>Conexión deshabilitada</h1><p><a href=\"/\">Volver</a></p>");
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // Configurar el LED interno como salida
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Iniciar el punto de acceso
  Serial.println();
  Serial.print("Configurando punto de acceso...");
  WiFi.softAP(ssid, password);

  // Mostrar la IP del AP
  Serial.println("Punto de acceso configurado");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Configurar los manejadores de rutas
  server.on("/", handleRoot);
  server.on("/ledOn", handleLedOn);
  server.on("/ledOff", handleLedOff);
  server.on("/eventos", handleLedOnEventos);
  server.on("/download", handleDownload); // Añadir el manejador de descarga

  // Iniciar el servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  // Inicializar la tarjeta SD
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Error inicializando la tarjeta SD");
    return;
  }
  Serial.println("Tarjeta SD inicializada");

  // Inicializar la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // No continuar
  }
  display.display();
  delay(2000); // Pausa para permitir que la pantalla se inicialice
  display.clearDisplay();
}

void loop() {
  server.handleClient();
  if (habilitacionComuni) {
    ejecutarDespuesDeTiempo(enviarMCT, 1000);
  }
  if (pedidoEventos) {
    ComunicacionMCT("10 02 80 01 04 40 02 10 03 54 b3");
    pedidoEventos = false;
  }

  if (Serial.available()) {
    String receivedString = Serial.readString();
    int cantidadString = receivedString.length();
    if (grabandoEventos) {
      GuardarDatos(receivedString);
    }

    if (cantidadString > 19) {
      contadorCaracterSerialRead = 0;
    } else {
      contadorCaracterSerialRead++;
    }

    testdrawchar(contadorCaracterSerialRead);

    if (contadorCaracterSerialRead > 20) {
      testdrawchar("Conexion Finalizada");
      contadorCaracterSerialRead = 0;
      paquete = true;
      grabandoEventos = false;
    }
  }
}

// Funciones
void GuardarDatos(String receivedString) {
  String hexData = "";
  for (int i = 0; i < receivedString.length(); i++) {
    char c = receivedString.charAt(i);
    String hexByte = String(c, HEX);
    if (hexByte.length() == 1) {
      hexByte = "0" + hexByte; // Asegura que siempre tenga dos dígitos
    }
    hexData += hexByte;
  }

  String formattedHexData = "";
  if (paquete) {
    formattedHexData = "M";
    paquete = false;
  }
  for (int i = 0; i < hexData.length(); i += 2) {
    formattedHexData += hexData.substring(i, i + 2) + " ";
  }

  myFile = SD.open("datalog.txt", FILE_WRITE);
  if (myFile) {
    testdrawchar("Guardando............");
    testdrawchar(formattedHexData);
    display.clearDisplay();
    myFile.print(formattedHexData);
    myFile.println();
    myFile.close();
  } else {
    Serial.println("Error al escribir en el archivo");
    while (1); // Detener la ejecución en caso de error
  }
}

void ComunicacionMCT(const std::string& dato) {
  std::istringstream iss(dato);
  std::vector<uint8_t> valores;
  int valor;
  while (iss >> std::hex >> valor) {
    valores.push_back(valor);
  }
  Serial.write(valores.data(), valores.size());
}

void enviarMCT() {
  std::string dato = "10 06";
  ComunicacionMCT(dato);
}

void ejecutarDespuesDeTiempo(void (*funcionAEjecutar)(), unsigned long tiempo) {
  static unsigned long tiempoInicio = 0;
  static bool ejecutando = false;

  if (!ejecutando) {
    tiempoInicio = millis();
    ejecutando = true;
  }

  if (millis() - tiempoInicio >= tiempo) {
    funcionAEjecutar();
    ejecutando = false;
  }
}

template <typename T>
void testdrawchar(T valor) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  if (yPos + lineHeight >= SCREEN_HEIGHT) {
    display.clearDisplay();
    yPos = 0;
  }

  display.setCursor(0, yPos);
  display.print(valor);
  yPos += lineHeight;
  display.display();
}
