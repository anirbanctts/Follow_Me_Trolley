/**
 * Copyright 2016 
 *
 * You are hereby granted a non-exclusive, worldwide, royalty-free license to
 * use, copy, modify, and distribute this software in source code or binary
 * form.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE
 *
 * @Module FollowMeTrolley
 */

//Import Software Serial library for bluetooth communication over serial port
#include <SoftwareSerial.h>

//Configure Transmission and Receiver Pins for BTT communication and Provide to SoftwareSerial
const int BTT_TriggerPin = 10;
const int BTT_ReceiverPin = 11;
SoftwareSerial BTT(BTT_TriggerPin, BTT_ReceiverPin); 

//Setup Pins for Ultrasonic Transmitter and Receiver
const int TriggerPin_Left = 4;
const int EchoPin_Left = 2;
const int TriggerPin_Right = 6;
const int EchoPin_Right = 3;
const int TriggerPin_Front = 14;
const int EchoPin_Front = 16;
const int BuzzerPin = 17;

//Setup Pins For Left BO Motor Control 
int LeftMotor_DC_Pin = 7; //Direction Controller
int LeftMotor_BC_Pin = 8; //Break Controller
int LeftMotor_SC_Pin = 9; //Speed Controller

//Setup Pins For Right BO Motor Control
int RightMotor_DC_Pin = 12; //Direction Controller
int RightMotor_BC_Pin = 13; //Break Controller
int RightMotor_SC_Pin = 5; //Speed Controller

//Setup correction 
long errorSum = 0;
long error = 0;
long difference = 0;
boolean direction = 0;
float errorCorrection = 0;
boolean leftReceived = false;
boolean rightReceived = false;
long time_LeftReceiver = 0;
long time_RightReceiver = 0;
long driveTimeout = 0;
long distance = 0;


//Initiate IO Operation, One Time bootstrapping
void setup()
{
  //Setup serial comms at 38400 baud rate for Bluetooth communication and initiate Bluetooth comms
  Serial.begin(38400); 
  BTT.begin(38400);

  //Initiate Left Ultrasonic sensor 
  pinMode(TriggerPin_Left, OUTPUT);
  pinMode(EchoPin_Left, INPUT);
  
  //Initiate Right Ultrasonic sensor
  pinMode(TriggerPin_Right, OUTPUT);
  pinMode(EchoPin_Right, INPUT);

  //Initiate Front Ultrasonic sensor
  pinMode(TriggerPin_Front, OUTPUT);
  pinMode(EchoPin_Front, INPUT);

  //Initiate Pin for Buzzer/Horn 
  pinMode(BuzzerPin, OUTPUT);
  digitalWrite(BuzzerPin, LOW);  

  //Initiate Left Motor Pins
  pinMode(LeftMotor_DC_Pin, OUTPUT);
  pinMode(LeftMotor_BC_Pin, OUTPUT);
  pinMode(LeftMotor_SC_Pin, OUTPUT);

  //Initiate Right Motor Pins
  pinMode(RightMotor_DC_Pin, OUTPUT);
  pinMode(RightMotor_BC_Pin, OUTPUT);
  pinMode(RightMotor_SC_Pin, OUTPUT);

  // Setting up both motor to move straight
  digitalWrite(LeftMotor_DC_Pin, HIGH);
  digitalWrite(LeftMotor_BC_Pin, LOW);
  digitalWrite(RightMotor_DC_Pin, LOW);
  digitalWrite(RightMotor_BC_Pin, HIGH);
  
  // set Speed for both the wheels to 0, so that it starts with stopped condition
  analogWrite(LeftMotor_SC_Pin,0);
  analogWrite(RightMotor_SC_Pin, 0);
}

