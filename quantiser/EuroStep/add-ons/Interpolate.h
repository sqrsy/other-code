/*
Class Name: Interpolate

Purpose: Manage the interpolation between two byte-wise samples.

Dependencies: None.

Use: Create an instance of the class and configure settings, then run:
-- set_inputs(array_1, array_2, length): point the class to the input arrays
-- set_output(array_3): point the class to where to save the result
-- interpolate(pct): interpolate between the two arrays (as a percent)
-- get_output_at(index): get the interpolated value at an index
*/

class Interpolate {

private:

  byte* sample_1;
  byte* sample_2;
  int sample_length = 0;
  int current_percentage = 0;
  double scale = 0;
  byte* output;

public:

  ///////////////////////////////////////////////////////////////////////////////
  /// Getters and setters
  ///////////////////////////////////////////////////////////////////////////////

  void set_inputs(byte* array_1, byte* array_2, int length) {
    sample_1 = array_1;
    sample_2 = array_2;
    sample_length = length;
  }

  void set_output(byte* array_3) {
    output = array_3;
  }

  byte get_output_at(int index) {
    return output[index];
  }

  ~Interpolate() {
    delete[] sample_1;
    delete[] sample_2;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Do the work
  ///////////////////////////////////////////////////////////////////////////////

  void interpolate(int pct) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    scale = pct / 100.0;
    for (int i = 0; i < sample_length; i++) {
      output[i] = (1 - scale) * sample_1[i] + scale * sample_2[i];
    }
  }
};
