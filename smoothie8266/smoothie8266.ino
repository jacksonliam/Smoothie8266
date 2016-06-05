#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library

#include <ESP8266mDNS.h>          // Allows us to get the smootie.local domain name

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


#include "smoothie8266data.h"

// Baudrate for smoothieboard
#define SMOOTIE_BAUD ( 115200 )
#define SMOOTHIE_AP_NAME "Smoothie8266"
#define SMOOTIE_MDNS_NAME "smoothie"

//Strint constants (stored in PGM)
const char mediatype_html[] PROGMEM = "text/html";

//Server object
ESP8266WebServer server ( 80 );

/**
 * Handler function for root "/" URI. Serves index.html.
 */
void handleRoot() {
  server.send_P( 200, mediatype_html, data_index_html );
}

/**
 * Handler function for JS file "/function.js"
 */
void handleJS() {
  server.send_P( 200, PSTR("text/javascript"), data_functions_js );
}


/**
 * Handles when the client requests an address we don't know - 404
 */
 void handleNotFound() {
  /* might be useful:
  message += server.uri();
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += server.args();

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  */

  server.send_P(404, mediatype_html, PSTR("<html><body bgcolor='white'><center><h1>404 - file not found</h1><h3>Go <a href='/'>here</a> instead.</h3></center></body></html>"));
}

/**
 * Handle when a command is sent to the server and the client want's the response.
 */
void handleCommand() {
  String message = "";
  String response = "";
  
  message += server.arg("plain");
  if(!message.endsWith("\n")){
    message += "\n";
  }

  //Skip over any crap in the pipe.
  while(Serial.available()) {
    Serial.read();
  }
  //Send the command.
  Serial.print(message);
  Serial.flush();
  //Skip over the response.
  while(Serial.available()) {
    response += Serial.readStringUntil('\n');
  }

  server.send(200, "text/plain", response);
}

/**
 * Handle when a command is send to the server but the client doesn't want any response.
 */
void handleCommandSilent() {
  String message = server.arg("plain");
  if(!message.endsWith("\n")){
    message += "\n";
  }

  //Skip over any crap in the pipe.
  while(Serial.available()) {
    Serial.read();
  }
  //Send the command.
  Serial.print(message);
  Serial.flush();
  //Skip over the response.
  while(Serial.available()) {
    Serial.read();
  }

  server.send(200);
}

/**
 * Test server input/output
 */
 void handleTest() {
  String message = "URI: ";
  message += server.uri();
  message += "method";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "args: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send(200, "text/plain", message);
 }

/**
 * Arduino setup function. Handles config of the web server, mDNS service and serial port.
 */
void setup() {
  // Setup serial first, because we don't want wifi messages being spaffed out.
  Serial.begin(SMOOTIE_BAUD, SERIAL_8N1);
  
  // setup wifi connection
  WiFiManager wifiManager;
  
  // Fetches ssid and pass from eeprom and tries to connect.
  // If it does not connect it starts an access point with the specified name,
  // then goes into a blocking loop awaiting configuration.
  wifiManager.autoConnect(SMOOTHIE_AP_NAME);

  // Setup mDNS so that IP can be discovered.
  MDNS.begin(SMOOTIE_MDNS_NAME);

  // Setup server object
  server.on ( "/", handleRoot );
  server.on ( "/index.htm", handleRoot );
  server.on ( "/functions.js", handleJS );
  server.on ( "/command", handleCommand );
  server.on ( "/command_silent", handleCommandSilent );
  server.on ( "/test", handleTest );
  server.onNotFound ( handleNotFound );

  // Start the server
  server.begin();

  // Add the webserver to mDNS service discovery
  MDNS.addService("http", "tcp", 80);
}

/**
 * Arduino loop function. As the name suggests is called in a loop. 
 * It's only job is to wait for requests and handle them as they come in.
 */
void loop() {
  server.handleClient();
}
