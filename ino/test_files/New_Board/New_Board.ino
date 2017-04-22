//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 

#include <SoftwareSerial.h>

SoftwareSerial GSMSerial(7, 8);
String serialStuff;

void setup()
{
  GSMSerial.begin(19200);               // the GPRS/GSM baud rate   
  Serial.begin(19200);                 // the GPRS/GSM baud rate   
  GSMSerial.println("AT+CGPSPWR=1");
  delay(300);
  GSMSerial.println("AT+CGPSRST=0");
  delay(300);
}

void loop()
{
  delay(3000);
  GSMSerial.flush();
  GSMSerial.println("AT+CGPSINF=32");
  delay(300);
  if(GSMSerial.available())
    Serial.println(GSMSerial.readString());
    
  delay(3000);
  GSMSerial.flush();
  GSMSerial.println("AT+CGPSSTATUS?");
  delay(300);
  if(GSMSerial.available()){
    serialStuff = GSMSerial.readString();
    Serial.println(serialStuff);
    Serial.print("serialStuff.indexOf('3D Fix'): ");
    Serial.println(String(serialStuff.indexOf('3D Fix'),DEC));

    if(serialStuff.indexOf('3D Fix') != -1)
      Serial.println("\n~~~~~~~~~~~~~~~~\nGOT IT\n~~~~~~~~~~~\n");
    
    Serial.println("\n\nTime is: ");
    Serial.println(millis());
  }
}
