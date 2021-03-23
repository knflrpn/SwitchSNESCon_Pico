/* 
 * The MIT License (MIT)
 *
 * Code last modified 2021 KNfLrPn
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "hardware/gpio.h"

#include "usb_descriptors.h"
#include "SwitchSNESCon_Pico.h"

#include "hardware/pio.h"
// The assembled program:
#include "SNESConRead.pio.h"


//--------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------

// PIO numbers
PIO pio;
uint sm;
// SNES Controller state
uint SNESCon_state;
// USB Controller state
USB_ControllerReport_Input_t current_con;

//--------------------------------------------------------------------
// Main
//--------------------------------------------------------------------
int main(void) {
    board_init();

    // zero-out the controller state
    USBCon_init();

    // Set up USB
    tusb_init();

    // Choose which PIO instance to use
    pio = pio0;
    // Load assembled program into the PIO's instruction memory.
    uint offset = pio_add_program(pio, &SNESConRead_program);
    // Find a free state machine on the chosen PIO (erroring if there are
    // none). Configure it to run the program, and start it, using the
    // helper function included in the .pio file.
    sm = pio_claim_unused_sm(pio, true);
    SNESConRead_program_init(pio, sm, offset, PIN_CLOCK, PIN_LATCH, PIN_DATA);

    // Forever loop
    while (1) {
        get_SNESCon_state(pio, sm);
        gpio_put(25, (bool)(SNESCon_state) ); // LED feedback
        translate_SNES2USB();
        tud_task(); // tinyusb device task
        hid_task();
    }

    return 0;
}

//--------------------------------------------------------------------
// Device callbacks
//--------------------------------------------------------------------

// Invoked when device is mounted
void tud_mount_cb(void) {
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
}

//--------------------------------------------------------------------
// USB and SNES Controller functions
//--------------------------------------------------------------------

void hid_task(void) {

    // Remote wakeup (unused?)
    if (tud_suspended() && false) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }

    // report controller data
    if (tud_hid_ready()) {
        tud_hid_report(0, &current_con, sizeof(USB_ControllerReport_Input_t));
    }
}

/*
/  Pulls data from the PIO.
*/
void get_SNESCon_state(PIO pio, uint sm){
    // If there's data ready, copy it to the global variable SNESCon_state
    if( ! pio_sm_is_rx_fifo_empty(pio, sm) ) {
        SNESCon_state = pio_sm_get(pio, sm);
        SNESCon_state = (~SNESCon_state)&0xFFFF;        // convert to active-1
    }
}

/*
/  Translates button state from the SNES controller to data for the
/  USB controller.
*/
void translate_SNES2USB(){
    uint dpad = (SNESCon_state & 0x0F00) >> 8;           // D pad values
    uint hat;
    // Convert D-pad presses to HAT
    switch(dpad) {
        case 1: // right
        hat = 2;
        break;
        
        case 5: // down+right
        hat = 3;
        break;
        
        case 4: // down
        hat = 4;
        break;
        
        case 6: // down+left
        hat = 5;
        break;
        
        case 2: // left
        hat = 6;
        break;
        
        case 10: // up+left
        hat = 7;
        break;
        
        case 8: // up
        hat = 0;
        break;
        
        case 9: // up+right
        hat = 1;
        break;
        
        default:
        hat = 8;
    }
    current_con.HAT = hat;

    // Translate button presses to the "Buttons" data value
    // for the USB controller.
    uint btns = 0;
    if(SNESCon_state & 0b0100000000000000) // Y
        btns = btns | 1; // Y
    if(SNESCon_state & 0b1000000000000000) // B
        btns = btns | 2; // B
    if(SNESCon_state & 0b0000000010000000) // A
        btns = btns | 4; // A
    if(SNESCon_state & 0b0000000001000000) // X
        btns = btns | 8; // X
    if(SNESCon_state & 0b0000000000100000) // L
        btns = btns | 16; // L
    if(SNESCon_state & 0b0000000000010000) // R
        btns = btns | 32; // R
    if(SNESCon_state & 0b0001000000000000) // Start
        btns = btns | 512; // +
    if(SNESCon_state & 0b0010000000000000) // Select
        btns = btns | 256; // -

    current_con.Button = btns;
}

/*
/  Initialize the buffer and other controller variables.
*/
void USBCon_init(){
    // Configure a neutral controller state
    current_con.LX = 128;
    current_con.LY = 128;
    current_con.RX = 128;
    current_con.RY = 128;
    current_con.HAT = 0x08;
    current_con.Button = 0;
}

//--------------------------------------------------------------------
// Unknown code
//--------------------------------------------------------------------

// Pretty sure this doesn't do anything in the current configuration.
uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

// Also doesn't do anything.
void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

