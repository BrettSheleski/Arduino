#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

const char* ssid = "SHELLSOFT";
const char* password = "buckfutter";

const int inputPin = 0; //GPIO0
const int inputSignal = LOW;
int previousInputValue = !inputSignal;

const int outputPin = 2; // GPIO2
const bool isActiveLow = true;
bool isOn = false;

const unsigned int webserverPort = 80;
ESP8266WebServer server(webserverPort);

unsigned long previousMillis = 0; // last time update

// set inerval to 5 minutes
long interval = 300000; // interval at which to do something (milliseconds)

///
/// UDP Server Info
///

WiFiUDP Udp;
byte packetBuffer[512]; //buffer to hold incoming and outgoing packets

const char* friendlyName = "Stamper";           // uPNP Friendly Name
const char* serialNumber = "221517K0101768";                  // anything will do
const char* uuid = "348cc43d-d8d8-4237-b266-ee9d80ed5413";    // anything will do

const IPAddress upnpMulticastIPAddress(239, 255, 255, 250);

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(inputPin, INPUT);

  isOn ? outputOn() : outputOff();

  initializeWifi();
}

char* getDateString()
{
  //Doesn't matter which date & time, will work
  //Optional: replace with NTP Client implementation
  return "Wed, 29 Jun 2016 00:13:46 GMT";
}

void upnpAdvertise()
{
  Serial.println("UPnP Advertise");

  //This is absolutely neccessary as Udp.write cannot handle IPAddress or numbers correctly like Serial.print
  IPAddress myIP = WiFi.localIP();
  char ipChar[20];
  snprintf(ipChar, 20, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
  char portChar[7];
  snprintf(portChar, 7, ":%d", webserverPort);

/*
NOTIFY * HTTP/1.1
HOST: 239.255.255.250:1900
CACHE-CONTROL: max-age = seconds until advertisement expires
LOCATION: URL for IPP Printer with 'ipp' scheme
NT: search target
NTS: ssdp:alive
SERVER: OS / version, IPP / 1.1, product / version
USN: advertisement UUID
*/
 

  Udp.beginPacket(upnpMulticastIPAddress, 1900);
  Udp.write("NOTIFY * HTTP/1.1\r\n");
  Udp.write("HOST: 239.255.255.250:1900\r\n");
  Udp.write("CACHE-CONTROL: max-age=300\r\n");
  Udp.write("LOCATION: ");
  Udp.write("http://");
  Udp.write(ipChar);
  Udp.write(portChar);
  Udp.write("/setup.xml\r\n");
  Udp.write("NT: search target \r\n");
  Udp.write("NTS:ssdp:alive\r\n");
  Udp.write("SERVER: Unspecified, UPnP/1.0, Unspecified\r\n");
  Udp.write("USN: uuid:Socket-1_0-");
  Udp.write(serialNumber);
  Udp.write("::urn:Sheleski:device:**\r\n");
  Udp.write("\r\n");
  Udp.endPacket();
}

void responseToSearchUdp(IPAddress& senderIP, unsigned int senderPort) 
{
  Serial.println("responseToSearchUdp");

  //This is absolutely neccessary as Udp.write cannot handle IPAddress or numbers correctly like Serial.print
  IPAddress myIP = WiFi.localIP();
  char ipChar[20];
  snprintf(ipChar, 20, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
  char portChar[7];
  snprintf(portChar, 7, ":%d", webserverPort);

  Udp.beginPacket(senderIP, senderPort);
  Udp.write("HTTP/1.1 200 OK\r\n");
  Udp.write("CACHE-CONTROL: max-age=86400\r\n");
  Udp.write("DATE: ");
  Udp.write(getDateString());
  Udp.write("\r\n");
  Udp.write("EXT:\r\n");
  Udp.write("LOCATION: ");
  Udp.write("http://");
  Udp.write(ipChar);
  Udp.write(portChar);
  Udp.write("/setup.xml\r\n");
  Udp.write("OPT: \"http://schemas.upnp.org/upnp/1/0/\"); ns=01\r\n");
  Udp.write("01-NLS: ");
  Udp.write(uuid);
  Udp.write("\r\n");
  Udp.write("SERVER: Unspecified, UPnP/1.0, Unspecified\r\n");
  Udp.write("X-User-Agent: redsonic\r\n");
  Udp.write("ST: urn:Belkin:device:**\r\n");
  Udp.write("USN: uuid:Socket-1_0-");
  Udp.write(serialNumber);
  Udp.write("::urn:Belkin:device:**\r\n");
  Udp.write("\r\n");
  Udp.endPacket();
}

void UdpMulticastServerLoop()
{
  int numBytes = Udp.parsePacket();
  if (numBytes <= 0)
    return;

  IPAddress senderIP = Udp.remoteIP();
  unsigned int senderPort = Udp.remotePort();
  
  // read the packet into the buffer
  Udp.read(packetBuffer, numBytes); 

  // print out the received packet
  //Serial.write(packetBuffer, numBytes);

  // check if this is a M-SEARCH for WeMo device
  String request = String((char *)packetBuffer);
  int mSearchIndex = request.indexOf("M-SEARCH");
  int mBelkinIndex = request.indexOf("urn:Belkin:device:");   //optional
  if (mSearchIndex < 0 || mBelkinIndex < 0)
    return;

  // send a reply, to the IP address and port that sent us the packet we received
  responseToSearchUdp(senderIP, senderPort);
}

void handleSetupXml()
{
  Serial.println("handleSetupXml");
    
  String body = "<?xml version=\"1.0\"?>\r\n";
  body += "<root>\r\n";
  body += "  <device>\r\n";
  body += "    <deviceType>urn:OriginallyUS:device:controllee:1</deviceType>\r\n";
  body += "    <friendlyName>";
  body += friendlyName;
  body += "</friendlyName>\r\n";
  body += "    <manufacturer>Belkin International Inc.</manufacturer>\r\n";
  body += "    <modelName>Emulated Socket</modelName>\r\n";
  body += "    <modelNumber>3.1415</modelNumber>\r\n";
  body += "    <UDN>uuid:Socket-1_0-";
  body += serialNumber;
  body += "</UDN>\r\n";
  body += "  </device>\r\n";
  body += "</root>";

  String header = "HTTP/1.1 200 OK\r\n";
  header += "Content-Type: text/xml\r\n";
  header += "Content-Length: ";
  header += body.length();
  header += "\r\n";
  header += "Date: ";
  header += getDateString();
  header += "\r\n";
  header += "X-User-Agent: redsonic\r\n";
  header += "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n";
  header += "connection: close\r\n";
  header += "LAST-MODIFIED: Sat, 01 Jan 2000 00:00:00 GMT\r\n";
  header += "\r\n";
  header += body;

  Serial.println(header);
  
  server.sendContent(header);
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

  server.on("/setup.xml", handleSetupXml);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  readInput();

  server.handleClient();

  UdpMulticastServerLoop();   //UDP multicast receiver

  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis > interval) {
     previousMillis = currentMillis;  

     upnpAdvertise();
  }
}

void readInput() {
  int currentInputValue = digitalRead(inputPin);

  if (currentInputValue == inputSignal && currentInputValue != previousInputValue) {
    outputToggle();
  }

  previousInputValue = currentInputValue;
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
