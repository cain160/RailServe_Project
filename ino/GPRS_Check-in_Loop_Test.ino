//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 

#include <SoftwareSerial.h>

SoftwareSerial GSMSerial(7, 8);
int counter = 0;
String ID = "ID=591";
void setup()
{
  GSMSerial.begin(19200);               // the GPRS/GSM baud rate   
  Serial.begin(19200);                 // the GPRS/GSM baud rate   
  
  GSMSerial.flush();   //Clear existing garbage on console 
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
    
  GSMSerial.println("AT+CGPSPWR=1");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
  
  GSMSerial.println("AT+CGPSRST=0");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
  
  delay(3000);  
  delay(3000);  
}

void loop()
{
  String current_coordinates = get_coordinates();
  init_GPRS_connection();
  String check_in_string = ID + "," + current_coordinates;
  checkIn(check_in_string);
}

String get_coordinates(){
  String SerialGarbage = "";
  GSMSerial.println("AT+CGPSINF=2");
  delay(300);
  while(GSMSerial.available())
    SerialGarbage = GSMSerial.readString();
  String Coordinates = SerialGarbage.substring(27,62);
  return Coordinates;
}

void init_GPRS_connection(){
  GSMSerial.println("AT+CGATT?");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());

  GSMSerial.println("AT+CSTT=\"internetd.gdsp\"");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());

  GSMSerial.println("AT+CIICR");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());

  GSMSerial.println("AT+CIFSR");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
}

void checkIn(String StringToSend)
{  
  GSMSerial.println("AT+CIPSTART=\"UDP\",\"50.160.250.112\",\"55057\"");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());

  GSMSerial.println("AT+CIPSEND\r");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
  
  GSMSerial.print(StringToSend);
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());

  GSMSerial.println("");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());

  GSMSerial.println("AT+CIPCLOSE");
  delay(3000);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
}
