################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
tests/%.obj: ../tests/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1230/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmsp --code_model=small -Ooff --use_hw_mpy=none --include_path="C:/Users/bapti/Documents/MSP-Debugger/Software/msp430BackchannelLib/include" --include_path="C:/Users/bapti/Documents/MSP-Debugger/Software/msp430JtagDriverTest/tests" --include_path="C:/ti/ccs1230/ccs/ccs_base/msp430/include" --include_path="C:/Users/bapti/Documents/MSP-Debugger/Software/msp430JtagDriverLib/include" --include_path="C:/Users/bapti/Documents/MSP-Debugger/Software/msp430JtagDriverTest" --include_path="C:/ti/ccs1230/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power="all" --define=__MSP430G2553__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="tests/$(basename $(<F)).d_raw" --obj_directory="tests" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


