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
 * @Module FollowMeTrolley - Signal Transmitter
 */

//Import Software Serial library for bluetooth communication over serial port
#include <SoftwareSerial.h>

//Setup Pins for Ultrasonic Transmitter
#define trigPin 2
#define echoPin 3

//Configure Transmission and Receiver Pins for BTT communication and Provide to SoftwareSerial
const int BTT_TriggerPin = 10;
const int BTT_ReceiverPin = 11;
SoftwareSerial BTT(BTT_TriggerPin, BTT_ReceiverPin); 
int data;

//Initiate IO Operation, One Time bootstrapping
void setup() {
  //Setup serial comms at 38400 baud rate for Bluetooth communication and initiate Bluetooth comms
  Serial.begin(38400);
  BTT.begin(38400);
  
  //Initiate Ultrasonic Transmitter,
  //Point to note: Even we are not receiving the signal at the transmitter, still we have to provide the receiving PIN for the circuit to complete
  //SR - 04 does not work as seperate transducers, it is designed to work in couple mode
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

//Continuous infinite running loop
void loop() {
  //Check if any incoming Bluetooth command is available to process
  while(BTT.available())
  { 
    //Read bytes from Serial port
    data = BTT.read();

    //If data == 0, Transmit a high pulse to receivers mounted at Trolley
    if(data == 0)
    {
      TransmitSignal();
    }
    //Clear all the data that might have arrived at Serial Port during the block execution
    BTT.flush();
  }
}

/**
**Transmit Ultrasonic Sound Wave pulse of 20 Microseconds
*First set the transmission PIN to Low then hight followed by again low to reset
*/
void TransmitSignal(){
  //Create a low pulse of 2 MicroSecond
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  //Create a High pulse of 20 MicroSecond
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(20);
  //Create a low pulse of 2 MicroSecond
  digitalWrite(trigPin, LOW);
}

