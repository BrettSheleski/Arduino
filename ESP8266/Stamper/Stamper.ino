#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "SHELLSOFT";
const char* password = "buckfutter";

const int inputPin = 0; //GPIO0
int inputValue = LOW;

const int outputPin = 2; // GPIO2
const bool isActiveLow = true;
bool isOn = false;

ESP8266WebServer server(80); //creating the server at port 80

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(inputPin, INPUT);

  isOn ? outputOn() : outputOff();

  initializeWifi();
}

void outputOn() {
  if (!isOn) {
    pinMode(outputPin, OUTPUT);

    digitalWrite(outputPin, isActiveLow ?  LOW : HIGH);
    isOn = true;
  }
}

void outputOff() {
  if (isOn) {
    pinMode(outputPin, INPUT);
    isOn = false;
  }
}

void outputToggle() {
  isOn ? outputOff() : outputOn();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleOn() {
  outputOn();

  sendStatusMessage();
}

void handleOff() {
  outputOff();

  sendStatusMessage();
}

void handleRoot() {
  sendStatusMessage();
}

void sendStatusMessage() {
  String message;
  if (isOn) {
    message = "{'output' : 'on'}";
  }
  else {
    message = "{'output' : 'off'}";
  }

  server.send(200, "application/json", message);
}

void initializeWifi() {

  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  readInput();

  server.handleClient();
}

void readInput() {
  int input = digitalRead(inputPin);

  if (input == HIGH && input != inputValue) {
    outputToggle();
  }

  inputValue = input;
}


