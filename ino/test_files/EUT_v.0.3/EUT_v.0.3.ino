//#include <MemoryFree.h>

//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 

//a #include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>


int Powerkey = 9;
//a SoftwareSerial GSMSerial(7, 8);
File myFile;
String serialText;
int cellStrength;
boolean updateSuccess = false;
boolean canSend = false;
boolean OnOff = false; //Should be set to false for realtime usage
boolean GPSfix = false;
String deviceID;
String UDPupdate;

void setup()
{
  //a GSMSerial.begin(19200);               // the GPRS/GSM baud rate   
  Serial.begin(19200);                 // the GPRS/GSM baud rate   

  //Code to initialize interrupts. One on each pin. I had trouble with the "changing" interrupt.
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  attachInterrupt(0,interrupt0,FALLING);
  attachInterrupt(1,interrupt1,RISING);
  
  //a Serial.print(F("freeMemory()="));
  //a Serial.println(freeMemory());

  //checkPower();
  powerOn();
  
  //a Serial.println(F("Initializing SD card..."));

  if (!SD.begin(10)) {
    //a Serial.println(F("initialization failed!"));
    return;
  }
  //a Serial.println(F("initialization done."));

  /*a Serial.println(F("Generating file..."));
  if(SD.exists("GPScache.txt")){
    Serial.println(F("Deleting old file..."));
    SD.remove("GPScache.txt");
    Serial.println(F("Old file deleted"));
  } 
  */
  if((myFile = SD.open("GPScache.txt", FILE_WRITE)) == false){
    //a Serial.println(F("Failed to make file! Immediate attention required!")); 
  }
  else  
    //a Serial.println(F("New file created"));
    myFile.close();


  Serial.println("AT+CGPSPWR=1");
  delay(300);
  while(Serial.available())
    Serial.flush();
    //a Serial.print(GSMSerial.readString());
  Serial.println("AT+CGPSRST=0");
  delay(300);
  while(Serial.available())
    Serial.flush();
    //a Serial.print(GSMSerial.readString());
  delay(300);
  
  Serial.println("AT+CGSN");
  serialText=Serial.readString();
  int startOfID = serialText.indexOf('\n');
  serialText = serialText.substring(startOfID,startOfID+16);
  serialText.trim();
  deviceID = serialText;
  
  /*Serial.print(F("Device serial is: "));
  Serial.println(deviceID);
  */
  if(digitalRead(2) == HIGH){
    //a Serial.println(F("Equipment is on!"));
    OnOff = true;
  }
  else{
    //a Serial.println(F("Equipment is off!"));
    OnOff = false;
  }
}

void loop()
{ 
    //Get location info in $GPRMC format. Contains date.
    updateSuccess = false;
    waitForFix();
    Serial.flush();
    delay(300);
    Serial.println("AT+CGPSINF=32");
    serialText = Serial.readString();
    
    int startOfInfo = serialText.indexOf(' ');
    /*Serial.println("\nThis is the return from CGPSINF:\n");
    Serial.print(serialText);
    Serial.println("\n*****END*****");
    */
    if(serialText.indexOf("ERROR") > 0){
      //a Serial.println("GPS ERROR: GPS LIKELY NEEDS TO BE RESET.");
    }
    else{
      serialText = serialText.substring(startOfInfo, startOfInfo + 60);
      serialText.trim();
      if(serialText.indexOf('0000.0000') == -1)
        updateSuccess = true;
    }

    /*
    Serial.print("This is the return from substring:\n");
    Serial.print(serialText);
    Serial.println("\n*****END*****");
    */
    
    UDPupdate = deviceID + "," + serialText;
    
    if(OnOff)
      UDPupdate += ",ON";
    else
      UDPupdate += ",OFF";

    //a Serial.print("UDPupdate: ");
    //a Serial.println(UDPupdate);
    
    Serial.println("AT+CSQ");
    serialText=Serial.readString();
    serialText = serialText.substring(serialText.indexOf(' '),serialText.indexOf(','));
    serialText.trim();
    cellStrength = serialText.toInt();
    //a Serial.print("Call Strength: ");
    //a Serial.println(cellStrength);
    if(cellStrength > 5 && updateSuccess){
      canSend = true;
      //a Serial.print("Device can send data\n");
      printToSD("Success!");
      printToSD(UDPupdate);
    }  
    else if(updateSuccess){
      canSend = false;
      //a Serial.print(".\nStoring data until within good signal range.\n"); 
      printToSD("Failed! - low signal");
      printToSD(UDPupdate);
    }
    else{
      printToSD("Failed! - no GPS fix");
      printToSD(UDPupdate);
    }

    
    //a Serial.println("\n\n\n\n\n");
    
  if(OnOff){
    delay(3000);
    }
  else{
    powerOff();
    delay(60000);
    powerOn();
  }
}

