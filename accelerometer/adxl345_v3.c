#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#include <limits.h>

#define DevAddr  0x53  //device address
#define TimeStep 10 // sample interval in ms
#define MeasurePeriod 5 // in seconds
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
		acc_x[i] = 0.004 * ((float)((short int)((unsigned int)(sample[1] << 8) + (unsigned int)sample[0]))); // acc.x
		acc_y[i] = 0.004 * ((float)((short int)((unsigned int)(sample[3] << 8) + (unsigned int)sample[2]))); // acc.y
		acc_z[i] = 0.004 * ((float)((short int)((unsigned int)(sample[5] << 8) + (unsigned int)sample[4]))); // acc.z
		delay(1);
	}
	printf(" -> ");
	return;
}

void array_mult(float k, unsigned int data_length, float x[], float y[])
{
	int n;
	for(n=0; n<data_length; n++){
		y[n]= k * x[n];
	}
	return;
	}

//Preliminary fir filtering function (coeff length can be only odd, the order is this value + 1) 
void fir_filter(unsigned int coeff_length, float coeff[], unsigned int data_length, float x[], float y[]){
	int n, i;
	int order = coeff_length-1; //N
	
	// put y to zero
	for(n=0; n<data_length; n++){
		y[n]=0;
	}
	
	// pure fir filtering
	for(n=order/2; n<data_length-order/2; n++){
		for(i=0; i<coeff_length; i++){
			y[n]= y[n] + coeff[i] * x[n + order/2 -i];
			//printf("n: %d i: %d y[n]: %.8f coeff[i]: %.8f x[n + order/2 -i]: %.8f\n", n, i, y[n], coeff[i], x[n + order/2 -i]);
		}
	}
	
	return;
	}

void derivative(unsigned int data_length, float x[], float y[], float x_past){
	int n;
	
	// discrete derivative (first sample)
	y[0]= x[0] - x_past;
	
	// discrete derivative
	for(n=1; n<data_length; n++){
		y[n]= x[n] - x[n - 1];
	}
	
	return;
	}

void integral(unsigned int data_length, float x[], float y[], float c0){
	int n;
	
	y[0] = c0;

	// discrete derivative
	for(n=1; n<data_length; n++){
		y[n]= y[n-1] + x[n];
	}
	
	return;
	}


int main(void)
{
	int fd;
	int t;
	float acc_x[SamplesNumber];
	float acc_y[SamplesNumber];
	float acc_z[SamplesNumber];
	
	float test[] = {2, 3, 4, 5, 6, 5, 4, 3, 2};//9
	float tmp[9];//9
	float out[9];//9

	//derivative(9, test, tmp, 0);
	//integral(9, tmp, out, 0);
	//for(t=0; t<9; t++){printf("%.4f  %.4f  %.4f\n", test[t], tmp[t], out[t]);}
	
	//float coeff[]= {0.0909090909091, 0.0909090909091, 0.0909090909091, 0.0909090909091, 0.0909090909091, 0.0909090909091, 0.0909090909091, 0.0909090909091, 0.0909090909091, 0.0909090909091, 0.0909090909091}; // length 11
	
	//float coeff[]= {0, 0, 0}; // length 3
	
	float coeff[]= {
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653,
		0.0204081632653}; // length 49
		
	float filter_acc_x[SamplesNumber];
	float filter_acc_y[SamplesNumber];
	float filter_acc_z[SamplesNumber];
	
	float der_acc_x[SamplesNumber];
	float der_acc_y[SamplesNumber];
	float der_acc_z[SamplesNumber];
	
	printf("Initializing I2C interface.\n");
	fd = wiringPiI2CSetup(DevAddr);

	printf("Initializing acceleromenter device %d.\n", fd);
	adxl345_init(fd);
	delay(2500);
	
	printf("Measurng %d seconds...\n", MeasurePeriod);
	adxl345_read_xyz(fd, SamplesNumber, acc_x, acc_y, acc_z);
	printf("Done!\n");

	printf("Preparing file 'data.dat'.\n");
	FILE *f = fopen("./data.dat", "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		return 1;
	}
	fprintf(f, "#t:   \tx:        \ty:        \tz:        \n");
	for(t = 0; t < SamplesNumber; t++){// t counts the time step
		//printf("t:%.3f \tx: %.8f \ty: %.8f \tz: %.8f\n", (((float)t)*TimeStep)/1000, acc_xyz.x_norm, acc_xyz.y_norm, acc_xyz.z_norm);
		fprintf(f, "%.3f\t%.8f\t%.8f\t%.8f\n", (((float)t)*TimeStep)/1000, acc_x[t], acc_y[t], acc_z[t]);
		//printf("%.8f, ", acc_x[t]);
	}	
	fclose(f);
	
	printf("Filtering (integral/derivative).\n");
	//fir_filter(49, coeff, SamplesNumber, acc_x, filter_acc_x);
	//fir_filter(49, coeff, SamplesNumber, acc_y, filter_acc_y);
	//fir_filter(49, coeff, SamplesNumber, acc_z, filter_acc_z);
	
	//array_mult(49, SamplesNumber, filter_acc_x, filter_acc_x);
	//array_mult(49, SamplesNumber, filter_acc_x, filter_acc_y);
	//array_mult(49, SamplesNumber, filter_acc_x, filter_acc_z);
		
	derivative(SamplesNumber, acc_x, der_acc_x, 0);
	derivative(SamplesNumber, acc_y, der_acc_y, 0);
	derivative(SamplesNumber, acc_z, der_acc_z, 0);
	
	integral(SamplesNumber, der_acc_x, filter_acc_x, 0);
	integral(SamplesNumber, der_acc_y, filter_acc_y, 0);
	integral(SamplesNumber, der_acc_z, filter_acc_z, 0);
	
	
	printf("Preparing file 'data_fir.dat'.\n");
	f = fopen("./data_fir.dat", "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		return 1;
	}
	fprintf(f, "#t:   \tx:        \ty:        \tz:        \n");
	for(t = 0; t < SamplesNumber; t++){// t counts the time step
		//printf("t:%.3f \tx: %.8f \ty: %.8f \tz: %.8f\n", (((float)t)*TimeStep)/1000, acc_xyz.x_norm, acc_xyz.y_norm, acc_xyz.z_norm);
		fprintf(f, "%.3f\t%.8f\t%.8f\t%.8f\n", (((float)t)*TimeStep)/1000, filter_acc_x[t], filter_acc_y[t], filter_acc_z[t]);
	}	
	fclose(f);

	printf("Program terminated. Goodbye!\n");
	return 0;
}
