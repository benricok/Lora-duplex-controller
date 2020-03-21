#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <LoRa.h>
#include <AESLib.h>

uint8_t key[] = {0x2C, 0x66, 0x54, 0x34, 0xE3, 0xAE, 0xC7, 0x32,
                 0xC4, 0x36, 0xC8, 0xBE, 0xF3, 0x72, 0x22, 0x36};

const char *wifi_ssid = "KROGWiFi";
const char *wifi_password = "Soet2909admin";
const char *adminUser = "Iot";
const char *adminPassword = "Iot6541";

const int csPin = D8; 
const int resetPin = D1; 
const int irqPin = D2;

byte msgCount = 0;        
byte localAddress = 0xE6;    
byte destination = 0x3C;     
long lastSendTime = 0;   
int interval = 2000;
long lastRecieveTime = 0;   
int nodeConectedTimeout = 12000;     

String newUser; 
String newPassword;
String newWifi; 
String newWifiPassword;

boolean LEDON1 = false;
boolean LEDON2 = false;
boolean LEDON3 = false;
boolean LEDON4 = false;

String LEDON1Converted = "OFF";
String LEDON2Converted = "OFF";
String LEDON3Converted = "OFF";
String LEDON4Converted = "OFF";

int varPod = 0;
boolean varSwitch = false;
int nodeConnected = 1;

ESP8266WebServer server(80);
 
void handleLogin()
{
  String msg;

  if (server.hasHeader("Cookie"))
  {   
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }

  if (server.hasArg("DISCONNECT"))
  {
    Serial.println("Disconnection");
    String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=0\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }

  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD"))
  {
    if (server.arg("USERNAME") == adminUser &&  server.arg("PASSWORD") == adminPassword )
    {
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=1\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header);
      Serial.println("Log in Successful by user");
      return;
    }
  msg = "<p class='wrongInput'>Wrong username/password! try again.</p>";
  Serial.println("Log in Failed");
  }

  String html ="<!DOCTYPE html> <html> <head> <title>Iot-controller dashboard</title> <meta name=\"viewport\" content=\"width=device-width, minimumscale=1.0, maximum-scale=1.0, initial-scale=1\" /> <style> header {text-align: center; min-height: 110px; background-color: lightseagreen; color: white} h1 {font-size: 60px; margin-top: 0px; font-family: Georgia, 'Times New Roman', Times, serif} h2 {font-family: Arial, Helvetica, sans-serif} body {text-align: center} .inputSubmit { clear: both; position:relative; margin-top: 10px; -moz-box-shadow: 0px 1px 0px 0px #fff6af; -webkit-box-shadow: 0px 1px 0px 0px #fff6af; box-shadow: 0px 1px 0px 0px #fff6af; background:-webkit-gradient(linear, left top, left bottom, color-stop(0.05, #ffec64), color-stop(1, #ffab23)); background:-moz-linear-gradient(top, #ffec64 5%, #ffab23 100%); background:-webkit-linear-gradient(top, #ffec64 5%, #ffab23 100%); background:-o-linear-gradient(top, #ffec64 5%, #ffab23 100%); background:-ms-linear-gradient(top, #ffec64 5%, #ffab23 100%); background:linear-gradient(to bottom, #ffec64 5%, #ffab23 100%); filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#ffec64', endColorstr='#ffab23',GradientType=0); background-color:#ffec64; -moz-border-radius:6px; -webkit-border-radius:6px; border-radius:6px; border:1px solid #ffaa22; display:inline-block; cursor:pointer; color:#333333; font-family:Arial; font-size:15px; padding:8px 50px; text-decoration:none; text-shadow:0px 1px 0px #ffee66; } .inputSubmit:hover { background:-webkit-gradient(linear, left top, left bottom, color-stop(0.05, #ffab23), color-stop(1, #ffec64)); background:-moz-linear-gradient(top, #ffab23 5%, #ffec64 100%); background:-webkit-linear-gradient(top, #ffab23 5%, #ffec64 100%); background:-o-linear-gradient(top, #ffab23 5%, #ffec64 100%); background:-ms-linear-gradient(top, #ffab23 5%, #ffec64 100%); background:linear-gradient(to bottom, #ffab23 5%, #ffec64 100%); filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#ffab23', endColorstr='#ffec64',GradientType=0); background-color:#ffab23; } .inputSubmit:active { position:relative; top:2px; } .password {margin-top: 4px;margin-right: 30px} .wrongInput {color: red; margin-top: 6px;} </style> </head> <header> <h1> Dashboard </h1> </header> <body> <form action='/login' method='POST'> User: <input type='text' name='USERNAME' placeholder=' username'><br> Password: <input type='password' class=\"password\" name='PASSWORD' placeholder=' password'> <br class=\"error\"> ";
  html += msg;
  html +=" <br> <input type='submit' class=\"inputSubmit\" name='SUBMIT' value='Submit'><br> </form> <br> <a href='/info'>info</a> </body> </html>";

  server.send(200, "text/html", html);
}

