################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/HX711.c \
../src/LCD1602.c \
../src/cr_startup_lpc11xx.c \
../src/crp.c \
../src/el_scale_big.c \
../src/io.c \
../src/sch.c \
../src/sleep.c \
../src/sysinit.c 

OBJS += \
./src/HX711.o \
./src/LCD1602.o \
./src/cr_startup_lpc11xx.o \
./src/crp.o \
./src/el_scale_big.o \
./src/io.o \
./src/sch.o \
./src/sleep.o \
./src/sysinit.o 

C_DEPS += \
./src/HX711.d \
./src/LCD1602.d \
./src/cr_startup_lpc11xx.d \
./src/crp.d \
./src/el_scale_big.d \
./src/io.d \
./src/sch.d \
./src/sleep.d \
./src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -DCORE_M0 -D__USE_LPCOPEN -DNO_BOARD_LIB -D__USE_CMSIS_DSPLIB=CMSIS_DSPLIB_CM0 -D__LPC11XX__ -I"/home/tgjuranec/lpc11xx/el_scale_big/inc" -I"/home/tgjuranec/lpc11xx/lpc_chip_11cxx_lib/inc" -I"/home/tgjuranec/lpc11xx/CMSIS_DSPLIB_CM0/inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