void loop()
{
    //Check if trollet is about to hit any object then Stop the trolley and return
    bool stopFlg = StopToAvoidObstracle();
    if(stopFlg)
    {
      return;
    }

    // establish variables for duration of the ping, 
    // and the distance result in inches and centimeters:
    long differenceSum, differenceAvg, counter;
    long distanceSum, distanceAvg;
    
    differenceSum = 0;
    differenceAvg = 0;
    counter = 0;
    
    distanceSum = 0;
    distanceAvg = 0;
    
    
    //For proper Accuracy read 5 Pings consecutively and then make the decision to Move and Choose the direction       
    for (int i = 0; i < 5; i++)
    {
      long tempInt = calculatePingDifference();
      
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
      differenceAvg = 0;
      distanceAvg = 0;
     }
    
      
    if (differenceSum != 0)
    {            
      updateDrives(differenceAvg, distanceAvg); 
      driveTimeout = 0;            
    }
    else
      driveTimeout++;

    //If receiver is unable to receive desired range for 5 consecutive cycle then Stop the trolley
    //Let the Shopper Mannualy aligned the Trolley and Remote       
    if (driveTimeout > 5)
    {
     Stop();
    }
}

//Calculate the difference of Receive Time between Left and Right Receiver
// + value represent Transmitter is moving towards Right
// - value represent Transmitter is moving towards Left
long calculatePingDifference(){
  long timeout = 0;
  long duration = 0;
  boolean started = false;
  
  BTT.print(0);
  delayMicroseconds(500);
        
  digitalWrite(TriggerPin_Left, LOW);
  digitalWrite(TriggerPin_Right, LOW);
  delayMicroseconds(2);
  digitalWrite(TriggerPin_Left, HIGH);
  digitalWrite(TriggerPin_Right, HIGH);
  delayMicroseconds(20);
  digitalWrite(TriggerPin_Left, LOW);
  digitalWrite(TriggerPin_Right, LOW);

  //Give fair ammount of time to establish communication between Transmitter and Receiver
  delayMicroseconds(900);

  //Set Flag for both the Receiver to received status false
  leftReceived = false;
  rightReceived = false;
  time_LeftReceiver = 0;
  time_RightReceiver = 0;
  difference = 0;  

  while ((leftReceived == 0) || (rightReceived == 0))
  {
    //PIND -- Direct Port manipulation for Microsecond Accuracy
    if (((PIND & B00000100) == 0) && !leftReceived){
      leftReceived = true;
      time_LeftReceiver = timeout;
    }
    if (((PIND & B00001000) == 0) && !rightReceived){
      rightReceived = true;
      time_RightReceiver = timeout;
    }
    delayMicroseconds(1);
    timeout++;

    //If Signal is not received within 19 Milli Seconds then exit and consider Stopping the trolley
    if (timeout > 19000)
      break;
  }
  
  difference = time_LeftReceiver - time_RightReceiver;
  return difference;
}


//Calculate Trolley direction and Speed and Update the Motor Driver
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

  if (abs(differenceAvg) < 30)
  {
    Serial.println("Move straight");
    MoveStraight();
  }
  else
  {
    if(direction == 0)
    {
      //Move in Right Direction
      MoveRight();
    }
    else if (direction == 1)
    {
      //Move in Left Direction
      MoveLeft();
    }
  }
  
}

void MoveStraight()
{
  analogWrite(LeftMotor_SC_Pin,130);
  analogWrite(RightMotor_SC_Pin, 130);
}

void MoveLeft()
{
  analogWrite(LeftMotor_SC_Pin, 120);
  analogWrite(RightMotor_SC_Pin, 30);
}

void MoveRight()
{
  analogWrite(LeftMotor_SC_Pin, 30);
  analogWrite(RightMotor_SC_Pin, 120);  
}

void Stop()
{
  analogWrite(LeftMotor_SC_Pin, 0);
  analogWrite(RightMotor_SC_Pin, 0);  
}

bool StopToAvoidObstracle()
{
  digitalWrite(TriggerPin_Front, LOW);
  delayMicroseconds(2);
  digitalWrite(TriggerPin_Front, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin_Front, LOW);  

  long dur = pulseIn(EchoPin_Front, HIGH);
  long dist = (dur/2) / 29.1;
  Serial.print("Obstruction at :");
  Serial.println(dist);
  if (dist > 0 && dist <= 25)
  {
    Stop();  
    digitalWrite(BuzzerPin, HIGH);
    delay(500);
    digitalWrite(BuzzerPin, LOW);
    delay(500);

    Serial.println("stop");
    return true;
  }
  delay(200);
  Serial.println("Keep running");
  return false;
}



