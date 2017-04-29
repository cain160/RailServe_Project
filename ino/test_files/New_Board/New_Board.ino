//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 
String serialText;
boolean GPSfix = false;

void setup()
{
  Serial1.begin(19200);               // the GPRS/GSM baud rate   
  Serial.begin(19200);                 // the GPRS/GSM baud rate   
  Serial1.println("AT+CGPSPWR=1");
  delay(300);
  Serial.println(Serial1.readString());
  Serial1.println("AT+CGPSRST=0");
  delay(300);
  Serial.println(Serial1.readString());
  Serial1.println("AT+CGPSSTATUS?");
  delay(300);
  Serial.println(Serial1.readString());
}

void loop()
{
  waitForFix();
  if(GPSfix){
    Serial.println("Got a fix");
  }
  else{
    Serial.println("Didn't get a fix");
  }
  
  Serial1.println("AT+CGPSSTATUS?");
  delay(300);
  Serial.println(Serial1.readString());
  
  waitForFix();
  
  delay(5000);
  Serial1.println("AT+CGPSPWR=1");
  delay(300);
  Serial1.println("AT+CGPSRST=0");
  delay(300);
  Serial1.println("AT+CGPSSTATUS?");
  delay(300);
  Serial1.flush();
  Serial.println("Reset again");
  
  Serial1.println("AT+CGPSSTATUS?");
  delay(300);
  Serial.println(Serial1.readString());
}

void waitForFix(){
  GPSfix = false;
  int startTime = millis();
  boolean timeout = false;
  Serial1.flush();
  Serial1.println("AT+CGPSSTATUS?");
  serialText = Serial.readString();
  int now = millis();
  
  while(serialText.indexOf("3D Fix") <= 0){
    Serial1.flush();
    Serial1.println("AT+CGPSSTATUS?");
    serialText = Serial1.readString();
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
    Serial.println("Got it!");
    /*
    Serial1.flush();
    Serial1.println("AT+CGPSSTATUS?");
    serialText = Serial1.readString();
    Serial.println("That took: ");
    Serial.println(now - startTime);
    */
  }
}
