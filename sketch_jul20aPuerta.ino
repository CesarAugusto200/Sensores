int sensorPin = 2; 
int ledPin = 4; 
int sensorState = 0; 

void setup() {
  pinMode(sensorPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200); 
}

void loop() {
  sensorState = digitalRead(sensorPin); 
  if (sensorState == LOW) {
    digitalWrite(ledPin, LOW); 
    Serial.println("Puerta Cerrada"); 
  } else {
    digitalWrite(ledPin, HIGH); 
    Serial.println("Puerta Abierta"); 
  }
  delay(500); // Espera 500 milisegundos antes de leer el sensor nuevamente
}
