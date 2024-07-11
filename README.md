## Introduction
This project is a combination of firmware and hardware that displays the assembly instructions stored in the flash memory of an MSP430G2553, specifically the 20 pin DIP package. Please navigate to each section as necessary: [Example Operation](#example-operation), [Firmware](#firmware).

The device is shown below and supports the following button interactions:
* **UP**: Move back one instruction. Note that moving back one instruction is equivalent to moving lower in memory because the program counter increments positively each target CPU cycle.
  
* **DOWN**: Move forward one instruction. Note that moving forward one instruction is equivalent to moving higher in memory because the program counter incrememnts positively each target CPU cycle.
  
* **FOLLOW JMP**: When the cursor is on a jump instruction, this button moves the cursor to the location specified by the jump.
  
* **BIN <-> ASM**: Switch the display view between machine code (binary), displayed in hexadecimal, and RISC assembly instructions. This button highlights that assembly instructions on the target correspond to variable length machine code.
  
* **RESET**: Move the cursor back to `0xC000`, which is the start of code memory. This is implemented by simply resetting the MCU.
  
* **USB RESET**: Reset the USB to UART bridge. USB data lines are currently in a broken state, so this button does nothing.

<br>
<p align="center">
<img src="https://github.com/jdbaptista/MSP-Debugger/assets/18275346/d3271cbf-acb2-4833-b5aa-033f23f0a4fc" width=50% height=50%>
</p>

## Example Operation
The following example code from Texas Instruments is used in this example of the device's operation. The code blinks P1.0 repeatedly. Note that all assembly instructions displayed by the device are confirmed to be correct by Code Composer Studio's professional disassembler.
```C
#include <msp430.h>				

void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;		// stop watchdog timer
	P1DIR |= 0x01;					// configure P1.0 as output

	volatile unsigned int i;		// volatile to prevent optimization

	while(1)
	{
		P1OUT ^= 0x01;				// toggle P1.0
		for(i=10000; i>0; i--);     // delay
	}
}
```
The target is loaded with the assembled machine code for this program and placed in the MSP-Debugger. Unfortunately, the display and USB to UART bridge are currently broken, so an MSP-FET (from any MSP430 development board) is connected to the JTAG header of the debugger. The MSP-FET's backchannel UART to USB bridge is used to convey instructions to the user, operating at** 9600 baud with an 8-bit frame, no parity bit, and importantly 2 stop bits** (for data integrity). The UART encoding is ISO-8859-1, which is the default in CCS. Immediately after RESET the following instruction are extracted from the target and displayed.
```
0xC000: SUB.B #2 SP <
0xC002: MOV.B #0x5A80 &0x0120
0xC008: BIS.B #1 &0x0022
0xC00C: XOR.B #1 &0x0021
```
Using the **DOWN** button moves the cursor from `0xC000` to `0xC002`.
```
0xC002: MOV.B #0x5A80 &0x0120 <
0xC008: BIS.B #1 &0x0022
0xC00C: XOR.B #1 &0x0021
0xC010: MOV.B #0x2710 0x0000(SP)
```
An extremely well trained eye will recognize `&0x0120` as the location of the `WDTCTL` register, which can be confirmed by the peripheral file map on page 18 of the [MSP430G2x53 Datasheet](https://www.ti.com/lit/ds/symlink/msp430g2553.pdf?ts=1720602399535&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSP430G2553%253Futm_source%253Dgoogle%2526utm_medium%253Dcpc%2526utm_campaign%253Depd-null-null-GPN_EN-cpc-pf-google-kr%2526utm_content%253DMSP430G2553%2526ds_k%253D%257B_dssearchterm%257D%2526DCM%253Dyes%2526gad_source%253D1%2526gclid%253DCjwKCAjw4ri0BhAvEiwA8oo6Fw8EdAuptheRVE8vx0Q2j3G3iyDjTU-aOLXieh8M-imSlC9cQUR6_hoClwEQAvD_BwE%2526gclsrc%253Daw.ds%2526bm-verify%253DAAQAAAAJ_____3QLXOZbbdruMI2eB4mFy5KSyEb-yU9yfcNR8-6ZE-F4IlQA15TnUbnZYGHUclM_YK0JJvwZfmZV1vz0BoNFahgPHVY7BCm4W1e8hj6dfjPbReoJt03aw_CYG9J3Y8_jl0rNTcSpbsnRtODv2ub_rMMIkYIZ8sZN9mj6PYFAQ1CDy5mA6PGPGiCbLLn4tzBTBUOk0bJGTj9kveVfehS7i0km-maJESLOX2xoAFatjT2Bv-a8B9gvZcxtGENvp6eMYJDLbmRf67x4X7TE7tt88LH3n9FraEEjZPNjqllkWuL5OOONuBvPF598xP5kkYnJk3wzH9hIroxr1CUU2mTA124pgKzPR_rSHUN9RBbZlIETbuTA60Tk97_1ntDEeGNxAcWS2jB9cD66YYZ0SP0K35RqUrqlAPRsbdokSfG5fS__GGfM8jKUantdnKKpmyNt6DdoRJ1r1NB9CKWKb-SPG5tTHcpbzj1IFa38Zdux2S1X93eAYTW28T1i1pfPwFwJtyAuMkVxw52pn_Y16Fv4n5pcFP_gVm7DHllf_3j6l4I_ipVjaMjKLlI1TF2-Ir8mL8IGx4NuPwbIxyWDxJgmZ_jNEwqQak_lFWplGfgBfkp1_twV8Zrt-tub-1XrNvhwLE4xQLYsvy4ZATQQBHijVjMIYRRXAuL75RCAAUYcWCXUhR5zKtjJhYKP3NkYnMghe8Nj_tdsmRdDuz-CPD87). Someone without access to the C source code can reference page 363 of the [MSP430G2xx User Guide](https://www.ti.com/lit/ug/slau144k/slau144k.pdf?ts=1720622345813&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSP430F2274-EP%253Fbm-verify%253DAAQAAAAJ_____2GXDeU4IIaEds7sCQitbINeDMGTgqNh7F20H7LVJyrs5LsYNIkQrF5ycgp2Z0Yzd4j0V7synm_dgqfLwBr7S5VesPGUssLDDUfg7mGj_mgtHY82RE3zgMG80y9GDnAMYK8hv1U5Bq9QLLAvt0XehujFdDcILVLVdafgQQ4y_G74JWcE-jRRkwir5ItF99koK6Pg_AP0E-3rrXrP4YO819zT--lXgtbtaeA9YqF_kJtLNL18SaURyUQHTQi5XznOoq8sb0SznRHjyRujqGne18cY0YsrFImOVD6oPzmZqavXlbABc6vvCKw) to determine that the value `#0x5A80` being moved into `WDTCTL` register writes the correct watchdog password `0x5A00` (`WDTPW`) and suspends the watchdog's operation with `0x0080` (`WDTHOLD`). The watchdog timer will reset the microcontroller if not 'pet' in time by setting `WDTCNTCL`, so this allows the microcontroller to operate without fear of the watchdog.

Using the **BIN <-> ASM** button switches the display from assembly to machine code.
```
0xC002: 0x40B2 0x5A80 0x0120 <
0xC008: 0xD3D2 0x0022 
0xC00C: 0xE3D2 0x0021 
0xC010: 0x40B1 0x2710 0x0000 
```
In this view it can be seen that the instruction to suspend the watchdog takes 3 words, ie. 48 bits, to encode. This is because the `MOV` instruction is a double operand instruction type and the opcode `0x40B2` specifies immediate mode for the first operand, which stores the immediate in the second word, and absolute mode for the second operand, which stores its address in the third word. These modes are internally specified by indirect autoincrement mode with the PC register and indexed mode with the SR register, respectively.

The while loop on line 1 should correspond to a jump instruction somewhere in memory, which we navigate forward to discover.
```
0xC01A: JEQ 0xC00C <
0xC01C: SUB.B #1 0x0000(SP)
0xC020: CMP.B #0 0x0000(SP)
0xC024: JEQ 0xC00C
```
The first jump instruction found is at `0xC01A`, jumping to `0xC00C`. This isn't useful in itself because we don't know what was being compared, which was an instruction that was skipped over.

Using the **UP** button moves the cursor backward by one instruction.
```
0xC016: CMP.B #0 0x0000(SP) <
0xC01A: JEQ 0xC00C
0xC01C: SUB.B #1 0x0000(SP)
0xC020: CMP.B #0 0x0000(SP)
```
This instruction is comparing 0 to some variable located at the stack pointer. This should be `i` from line 12 of the source code because it is the only variable on the stack. Therefore the jump must correspond to finishing the delay.

Moving the cursor down to the `JEQ 0xC00C` isntruction and using the **FOLLOW JMP** instruction brings the cursor to `0xC00C`.
```
0xC00C: XOR.B #1 &0x0021 <
0xC010: MOV.B #0x2710 0x0000(SP)
0xC016: CMP.B #0 0x0000(SP)
0xC01A: JEQ 0xC00C
```
Finally, it is seen that this `XOR` instruction corresponds to toggling P1.0, as it is the first thing to occur within the while loop!

## Firmware
An MSP430G2553, aside from being the target type, is also used as the main microcontroller for the device. Firmware was developed using register level C programming in Code Composer Studio. Development followed information derived from the [MSP430G2xx User Guide](https://www.ti.com/lit/ug/slau144k/slau144k.pdf?ts=1720622345813&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSP430F2274-EP%253Fbm-verify%253DAAQAAAAJ_____2GXDeU4IIaEds7sCQitbINeDMGTgqNh7F20H7LVJyrs5LsYNIkQrF5ycgp2Z0Yzd4j0V7synm_dgqfLwBr7S5VesPGUssLDDUfg7mGj_mgtHY82RE3zgMG80y9GDnAMYK8hv1U5Bq9QLLAvt0XehujFdDcILVLVdafgQQ4y_G74JWcE-jRRkwir5ItF99koK6Pg_AP0E-3rrXrP4YO819zT--lXgtbtaeA9YqF_kJtLNL18SaURyUQHTQi5XznOoq8sb0SznRHjyRujqGne18cY0YsrFImOVD6oPzmZqavXlbABc6vvCKw), the [MSP430G2x53 Datasheet](https://www.ti.com/lit/ds/symlink/msp430g2553.pdf?ts=1720602399535&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSP430G2553%253Futm_source%253Dgoogle%2526utm_medium%253Dcpc%2526utm_campaign%253Depd-null-null-GPN_EN-cpc-pf-google-kr%2526utm_content%253DMSP430G2553%2526ds_k%253D%257B_dssearchterm%257D%2526DCM%253Dyes%2526gad_source%253D1%2526gclid%253DCjwKCAjw4ri0BhAvEiwA8oo6Fw8EdAuptheRVE8vx0Q2j3G3iyDjTU-aOLXieh8M-imSlC9cQUR6_hoClwEQAvD_BwE%2526gclsrc%253Daw.ds%2526bm-verify%253DAAQAAAAJ_____3QLXOZbbdruMI2eB4mFy5KSyEb-yU9yfcNR8-6ZE-F4IlQA15TnUbnZYGHUclM_YK0JJvwZfmZV1vz0BoNFahgPHVY7BCm4W1e8hj6dfjPbReoJt03aw_CYG9J3Y8_jl0rNTcSpbsnRtODv2ub_rMMIkYIZ8sZN9mj6PYFAQ1CDy5mA6PGPGiCbLLn4tzBTBUOk0bJGTj9kveVfehS7i0km-maJESLOX2xoAFatjT2Bv-a8B9gvZcxtGENvp6eMYJDLbmRf67x4X7TE7tt88LH3n9FraEEjZPNjqllkWuL5OOONuBvPF598xP5kkYnJk3wzH9hIroxr1CUU2mTA124pgKzPR_rSHUN9RBbZlIETbuTA60Tk97_1ntDEeGNxAcWS2jB9cD66YYZ0SP0K35RqUrqlAPRsbdokSfG5fS__GGfM8jKUantdnKKpmyNt6DdoRJ1r1NB9CKWKb-SPG5tTHcpbzj1IFa38Zdux2S1X93eAYTW28T1i1pfPwFwJtyAuMkVxw52pn_Y16Fv4n5pcFP_gVm7DHllf_3j6l4I_ipVjaMjKLlI1TF2-Ir8mL8IGx4NuPwbIxyWDxJgmZ_jNEwqQak_lFWplGfgBfkp1_twV8Zrt-tub-1XrNvhwLE4xQLYsvy4ZATQQBHijVjMIYRRXAuL75RCAAUYcWCXUhR5zKtjJhYKP3NkYnMghe8Nj_tdsmRdDuz-CPD87), and the [MSP430 JTAG Programming Guide](https://www.ti.com/lit/ug/slau320aj/slau320aj.pdf?ts=1720599451382&ref_url=https%253A%252F%252Fwww.google.com%252F). Three libraries were developed to handle separate facets of the project requirements:

* **UART Driver**: This library handles control of the USCI peripheral to communicate via the UART module. This driver handles configuration of USCI and UART registers, buffering of user printing output, and transmission with a builtin interrupt handler. The driver offers a user-friendly API (ex. `wait_print(char *input)` and `uart_print_hex(uint16_t input)`) optimized for spaced calls. The `wait_print` function is most likely to be used, and currently implements polling until the previous message is finished being sent.
  
* **JTAG Driver**: This library handles interfacing with the target through its JTAG debugging interface. Internally, the interface is implemented as an FSM that is controlled by five input and output lines: TDI, TDO, TMS, TCK, and TEST. This driver works by bit banging the correct FSM sequences on GPIO pins. The API offers a variety of functions (specified by the [JTAG Programming Guide](https://www.ti.com/lit/ug/slau320aj/slau320aj.pdf?ts=1720599451382&ref_url=https%253A%252F%252Fwww.google.com%252F)), but the most important for this project is `readMem(uint16_t address)`. This function returns the value stored in the target at a particular memory address.
  
* **Disassembler**: This library handles conversion of machine code to RISC assembly instruction. Information about the representation of each instruction as machine code can be found in the [MSP430G2xx User Guide](https://www.ti.com/lit/ug/slau144k/slau144k.pdf?ts=1720622345813&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSP430F2274-EP%253Fbm-verify%253DAAQAAAAJ_____2GXDeU4IIaEds7sCQitbINeDMGTgqNh7F20H7LVJyrs5LsYNIkQrF5ycgp2Z0Yzd4j0V7synm_dgqfLwBr7S5VesPGUssLDDUfg7mGj_mgtHY82RE3zgMG80y9GDnAMYK8hv1U5Bq9QLLAvt0XehujFdDcILVLVdafgQQ4y_G74JWcE-jRRkwir5ItF99koK6Pg_AP0E-3rrXrP4YO819zT--lXgtbtaeA9YqF_kJtLNL18SaURyUQHTQi5XznOoq8sb0SznRHjyRujqGne18cY0YsrFImOVD6oPzmZqavXlbABc6vvCKw), specifically the CPU section. The MSP430G2553 contains a 16-bit CPU supporting variable length instructions ranging from a minimum of 1 word to a maximum of 3 words. This presented a significant design challenge as moving one assembly instruction backward is ambiguously a shift of 1, 2, or 3 words in memory with little to hint at which is correct. This ambiguity stems from the various addressing modes (ex. indexed mode) and instruction types (ex. single operand). The disassembler has no remedy to this issue; rather, it is currently solved by the main program which simply begins from the beginning of flash memory and iterates forward until finding the correct instruction. It is planned to speed up this process with a significantly sized cache of nearby instruction locations.

The main program handles user button input with interrupts on the corresponding GPIO pins and a simple button debounce system using the timer peripheral. Currently the program busy waits until recieving a command, however this is planned to be replaced with the use of low power modes.
