#include <stdint.h>
#include "hardware/pio.h"

// SNES connection information
#define PIN_CLOCK 0 // pin to attach SNES controller clock
#define PIN_LATCH 1 // pin to attach SNES controller latch
#define PIN_DATA 2 // pin to connect SNES controller data

// Controller HID report structure.
typedef struct {
	uint16_t Button; // 16 buttons; see ControllerButtons_t for bit mapping
	uint8_t  HAT;    // HAT switch; one nibble w/ unused nibble
	uint8_t  LX;     // Left  Stick X
	uint8_t  LY;     // Left  Stick Y
	uint8_t  RX;     // Right Stick X
	uint8_t  RY;     // Right Stick Y
	uint8_t  VendorSpec;
} USB_ControllerReport_Input_t;

// Enumeration for controller buttons.
typedef enum {
	KEY_Y       = 0x01,
	KEY_B       = 0x02,
	KEY_A       = 0x04,
	KEY_X       = 0x08,
	KEY_L       = 0x10,
	KEY_R       = 0x20,
	KEY_ZL      = 0x40,
	KEY_ZR      = 0x80,
	KEY_SELECT  = 0x100,
	KEY_START   = 0x200,
	KEY_LCLICK  = 0x400,
	KEY_RCLICK  = 0x800,
	KEY_HOME    = 0x1000,
	KEY_CAPTURE = 0x2000,
} ControllerButtons_t;



void get_SNESCon_state(PIO pio, uint sm);
void translate_SNES2USB();
void hid_task(void);
void USBCon_init();
