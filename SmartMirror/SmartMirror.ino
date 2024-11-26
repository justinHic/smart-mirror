#include "Arduino.h"
#include "NeuralNetwork.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);

  Serial.println("Ready");

  NeuralNetwork* m_nn = new NeuralNetwork();
  Serial.println("Created Neural Net");
}
