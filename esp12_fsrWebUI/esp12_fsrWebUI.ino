#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>

//define pins
#define SDA_PIN 4
#define SCL_PIN 5

const int16_t I2C_PRIMARY = 0x42;
const int16_t I2C_SECONDARY = 0x08;

//neetwork details
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

//ip details
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

uint8_t LED1pin = D6;
bool LED1status = LOW;

unsigned int returnValue = 0;
byte highB;
byte lowB;

//containers for read values
unsigned int a0read = 0;
unsigned int a1read = 0;
unsigned int a2read = 0;
unsigned int a3read = 0;

//containers for chosen values
unsigned int a0choose = 250;
unsigned int a1choose = 250;
unsigned int a2choose = 250;
unsigned int a3choose = 250;

//containers for threshold values
unsigned int a0thresh = 250;
unsigned int a1thresh = 250;
unsigned int a2thresh = 250;
unsigned int a3thresh = 250;

//array for write values
byte writeBytes[8];

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN, I2C_PRIMARY);
  Serial.begin(115200);
  pinMode(LED1pin, OUTPUT);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/read_sensors", handle_read_sensors);
  server.on("/write_thresholds", handle_write_thresholds);
  server.on("/read_thresholds", handle_read_thresholds);
  server.on("/color_prev", handle_color_prev);
  server.on("/color_next", handle_color_next);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  if(LED1status)
  {digitalWrite(LED1pin, HIGH);}
  else
  {digitalWrite(LED1pin, LOW);}
  delay(1);
}

void handle_OnConnect() {
  LED1status = !LED1status;
  Serial.println("OnConnect");
  handle_read_thresholds();
}

void handle_read_sensors() {
  LED1status = !LED1status;
  Wire.beginTransmission(I2C_SECONDARY);
  Wire.write("r");              // sends instruction r (read)
  Wire.endTransmission();    // stop transmitting
  Wire.requestFrom(I2C_SECONDARY, 9);    // request 9 bytes
  char c = Wire.read(); // receive a byte as character
  Serial.print(c);         // print the character
  lowB = Wire.read(); // receive a byte as character
  highB = Wire.read();
  a0read = (highB<<8) | lowB;
  Serial.println("A0: "+String(a0read));
  lowB = Wire.read(); // receive a byte as character
  highB = Wire.read();
  a1read = (highB<<8) | lowB;
  Serial.println("A1: "+String(a1read));
  lowB = Wire.read(); // receive a byte as character
  highB = Wire.read();
  a2read = (highB<<8) | lowB;
  Serial.println("A2: "+String(a2read));
  lowB = Wire.read(); // receive a byte as character
  highB = Wire.read();
  a3read = (highB<<8) | lowB;
  Serial.println("A3: "+String(a3read));
  
  Serial.println("ReadSensors");
  server.send(200, "text/html", SendHTML(true)); 
}

void handle_read_thresholds() {
  Wire.beginTransmission(I2C_SECONDARY); 
  Wire.write("t");              // sends instruction t (get threshold)
  Wire.endTransmission();    
  Wire.requestFrom(I2C_SECONDARY, 9);    // request 9 bytes
  char c = Wire.read(); 
  lowB = Wire.read(); // receive a byte as character
  highB = Wire.read();
  a0thresh = (highB<<8) | lowB;
  lowB = Wire.read(); // receive a byte as character
  highB = Wire.read();
  a1thresh = (highB<<8) | lowB;
  lowB = Wire.read(); // receive a byte as character
  highB = Wire.read();
  a2thresh = (highB<<8) | lowB;
  lowB = Wire.read(); // receive a byte as character
  highB = Wire.read();
  a3thresh = (highB<<8) | lowB;
  Serial.print(c);// print the character
  Serial.println("A0 Thresh: "+String(a0thresh));
  Serial.println("A1 Thresh: "+String(a1thresh));
  Serial.println("A2 Thresh: "+String(a2thresh));
  Serial.println("A3 Thresh: "+String(a3thresh));
  server.send(200, "text/html", SendHTML(true)); 
}

void handle_write_thresholds() {
  LED1status = !LED1status;
  a0choose = (unsigned int)server.arg("a0chosen").toInt();
  a1choose = (unsigned int)server.arg("a1chosen").toInt();
  a2choose = (unsigned int)server.arg("a2chosen").toInt();
  a3choose = (unsigned int)server.arg("a3chosen").toInt();
  Serial.println("A0 Chosen: "+(String)a0choose);
  Serial.println("A1 Chosen: "+(String)a1choose);
  Serial.println("A2 Chosen: "+(String)a2choose);
  Serial.println("A3 Chosen: "+(String)a3choose);
  writeBytes[0] = lowByte(a0choose);
  writeBytes[1] = lowByte((unsigned int)a0choose>>8);
  writeBytes[2] = lowByte(a1choose);
  writeBytes[3] = lowByte((unsigned int)a1choose>>8);
  writeBytes[4] = lowByte(a2choose);
  writeBytes[5] = lowByte((unsigned int)a2choose>>8);
  writeBytes[6] = lowByte(a3choose);
  writeBytes[7] = lowByte((unsigned int)a3choose>>8);
  
  Wire.beginTransmission(I2C_SECONDARY);
  Wire.write("w");              // send instruction w (write)
  for (int i=0; i<sizeof(writeBytes); i=i+1) {
    Wire.write(writeBytes[i]);
  }
  Wire.endTransmission();    // stop transmitting
  Wire.requestFrom(I2C_SECONDARY, 1);    // request 1 bytes from device #8
  char c = Wire.read(); // receive a byte as character
  Serial.print(c);         // print the character
  Serial.println("WriteThresholds");
  handle_read_thresholds();
  //server.send(200, "text/html", SendHTML(false)); 
}

