#include <MemoryFree.h>

//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to USB serial
//GPRS Shield is connected to the Hardware UART on Serial1 pins. Could be remapped to Serial instead with pin jumpers instead 
//of jumper wires, but USB serial must be disabled

#include <SPI.h>
#include <SD.h>

int Powerkey = 9;
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
  Serial1.begin(19200);               // the GPRS/GSM baud rate   
  Serial.begin(19200);                 // the PC baud rate   
  Serial1.flush();

  //Code to initialize interrupts. One on each pin. I had trouble with the "changing" interrupt.
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  attachInterrupt(0,interrupt0,FALLING);
  attachInterrupt(1,interrupt1,RISING);
  
/* Serial print of available memory while troubleshooting low memory issues.
  Serial.print(F("freeMemory()="));
  Serial.println(freeMemory());
*/
  
  //checkPower();
  powerOn();
  
  Serial.println(F("Initializing SD card..."));

  if (!SD.begin(10)) {
    Serial.println(F("initialization failed!"));
    return;
  }
  Serial.println(F("initialization done."));

  Serial.println(F("Generating file..."));
  
  /* Used to delete old log to replace with new one here.
  if(SD.exists("GPScache.txt")){
    Serial.println(F("Deleting old file..."));
    SD.remove("GPScache.txt");
    Serial.println(F("Old file deleted"));
  } 
  */
  
  if((myFile = SD.open("GPScache.txt", FILE_WRITE)) == false){
    Serial.println(F("Failed to make file! Immediate attention required!")); 
  }
  else
    Serial.println(F("New file created"));
  myFile.close();
  


  //Resets GPS shield/board
  Serial1.println("AT+CGPSPWR=1");
  delay(300);
  while(Serial1.available())
    Serial.print(Serial1.readString());
  Serial1.println("AT+CGPSRST=0");
  delay(300);
  while(Serial1.available())
    Serial.print(Serial1.readString());
  delay(1000);
  
  //Command to query board for unique IMEI. Used as hardware ID
  Serial1.println("AT+CGSN");
  serialText=Serial1.readString();
  delay(300);
  serialText+=Serial1.readString();
  int startOfID = serialText.indexOf('\n');
  serialText = serialText.substring(startOfID,startOfID+16);
  serialText.trim();
  deviceID = serialText;
  
  Serial.print(F("Device serial is: "));
  Serial.println(deviceID);
  
  if(digitalRead(2) == HIGH){
    Serial.println(F("Equipment is on!"));
    OnOff = true;
  }
  else{
    Serial.println(F("Equipment is off!"));
    OnOff = false;
  }
}

void loop()
{ 
    //Get location info in $GPRMC format. Contains date.
    updateSuccess = false;
    waitForFix();
    delay(1000);
    Serial1.flush();
    Serial1.println("AT+CGPSINF=32");
    //Multiple attempted reads to account for occasional buffer filling preventing full Serial read from GPS Shield
    serialText = Serial1.readString();
    delay(300);
    serialText += Serial1.readString();
    delay(300);
    serialText += Serial1.readString();
    
    //Checks for start and end of useful data. Trims accordingly.
    int startOfInfo = serialText.indexOf(' ');
    int endOfInfo = serialText.indexOf("\,\,\,");
    Serial.println("\nThis is the return from CGPSINF:\n");
    Serial.print(serialText);
    Serial.println("\n*****END*****");
    
    if(serialText.indexOf("ERROR") > 0){
      Serial.println("GPS ERROR: GPS LIKELY NEEDS TO BE RESET.");
      //Should put some code here to reset GPS and wait for fix again.
    }
    else{
      serialText = serialText.substring(startOfInfo, endOfInfo);
      serialText.trim();
      updateSuccess = true;
    }

    //Prints parsed GPS info to serial for debugging
    Serial.print("This is the return from substring:\n");
    Serial.print(serialText);
    Serial.println("\n*****END*****");
    
    
    UDPupdate = deviceID + "," + serialText;
    
    if(OnOff)
      UDPupdate += ",ON";
    else
      UDPupdate += ",OFF";

    Serial.print("UDPupdate: ");
    Serial.println(UDPupdate);
    
    //AT Command to query board for strength of GPRS fix
    Serial1.println("AT+CSQ");
    serialText=Serial1.readString();
    serialText = serialText.substring(serialText.indexOf(' '),serialText.indexOf(','));
    serialText.trim();
    cellStrength = serialText.toInt();
    Serial.print("Call Strength: ");
    Serial.println(cellStrength);
    if(cellStrength > 5 && updateSuccess){
      canSend = true;
      Serial.print("Device can send data\n");
      printToSD("Success!");
      printToSD(UDPupdate);
      delay(2000);
      init_GPRS_connection();
      delay(2000);
      checkIn(UDPupdate);
    }  
    else if(updateSuccess){
      canSend = false;
      Serial.print(".\nStoring data until within good signal range.\n"); 
      printToSD("Failed! - low signal");
      printToSD(UDPupdate);
    }
    else{
      printToSD("Failed! - no GPS fix");
      printToSD(UDPupdate);
    }

    
    Serial.println("\n\n\n\n\n");
    
  if(OnOff){
    delay(500);
    }
  else{
    powerOff();
    delay(15000);
    powerOn();
  }
}

