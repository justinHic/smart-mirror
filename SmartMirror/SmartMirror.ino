#include "Arduino.h"
#include "model_data.h"

#include <TensorFlowLite.h>


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);

  Serial.println("Ready");
  
}
