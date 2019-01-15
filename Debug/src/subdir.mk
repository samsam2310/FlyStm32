################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/gy80.c \
../src/main.c \
../src/maintimer.c \
../src/motor.c \
../src/pid.c \
../src/stm32l4xx_hal_msp.c \
../src/stm32l4xx_it.c \
../src/syscalls.c \
../src/system_stm32l4xx.c \
../src/uart.c 

OBJS += \
./src/gy80.o \
./src/main.o \
./src/maintimer.o \
./src/motor.o \
./src/pid.o \
./src/stm32l4xx_hal_msp.o \
./src/stm32l4xx_it.o \
./src/syscalls.o \
./src/system_stm32l4xx.o \
./src/uart.o 

C_DEPS += \
./src/gy80.d \
./src/main.d \
./src/maintimer.d \
./src/motor.d \
./src/pid.d \
./src/stm32l4xx_hal_msp.d \
./src/stm32l4xx_it.d \
./src/syscalls.d \
./src/system_stm32l4xx.d \
./src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32L4 -DSTM32L476RGTx -DNUCLEO_L476RG -DDEBUG -DSTM32L476xx -DUSE_HAL_DRIVER -I"C:/Users/Fonger/workspace/Gy80/HAL_Driver/Inc/Legacy" -I"C:/Users/Fonger/workspace/Gy80/inc" -I"C:/Users/Fonger/workspace/Gy80/CMSIS/device" -I"C:/Users/Fonger/workspace/Gy80/CMSIS/core" -I"C:/Users/Fonger/workspace/Gy80/HAL_Driver/Inc" -I"C:/Users/Fonger/workspace/Gy80/Utilities/STM32L4xx_Nucleo" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


