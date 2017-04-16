/*
 * Test program for interrupts that I finally got working. 
 * Uses a single line that goes to both digital pins 2 and 3 (interrupt 0 and 1, respectively).
 * This line needed to be split to a high resistance + ground, as it turns out. The timing code,
 * microsecond delays, and if checks ensure the interrupt isn't affected by switch bouncing or being
 * interrupted by other interrupts (due to bouncing/floating), which were issues while testing it.
 * 
 * Once per second, outputs current status over USB serial connection to PC serial monitor. 
 * Serial Monitor output for each on/off interrupt that occurs.
 */

volatile int state2 = LOW;
volatile int state3 = LOW;
bool counter = false;

void setup() {
  // put your setup code here, to run once:
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Starting...");
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);

  attachInterrupt(0,interrupt1a,FALLING);
  attachInterrupt(1,interrupt2a,RISING);
}

void loop() {
  delay(1000);
  Serial.print("~~~Currently: ");
  Serial.println(digitalRead(2));
  }

void interrupt1(){
  Serial.println("It's low!");}
  
void interrupt2(){
  Serial.println("It's HIGH!");}

void interrupt1a()
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

void interrupt2a()
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
