#include "EuroStep/hardware/quantiser.h"
#include "EuroStep/EuroStep.h"
#include "EuroStep/modules/Quantiser.h"

class make_Quantizer : public EuroStep::EuroStep {
public:

  // Instantiate the Quantizer class
  Quantiser quantiser_A;
  Quantiser quantiser_B;

  int transpose_mV_A;
  int transpose_mV_B;
  int transposed_input_A;
  int transposed_input_B;
  int output_A;
  int output_B;

  void on_step_do() {

    // Initialize the quantizer scales, for example, enabling all notes
    for (int i = 0; i < 12; i++) {
      if (switch_values[i]) {
        quantiser_A.enable_note(i);
        quantiser_B.enable_note(i);
      } else {
        quantiser_A.disable_note(i);
        quantiser_B.disable_note(i);
      }
    }

    // the pots transpose up to 1 octave
    transpose_mV_A = pot_values[0] * 10;  // +1 V at 100%
    transpose_mV_B = pot_values[1] * 10;

    // pass incoming voltage to quantiser A
    transposed_input_A = input_values[0] + transpose_mV_A;
    quantiser_A.run(transposed_input_A);
    output_A = quantiser_A.get_quantised_cv();

    // pass incoming voltage to quantiser B
    transposed_input_B = input_values[1] + transpose_mV_B;
    quantiser_B.run(transposed_input_B);
    output_B = quantiser_B.get_quantised_cv();

    // get outgoing voltage
    send_to_output(1, output_A);  // mismatch between input and output jacks
    send_to_output(0, output_B);
  }
};

make_Quantizer module;
void setup() {
  module.input_mode_is_analog[0] = true;
  module.input_mode_is_analog[1] = true;
  module.output_mode_is_analog[0] = true;  // send output 0 to DAC
  module.output_mode_is_analog[1] = true;  // send output 1 to DAC
  module.debug = false;                    // toggle debug
  module.start();                          // required to initialise pins
}

void loop() {
  module.step();  // runs all user-defined *_do() calls
}
