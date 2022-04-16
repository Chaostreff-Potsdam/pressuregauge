#include <ESP8266WiFi.h>
#include <Servo.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

Servo ObjServo;
static const int ServoGPIO = D4;
static const int beepPin = D3;
static const int beepPinGround = D2;

ESP8266WebServer server(80);
String header;
String valueString = String(0);
String pressureValueString = String(0);
int positon1 = 0;
int positon2 = 0;

const char DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP(192, 168, 112, 1);

String temp = "";

boolean isIp(String str) {
	for (int i = 0; i < str.length();i++) {
	int c = str.charAt(i);
		if (c != '.' && (c < '0' || c > '9')) {       
			return false;
     		}   
	}   
	return true;
}

String toStringIp(IPAddress ip) {   
	String res = "";
	for (int i = 0; i < 3; i++) {
		res += String((ip >> (8 * i)) & 0xFF) + ".";
   	}   
	res += String(((ip >> 8 * 3)) & 0xFF);
   	return res;
}

bool captivePortal() {   
	if (!isIp(server.hostHeader())) {
		Serial.println("Request redirected to captive portal");
     		server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
     		server.send ( 302, "text/plain", "");
 		server.client().stop();
 		return true;
	}   
	return false;
}

void handleNotFound() {   
   if (captivePortal())   {
      	   return;
   }   temp = "";
   // HTML Header   
   server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
   server.sendHeader("Pragma", "no-cache");
   server.sendHeader("Expires", "-1");
   server.setContentLength(CONTENT_LENGTH_UNKNOWN);
   // HTML Content   
   temp += "<! DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0,'>";
   temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
   temp += "body {background-color: powderblue;} </style>";
   temp += "<head><title>File not found</title></head>";
   temp += "<h2> 404 File Not Found</h2><br>";
   temp += "<h4>Debug Information:</h4><br>";
   temp += "<body>";
   temp += "URI: ";
   temp += server.uri();
   temp += "\nMethod: ";
   temp += ( server.method() == HTTP_GET ) ? "GET" : "POST";
   temp += "<br>Arguments: ";
   temp += server.args();
   temp += "\n";
   for ( uint8_t i = 0; i < server.args(); i++ ) {     
	temp += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
   }  

   temp += "<br>Server Hostheader: " + server.hostHeader();
   for ( uint8_t i = 0; i < server.headers(); i++ ) {
	   temp += " " + server.headerName ( i ) + ": " + server.header ( i ) + "\n<br>";
   }   
   temp += "</table></form><br><br><table border=2 bgcolor = white width = 500 cellpadding =5 ><caption><p><h2>You may want to browse to:</h2></p></caption>";
   temp += "<tr><th>";
   temp += "<a href='/'>Main Page</a><br>";
   temp += "<a href='/wifi'>WIFI Settings</a><br>";
   temp += "</th></tr></table><br><br>";
   temp += "</body></html>";
   server.send ( 404, "", temp );
   server.client().stop();
   temp = "";
}

void handleRoot() {
   temp = "";
   short PicCount = 0;
   byte ServArgs = 0;
   //Building Page   // HTML Header   
   server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
   server.sendHeader("Pragma", "no-cache");
   server.sendHeader("Expires", "-1");
   server.setContentLength(CONTENT_LENGTH_UNKNOWN);
   // HTML Content   
   server.send ( 200, "text/html", temp );
   // Speichersparen - Schon mal dem Cleint senden   
   temp = "";
   temp += "<! DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0,'>";
   server.sendContent(temp);
   temp = "";
   temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
   server.sendContent(temp);
   temp = "";
   temp += "<head><title>Captive Portal</title></head>";
   temp += "<h2>Captive Portal</h2>";
   temp += "<body>";
   server.sendContent(temp);
   temp = "";
    temp += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    temp += "<title>Servo Angle Controller</title>";
    temp += "<link rel=\"icon\" href=\"data:,\">";
    temp += "<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}";
    temp += ".headertext{ font-weight:bold; font-family:Arial; text-align: center; color: brown ;}";
    temp += ".slider { width: 500px; }</style>";
    temp += "</head><body><h1><u><div class = \"headertext\"> Smart Gas Druck-Einsteller</u></h1>";
    temp += "<p><h3>Aktueller Druck</h3><span id=\"servoPos\"></span> Bar</p>";
    temp += "<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\" + valueString + 	\"/>";
    temp += "<script>var slider = document.getElementById(\"servoSlider\");"
        "var servoP = document.getElementById(\"servoPos\"); "
        "servoP.innerHTML = Math.round(slider.value / 180.0 * 300) ;"
        "slider.oninput = function() { slider.value = this.value; "
        "servoP.innerHTML = Math.round(this.value / 180.0 * 300); };"
        "function servo(pos) { "
        "var request = new XMLHttpRequest();"
        "request.open('GET', (\"/?value=\" + pos + \"&\"), true );"
        "request.send();"
        "}</script>";
   temp += "</body></html>";
   temp += "</body></html>";
   server.sendContent(temp);
   temp = "";
   server.client().stop();
}


