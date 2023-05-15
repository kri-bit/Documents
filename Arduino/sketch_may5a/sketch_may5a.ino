#include<Servo.h>
Servo myservo;
void setup(){
myservo.attach(D5);
Serial.begin(9600);
}
void loop(){
int pot = analogRead(A0);
pot=map(pot,0,1024,0,180);
myservo.write(pot);
}