void handle_color_prev() {
  LED1status = !LED1status;
  
  Wire.beginTransmission(I2C_SECONDARY);
  Wire.write("p");              // send instruction p (previous color)
  Wire.endTransmission();    // stop transmitting
  Wire.requestFrom(I2C_SECONDARY, 1);    // request 1 bytes from device #8
  char c = Wire.read(); // receive a byte as character
  Serial.print(c);         // print the character
  Serial.println("ColorPrevious");
  server.send(200, "text/html", SendHTML(false)); 
}

void handle_color_next() {
  LED1status = !LED1status;
  
  Wire.beginTransmission(I2C_SECONDARY);
  Wire.write("n");              // send instruction n (next color)
  Wire.endTransmission();    // stop transmitting
  Wire.requestFrom(I2C_SECONDARY, 1);    // request 1 bytes from device #8
  char c = Wire.read(); // receive a byte as character
  Serial.print(c);         // print the character
  Serial.println("ColorNext");
  server.send(200, "text/html", SendHTML(false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat){
  String html = "<!DOCTYPE html> <html>\n";
  String url = "/";
  //percentages for slider displays
  String a0perc = String((unsigned int)((((float)a0read)/((float)500))*100.0));
  String a1perc = String((unsigned int)((((float)a1read)/((float)500))*100.0));
  String a2perc = String((unsigned int)((((float)a2read)/((float)500))*100.0));
  String a3perc = String((unsigned int)((((float)a3read)/((float)500))*100.0));

  
  html +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  //stylesheet
  html +="<style>.slider0{-webkit-appearance: none; background: linear-gradient(to right, DeepSkyBlue "+a0perc+"%, gainsboro "+a0perc+"%);}</style>\n";
  html +="<style>.slider1{-webkit-appearance: none; background: linear-gradient(to right, DeepSkyBlue "+a1perc+"%, gainsboro "+a1perc+"%);}</style>\n";
  html +="<style>.slider2{-webkit-appearance: none; background: linear-gradient(to right, DeepSkyBlue "+a2perc+"%, gainsboro "+a2perc+"%);}</style>\n";
  html +="<style>.slider3{-webkit-appearance: none; background: linear-gradient(to right, DeepSkyBlue "+a3perc+"%, gainsboro "+a3perc+"%);}</style>\n";
  html +="<style>.button{text-align:center; padding:12px; border:none; margin: 4px 4px;}</style>\n";
  html +="<style>.buttonContainer{text-align:center;}</style>\n";

  //slider and write thresholds button
  html +="<form action=\"/write_thresholds\">"
        "<input type=\"range\" min=\"0\" max=\"500\" value=\""+String(a0thresh)+"\" class=\"slider0\" id=\"a0thresh\" style=\"width: 100%\" name=\"a0chosen\" oninput=\"this.nextElementSibling.value = this.value\">"
        "<output>"+String(a0thresh)+"</output>"
        "<p style=\"text-align:right; color:DeepSkyBlue; font-weight: bold\">"+String(a0read)+"</p>"
        "<input type=\"range\" min=\"0\" max=\"500\" value=\""+String(a1thresh)+"\" class=\"slider1\" id=\"a1thresh\" style=\"width: 100%\" name=\"a1chosen\" oninput=\"this.nextElementSibling.value = this.value\">"
        "<output>"+String(a1thresh)+"</output>"
        "<p style=\"text-align:right; color:DeepSkyBlue; font-weight: bold\">"+String(a1read)+"</p>"
        "<input type=\"range\" min=\"0\" max=\"500\" value=\""+String(a2thresh)+"\" class=\"slider2\" id=\"a2thresh\" style=\"width: 100%\" name=\"a2chosen\" oninput=\"this.nextElementSibling.value = this.value\">"
        "<output>"+String(a2thresh)+"</output>"
        "<p style=\"text-align:right; color:DeepSkyBlue; font-weight: bold\">"+String(a2read)+"</p>"
        "<input type=\"range\" min=\"0\" max=\"500\" value=\""+String(a3thresh)+"\" class=\"slider3\" id=\"a3thresh\" style=\"width: 100%\" name=\"a3chosen\" oninput=\"this.nextElementSibling.value = this.value\">"
        "<output>"+String(a3thresh)+"</output>"
        "<p style=\"text-align:right; color:DeepSkyBlue; font-weight: bold\">"+String(a3read)+"</p>"
        "<br>"
        "<div class=buttonContainer>"
        "<input class=\"button\" type=\"submit\" value=\"write thresholds\" style=\"background-color: gainsboro;\">"
        "</div>"
        "</form>";

  //read thresholds, read sensors buttons
  html +="<div class=buttonContainer>";
  html +="<form><button class=\"button\" type=\"submit\" formaction=\"/read_thresholds\" style=\"background-color: gainsboro;\">read thresholds</button>\n";
  html +="<br>";
  html +="<form><button class=\"button\" type=\"submit\" formaction=\"/read_sensors\" style=\"background-color: DeepSkyBlue;\">read sensors</button>\n";
  html +="<br>";
  html +="<form><button class=\"button\" type=\"submit\" formaction=\"/color_prev\" style=\"background-color: gainsboro; float: left;\">Prev Color</button>";
  html +="<form><button class=\"button\" type=\"submit\" formaction=\"/color_next\" style=\"background-color: gainsboro; float: right;\">Next Color</button>\n";

  html += "</div>";
  
  
  html +="</body>\n";
  html +="</html>\n";
  return html;
}
