#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Configuración del punto de acceso
const char* ssid = "Mi_Red_AP";
const char* password = "12345678";

// Creación del servidor web
ESP8266WebServer server(80);

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
  html += "<p><a href=\"/ledOn\">Encender LED</a></p>";
  html += "<p><a href=\"/ledOff\">Apagar LED</a></p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleLedOn() {
  digitalWrite(LED_BUILTIN, LOW); // Encender LED (LOW porque el LED interno está invertido)
  server.send(200, "text/html", "<h1>LED Encendido</h1><p><a href=\"/\">Volver</a></p>");
}

void handleLedOff() {
  digitalWrite(LED_BUILTIN, HIGH); // Apagar LED
  server.send(200, "text/html", "<h1>LED Apagado</h1><p><a href=\"/\">Volver</a></p>");
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // Configurar el LED interno como salida
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Apagar LED

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

  // Iniciar el servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
}
