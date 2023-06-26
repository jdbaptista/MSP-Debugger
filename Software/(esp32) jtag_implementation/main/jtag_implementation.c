#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define HIGH 1
#define LOW 0
#define INPUT GPIO_MODE_INPUT
#define OUTPUT GPIO_MODE_OUTPUT

// JTAG pins are from the perspective of the MSP430
#define RST GPIO_NUM_14 // MSP430 reset                     (RX)  ->  (16) s2 (14/22)(GPIO/PHYSICAL)
#define TMS GPIO_NUM_45 // JTAG state machine control       (TX)  ->  (7)     (45/26)
#define TCK GPIO_NUM_34 // JTAG clock input                 (MO)  ->  (6)     (34/26)
#define TDI GPIO_NUM_33 // JTAG data input and TCLK input   (MI)  ->  (14)    (33/24)
#define TDO GPIO_NUM_21 // JTAG data output                 (21)  ->  (15)    (21/23)
#define TEN GPIO_NUM_13 // JTAG enable                      (SCL) ->  (17)    (13/21)

#define LOCATION 0x00

const uint8_t IR_ADDR_16BIT = 0x83;
const uint8_t IR_ADDR_CAPTURE = 0x84;
const uint8_t IR_DATA_TO_ADDR = 0x85;
const uint8_t IR_DATA_16BIT = 0x41;
const uint8_t IR_DATA_QUICK = 0x43;
const uint8_t IR_BYPASS = 0xFF;
const uint8_t IR_CNTRL_SIG_16BIT = 0x13;
const uint8_t IR_CNTRL_SIG_CAPTURE = 0x14;
const uint8_t IR_CNTRL_SIG_RELEASE = 0x15;
const uint8_t IR_DATA_PSA = 0x44;
const uint8_t IR_SHIFT_OUT_PSA = 0x46;
const uint8_t IR_Prepare_Blow = 0x22;
const uint8_t IR_Ex_Blow = 0x24;
const uint8_t IR_JMB_EXCHANGE = 0x61;

/*
    Shifts an 8-bit JTAG instruction into the JTAG
    instruction register via the TDI. At the same time,
    the 8-bit JTAG ID is shifted out via the TDO. Each
    instruction bit is captured from TDI on the rising
    edge of the TCK. Shifted LSB first.

    Returns: 8-bit JTAG ID
*/
uint8_t IR_SHIFT(uint8_t input_data) {
    uint16_t ret = 0x00;
    int prevTDI = gpio_get_level(TDI);

    // set TAP machine to Shift-IR state
    // TMS set through falling and rising edge
    gpio_set_level(TMS, HIGH);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 1 (Select DR)

    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 1 (Select IR)

    gpio_set_level(TMS, LOW);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 0 (Capture IR)

    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 0 (Shift IR)

    // shift data into IR
    uint8_t bit;
    for (int i = 0; i < 7; i++) {
        bit = input_data >> i;
        bit &= 0x01; // send the selected bit
        gpio_set_level(TDI, (uint32_t) bit);
        gpio_set_level(TCK, LOW);
        gpio_set_level(TCK, HIGH);
        uint16_t output = gpio_get_level(TDO);
        ret |= output << (7 - i);
    }

    // Send MSB and return to Run/Idle
    bit = input_data >> 7;
    bit &= 0x01;
    gpio_set_level(TMS, HIGH);
    gpio_set_level(TDI, bit);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 1 (Exit IR)
    ret |= gpio_get_level(TDO);

    gpio_set_level(TDI, prevTDI);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 1 (Update IR)

    gpio_set_level(TMS, LOW);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 0 (IDLE)

    for (int i = 0; i < 4; i++) {
        gpio_set_level(TCK, LOW);
        gpio_set_level(TCK, HIGH);
    }

    return ret;
}

