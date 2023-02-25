bool data[16];
void update_sn_code(bool data[16], int new_value, bool volume_active, bool glitch_switch_active, bool verbose, long FREQUENCY = 8000000){

  data[0] = 1; // 1* let's chip know it's first byte
  data[1] = glitch_switch_active;
  data[2] = glitch_switch_active; // 100 for tone address, 111 for noise address
  data[3] = volume_active; // 1**0 for tone, 1**1 for volume
  
  if(volume_active){

    new_value = 31 - new_value; // invert volume value to attenuation value
    int num_positions = 4; // volume is a 4-bit code
    int positions[num_positions] = {7, 6, 5, 4}; // these are the locations where to write the number
    long dacCode = new_value;
    if(verbose) Serial.print ("The value is: ");
    if(verbose) Serial.println (dacCode);
    for(int i = num_positions - 1; i > -1; i--){
      int base = power_int(2, i);
      int index = positions[i];
      data[index] = dacCode / base;
      dacCode = dacCode % base;
    }

  }else{

    data[8] = 0; // 0* let's chip know it's second byte
    data[9] = 0; // chip doesn't care, but set to 0 for safety

    int num_positions = 10; // tone is a 10-bit code
    int positions[num_positions] = {7, 6, 5, 4, 15, 14, 13, 12, 11, 10}; // these are the locations where to write the number
    long dacCode = FREQUENCY/(32.0*new_value); // 8 Mhz clock divide by 32*Hz
    if(verbose) Serial.print ("The value is: ");
    if(verbose) Serial.println (dacCode);
    for(int i = num_positions - 1; i > -1; i--){
      int base = power_int(2, i);
      int index = positions[i];
      data[index] = dacCode / base;
      dacCode = dacCode % base;
    }
  }
  
  if(verbose){
    Serial.print ("The code is: ");
    Serial.println ("");
    for(int i = 0; i < 16; i++){
      Serial.print (data[i]);
    }
    Serial.println ("");
  }
}
