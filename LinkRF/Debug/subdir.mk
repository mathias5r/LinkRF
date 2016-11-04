################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ARQ.cpp \
../CRC.cpp \
../Dispacher.cpp \
../File.cpp \
../Frame.cpp \
../Framework.cpp \
../Serial.cpp \
../Tun.cpp \
../main.cpp 

OBJS += \
./ARQ.o \
./CRC.o \
./Dispacher.o \
./File.o \
./Frame.o \
./Framework.o \
./Serial.o \
./Tun.o \
./main.o 

CPP_DEPS += \
./ARQ.d \
./CRC.d \
./Dispacher.d \
./File.d \
./Frame.d \
./Framework.d \
./Serial.d \
./Tun.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