/*
    Shifts a 16-bit word into the JTAG data register (DR).
    The word is shifted, MSB first, via the TDI. At the
    same time, the last captured and stored value in the
    addressed data register is shifted out via the TDO. A
    new bit is present at TDO with a falling edge of TCK.
    Shifted MSB first.

    Returns: Last captured and stored value in the
    addressed data register.
*/
uint16_t DR_SHIFT(uint16_t input_data) {
    uint16_t ret = 0x0000;
    int prevTDI = gpio_get_level(TDI);

    // set TAP machine to Shift-DR state
    // TMS set through falling and rising edge
    gpio_set_level(TMS, HIGH);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 1 (Select DR)

    gpio_set_level(TMS, LOW);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 0 (Capture DR)

    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 0 (Shift DR)

    // shift data into DR
    uint16_t bit;
    for (int i = 15; i > 0; i--) {
        bit = input_data >> i;
        bit &= 0x0001; // send the selected bit
        gpio_set_level(TDI, (uint32_t) bit);
        gpio_set_level(TCK, LOW);
        gpio_set_level(TCK, HIGH);
        uint16_t output = gpio_get_level(TDO);
        ret |= output << i;
    }
    // Send LSB and return to Run/Idle
    bit = input_data;
    bit &= 0x0001;
    gpio_set_level(TMS, HIGH);
    gpio_set_level(TDI, bit);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 1 (Exit DR)
    ret |= gpio_get_level(TDO);

    gpio_set_level(TDI, prevTDI);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 1 (Update DR)

    gpio_set_level(TMS, LOW);
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH); // 0 (IDLE)

    for (int i = 0; i < 4; i++) {
        gpio_set_level(TCK, LOW);
        gpio_set_level(TCK, HIGH);
    }

    return ret;
}

/*
    Sets TCLK to LOW, which acts as the falling edge of
    the CPU clock. Executed in the Run/Idle state. Note
    that the MSP430 is not pipelined, so a full TCLK cycle
    executes the CPU instruction located at the PC.
*/
void ClrTCLK() {
    gpio_set_level(TDI, LOW);
}

/*
    Sets TCLK to HIGH, which acts as the rising edge of
    the CPU clock. Executed in the Run/Idle state. Note
    that the MSP430 is not pipelined, so a full TCLK cycle
    executes the CPU instruction located at the PC.
*/
void SetTCLK() {
    gpio_set_level(TDI, HIGH);
}

/*
    Takes the CPU under JTAG Control.
*/
void GetDevice() {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT((uint16_t) 0x2401);
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    printf("Syncing CPU...\n");
    while (true) {
        uint16_t TDOword = DR_SHIFT((uint16_t) 0x0000);
        if ((TDOword & 0x0200) != 0) {
            printf("Sync Successful!\n");
            return;
        }
    }   
}

/*
    Releases CPU from JTAG control. The target CPU
    starts program execution with the address stored
    at location 0x0FFFE (reset vector).
    
    This function is very distinct from ReleaseCPU!
*/
void ReleaseDevice() {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT((uint16_t) 0x2C01); // apply reset
    DR_SHIFT((uint16_t) 0x2401); // remove reset
    IR_SHIFT(IR_CNTRL_SIG_RELEASE);
}

/*
    Sets the CPU to instruction-fetch state. This is used
    to execute an instruction presented by a host over the
    JTAG port.
*/
void SetInstrFetch() {
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    uint16_t data = DR_SHIFT((uint16_t) 0x0000);
    for (int i = 0; i < 8; i++) {
        printf("InstrFetch: 0x%X\n", data);
        if ((data & 0x0080) != 0) return;
        ClrTCLK();
        SetTCLK();
    }
    printf("SetInstrFetch Unsuccessful!\n");
}

/*
    Loads the target device CPU's program counter 
    with the desired 16-bit address.
*/
void SetPC(uint16_t addr) {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT((uint16_t) 0x3401);
    IR_SHIFT(IR_DATA_16BIT);
    DR_SHIFT((uint16_t) 0x4030);
    ClrTCLK();
    SetTCLK();
    DR_SHIFT(addr);
    ClrTCLK();
    SetTCLK();
    IR_SHIFT(IR_ADDR_CAPTURE);
    ClrTCLK();
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT((uint16_t) 0x2401);
}

/*
    Force a power-up reset of CPU
*/
void ExecutePOR() {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT((uint16_t) 0x2c01);
    DR_SHIFT((uint16_t) 0x2401);
    ClrTCLK();
    SetTCLK();
    ClrTCLK();
    SetTCLK();
    ClrTCLK();
    IR_SHIFT(IR_ADDR_CAPTURE);
    SetTCLK();
}

