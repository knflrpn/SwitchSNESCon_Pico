# SwitchSNESCon_Pico
Translation from SNES controller to Switch (or PC) controller using a Raspberry Pico

The included .uf2 can be put on the Pico directly.  It uses
- GPIO 0 for clock
- GPIO 1 for latch
- GPIO 2 for data

You can power the SNES controller directly from Pico 5V (pin 40).  The outputs to the SNES controller (clock and latch) work fine with 3.3 V.

**The SNES controller puts out 5 V and the Pico GPIO is 3.3 V**, so don't directly connect the SNES controller's data to the Pico.  That said, I haven't had any issue **using a 10k resistor inline with the data**.  A proper level shifter is of course a better solution.
