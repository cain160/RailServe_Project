#include <MemoryFree.h>

//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 

#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>


int Powerkey = 9;
SoftwareSerial GSMSerial(7, 8);
File myFile;
String serialText;
String serialText2;
String serialTextCopy;
int cellStrength;
boolean canSend = false;
boolean OnOff = true; //Should be set to false for realtime usage

void setup()
{
  GSMSerial.begin(19200);               // the GPRS/GSM baud rate   
  Serial.begin(19200);                 // the GPRS/GSM baud rate   

  //Code to initialize interrupts. One on each pin. I had trouble with the "changing" interrupt.
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  attachInterrupt(0,interrupt0,FALLING);
  attachInterrupt(1,interrupt1,RISING);
  
  Serial.print("freeMemory()=");
  Serial.println(freeMemory());

  checkPower();
  //powerOn();
  
  Serial.println("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  Serial.println("Generating file...");
  if(SD.exists("GPScache.txt")){
    Serial.println("Deleting old file...");
    SD.remove("GPScache.txt");
    Serial.println("Old file deleted");
  } 
  if((myFile = SD.open("GPScache.txt", FILE_WRITE)) == false)
    Serial.println("Failed to make file! Immediate attention required!");
  else  
    Serial.println("New file created");
    myFile.close();

  /*
  GSMSerial.println("AT+CGPSPWR=1");
  delay(300);
  while(GSMSerial.available())
    Serial.print(GSMSerial.readString());
  GSMSerial.println("AT+CGPSRST=0");
  delay(300);
  while(GSMSerial.available())
    Serial.print(GSMSerial.readString());
  delay(300);
*/
  Serial.print("freeMemory()=");
  Serial.println(freeMemory());
}

void loop()
{ 
  if(OnOff){
    //Get location info in $GPRMC format. Contains date.
    GSMSerial.flush();
    GSMSerial.println("AT+CGPSINF=32");
    serialText = GSMSerial.readString();
    int startOfInfo = serialText.indexOf(' ');
    Serial.print("\nThis is the return from CGPSINF:\n");
    Serial.print(serialText);
    Serial.println("*****END*****");
    if(serialText.indexOf("ERROR") > 0)
      Serial.println("GPS ERROR: GPS LIKELY NEEDS TO BE RESET.");
    else{
      serialText = serialText.substring(startOfInfo, startOfInfo + 60);
      //serialText.trim();
    }  
    Serial.print("\nThis is the return from substring:\n");
    Serial.print(serialText);
    Serial.println("\n*****END*****");
    
    GSMSerial.println("AT+CSQ");
    serialText2=GSMSerial.readString();
    serialText2 = serialText2.substring(serialText2.indexOf(' '),serialText2.indexOf(','));
    serialText2.trim();
    cellStrength = serialText2.toInt();
    Serial.print("Call Strength: ");
    Serial.println(cellStrength);
    if(cellStrength > 5){
      canSend = true;
      Serial.print("Device can send data\n");
    }  
    else{
      canSend = false;
      Serial.print("Storing data until within good signal range.\n"); 
    }  
    delay(3000); 
  }

  Serial.print("freeMemory()=");
  Serial.println(freeMemory());

  int stringCounter = 1;
  while(stringCounter < 10){
    serialTextCopy += serialText;
    serialTextCopy += "\n";
    Serial.print("\n\nLines in copy: ");
    Serial.println(stringCounter++);
    Serial.print("Size of copy: ");
    Serial.println(serialTextCopy.length());
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());
    delay(500);
  }
}

void printToSD(String StuffToWrite){
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("GPScache.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to GPScache.txt...");
    myFile.println(StuffToWrite);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening GPScache.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("GPScache.txt");
  if (myFile) {
    Serial.println("GPScache.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening GPScache.txt");
  }
}

void powerOn(){
  GSMSerial.flush();
  GSMSerial.println("");
  GSMSerial.println("AT");
  
  if(GSMSerial.available()){
    Serial.println("Already on");
    Serial.println(GSMSerial.readString());
  }
  else{
    Serial.println("Nothing to print");
    digitalWrite(Powerkey, LOW); 
    delay(1000);               // wait for 1 second
    digitalWrite(Powerkey, HIGH);
    Serial.println("Turned it on/off?");
    delay(7000);
    Serial.println("Turned on?");
    GSMSerial.begin(19200);     
    GSMSerial.println("");
    GSMSerial.println("AT");
    if(GSMSerial.available()){
      Serial.println(GSMSerial.readString());
    }
  }
}

void powerOff(){
  GSMSerial.flush();
  GSMSerial.println("");
  GSMSerial.println("AT");
  
  if(GSMSerial.available()){
    Serial.println("Currently on");
    Serial.println(GSMSerial.readString());
    digitalWrite(Powerkey, LOW); 
    delay(1000);               // wait for 1 second
    digitalWrite(Powerkey, HIGH);
    Serial.println("Turned it off?");
  }
  else{
    Serial.println("Already off?");
    GSMSerial.flush();
  }
}

void checkIn(String StringToSend)
{
  GSMSerial.flush();   //Clear existing Output on console 
  GSMSerial.println("AT+CIPSTART=\"UDP\",\"50.160.250.112\",\"55057\"");
  //GSMSerial.println("AT+CIPSTART=\"UDP\",\"cs4850railservetest.ddns.net\",\"55057\"");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
  delay(2000);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());

  GSMSerial.println("AT+CIPSEND\r");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
  
  GSMSerial.print(StringToSend);
  delay(1000);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());

  delay(1000);
  GSMSerial.println("");
  delay(300);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());

  GSMSerial.println("AT+CIPCLOSE");
  delay(3000);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
}

void power()
{
  digitalWrite(Powerkey, LOW); 
  delay(1000);               // wait for 1 second
  digitalWrite(Powerkey, HIGH);
  Serial.println("Turned it on/off?");
}

void checkPower(){
  GSMSerial.flush();
  GSMSerial.println("");
  GSMSerial.println("AT");
  delay(1000);
  
  if(GSMSerial.available()){
    Serial.println("Already on");
    Serial.println(GSMSerial.readString());
  }
  else{
    Serial.println("Nothing to print");
    power();
    delay(7000);
    Serial.println("Turned on?");
    GSMSerial.begin(19200);     
    GSMSerial.println("");
    GSMSerial.println("AT");
    delay(10000);
    if(GSMSerial.available()){
      Serial.println(GSMSerial.readString());
    }
  }
}

void interrupt0()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 200){
  delayMicroseconds(16383);
  if(digitalRead(3) == LOW)
    Serial.println("Equipment is off!");
 }
 last_interrupt_time = interrupt_time;
}

void interrupt1()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 200){
    delayMicroseconds(16383);
    if(digitalRead(2) == HIGH)
      Serial.println("Equipment is on!");
 }
 last_interrupt_time = interrupt_time;
}
