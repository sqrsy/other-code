# What is it?

This repository contains code for select modules available in the modules repository.

# Programs

* quantiser:
	* Author: sqrsy
	* Purpose: Dual quantiser with scale controlled by switches.
	* Jacks 1 & 2: Input CV.
	* Pots 1 & 2: Transpose input CV (adds up to 1V to input CV).
	* Switches 1-12: Set scale used to quantise input.
	* Jacks 3 & 4: Quantised output.

* sn76489
	* Author: sqrsy
	* Purpose: Provide CV interface to SN76489 chip.
	* Jack 1: Input CV for VCO, converted to chip instructions.
	* Pot 1: Fine tune pitch for VCO.
	* Jack 2: Input CV for VCA, converted to chip instructions.
	* Pot 2: Coarse adjust level for VCA.
	* Switch 1: Toggles whether to write to noise channel. Useful for percussion.
	* Jack 3: Audio out.

# Disclaimer

All files are provided without any guarantee. If you notice a mistake, please submit an Issue and I will fix it.