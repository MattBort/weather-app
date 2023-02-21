################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
simplelink/source/%.obj: ../simplelink/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/home/student/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/home/student/ti/ccs1010/ccs/ccs_base/arm/include" --include_path="/home/student/workspace_v10/http-example/cli_uart" --include_path="/home/student/ti/ccs1010/ccs/ccs_base/arm/include/CMSIS" --include_path="/home/student/workspace_v10/http-example" --include_path="/home/student/workspace_v10/http-example" --include_path="/home/student/workspace_v10/http-example/simplelink/include" --include_path="/home/student/workspace_v10/http-example/simplelink/source" --include_path="/home/student/workspace_v10/http-example/board" --include_path="/home/student/workspace_v10/http-example/cli_uart" --include_path="/home/student/workspace_v10/http-example/driverlib/MSP432P4xx" --include_path="/home/student/workspace_v10/http-example/spi_cc3100" --include_path="/home/student/workspace_v10/http-example/uart_cc3100" --include_path="/home/student/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="simplelink/source/$(basename $(<F)).d_raw" --obj_directory="simplelink/source" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


