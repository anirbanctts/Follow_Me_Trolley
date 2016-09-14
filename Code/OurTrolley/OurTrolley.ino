#include <SoftwareSerial.h>

SoftwareSerial BTT(10, 11); // RX, TX
const int trigPin1 = 4;
const int echoPin1 = 2;
const int trigPin2 = 6;
const int echoPin2 = 3;

const int trigPinObs = 14;
const int echoPinObs = 16;

//first motor
int motor1Pin1 = 7;
int motor1Pin2 = 8;
int enablePin = 9;

//second motor
int secondMotor1Pin1 = 12;
int secondMotor1Pin2 = 13;
int secondEnablePin = 5;



const int distanceTimeout = 275;
const float Kp = .5;
const float Ki = 0.025;
long errorSum = 0;
long error = 0;
long difference = 0;
boolean direction = 0;
float errorCorrection = 0;
boolean finished = false;
boolean oneFire = false;
boolean twoFire = false;
long setPoint1 = 0;
long setPoint2 = 0;
boolean debug = true;
long driveTimeout = 0;
long distance = 0;
long blinkTimer = 0;

void setup()
{
  Serial.begin(38400); //38400  
  BTT.begin(38400);
 
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  // obstacle avoiding sr-04
  pinMode(trigPinObs, OUTPUT);
  pinMode(echoPinObs, INPUT);
  pinMode(17, OUTPUT);
  
Serial.print("hi");
  // set all the other pins you're using as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enablePin, OUTPUT);
  //-------------------------------------
  pinMode(secondMotor1Pin1, OUTPUT);
  pinMode(secondMotor1Pin2, OUTPUT);
  pinMode(secondEnablePin, OUTPUT);


digitalWrite(17, LOW);
  // Setting up motor to move straight
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  
  digitalWrite(secondMotor1Pin1, LOW);
  digitalWrite(secondMotor1Pin2, HIGH);
  
  // set enablePin to 0 so that it starts with stopped condition
  analogWrite(enablePin,0);
  analogWrite(secondEnablePin, 0);


}

void loop()
{

  bool stopFlg = StopToAvoidObstracle();
  if(stopFlg)
  {
    Stop();  
  }
  else
  {
    //digitalWrite(motor1Pin2, LOW);
    //delay(1000);
        // establish variables for duration of the ping, 
        // and the distance result in inches and centimeters:
        long differenceSum, differenceAvg, counter;
        long distanceSum, distanceAvg;
        
        differenceSum = 0;
        differenceAvg = 0;
        counter = 0;
        
        distanceSum = 0;
        distanceAvg = 0;
        
        
          
        for (int i = 0; i < 4; i++)
        {
          long tempInt = doPingDiff();
          
          if (abs(tempInt) < 100){
            differenceSum = differenceSum + tempInt;
            counter++;  
          }
          distanceSum = distanceSum + distance;
          
          delay(20);
        }
        
        if (counter != 0)
        {
          differenceAvg = differenceSum/counter;
          distanceAvg = distanceSum/counter;
        }
        else
        {
          differenceSum = 0;
          distanceAvg = 0;
        }
        
          
        if (differenceSum != 0)
        {            
              updateDrives(differenceAvg, distanceAvg); 
              driveTimeout = 0;            
        }
        else
          driveTimeout++;
              
        if (driveTimeout > 5)
        {
         Stop();
        }
  }
        //delay(2000);
}

long doPingDiff(){
  long timeout = 0;
  long duration = 0;
  boolean started = false;
  
  BTT.print(0);
  delayMicroseconds(500);
        
  digitalWrite(trigPin1, LOW);
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(20);
  digitalWrite(trigPin1, LOW);
  digitalWrite(trigPin2, LOW);
         

  delayMicroseconds(900);
  oneFire = false;
  twoFire = false;
  finished = false;
  setPoint1 = 0;
  setPoint2 = 0;
  difference = 0;  

  while ((oneFire == 0) || (twoFire == 0))
  {
    if (((PIND & B00000100) == 0) && !oneFire){
      oneFire = true;
      setPoint1 = timeout;
    }
    if (((PIND & B00001000) == 0) && !twoFire){
      twoFire = true;
      setPoint2 = timeout;
    }
    delayMicroseconds(1);
    timeout++;
    if (timeout > 19000)
      break;
  }

  // setpoint 1 = echo at receiver 1 in microSec
  // setpoint 2 = echo at receiver 2 in microSec
  
  difference = setPoint1 - setPoint2;
   //distance  = (setPoint1 + setPoint2) / 10 ;

   
  return difference;
}



void updateDrives(long differenceAvg, long distanceAvg){
  error = differenceAvg;
  if ((error < 0) && (direction == 1)){
    direction = 0;
    errorSum = 0;
  }
  
  if ((error > 0) && (direction == 0)){
    direction = 1;
    errorSum = 0; 
  } 
  
  errorSum = error + errorSum;
  errorCorrection = Kp*error + Ki*errorSum;
  
   int turn = 128 + errorCorrection;
   int newSpeed = 128 - distanceAvg/2;
  
  if (turn > 250)
    turn = 250;
  if (turn < 5)
    turn = 5;
    
  if ( newSpeed < 5)
    newSpeed = 5;
  
  if (debug){ 
    Serial.print("Distance Correction ");
    Serial.println(distanceAvg);

  }
  Serial.print("avgggggggg:  - ");
  Serial.println(differenceAvg);
  if (abs(differenceAvg) < 30)
  {
    Serial.println("Move straight");
    MoveStraight();
  }
  else
  {
    if(direction == 0)
    {
      MoveRight();
      Serial.println("Move right");
    }
    else if (direction == 1)
    {
      MoveLeft();
      Serial.println("Move left");
    }
  }
  
}

void MoveStraight()
{
  analogWrite(enablePin,140);
  analogWrite(secondEnablePin, 140);
}

void MoveLeft()
{
  analogWrite(enablePin, 120);
  analogWrite(secondEnablePin, 30);
}

void MoveRight()
{
  analogWrite(enablePin, 30);
  analogWrite(secondEnablePin, 120);  
}

void Stop()
{
  analogWrite(enablePin, 0);
  analogWrite(secondEnablePin, 0);  
}

bool StopToAvoidObstracle()
{
  digitalWrite(trigPinObs, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinObs, LOW);  

  long dur = pulseIn(echoPinObs, HIGH);
  long dist = (dur/2) / 29.1;
Serial.print("Obsdist-");
Serial.println(dist);
  
  if (dist > 0 && dist <= 15)
  {
    //digitalWrite(17, LOW);
    //delay(500);
    digitalWrite(17, HIGH);
    delay(500);
    digitalWrite(17, LOW);
    delay(500);

    Serial.println("stop");
    return true;
  }
  else
  {
    //digitalWrite(17, LOW);
    //delayMicroseconds(10);
    Serial.println("lets go");
    return false;
  }
}



