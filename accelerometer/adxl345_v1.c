#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#define DevAddr  0x53  //device address

#define TimeStep 25 //sample interval in ms

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
	wiringPiI2CWriteReg8(fd, 0x31, 0x0b);
	wiringPiI2CWriteReg8(fd, 0x2d, 0x08);
//	wiringPiI2CWriteReg8(fd, 0x2e, 0x00);
	wiringPiI2CWriteReg8(fd, 0x1e, 0x00);
	wiringPiI2CWriteReg8(fd, 0x1f, 0x00);
	wiringPiI2CWriteReg8(fd, 0x20, 0x00);
	
	wiringPiI2CWriteReg8(fd, 0x21, 0x00);
	wiringPiI2CWriteReg8(fd, 0x22, 0x00);
	wiringPiI2CWriteReg8(fd, 0x23, 0x00);

	wiringPiI2CWriteReg8(fd, 0x24, 0x01);
	wiringPiI2CWriteReg8(fd, 0x25, 0x0f);
	wiringPiI2CWriteReg8(fd, 0x26, 0x2b);
	wiringPiI2CWriteReg8(fd, 0x27, 0x00);
	
	wiringPiI2CWriteReg8(fd, 0x28, 0x09);
	wiringPiI2CWriteReg8(fd, 0x29, 0xff);
	wiringPiI2CWriteReg8(fd, 0x2a, 0x80);
	wiringPiI2CWriteReg8(fd, 0x2c, 0x0a);
	wiringPiI2CWriteReg8(fd, 0x2f, 0x00);
	wiringPiI2CWriteReg8(fd, 0x38, 0x9f);
}

struct acc_dat adxl345_read_xyz(int fd)
{
	unsigned char x0, y0, z0, x1, y1, z1;
	struct acc_dat acc_xyz;

	x0 = 0xff - wiringPiI2CReadReg8(fd, 0x32);
	x1 = 0xff - wiringPiI2CReadReg8(fd, 0x33);
	y0 = 0xff - wiringPiI2CReadReg8(fd, 0x34);
	y1 = 0xff - wiringPiI2CReadReg8(fd, 0x35);
	z0 = 0xff - wiringPiI2CReadReg8(fd, 0x36);
	z1 = 0xff - wiringPiI2CReadReg8(fd, 0x37);

	acc_xyz.x = (int)(short int)((unsigned int)(x1 << 8) + (unsigned int)x0);
	acc_xyz.y = (int)(short int)((unsigned int)(y1 << 8) + (unsigned int)y0);
	acc_xyz.z = (int)(short int)((unsigned int)(z1 << 8) + (unsigned int)z0);
	acc_xyz.x_norm = (float)acc_xyz.x / (float)(-SHRT_MIN);
	acc_xyz.y_norm = (float)acc_xyz.y / (float)(-SHRT_MIN);
	acc_xyz.z_norm = (float)acc_xyz.z / (float)(-SHRT_MIN);

	return acc_xyz;
}

int main(void)
{
	int fd;
	struct acc_dat acc_xyz;

	fd = wiringPiI2CSetup(DevAddr);
	
	if(-1 == fd){
		perror("I2C device setup error");	
	}

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
		printf("t:%.3f \tx: %.8f \ty: %.8f \tz: %.8f\n", (((float)t)*TimeStep)/1000, acc_xyz.x_norm, acc_xyz.y_norm, acc_xyz.z_norm);
		fprintf(f, "%.3f\t%.8f\t%.8f\t%.8f\n", (((float)t)*TimeStep)/1000, acc_xyz.x_norm, acc_xyz.y_norm, acc_xyz.z_norm);

		delay(TimeStep);
	}
	
	fclose(f);
	
	return 0;
}