void InitializeHTTPServer(){
   server.on("/", handleRoot);
   server.on("/generate_204", handleRoot);
   server.on("/favicon.ico", handleRoot);
   server.on("/fwlink", handleRoot);
   server.on("/generate_204", handleRoot);
   server.onNotFound(handleNotFound);
   //server.collectHeaders(Headers, sizeof(Headers)/ sizeof(Headers[0]));
   server.begin();
}

void setup() {
  Serial.begin(115200);
  pinMode(beepPinGround, OUTPUT);
  digitalWrite(beepPinGround, LOW);
  pinMode(beepPin, OUTPUT);
  digitalWrite(beepPin, LOW);
  ObjServo.attach(ServoGPIO, 500, 2800);
  Serial.print("Making connection to ");
  
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Drucksensor");
  //WiFi.begin(ssid, password);
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //  Serial.print(".");
  //}
  //Serial.println("");
  //Serial.println("WiFi connected.");
  //Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());
  dnsServer.start(DNS_PORT, "*", apIP);
  server.begin();
  InitializeHTTPServer();
}



void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  /*if (client) {
    Serial.println("Hallo wir haben einen Besucher");
    String header = client.readStringUntil('\r');
    temp += "HTTP/1.1 200 OK");
    temp += "Content-type:text/html");
    temp += "Connection: close");
    temp += );

    temp += "<!DOCTYPE html><html>");
    temp += "<head><meta name=\"viewport\" "
                   "content=\"width=device-width, initial-scale=1\">");
    temp += "<title>Servo Angle Controller</title>");
    temp += "<link rel=\"icon\" href=\"data:,\">");
    temp += "<style>body { text-align: center; font-family: \"Trebuchet "
                   "MS\", Arial; margin-left:auto; margin-right:auto;}");
    temp += ".headertext{ font-weight:bold; font-family:Arial ; "
                   "text-align: center; color: brown ;}");
    temp += ".slider { width: 500px; }</style>");
    temp += "</head><body><h1><u><div class = \"headertext\"> Smart Gas "
                   "Druck-Einsteller</u></h1>");
    temp += "<p><h3>Aktueller Druck</h3><span "
                   "id=\"servoPos\"></span> Bar</p>");
    temp += 
        "<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" "
        "id=\"servoSlider\" onchange=\"servo(this.value)\" value=\"" +
        valueString + "\"/>");
    temp += 
        "<script>var slider = document.getElementById(\"servoSlider\");"
        "var servoP = document.getElementById(\"servoPos\"); "
        "servoP.innerHTML = Math.round(slider.value / 180.0 * 300) ;"
        "slider.oninput = function() { slider.value = this.value; "
        "servoP.innerHTML = Math.round(this.value / 180.0 * 300); };"
        "function servo(pos) { "
        "var request = new XMLHttpRequest();"
        "request.open('GET', (\"/?value=\" + pos + \"&\"), true );"
        "request.send();"
        "}</script>");
    temp += "</body></html>");

    if (header.indexOf("GET /?value=") >= 0) {
      positon1 = header.indexOf('=');
      positon2 = header.indexOf('&');
      valueString = header.substring(positon1 + 1, positon2);

      const int angle = valueString.toInt();
      ObjServo.write(angle);
      if (angle < 30) {
          digitalWrite(beepPin, HIGH);
      } else {
          digitalWrite(beepPin, LOW);
      }

      pressureValueString = valueString.toInt() / 180.0 * 2000 + 500;
      Serial.println(valueString);
    }

    header = "";*/
}
