#include <SoftwareSerial.h>
#define PIN_JSN_SR04T_TX 4
#define PIN_JSN_SR04T_RX 5

SoftwareSerial jsnSerial(PIN_JSN_SR04T_TX, PIN_JSN_SR04T_RX);

void setup() {
  jsnSerial.begin(9600);
}

int getDistance() {
  unsigned int distance;
  byte startByte, h_data, l_data, sum = 0;
  byte buf[3];
  
  startByte = (byte)jsnSerial.read();
  if(startByte == 255){
    jsnSerial.readBytes(buf, 3);
    h_data = buf[0];
    l_data = buf[1];
    sum = buf[2];
    distance = (h_data<<8) + l_data;
    if(((startByte + h_data + l_data)&0xFF) != sum){
      return 0;
    }
    else{
      return distance;
    } 
  } 
  else return 0;
}

void loop() {
  unsigned int distance;

  jsnSerial.write(0x55);
  delay(50);
  if(jsnSerial.available()){
    distance = getDistance();
  }
}