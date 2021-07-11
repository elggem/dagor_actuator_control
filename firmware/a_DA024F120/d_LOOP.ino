
//###########################################
// LOOP
//###########################################

// Do not add delay inside the loop void.
void loop() {
  
  // These functions have to run as fast as possible
  timeManagement();

  motor.loopFOC();
  //command.run();
  motor.move();

  /*
  float ea = motor.electricalAngle();
  DQCurrent_s current = current_sense.getFOCCurrents(ea);
  Serial.print(current.d);
  Serial.print(", ");
  Serial.println(current.q);
  printCurrents(true);
  */
  
  
//  PhaseCurrent_s currents = current_sense.getPhaseCurrents();
//  Serial.print(currents.a); // milli Amps
//  Serial.print("\t");
//  Serial.print(currents.b); // milli Amps
//  Serial.print("\t");
//  Serial.println(currents.c); // milli Amps
  
//  Serial.print("\t");
//
//  wifi_ps_type_t config;
//  esp_wifi_get_ps(&config);
//  Serial.println(config);


  // tested: wifi_ant_config.tx_ant_mode (stays), max_tx_power (stays), wifi_ps_type_t

    outputData.position = sensor.getAngle() - sensorOffset;

    PhaseCurrent_s currents = current_sense.getPhaseCurrents();
    outputData.current_a = currents.a;
    outputData.current_b = currents.b;

    
  // Fixed rate functions
  // Functions inside this "if" will execute at a 5hz rate. Un/ comment the functions that you wish to use.
  if(stateT >= 1000000/callerFixedFreq){
    stateT = 0;

    // Un/ comment the functions bellow that you wish to use.
    tempStatus();
    faultStatus();
    //voltageMonitor(true);
    //rotorPosition();
    //printCurrents(true);
    
    float vOut = analogRead(vTemp);
    outputData.temperature = (((vOut*3.3)/4095)-1.8577)/-0.01177;
    
  }

}
