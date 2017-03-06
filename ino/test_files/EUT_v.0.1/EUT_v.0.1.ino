//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 

#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

SoftwareSerial GSMSerial(7, 8);
File myFile;
String serialText;
int cellStrength;
boolean canSend = false;
boolean OnOff = true; //Should be set to false for realtime usage

void setup()
{
  GSMSerial.begin(19200);               // the GPRS/GSM baud rate   
  Serial.begin(19200);                 // the GPRS/GSM baud rate   
  
  Serial.println("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  Serial.println("Generating file...");
  if(SD.exists("coords.txt")){
    Serial.println("Deleting old file...");
    SD.remove("coords.txt");
    Serial.println("Old file deleted");
  } 
  if((myFile = SD.open("coords.txt", FILE_WRITE)) == false)
    Serial.println("Failed to make file! Immediate attention required!");
  else  
    Serial.println("New file created");
    myFile.close();
  
  GSMSerial.println("AT+CGPSPWR=1");
  delay(300);
  while(GSMSerial.available())
    Serial.print(GSMSerial.readString());
  GSMSerial.println("AT+CGPSRST=0");
  delay(300);
  while(GSMSerial.available())
    Serial.print(GSMSerial.readString());
  delay(300);
  
//  GSMSerial.println("AT+CGPSINF=2");
//  serialText = GSMSerial.readString();
//  Serial.println("This is the return from CGPSINF*********:" + serialText + "*****END*****");
//  if(serialText.indexOf("ERROR") > 0)
//    Serial.println("GPS ERROR: GPS LIKELY NEEDS TO BE RESET.");
//  else if(serialText.indexOf('W') > 0)
//    serialText = serialText.substring(serialText.indexOf(' '), (serialText.indexOf('W') + 1));
//  else if(serialText.indexOf('E') > 0)
//    serialText = serialText.substring(serialText.indexOf(' '), (serialText.indexOf('E') + 1));  
//  Serial.println("This is the return from substring*********:" + serialText + "*****END*****");
//  delay(30000);
}

void loop()
{ 
  if(OnOff){
    //Get location info in $GPRMC format. Contains date.
    GSMSerial.println("AT+CGPSINF=32");
    serialText = GSMSerial.readString();
    int startOfInfo = serialText.indexOf(' ');
    Serial.println("\nThis is the return from CGPSINF:\n" + serialText + "*****END*****");
    if(serialText.indexOf("ERROR") > 0)
      Serial.println("GPS ERROR: GPS LIKELY NEEDS TO BE RESET.");
    else{
      serialText = serialText.substring(startOfInfo, startOfInfo + 60);
      serialText.trim();
    }  
    Serial.println("This is the return from substring:\n" + serialText + "\n*****END*****");
    
    GSMSerial.println("AT+CSQ");
    serialText=GSMSerial.readString();
    serialText = serialText.substring(serialText.indexOf(' '),serialText.indexOf(','));
    serialText.trim();
    cellStrength = serialText.toInt();
    if(cellStrength > 5){
      canSend = true;
      Serial.print("Device can send data\n");
    }  
    else{
      canSend = false;
      Serial.print("Storing data until within good signal range.\n"); 
    }  
    delay(30000); 
  }  
//  cellStrength = serialText.toInt();
//  if(cellStrength > 5){
//    canSend = true;
//    Serial.print("Device can send data");
//  }  
//  else
//    Serial.print("Storing data until within good signal range."); 
//  delay(30000);  
//  delay(3000);
//  GSMSerial.println("AT+CGPSINF=2\r");
//  while(GSMSerial.available()){
//    String SerialGarbage = GSMSerial.readString();
//
//    String TrimmedSerialGarbage = SerialGarbage.substring(102,126);
//    String SerialGarbageFirstDigit = TrimmedSerialGarbage.substring(0,1);
//    
//    Serial.println("SerialGarbage String is:\n\r" + SerialGarbage + "\n\r~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r");
//    Serial.println("TrimmedSerialGarbage String is:\n\r" + TrimmedSerialGarbage + "\n\r~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r");
//    Serial.println("SerialGarbage starts with:\n\r" + SerialGarbageFirstDigit + "\n\r~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r");
//
//    if(SerialGarbageFirstDigit == ""){
//      Serial.println("SerialGarbage String is empty, restarting GPS!");
//      GSMSerial.println("AT+CGPSPWR=1");
//      delay(300);
//      while(GSMSerial.available())
//        Serial.print((char)GSMSerial.read());
//      GSMSerial.println("AT+CGPSRST=0");
//      delay(300);
//      while(GSMSerial.available())
//        Serial.print((char)GSMSerial.read());
//      delay(300);  
//    }
//    else if(SerialGarbageFirstDigit == "0"){
//      Serial.println("GPS isn't fixed! Hold on!");
//      printToSD("GPS isn't fixed!");
//      printToSD(SerialGarbage);
//      //delay(3000);
//    }
//    else {
//      printToSD(TrimmedSerialGarbage);
//    }
//  }
//  //This was a code block to test if I was getting satellites at all.
//  delay(1000);
//  GSMSerial.println("AT+CGPSINF=0\r");
//  while(GSMSerial.available()){
//     //Serial.print("GMSerial: " + GSMSerial.readString());
//    String SerialGarbage0 = GSMSerial.readString();
//    Serial.println("SerialGarbage0 String is:\n\r" + SerialGarbage0 + "\n\r~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r");
//    printToSD(SerialGarbage0);
//  }
//  //
}

void printToSD(String SerialGarbage){
  
  

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("testLogb.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to testLogb.txt...");
    myFile.println(SerialGarbage);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening testLogb.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("testLogb.txt");
  if (myFile) {
    Serial.println("testLogb.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening testLogb.txt");
  }
}

