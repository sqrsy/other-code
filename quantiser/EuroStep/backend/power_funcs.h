/**
 * Computes the power of a base raised to an exponent.
 *
 * @param a The base.
 * @param b The exponent (expected to be non-negative).
 * @return The result of a raised to the power of b.
 */
int power_int(int a, int b) {
  int c = 1;
  for (int i = 0; i < b; i++) {
    c = c * a;
  }
  return c;
}

/**
 * Computes the power of a base raised to an exponent.
 *
 * @param a The base.
 * @param b The exponent (expected to be non-negative).
 * @return The result of a raised to the power of b.
 */
long power_long(int a, int b) {
  long c = 1;
  for (int i = 0; i < b; i++) {
    c = c * a;
  }
  return c;
}

/**
 * Computes the power of a base raised to an exponent.
 *
 * @param a The base.
 * @param b The exponent (expected to be non-negative).
 * @return The result of a raised to the power of b.
 */
float power_float(float a, float b) {
  float c = exp(b * log(a));
  return c;
}
