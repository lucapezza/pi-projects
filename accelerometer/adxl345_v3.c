#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#define DevAddr  0x53  //device address

#define TimeStep 10 //sample interval in ms

struct acc_dat{
	int x;
	int y;
	int z;
	float x_norm;
	float y_norm;
	float z_norm;
};

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

struct acc_dat adxl345_read_xyz(int fd)
{
	unsigned char x0, y0, z0, x1, y1, z1;
	struct acc_dat acc_xyz;
	
	while((0x1f & wiringPiI2CReadReg8(fd, 0x39)) == 0){;}
	
	unsigned char tmp[8];
	if (wiringPiI2CReadReg48(fd, 0x32, tmp) == -1) {printf("ERROR LUCA!\n");} 
	
	/*
	for (i=0;i<8;i++){
		printf("%02x ",tmp[i]);
    }
    printf("\n");
    */

    x0 = tmp[1];
    x1 = tmp[2];
    y0 = tmp[3];
    y1 = tmp[4];
    z0 = tmp[5];
    z1 = tmp[6];
	
	acc_xyz.x = (int)(short int)((unsigned int)(x1 << 8) + (unsigned int)x0);
	acc_xyz.y = (int)(short int)((unsigned int)(y1 << 8) + (unsigned int)y0);
	acc_xyz.z = (int)(short int)((unsigned int)(z1 << 8) + (unsigned int)z0);
	acc_xyz.x_norm = 16 * (float)acc_xyz.x / (4096);
	acc_xyz.y_norm = 16 * (float)acc_xyz.y / (4096);
	acc_xyz.z_norm = 16 * (float)acc_xyz.z / (4096);

	return acc_xyz;
}

int main(void)
{
	int fd;
	struct acc_dat acc_xyz;

	fd = wiringPiI2CSetup(DevAddr);
	
	FILE *f = fopen("./data.dat", "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		return 1;
	}

	fprintf(f, "#t:   \tx:        \ty:        \tz:        \n");

	adxl345_init(fd);
	int t;
	for(t = 0; t <= 10000/TimeStep; t++){ // t counts the time step
		acc_xyz = adxl345_read_xyz(fd);
		//printf("t:%.3f \tx: 0x%08x \ty: 0x%08x \tz: 0x%08x\n", (((float)t)*TimeStep)/1000, acc_xyz.x, acc_xyz.y, acc_xyz.z);
		printf("t:%.3f \tx: %.8f \ty: %.8f \tz: %.8f\n", (((float)t)*TimeStep)/1000, acc_xyz.x_norm, acc_xyz.y_norm, acc_xyz.z_norm);
		fprintf(f, "%.3f\t%.8f\t%.8f\t%.8f\n", (((float)t)*TimeStep)/1000, acc_xyz.x_norm, acc_xyz.y_norm, acc_xyz.z_norm);

		delay(1);
	}
	
	fclose(f);
	
	return 0;
}

