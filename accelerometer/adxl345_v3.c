#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#include <limits.h>

#define DevAddr  0x53  //device address
#define TimeStep 10 // sample interval in ms
#define MeasurePeriod 10 // in seconds
#define SamplesNumber (1000*MeasurePeriod/TimeStep)

void adxl345_init(int fd)
{
	wiringPiI2CWriteReg8(fd, 0x31, 0x0b); // Range: (16g)
	//wiringPiI2CWriteReg8(fd, 0x31, 0x08); // Range (2g)
	
	wiringPiI2CWriteReg8(fd, 0x2d, 0x08); // Measure mode

	//WiringPiI2CWriteReg8(fd, 0x2e, 0x00); // ??
	
	wiringPiI2CWriteReg8(fd, 0x1e, 0x00); // X offset
	wiringPiI2CWriteReg8(fd, 0x1f, 0x00); // Y offset
	wiringPiI2CWriteReg8(fd, 0x20, 0x00); // Z offset
	
	wiringPiI2CWriteReg8(fd, 0x21, 0x00); // Tap detection
	wiringPiI2CWriteReg8(fd, 0x22, 0x00); // Tap detection
	wiringPiI2CWriteReg8(fd, 0x23, 0x00); // Tap detection

	wiringPiI2CWriteReg8(fd, 0x24, 0x01); // Activity threshold ??
	wiringPiI2CWriteReg8(fd, 0x25, 0x0f); // Inactivity threshold ??
	wiringPiI2CWriteReg8(fd, 0x26, 0x2b); // Inactivity time ??
	wiringPiI2CWriteReg8(fd, 0x27, 0x00); // Act./Inac. ctrl (all off)
	
	wiringPiI2CWriteReg8(fd, 0x28, 0x09); // Free-fall (not used)
	wiringPiI2CWriteReg8(fd, 0x29, 0xff); // Free-fall (not used)
	
	//wiringPiI2CWriteReg8(fd, 0x2a, 0x80); // Tap ctrl (??) 
	wiringPiI2CWriteReg8(fd, 0x2a, 0x00);  // Tap ctrl (all off) 
	
	wiringPiI2CWriteReg8(fd, 0x2c, 0x0a); // Normal power + Rate (100Hz)
	
	wiringPiI2CWriteReg8(fd, 0x2f, 0x00); // Interrupts (all disabled)
	
	wiringPiI2CWriteReg8(fd, 0x38, 0x9f); // FIFO mode (stream)
	//wiringPiI2CWriteReg8(fd, 0x38, 0x1f); // FIFO mode (stream)
}

void adxl345_read_xyz(int fd, unsigned int data_length, float acc_x[], float acc_y[], float acc_z[])
{
	int i;
	unsigned char sample[6];
	for (i = 0; i < data_length; i++)
	{
		while((0x1f & wiringPiI2CReadReg8(fd, 0x39)) == 0){;} // waiting for a sample			
		printf("\r%d s", (i*TimeStep)/1000);
		fflush(stdout); 
		wiringPiI2CReadBurst(fd, 0x32, 6, sample) ;
		acc_x[i] = 16 * (float)((short int)((unsigned int)(sample[1] << 8) + (unsigned int)sample[0])) / (4096); // acc.x
		acc_y[i] = 16 * (float)((short int)((unsigned int)(sample[3] << 8) + (unsigned int)sample[2])) / (4096); // acc.y
		acc_z[i] = 16 * (float)((short int)((unsigned int)(sample[5] << 8) + (unsigned int)sample[4])) / (4096); // acc.z
		delay(1);
	}
	printf(" -> ");
	return;
}

int main(void)
{
	int fd;
	float acc_x[SamplesNumber];
	float acc_y[SamplesNumber];
	float acc_z[SamplesNumber];
	
	printf("Initializing I2C interface.\n");
	fd = wiringPiI2CSetup(DevAddr);

	printf("Initializing acceleromenter device %d.\n", fd);
	adxl345_init(fd);
	delay(2500);
	
	printf("Measurng %d seconds...\n", MeasurePeriod);
	adxl345_read_xyz(fd, SamplesNumber, acc_x, acc_y, acc_z);
	printf("Done!\n");

	printf("Preparing data file 'data.dat'.\n");
	FILE *f = fopen("./data.dat", "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		return 1;
	}
	fprintf(f, "#t:   \tx:        \ty:        \tz:        \n");
	int t;
	for(t = 0; t < SamplesNumber; t++){// t counts the time step
		//printf("t:%.3f \tx: %.8f \ty: %.8f \tz: %.8f\n", (((float)t)*TimeStep)/1000, acc_xyz.x_norm, acc_xyz.y_norm, acc_xyz.z_norm);
		fprintf(f, "%.3f\t%.8f\t%.8f\t%.8f\n", (((float)t)*TimeStep)/1000, acc_x[t], acc_y[t], acc_z[t]);
	}	
	fclose(f);

	printf("Program terminated. Goodbye!\n");
	return 0;
}
