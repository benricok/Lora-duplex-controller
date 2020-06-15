
#include <SPI.h>
#include <LoRa.h>

const int csPin = 7;      
const int resetPin = 8;    
const int irqPin = 2;      

const int LEDM = 10;
const int LED1 = 9;
const int LED2 = 6;
const int LED3 = 5;
const int LED4 = 3;
const int Pod = A1;
const int Switch = 4;

int PodVar = 0;
boolean SwitchVar = false;

boolean LEDON1 = false;
boolean LEDON2 = false;
boolean LEDON3 = false;
boolean LEDON4 = false;

byte msgCount = 0;      
byte localAddress = 0x3C;   
byte destination = 0xE6;   
long lastSendTime = 0;   
int interval = 2000;       

void setup() 
{
  Serial.begin(115200);           
  Serial.println("LoRa NODE");
  Serial.println("new version");

  pinMode(Switch, INPUT);
  pinMode(LEDM, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(433E6)) 
  {          
    Serial.println("LoRa init failed. Check your connections.");
    while (true);           
  }
  LoRa.setSpreadingFactor(10);
  LoRa.setSyncWord(0xE8);  
  Serial.println("LoRa init succeeded.");
  Serial.println();
}

void loop() 
{
  PodVar = analogRead(Pod);
  SwitchVar = digitalRead(Switch);

  if (millis() - lastSendTime > interval) 
  { 
    sendMessage(PodVar, SwitchVar);
    Serial.println();
    Serial.println("##################################");
    Serial.println(" Sending Packet: Pod(" + String(PodVar) + ") Switch(" + String(SwitchVar) + ")");
    Serial.println(" Packet destination address: 0xE6");
    Serial.println("##################################");
    Serial.println();
    lastSendTime = millis(); 
    interval = 5000;
  }

  onReceive(LoRa.parsePacket());

  if (LEDON1 == true) 
  {
    digitalWrite(LED1, HIGH);
  }
    else
  {
    digitalWrite(LED1, LOW);
  }

  if (LEDON2 == true) 
  {
    digitalWrite(LED2, HIGH);
  }
    else
  {
    digitalWrite(LED2, LOW);
  }

  if (LEDON3 == true) 
  {
    digitalWrite(LED3, HIGH);
  }
    else
  {
    digitalWrite(LED3, LOW);
  }

  if (LEDON4 == true) 
  {
    digitalWrite(LED4, HIGH);
  }
    else
  {
    digitalWrite(LED4, LOW);
  } 
}

void sendMessage(int PodOutgoing, boolean SwitchOutgoing) 
{
  LoRa.beginPacket();   
  LoRa.write(destination);    
  LoRa.write(localAddress);    
  LoRa.write(msgCount);           
  LoRa.write(PodOutgoing);
  LoRa.write(SwitchOutgoing);         
  LoRa.endPacket();     
  msgCount++; 
}

void onReceive(int packetSize) 
{
  if (packetSize == 0) return;

  int recipient = LoRa.read();       
  byte sender = LoRa.read();       
  byte incomingMsgId = LoRa.read();

  LEDON1 = LoRa.read();
  LEDON2 = LoRa.read();
  LEDON3 = LoRa.read();
  LEDON4 = LoRa.read();

  if (recipient != localAddress && recipient != 0xFF) 
  {
    Serial.println("This message is not for me.");
    return;      
  }


  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Packet ID: " + String(incomingMsgId));
  Serial.println("Packet content: Led1:" + String(LEDON1) + " Led2:" + String(LEDON2) + " Led3:" + String(LEDON3) + " Led4:" + String(LEDON4));
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));

  digitalWrite(LEDM, HIGH);
  delay(1000);
  digitalWrite(LEDM, LOW);
}
