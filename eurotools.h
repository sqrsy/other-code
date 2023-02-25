int power_int(int a, int b){
  int c = 1;
  for(int i = 0; i < b; i++){
    c = c*a;
  }
   return c;
}

long power_long(int a, int b){
  long c = 1;
  for(int i = 0; i < b; i++){
    c = c*a;
  }
   return c;
}

float power_float(float a, float b){
  float c = exp(b * log(a));
  return c;
}

// given r1 and r2, function will back-calculate real value before voltage division
float read_analog_mV(int pin_in, int r1 = 0, int r2 = 0){

  int x = analogRead(pin_in);
  float mV = 0;
  if(r1 == 0 & r2 == 0){
    mV = x * 4.9;
  }else{
    mV = x * 4.9 * (r1 + r2) / r2;
  }
  return(mV);
}

int read_analog_pct(int pin_in, int max_mV, int r1 = 0, int r2 = 0, int tolerance = 4){

  float mV = read_analog_mV(pin_in, r1, r2);
  float pct_flt = 100 * mV / max_mV;
  int pct = min(pct_flt, 100);
  pct = pct / tolerance * tolerance; // tolerate input variability
  return(pct);
}

// can declare output as int if needed
float pct_as_decimal_offset(int pct, int plus_or_minus, bool reverse = false){

  if(reverse){
    pct = 100 - pct;
  }
  float new_val = (pct - 50) / (50.0 / plus_or_minus);
  if(new_val < 0){
    new_val = max(new_val, -1 * plus_or_minus);
  }else{
    new_val = min(new_val, plus_or_minus);
  }
  return(new_val);
}

float pct_as_base2_offset(int pct, int exp_mid, int plus_or_minus, bool reverse = false){

  if(reverse){
    pct = 100 - pct;
  }
  float exp_offset = pct_as_decimal_offset(pct, plus_or_minus);
  float exp = exp_mid + exp_offset;
  float new_val = power_float(2, exp);
  return(new_val);
}

float mV_to_Hz(int mV, int default_Hz){

  int default_Hz_as_volts = log(default_Hz) / log(2) * 1000;
  float Hz = power_float(2, (default_Hz_as_volts+mV)/1000.0);
  return(Hz);
}

long mV_to_integer(int mV, int max_val, double scale_factor = 1, int max_voltage = 5){

  double pct = mV / (max_voltage * 1000.0);
  int scale_val = scale_factor * pct * max_val;
  int new_val = min(scale_val, max_val);
  return(new_val);
}

int clip_integer(int value, int min, int max){

  int new_val;
  if(value < min){
    new_val = min;
  }else if(value > max){
    new_val = max;
  }else{
    new_val = value;
  }
  return(new_val);
}
