#include <ESP8266WebServer.h>
//#include <ESP8266WiFi.h>

const char* ssid = "SHELLSOFT";
const char* password = "buckfutter";

const int inputPin = 0; //GPIO0

const int outputPin = 2; // GPIO2
const bool isActiveLow = true;
bool isOn = false;

//WiFiServer server(80);
ESP8266WebServer server(80); //creating the server at port 80

void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(inputPin, INPUT);

  isOn ? outputOn() : outputOff();

  initializeWifi();
}

void handleWifiPost(){
  
}

void initializeWifi() {
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //WiFi.begin(ssid, password);
  server.begin

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request

  int value = LOW;
  if (request.indexOf("/LED=ON") != -1) {
    digitalWrite(outputPin, HIGH);
    value = HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1) {
    digitalWrite(outputPin, LOW);
    value = LOW;
  }

  // Set ledPin according to the request
  //digitalWrite(ledPin, value);

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); // do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("Led pin is now: ");

  if (value == HIGH) {
    client.print("On");
  } else {
    client.print("Off");
  }
  client.println("<br><br>");
  client.println("Click <a href=\"/LED=ON\">here</a> turn the LED on pin 2 ON<br>");
  client.println("Click <a href=\"/LED=OFF\">here</a> turn the LED on pin 2 OFF<br>");
  client.println("</html>");

  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
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