/*
    Stopping of the CPU via the HALT_JTAG bit of the JTAG
    control signal register, which is set to 1 here.
*/
void HaltCPU() {
    // Execute JMP $ instr to maintain state
    IR_SHIFT(IR_DATA_16BIT);
    DR_SHIFT((uint16_t) 0x3FFF);
    ClrTCLK();
    // set halt bit in cntrl signal
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT((uint16_t) 0x2409);
    SetTCLK();
}

/*
    Starting of the CPU via the HALT_JTAG bit of the JTAG
    control signal register, which is set to 0 here.
*/
void ReleaseCPU() {
    ClrTCLK();
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT((uint16_t) 0x2401);
    IR_SHIFT(IR_ADDR_CAPTURE);
    SetTCLK();
}

/*
    Reads one word (2 bytes) of memory at addr.
*/
uint16_t ReadMem(uint16_t addr) {
    ClrTCLK();
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT((uint16_t) 0x2409); // one word, not byte
    IR_SHIFT(IR_ADDR_16BIT);
    DR_SHIFT(addr);
    IR_SHIFT(IR_DATA_TO_ADDR);
    SetTCLK();
    ClrTCLK();
    uint16_t data = DR_SHIFT((uint16_t) 0x0000);
    return data;
}

void WriteMem(uint16_t addr, uint16_t data) {
    ClrTCLK();
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT((uint16_t) 0x2408);
    // printf("\tcontrol check: \n");
    // IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    // printf("\tcontrol signal is: 0x%.4x\n", DR_SHIFT(0x1111));
    IR_SHIFT(IR_ADDR_16BIT);
    DR_SHIFT(addr);
    // printf("\taddress check: \n");
    // IR_SHIFT(IR_ADDR_CAPTURE);
    // printf("\tAddress is: 0x%.4x\n", DR_SHIFT(0x4444));
    IR_SHIFT(IR_DATA_TO_ADDR);
    DR_SHIFT(data);
    SetTCLK();
}

void RWTest() {
    // write data
    uint16_t addr1 = 0xFFF0; // part of RAM (I think)
    uint16_t addr2 = 0xFF88;
    uint16_t addr3 = 0x0332;
    uint16_t addr4 = 0x0200;
    // WriteMem(addr1, 0xDEAD);
    // WriteMem(addr2, 0xBEEF);
    WriteMem(addr3, 0xB0BA);
    WriteMem(addr4, 0xCAFE);
    ReadMem(addr1);
    ReadMem(addr2);
    ReadMem(addr3);
    ReadMem(addr4);
}

void ReadCode(uint16_t start_addr, uint16_t stop_addr) {
    for (uint16_t curr_addr = start_addr; curr_addr < stop_addr; curr_addr+=4) {
        if (curr_addr % 64 == 0) {
            printf("\nAddress 0x%.4x: ", curr_addr);
        }
        printf("0x%.4x ", ReadMem(curr_addr));
    }
    printf("\n");
}

void RegisterTest() {
    uint8_t output;
    for (uint8_t i = 0; i < 10; i++) {
        output = IR_SHIFT(i);
        if (output != (uint8_t) 0x89) {
            printf("IR_SHIFT failed to return JTAG ID!\n");
            return;
        }
    }
    printf("IR_SHIFT test successful...\n");
    printf("Setting IR to IR_ADDR_16BIT, ie. 0x83\n");
    IR_SHIFT((uint8_t) 0x83);
    uint16_t output2;
    DR_SHIFT((uint16_t) 0);
    output2 = DR_SHIFT((uint16_t) 0x4411);
    if (output2 != (uint16_t) 0) {
        printf("DR_SHIFT failed to return previous input data!\n");
        printf("Expected: 0x%x\n", (uint16_t) 0x0000);
        printf("Received: 0x%x\n", (uint16_t) output2);
        return;
    }
    output2 = DR_SHIFT((uint16_t) 0xDEAD);
    if (output2 != (uint16_t) 0x4411) {
        printf("DR_SHIFT failed to return previous input data! %x\n", 0x4411);
        return;
    }
    output2 = DR_SHIFT((uint16_t) 0xDEAD);
    if (output2 != (uint16_t) 0xDEAD) {
        printf("DR_SHIFT failed to return previous input data! %x\n", 0xDEAD);
        return;
    }

    printf("Setting IR to IR_BYPASS, ie. 0xFF\n");
    IR_SHIFT((uint8_t) 0xFF);
    DR_SHIFT((uint16_t) 0);
    output2 = DR_SHIFT((uint16_t) 0x4411);
    if (output2 != (uint16_t) 0x2208) {
        printf("DR_SHIFT failed to return previous input data! %x\n", 0x2208);
        return;
    }
    DR_SHIFT((uint16_t) 0xFFFF);
    output2 = DR_SHIFT((uint16_t) 0x8EAD);
    if (output2 != (uint16_t) 0x4756) {
        printf("DR_SHIFT failed to return previous input data! %x\n", 0x4756);
        return;
    }
    DR_SHIFT((uint16_t) 0);
    output2 = DR_SHIFT((uint16_t) 0xBEEF);
    if (output2 != (uint16_t) 0x5F77) {
        printf("DR_SHIFT failed to return previous input data! %x\n", 0x5F77);
        return;
    }

    printf("DR_SHIFT test successful...\n");
    return;
}

