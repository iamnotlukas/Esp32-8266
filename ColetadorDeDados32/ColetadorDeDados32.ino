#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <LittleFS.h>
#include <DNSServer.h>

const char* ssid = "WIFI TUMIARU";
const char* password = "";

AsyncWebServer server(80);
DNSServer dnsServer;

void getIndex(AsyncWebServerRequest *request) {
 if (LittleFS.exists("/index.html")) {
  Serial.println("O arquivo index existe!");
  request->send(LittleFS, "/index.html", "text/html");
 } else {
  Serial.println("o arquivo nao existe");
  request->send(200, "text/plain", "Nao foi possível abrir o arquivo index.html");
 }
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
 Serial.printf("Writing file: %s\n", path);
 File file = fs.open(path, FILE_APPEND);
 if (!file) {
  Serial.println("Failed to open file for writing");
  return;
 }
 if (file.println(message)) {
  Serial.println("File written");
 } else {
  Serial.println("Write failed");
 }
 file.close();
}

void setupServer() {
 Serial.println("setupServer");
 server.on("/", HTTP_GET, getIndex);
 server.on("/get", HTTP_GET, [](AsyncWebServerRequest * request) {
  String cpf;
  String birthdate;
  String dataToSave;
  // GET cpf value on <ESP_IP>/get?cpf=<cpf>
  if (request->hasParam("cpf")) {
    cpf = request->getParam("cpf")->value();
  } else {
    request->send(200, "text/text", "CPF não fornecido");
    return;
  }
  // GET birthdate value on <ESP_IP>/get?birthdate=<birthdate>
  if (request->hasParam("birthdate")) {
    birthdate = request->getParam("birthdate")->value();
  } else {
    request->send(200, "text/text", "Data de Nascimento não fornecida");
    return;
  }

  dataToSave = "CPF:\n" + cpf + "\nData de Nascimento:\n" + birthdate + "\n---------------------\n";
  writeFile(LittleFS, "/dados.txt", dataToSave.c_str());

  request->send(200, "text/text", "Dados salvos com sucesso!");
 });

 server.on("/dados", HTTP_GET, [](AsyncWebServerRequest * request) {
  if (LittleFS.exists("/dados.txt")) {
    request->send(LittleFS, "/dados.txt", "text/plain");
  } else {
    request->send(200, "text/plain", "Arquivo dados.txt não encontrado");
  }
 });

 server.begin();
}

void setup(void) {
 Serial.begin(115200);
 LittleFS.begin();

 WiFi.mode(WIFI_AP); // Configura o ESP32 para funcionar como um ponto de acesso
 WiFi.softAP(ssid, password); // Configura o SSID e a senha do ponto de acesso

 dnsServer.start(53, "*", WiFi.softAPIP());

 Serial.print("Ponto de acesso criado. Endereço IP: ");
 Serial.println(WiFi.softAPIP());

 setupServer();
 Serial.println("Servidor iniciado");
}

void loop(void) {
 dnsServer.processNextRequest();
}
