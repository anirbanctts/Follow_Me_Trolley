#define trigPin 2
#define echoPin 5

void setup() {
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  long duration, distance;
  //Trigger Sensor
  TriggerSensors();
  
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;

  if (distance >= 400 || distance <= 2){
    Serial.println("Out of range1");
  }
  else {
    Serial.print(distance);
    Serial.println(" cm1");
  }
}

void TriggerSensors(){
  //digitalWrite(trigPin, LOW);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  }
