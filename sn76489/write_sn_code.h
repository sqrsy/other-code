// set up all PINS for the SN audio chip
int PIN_WE = 5;
int PIN_CE = 6;
int PIN_D0 = 12;
int PIN_D1 = 11;
int PIN_D2 = 10;
int PIN_D3 = 9;
int PIN_D4 = 8;
int PIN_D5 = 2;
int PIN_D6 = 3;
int PIN_D7 = 4;

void write_sn_code(bool data[16]){

  // write the first byte according for data 0-7
  digitalWrite(PIN_CE, 0);
  digitalWrite(PIN_D0, data[7]);
  digitalWrite(PIN_D1, data[6]);
  digitalWrite(PIN_D2, data[5]);
  digitalWrite(PIN_D3, data[4]);
  digitalWrite(PIN_D4, data[3]);
  digitalWrite(PIN_D5, data[2]);
  digitalWrite(PIN_D6, data[1]);
  digitalWrite(PIN_D7, data[0]);
  digitalWrite(PIN_WE, 0);
  delay(1);
  digitalWrite(PIN_CE, 1);
  digitalWrite(PIN_WE, 1);

  // write the first byte according for data 8-15
  if(data[1] == 0 & data[2] == 0 & data[3] == 0){ // this is a tone, must write second byte
  
    digitalWrite(PIN_CE, 0);
    digitalWrite(PIN_D0, data[15]);
    digitalWrite(PIN_D1, data[14]);
    digitalWrite(PIN_D2, data[13]);
    digitalWrite(PIN_D3, data[12]);
    digitalWrite(PIN_D4, data[11]);
    digitalWrite(PIN_D5, data[10]);
    digitalWrite(PIN_D6, data[9]);
    digitalWrite(PIN_D7, data[8]);
    digitalWrite(PIN_WE, 0);
    delay(1);
    digitalWrite(PIN_CE, 1);
    digitalWrite(PIN_WE, 1);
  }
}
