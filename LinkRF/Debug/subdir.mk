################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ARQ.cpp \
../CRC.cpp \
../File.cpp \
../Framework.cpp \
../Serial.cpp \
../main.cpp 

OBJS += \
./ARQ.o \
./CRC.o \
./File.o \
./Framework.o \
./Serial.o \
./main.o 

CPP_DEPS += \
./ARQ.d \
./CRC.d \
./File.d \
./Framework.d \
./Serial.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