void printToSD(String StuffToWrite){
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("GPScache.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    //a Serial.print("Writing to GPScache.txt...");
    myFile.println(StuffToWrite);
    // close the file:
    myFile.close();
    //a Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    //a Serial.println("error opening GPScache.txt");
  }

/*
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

  */
}

void waitForFix(){
  int startTime = millis();
  boolean timeout = false;
  Serial.flush();
  Serial.println("AT+CGPSINF=32");
  serialText = Serial.readString();
  while(serialText.indexOf("A") < 0 && !timeout){
    Serial.flush();
    Serial.println("AT+CGPSINF=32");
    serialText = Serial.readString();
    delay(5000);
    if((millis() - startTime) > 300000){
      timeout = true;
    }
  }
  if(!timeout){
    GPSfix = true;
  }
}

void powerOn(){
  Serial.flush();
  Serial.println("");
  Serial.println("AT");
  delay(1000);
  
  if(Serial.available()){
    //a Serial.println("Already on");
    //a Serial.println(GSMSerial.readString());
  }
  else{
    //a Serial.println("Nothing to print");
    digitalWrite(Powerkey, LOW); 
    delay(1000);               // wait for 1 second
    digitalWrite(Powerkey, HIGH);
    //a Serial.println("Turned it on/off?");
    delay(7000);
    //a Serial.println("Turned on?");
    Serial.begin(19200);     
    Serial.println("");
    Serial.println("AT");
    if(Serial.available()){
      //a Serial.println(GSMSerial.readString());
      Serial.flush();
    }
  }
  GPSfix = false;
}

void powerOff(){
  Serial.flush();
  Serial.println("");
  Serial.println("AT");
  
  if(Serial.available()){
    //a Serial.println("Currently on");
    //a Serial.println(GSMSerial.readString());
    digitalWrite(Powerkey, LOW); 
    delay(1000);               // wait for 1 second
    digitalWrite(Powerkey, HIGH);
    Serial.flush();
    //a Serial.println("Turned it off?");
  }
  else{
    //a Serial.println("Already off?");
    Serial.flush();
  }
  GPSfix = false;
}

void checkIn(String StringToSend)
{
  Serial.flush();   //Clear existing Output on console 
  //GSMSerial.println("AT+CIPSTART=\"UDP\",\"50.160.250.112\",\"55057\"");
  Serial.println("AT+CIPSTART=\"UDP\",\"cs4850udptest2.ddns.net\",\"55057\"");
  delay(300);
  /*
  while(Serial.available())
    //a Serial.print((char)GSMSerial.read());
  delay(2000);
  while(GSMSerial.available())
    Serial.print((char)GSMSerial.read());
  */
  Serial.println("AT+CIPSEND\r");
  delay(300);
  //a while(GSMSerial.available())
    //a Serial.print((char)GSMSerial.read());
  
  Serial.print(StringToSend);
  delay(1000);
  //a while(GSMSerial.available())
    //a Serial.print((char)GSMSerial.read());

  delay(1000);
  Serial.println("");
  delay(300);
  //a while(GSMSerial.available())
    //a Serial.print((char)GSMSerial.read());

  Serial.println("AT+CIPCLOSE");
  delay(3000);
  //a while(GSMSerial.available())
    //a Serial.print((char)GSMSerial.read());
}

void power()
{
  digitalWrite(Powerkey, LOW); 
  delay(1000);               // wait for 1 second
  digitalWrite(Powerkey, HIGH);
  //a Serial.println("Turned it on/off?");
}

boolean checkPower(){
  Serial.flush();
  Serial.println("");
  Serial.println("AT");
  delay(1000);
  
  if(Serial.available()){
    return true;
  }
  else{
    return false;
  }
}

void interrupt0()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 200){
  delayMicroseconds(16383);
  if(digitalRead(3) == LOW){
    //a Serial.println("Equipment is off!");
    OnOff = false;
    printToSD("Equipment off!");
  }
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
    if(digitalRead(2) == HIGH){
      //a Serial.println("Equipment is on!");
      OnOff = true;
    printToSD("Equipment on!");
    }
 }
 last_interrupt_time = interrupt_time;
}