bool is_authentified()
{
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie"))
  {   
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);

    if (cookie.indexOf("ESPSESSIONID=1") != -1) 
    {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;  
}

void handelGetData()
{
  String jsonData = "{\"data\":[";
  jsonData += "{\"dataValue\":\"";
  jsonData += varPod;
  jsonData += "\"},";
  jsonData += "{\"dataValue\":\"";
  jsonData += varSwitch;
  jsonData += "\"},";
  jsonData += "{\"dataValue\":\"";
  jsonData += LEDON1Converted;
  jsonData += "\"},";
  jsonData += "{\"dataValue\":\"";
  jsonData += LEDON2Converted;
  jsonData += "\"},";
  jsonData += "{\"dataValue\":\"";
  jsonData += LEDON3Converted;
  jsonData += "\"},";
  jsonData += "{\"dataValue\":\"";
  jsonData += LEDON4Converted;
  jsonData += "\"},";
  jsonData += "{\"dataValue\":\"";
  jsonData += nodeConnected;
  jsonData += "\"}";
  jsonData += "]}";

  String header;

  if (!is_authentified())
  {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  
  Serial.println("Client updated webpage");
  server.send(200, "application/json", jsonData);
}

void handleRoot()
{
  Serial.println("Enter handleRoot");
  String header;

  if (!is_authentified())
  {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }

  String html ="<!DOCTYPE html><html lang=\"en\"> <head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>Document</title> <style>header{text-align:center;min-height:110px;background-color:#20b2aa;color:#fff}h1{font-size:60px;margin-top:0;font-family:Georgia,Times New Roman,Times,serif}h2{font-size:30px;font-family:Arial,Helvetica,sans-serif}#Settings{display:none;margin-top:30px}#Data{display:block;margin-top:30px}.changeViewButton{clear:both;position:relative;margin-top:30px;-moz-box-shadow:0 1px 0 0 #fff6af;-webkit-box-shadow:0 1px 0 0 #fff6af;box-shadow:0 1px 0 0 #fff6af;background:-webkit-gradient(linear,left top,left bottom,color-stop(.05,#ffec64),color-stop(1,#ffab23));background:-moz-linear-gradient(top,#ffec64 5%,#ffab23 100%);background:-webkit-linear-gradient(top,#ffec64 5%,#ffab23);background:-o-linear-gradient(top,#ffec64 5%,#ffab23 100%);background:-ms-linear-gradient(top,#ffec64 5%,#ffab23 100%);background:linear-gradient(180deg,#ffec64 5%,#ffab23);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr=\"#ffec64\",endColorstr=\"#ffab23\",GradientType=0);background-color:#ffec64;-moz-border-radius:6px;-webkit-border-radius:6px;border-radius:6px;border:1px solid #fa2;display:inline-block;cursor:pointer;color:#333;font-family:Arial;font-size:15px;padding:8px 50px;text-decoration:none;text-shadow:0 1px 0 #fe6}.changeViewButton:hover{background:gradient(linear,left top,left bottom,color-stop(.05,#ffab23),color-stop(1,#ffec64));background:-moz-linear-gradient(top,#ffab23 5%,#ffec64 100%);background:-webkit-linear-gradient(top,#ffab23 5%,#ffec64);background:-o-linear-gradient(top,#ffab23 5%,#ffec64 100%);background:-ms-linear-gradient(top,#ffab23 5%,#ffec64 100%);background:linear-gradient(180deg,#ffab23 5%,#ffec64);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr=\"#ffab23\",endColorstr=\"#ffec64\",GradientType=0);background-color:#ffab23}.changeViewButton:active{position:relative;top:4px}.onButton{clear:both;position:relative;margin-left:5px;margin-top:10px;-moz-box-shadow:0 1px 0 0 #5bda2a;-webkit-box-shadow:0 1px 0 0 #5bda2a;box-shadow:0 1px 0 0 #5bda2a;background:-webkit-gradient(linear,left top,left bottom,color-stop(.05,#5bda2a),color-stop(1,#5bda2a));background:-moz-linear-gradient(top,#5bda2a 5%,#5bda2a 100%);background:-webkit-linear-gradient(top,#5bda2a 5%,#5bda2a);background:-o-linear-gradient(top,#5bda2a 5%,#5bda2a 100%);background:-ms-linear-gradient(top,#5bda2a 5%,#5bda2a 100%);background:linear-gradient(180deg,#5bda2a 5%,#5bda2a);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr=\"#ffec64\",endColorstr=\"#ffab23\",GradientType=0);background-color:#5bda2a;-moz-border-radius:6px;-webkit-border-radius:6px;border-radius:6px;border:1px solid #5bda2a;display:inline-block;cursor:pointer;color:#333;font-family:Arial;font-size:12px;padding:4px 13px;text-decoration:none;text-shadow:0 1px 0 #5bda2a}.onButton:hover{background:gradient(linear,left top,left bottom,color-stop(.05,#a7ff8c),color-stop(1,#a7ff8c));background:-moz-linear-gradient(top,#a7ff8c 5%,#a7ff8c 100%);background:-webkit-linear-gradient(top,#a7ff8c 5%,#a7ff8c);background:-o-linear-gradient(top,#a7ff8c 5%,#a7ff8c 100%);background:-ms-linear-gradient(top,#a7ff8c 5%,#a7ff8c 100%);background:linear-gradient(180deg,#a7ff8c 5%,#a7ff8c);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr=\"#a7ff8c\",endColorstr=\"#a7ff8c\",GradientType=0);background-color:#a7ff8c}.onButton:active{position:relative;top:4px}.offButton{clear:both;position:relative;margin-left:8px;margin-top:10px;-moz-box-shadow:0 1px 0 0 #f06262;-webkit-box-shadow:0 1px 0 0 #f06262;box-shadow:0 1px 0 0 #f06262;background:-webkit-gradient(linear,left top,left bottom,color-stop(.05,#f06262),color-stop(1,#f06262));background:-moz-linear-gradient(top,#f06262 5%,#f06262 100%);background:-webkit-linear-gradient(top,#f06262 5%,#f06262);background:-o-linear-gradient(top,#f06262 5%,#f06262 100%);background:-ms-linear-gradient(top,#f06262 5%,#f06262 100%);background:linear-gradient(180deg,#f06262 5%,#f06262);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr=\"#ffec64\",endColorstr=\"#ffab23\",GradientType=0);background-color:#f06262;-moz-border-radius:6px;-webkit-border-radius:6px;border-radius:6px;border:1px solid #f06262;display:inline-block;cursor:pointer;color:#333;font-family:Arial;font-size:12px;padding:4px 11px;text-decoration:none;text-shadow:0 1px 0 #f06262}.offButton:hover{background:gradient(linear,left top,left bottom,color-stop(.05,#df7e7e),color-stop(1,#df7e7e));background:-moz-linear-gradient(top,#df7e7e 5%,#df7e7e 100%);background:-webkit-linear-gradient(top,#df7e7e 5%,#df7e7e);background:-o-linear-gradient(top,#df7e7e 5%,#df7e7e 100%);background:-ms-linear-gradient(top,#df7e7e 5%,#df7e7e 100%);background:linear-gradient(180deg,#df7e7e 5%,#df7e7e);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr=\"#df7e7e\",endColorstr=\"#df7e7e\",GradientType=0);background-color:#df7e7e}.offButton:active{position:relative;top:4px}</style></head> <body> <header> <h1> Dashboard </h1> </header> <div id=\"Data\"> <h2 class=\"sensorHeader\">Sensors:</h2> <div id=\"Pod\"> <h3> Pod: <div id=\"PodInfo\">";
  html += varPod;
  html +="</div> </h3> </div> <div id=\"Switch\"> <h3> Switch: <div id=\"SwitchInfo\">";
  html += varSwitch;
  html +="</div> </h3> </div> <h2 class=\"ledHeader\">Leds:</h2> <form action=\"/\" method=\"POST\"> <div id=\"Led1\"> <h3> Led1: <div id=\"LEDON1info\">";
  html += LEDON1Converted;
  html +="</div> <br> <button class=\"onButton\" name=\"LEDON1\">ON</button><button class=\"offButton\" name=\"LEDOFF1\">OFF</button> </h3> </div> <div id=\"Led2\"> <h3> Led2: <div id=\"LEDON2info\">";
  html += LEDON2Converted;
  html +="</div> <br> <button class=\"onButton\" name=\"LEDON2\">ON</button><button class=\"offButton\" name=\"LEDOFF2\">OFF</button> </h3> </div> <div id=\"Led3\"> <h3> Led3: <div id=\"LEDON3info\">";
  html += LEDON3Converted;
  html +="</div> <br> <button class=\"onButton\" name=\"LEDON3\">ON</button><button class=\"offButton\" name=\"LEDOFF3\">OFF</button> </h3> </div> <div id=\"Led4\"> <h3> Led4: <div id=\"LEDON4info\">";
  html += LEDON3Converted;
  html +="</div> <br> <button class=\"onButton\" name=\"LEDON4\">ON</button><button class=\"offButton\" name=\"LEDOFF4\">OFF</button> </h3> </div> </form> </div> <div id=\"Settings\"> <h2>Settings:</h2> <h3>Total Connected Nodes:</h3> <div id=\"TNodes\">";
  html += nodeConnected;
  html +="</div><br> <h3>Update rate:</h3> <input type=\"range\" style=\"width:300px\" min=\"200\" max=\"5000\" value=\"2000\" id=\"fader\" step=\"1\"> <output for=\"fader\">2000</output> </div> <div> <button id=\"changeView1\" class=\"changeViewButton\">Settings</button> </div> <script>parcelRequire=function(e,r,n,t){var i=\"function\"==typeof parcelRequire&&parcelRequire,o=\"function\"==typeof require&&require;function u(n,t){if(!r[n]){if(!e[n]){var f=\"function\"==typeof parcelRequire&&parcelRequire;if(!t&&f)return f(n,!0);if(i)return i(n,!0);if(o&&\"string\"==typeof n)return o(n);var c=new Error(\"Cannot find module '\"+n+\"'\");throw c.code=\"MODULE_NOT_FOUND\",c}p.resolve=function(r){return e[n][1][r]||r};var l=r[n]=new u.Module(n);e[n][0].call(l.exports,p,l,l.exports,this)}return r[n].exports;function p(e){return u(p.resolve(e))}}u.isParcelRequire=!0,u.Module=function(e){this.id=e,this.bundle=u,this.exports={}},u.modules=e,u.cache=r,u.parent=i,u.register=function(r,n){e[r]=[function(e,r){r.exports=n},{}]};for(var f=0;f<n.length;f++)u(n[f]);if(n.length){var c=u(n[n.length-1]);\"object\"==typeof exports&&\"undefined\"!=typeof module?module.exports=c:\"function\"==typeof define&&define.amd?define(function(){return c}):t&&(this[t]=c)}return u}({\"vKFU\":[function(require,module,exports) { },{}],\"7QCb\":[function(require,module,exports) { \"use strict\";var e=this&&this.__awaiter||function(e,t,n,a){return new(n||(n=Promise))(function(r,o){function i(e){try{c(a.next(e))}catch(e){o(e)}}function u(e){try{c(a.throw(e))}catch(e){o(e)}}function c(e){var t;e.done?r(e.value):(t=e.value,t instanceof n?t:new n(function(e){e(t)})).then(i,u)}c((a=a.apply(e,t||[])).next())})},t=this&&this.__generator||function(e,t){var n,a,r,o,i={label:0,sent:function(){if(1&r[0])throw r[1];return r[1]},trys:[],ops:[]};return o={next:u(0),throw:u(1),return:u(2)},\"function\"==typeof Symbol&&(o[Symbol.iterator]=function(){return this}),o;function u(o){return function(u){return function(o){if(n)throw new TypeError(\"Generator is already executing.\");for(;i;)try{if(n=1,a&&(r=2&o[0]?a.return:o[0]?a.throw||((r=a.return)&&r.call(a),0):a.next)&&!(r=r.call(a,o[1])).done)return r;switch(a=0,r&&(o=[2&o[0],r.value]),o[0]){case 0:case 1:r=o;break;case 4:return i.label++,{value:o[1],done:!1};case 5:i.label++,a=o[1],o=[0];continue;case 7:o=i.ops.pop(),i.trys.pop();continue;default:if(!(r=(r=i.trys).length>0&&r[r.length-1])&&(6===o[0]||2===o[0])){i=0;continue}if(3===o[0]&&(!r||o[1]>r[0]&&o[1]<r[3])){i.label=o[1];break}if(6===o[0]&&i.label<r[1]){i.label=r[1],r=o;break}if(r&&i.label<r[2]){i.label=r[2],i.ops.push(o);break}r[2]&&i.ops.pop(),i.trys.pop();continue}o=t.call(e,i)}catch(e){o=[6,e],a=0}finally{n=r=0}if(5&o[0])throw o[1];return{value:o[0]?o[1]:void 0,done:!0}}([o,u])}}};Object.defineProperty(exports,\"__esModule\",{value:!0}),require(\"./index.css\");var n=!1,a=document.querySelector(\"#fader\");a.addEventListener(\"input\",function(){i(a.valueAsNumber)});var r=-1;function o(){return e(this,void 0,void 0,function(){var e;return t(this,function(t){switch(t.label){case 0:return[4,fetch(\"/data\")];case 1:return[4,t.sent().json()];case 2:return e=t.sent(),document.getElementById(\"PodInfo\").innerHTML=e.data[0].dataValue,document.getElementById(\"SwitchInfo\").innerHTML=e.data[1].dataValue,document.getElementById(\"LEDON1info\").innerHTML=e.data[2].dataValue,document.getElementById(\"LEDON2info\").innerHTML=e.data[3].dataValue,document.getElementById(\"LEDON3info\").innerHTML=e.data[4].dataValue,document.getElementById(\"LEDON4info\").innerHTML=e.data[5].dataValue,document.getElementById(\"TNodes\").innerHTML=e.data[6].dataValue,[2]}})})}function i(e){r>0&&clearInterval(r),r=setInterval(function(){o().catch(console.error)},e)}document.querySelector(\"button#changeView1\").addEventListener(\"click\",function(){0==n?(document.getElementById(\"Data\").style.display=\"none\",document.getElementById(\"Settings\").style.display=\"block\",document.getElementById(\"changeView1\").innerHTML=\"Show Data\",n=!0):(document.getElementById(\"Data\").style.display=\"block\",document.getElementById(\"Settings\").style.display=\"none\",document.getElementById(\"changeView1\").innerHTML=\"Settings\",n=!1)}),i(2e3); },{\"./index.css\":\"vKFU\"}]},{},[\"7QCb\"], null)</script> </body> </html>";


  server.send(200, "text/html", html);

  if (server.hasArg("LEDON1"))
  {
    LEDON1 = true;
    LEDON1Converted = "ON";
  } 
    else if (server.hasArg("LEDOFF1"))
  {
    LEDON1 = false;
    LEDON1Converted = "OFF";
  } 
    else if (server.hasArg("LEDON2"))
  {
    LEDON2 = true;
    LEDON2Converted = "ON";
  } 
    else if (server.hasArg("LEDOFF2"))
  {
    LEDON2 = false;
    LEDON2Converted = "OFF";
  } 
    else if (server.hasArg("LEDON3"))
  {
    LEDON3 = true;
    LEDON3Converted = "ON";
  } 
    else if (server.hasArg("LEDOFF3"))
  {
    LEDON3 = false;
    LEDON3Converted = "OFF";
  } 
    else if (server.hasArg("LEDON4"))
  {
    LEDON4 = true;
    LEDON4Converted = "ON";
  } 
    else if (server.hasArg("LEDOFF4"))
  {
    LEDON4 = false;
    LEDON4Converted = "OFF";
  }
  
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i=0; i<server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void sendMessage() 
{ 
  LoRa.beginPacket(); 
  LoRa.write(destination);   
  LoRa.write(localAddress);  
  LoRa.write(msgCount);
  LoRa.write(LEDON1);
  LoRa.write(LEDON2);
  LoRa.write(LEDON3);
  LoRa.write(LEDON4);  
  LoRa.endPacket();    
  msgCount++; 
  
}

void onReceive(int packetSize) 
{
  if (packetSize == 0) return;   
  
  int recipient = LoRa.read();  
  byte sender = LoRa.read();   
  byte incomingMsgId = LoRa.read();
    
  varPod = LoRa.read();
  varSwitch = LoRa.read();

  if (recipient != localAddress && recipient != 0xFF) 
  {
    Serial.println("This message is not for me.");
    return;          
  }

  lastRecieveTime = millis();
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Packet ID: " + String(incomingMsgId));
  Serial.println("Packet content: Pod:" + String(varPod) + " Switch:" + String(varSwitch));
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
}


void setup() 
{
  Serial.begin(115200);
  Serial.println();

  LoRa.setPins(csPin, resetPin, irqPin);

  if (! lora.begin(433E6));
  { 
    Serial.println("LoRa init failed. Check your connections.");
    while (true);   
  } 
    else
  {
    Serial.println("Sucsesfully initialized LoRa");
  }
  LoRa.setSyncWord(0xE8);
  LoRa.setSpreadingFactor(10);    

  if (!WiFi.begin(wifi_ssid, wifi_password)) 
  {
    Serial.println("Failed to start wifi adaptor");
    return;
  } 
    else 
  {
    Serial.println("Sucsesfully initialized the wifi adaptor");
  };

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wifi_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handelGetData);
  server.on("/login", handleLogin);
  server.on("/info", []() {server.send(200, "text/plain", "");});

  server.onNotFound(handleNotFound);
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  Serial.println("HTTP server started");
  Serial.println();
}

void loop()
{
  server.handleClient();
 
  onReceive(LoRa.parsePacket());
  
  if (millis() - lastSendTime > interval)
  { 
    sendMessage();
    Serial.println();
    Serial.println("###################################################");
    Serial.println(" Sending Packet Led1: " + String(LEDON1) + "  Led2: " + String(LEDON2) + "  Led3: " + String(LEDON3) + "  Led4: " + String(LEDON4));
    Serial.println(" Packet destination address: 0x3C");
    Serial.println("###################################################");
    Serial.println();
    lastSendTime = millis();          
    interval = 5000;   
  }

  if (millis() - lastRecieveTime < nodeConectedTimeout) 
  {
    nodeConnected = 1;
  } 
    else
  {
    nodeConnected = 0;
  }
}
