################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: el_scale_big.axf

# Tool invocations
el_scale_big.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"/home/tgjuranec/lpc11xx/lpc_chip_11cxx_lib/Debug" -L"/home/tgjuranec/lpc11xx/CMSIS_DSPLIB_CM0/lib" -Xlinker -Map="el_scale_big.map" -Xlinker --gc-sections -mcpu=cortex-m0 -mthumb -T "el_scale_Debug.ld" -o "el_scale_big.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) el_scale_big.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "el_scale_big.axf"; # arm-none-eabi-objcopy -v -O binary "el_scale_big.axf" "el_scale_big.bin" ; # checksum -p LPC1114_302 -d "el_scale_big.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
