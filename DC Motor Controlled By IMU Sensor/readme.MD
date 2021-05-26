
ATMega 328PB

1. Interfaces the provided  MPU-6050 6-DOF IMU Sensor to the ATmega328p using the I2C interface.  Displays the accelerometer and gyro data to the UART Terminal. 
 
2. Developed an algorithm to smooth the data from MPU-6050 6-DOF IMU Sensor. Displays the smooth accelerometer gyro data to the UART Terminal. 
 
3. Determines the roll angle of the MPU-6050. Using MPU6050 as the input, controls the direction and speed of the DC motor (FITO450 DC Motor). If the roll is negative the motor rotates anti-clockwise and if rolls positive the motor rotates clockwise. The speed of the motor is faster based on the angle of roll magnitude.
