//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 

#include <SoftwareSerial.h>

SoftwareSerial GSMSerial(7, 8);

void setup()
{
  GSMSerial.begin(19200);               // the GPRS/GSM baud rate   
  Serial.begin(19200);                 // the GPRS/GSM baud rate   
  GSMSerial.println("AT+CGPSPWR=1");
  delay(300);
  while(GSMSerial.available())
    readBites();
  GSMSerial.println("AT+CGPSRST=0");
  delay(300);
  while(GSMSerial.available())
    readBites();
  delay(3000);  
}

void loop()
{
  delay(30000);
  GSMSerial.println("AT+CGPSINF=2");
  while(GSMSerial.available())
    readBites();
}

void readBites(){
  if(Serial.available()){
    GSMSerial.print((char)Serial.read());
  }
  else if(GSMSerial.available()){
    Serial.print((char)GSMSerial.read());
  }
}

