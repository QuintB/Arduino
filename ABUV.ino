//note to self: you might need to turn every instance of 26 and 27 aside from the first few to outPin and outPin2 or 13 and 26, respectively

////// Button
const int  buttonPin  = 27;
const int  outPin     = 13;
const int  outPin2    = 26;
//const int  alsPin     = 34;
const int  freq       = 5000;
const int  ledChannel = 0;
const int  resolution = 8;
const long debounce   = 200;

//Trackers
int state = HIGH;
int reading = LOW;
int previous = LOW;
int alsValue = 0;
long timeP = 0;

////// Wifi
// Load Wi-Fi library
#include <WiFi.h>

const char* ssid     = "wifi";
const char* password = "password";

//const char* ssid     = "Quint's iPhone";
//const char* password = "5ztA!nZEnA!^";
//
//const char* ssid     = "SCAD Wireless";
//const char* password = "";

// Webserver Stuff
WiFiServer server(80);
String header;

// Auxiliar variables to store the current output state
String outPinState = "off";
String outPin2State = "off";

// Time Trackers
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;





void setup() {

  ////// Button
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(outPin, OUTPUT);
  pinMode(outPin2, OUTPUT);

  //pinMode(alsPin, INPUT);
  //ledcAttachPin(outPin, ledChannel);
  Serial.begin(9600);

  ////// Wifi

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {

  ////// Button
  reading = digitalRead(buttonPin);
  //alsValue = analogRead(alsPin);
  //Serial.println("ALS Reading: " + String(alsValue));

  if (reading == HIGH && previous == LOW && millis() - timeP > debounce) {
    if (state == HIGH) {
      state = LOW;
      Serial.println("Off");
    } else {
      state = HIGH;
      timeP = millis();
      Serial.println("On");
    }
  }

  digitalWrite(outPin, state);
  digitalWrite(outPin2, state);

  previous = reading;
  Serial.println(reading);

  ////// Wifi

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              outPinState = "on";
              state = 1;
              digitalWrite(outPin, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              outPinState = "off";
              state = 0;
              digitalWrite(outPin, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              outPin2State = "on";
              state = 1;
              digitalWrite(outPin2, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              outPin2State = "off";
              state = 0;
              digitalWrite(outPin2, LOW);
            }

            String HTML = "<!DOCTYPE html>\
            <html>\
            <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
            <link rel=\"icon\" href=\"data:,\">\
            <link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css?family=Inter\">\
              <style>\
                html { font-family: \"Inter\"; font-weight: bold; display: inline-block; margin: 0px auto; text-align: center;}\
                .button  {background-color: #1285ef; border-radius: 17px; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\
                .button2 {background-color: #DDDDDD;}\
              </style>\
            </head>\
            </body>\
                <h1>ESP32 Web Server</h1>\
                <p>ABUV " + outPinState + "</p>\
                <p>NODE " + outPin2State + "</p>\
            </body>\
            </html>\
              ";

            client.print(HTML);

            // If the outPinState is off, it displays the ON button
            if (outPinState == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            // If the outPin2State is off, it displays the ON button
            if (outPin2State == "off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");


            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