/*
    Drives one JTAG command on the MSP430 via standard
    4-Wire JTAG signals. Specifically, it reads one byte
    of memory at a given location. Refer to documentation:
    https://www.ti.com/lit/ug/slau320aj/slau320aj.pdf
*/
void app_main(void)
{
    // configure pins
    gpio_reset_pin(RST);
    gpio_reset_pin(TMS);
    gpio_reset_pin(TCK);
    gpio_reset_pin(TDI);
    gpio_reset_pin(TDO);
    gpio_reset_pin(TEN);
    gpio_set_direction(RST, OUTPUT);
    gpio_set_direction(TMS, OUTPUT);
    gpio_set_direction(TCK, OUTPUT);
    gpio_set_direction(TDI, OUTPUT);
    gpio_set_direction(TDO, INPUT);
    gpio_set_direction(TEN, OUTPUT);
    gpio_set_pull_mode(TDO, GPIO_PULLDOWN_ONLY);

    // enable JTAG access: case 2a, Fig.2-13
    // RST held low for JTAG, high for SBW
    gpio_set_level(RST, HIGH);
    gpio_set_level(TEN, LOW);
        // there is a ~28 microsecond delay
    gpio_set_level(TEN, HIGH);
    gpio_set_level(RST, LOW);
    gpio_set_level(TEN, LOW);
    gpio_set_level(TEN, HIGH);
    gpio_set_level(RST, HIGH);

    // Move TAP FSM to Run/IDLE for fuse check
    gpio_set_level(TMS, HIGH);
    for (int i = 0; i < 6; i++) {
        gpio_set_level(TCK, LOW);
        gpio_set_level(TCK, HIGH);  // FSM: TLR
    }
    gpio_set_level(TMS, LOW);
    gpio_set_level(TDI, HIGH);  // FSM: IDLE
    gpio_set_level(TCK, LOW);
    gpio_set_level(TCK, HIGH);

    gpio_set_level(TCK, LOW); 
    gpio_set_level(TCK, HIGH);

    // fuse check
    gpio_set_level(TMS, HIGH);   
    gpio_set_level(TMS, LOW);   
    gpio_set_level(TMS, HIGH);
    gpio_set_level(TMS, LOW);
    gpio_set_level(TMS, HIGH);
    gpio_set_level(TMS, LOW);

    // Sync and halt CPU
    GetDevice();
    SetInstrFetch();
    printf("Halting CPU...\n");
    HaltCPU();


    // read data
    printf("\n");
    for (uint16_t curr_start = 0xC000; curr_start <= 0xE000; curr_start += (uint16_t) 0x1000) {
        ReadCode(curr_start, curr_start + 0x1000);
        printf("\n");
    }

    // Exit JTAG
    printf("\n");
    printf("Releasing CPU...\n");
    ReleaseCPU();
    // relinquish JTAG access
    ReleaseDevice();
    gpio_set_level(TEN, LOW);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    // gpio_set_level(RST, LOW);
    // vTaskDelay(1 / portTICK_PERIOD_MS);
    // gpio_set_level(RST, HIGH);
}
