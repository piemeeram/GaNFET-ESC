################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Main/Startup/startup_stm32g473cetx.s 

OBJS += \
./Main/Startup/startup_stm32g473cetx.o 

S_DEPS += \
./Main/Startup/startup_stm32g473cetx.d 


# Each subdirectory must supply rules for building sources it contributes
Main/Startup/%.o: ../Main/Startup/%.s Main/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