//Simple SD write to log anything sent to this method to the SD card. Currently no methods to read logged data for resending
void printToSD(String StuffToWrite){
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("GPScache.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to GPScache.txt...");
    myFile.println(StuffToWrite);
    //close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    //if the file didn't open, print an error:
    Serial.println("error opening GPScache.txt");
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

//Waits for GPS fix. Queries GPS Shield for status of fix (can be no fix, 2d, or 3d fix), then loops until 3D fix (or 30 second timeout)
//Doesn't return value, but sets global GPSfix variable.
void waitForFix(){
  GPSfix = false;
  int startTime = millis();
  boolean timeout = false;
  Serial1.flush();
  Serial1.println("AT+CGPSSTATUS?");
  delay(300);
  serialText = Serial1.readString();
  Serial1.flush();
  int now = millis();
  
  while(serialText.indexOf("3D Fix") <= 0){
    Serial1.flush();
    Serial1.println("AT+CGPSSTATUS?");
    delay(300);
    serialText = Serial1.readString();
    Serial1.flush();
    /*
    Serial.println("Got this while waiting for fix: ");
    Serial.println(serialText);
    Serial.print("\nserialText.indexOf(\"3D Fix\"): ");
    Serial.println(serialText.indexOf("3D Fix"));
    now = millis();
    Serial.println("Time so far: ");
    Serial.println(now - startTime);
    */
    
    delay(1000);
    if((now - startTime) > 300000){
      timeout = true;
      Serial.println("Timed out");
    }
  }
  if(!timeout){
    GPSfix = true;
    /*
    Serial.println("Got it!");
    Serial1.flush();
    Serial1.println("AT+CGPSSTATUS?");
    serialText = Serial1.readString();
    Serial.println("That took: ");
    Serial.println(now - startTime);
    */
  }
}

//Checks if GPS Shield is on, turns on if not. Should probably call power() instead of having code for it here.
void powerOn(){
  Serial1.flush();
  Serial1.println("");
  Serial1.println("AT");
  delay(1000);
  
  if(Serial1.available()){
    Serial.println("Already on");
    Serial1.flush();
  }
  else{
    Serial.println("Nothing to print");
    digitalWrite(Powerkey, LOW); 
    delay(1000);               // wait for 1 second
    digitalWrite(Powerkey, HIGH);
    Serial.println("Turned it on/off?");
    delay(7000);
    Serial.println("Turned on?");
    Serial1.begin(19200);     
    Serial1.println("");
    Serial1.println("AT");
    if(Serial1.available()){
      Serial.println(Serial1.readString());
      Serial.flush();
    }
  }
  GPSfix = false;
}

//Checks if board power is on, then turns off if on
void powerOff(){
  Serial1.flush();
  Serial1.println("");
  Serial1.println("AT");
  
  if(Serial1.available()){
    Serial.println("Currently on");
    Serial.println(Serial1.readString());
    digitalWrite(Powerkey, LOW); 
    delay(1000);               // wait for 1 second
    digitalWrite(Powerkey, HIGH);
    Serial1.flush();
    Serial.println("Turned it off?");
  }
  else{
    Serial.println("Already off?");
    Serial1.flush();
  }
  GPSfix = false;
}

//Inits UDP socket to server and sends UDP packet containing the string, then closes socket
void checkIn(String StringToSend)
{
  Serial1.flush();   //Clear existing Output on console 
  //Serial1.println("AT+CIPSTART=\"UDP\",\"50.160.250.112\",\"55057\"");
  Serial1.println("AT+CIPSTART=\"UDP\",\"cs4850udptest2.ddns.net\",\"55057\"");
  delay(1000);
  
  Serial1.println("AT+CIPSEND\r");
  delay(1000);
  while(Serial1.available())
    Serial.print((char)Serial1.read());
  
  Serial1.print(StringToSend);
  delay(1000);
  while(Serial1.available())
    Serial.print((char)Serial1.read());

  delay(1000);
  Serial1.println("");
  delay(1000);
  while(Serial1.available())
    Serial.print((char)Serial1.read());

  Serial1.println("AT+CIPCLOSE");
  delay(3000);
  while(Serial1.available())
    Serial.print((char)Serial1.read());
}

//Queries GPS shield for IP after initializing GPRS connection
void init_GPRS_connection(){
  Serial1.println("AT+CGATT?");
  delay(300);
  while(Serial1.available())
    Serial.print((char)Serial1.read());

  Serial1.println("AT+CSTT=\"internetd.gdsp\"");
  delay(300);
  while(Serial1.available())
    Serial.print((char)Serial1.read());

  Serial1.println("AT+CIICR");
  delay(300);
  while(Serial1.available())
    Serial.print((char)Serial1.read());

  Serial1.println("AT+CIFSR");
  delay(300);
  while(Serial1.available())
    Serial.print((char)Serial1.read());
}

//Toggles power of GPS shield
void power()
{
  digitalWrite(Powerkey, LOW); 
  delay(1000);               // wait for 1 second
  digitalWrite(Powerkey, HIGH);
  Serial.println("Turned it on/off?");
}

//Checks if GPS board is on by printing AT and checking for any return from GPS board via UART
boolean checkPower(){
  Serial1.flush();
  Serial1.println("");
  Serial1.println("AT");
  delay(1000);
  
  if(Serial1.available()){
    Serial1.flush();
    return true;
  }
  else{
    return false;
  }
}

//Hardware interrupt to set OnOff to false if equipment is off. Needs pullup resistor to 
//deal with floating input. Pullup resistor not yet implemented
void interrupt0()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 200){
  delayMicroseconds(16383);
  if(digitalRead(3) == LOW){
    Serial.println("Equipment is off!");
    OnOff = false;
    printToSD("Equipment off!");
  }
 }
 last_interrupt_time = interrupt_time;
}

//Hardware interrupt to set OnOff to true if equipment is on
void interrupt1()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 200){
    delayMicroseconds(16383);
    if(digitalRead(2) == HIGH){
      Serial.println("Equipment is on!");
      OnOff = true;
    printToSD("Equipment on!");
    }
 }
 last_interrupt_time = interrupt_time;
}
