
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "i2c-dev.h"
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <error.h>
#include <inttypes.h>  // uint8_t, etc

#define COMMAND 0x80
#define CONTROL 0x00
#define ON 0x03
#define OFF 0x00
#define TIMING 0x01
#define WORD 0x20
#define LOW0 0x0C
#define HIGH0 0x0D
#define LOW1 0x0E
#define HIGH1 0x0F
#define GAIN 0x00
#define ID 0x0A
#define BLOCK 0x10

// Conversion Registers - Contains results of last conversion
#define D15 0x0F
#define D14 0x0E
#define D13 0x0D
#define D12 0x0C
#define D11 0x0B
#define D10 0x0A
#define D9 0x09
#define D8 0x08
#define D7 0x07
#define D6 0x06
#define D5 0x05
#define D4 0x04
#define D3 0x03
#define D2 0x02
#define D1 0x01
#define D0 0x00

// Pointer Register byte - Access by writing to the Pointer register byte
#define CONVERSION 0x00
#define CONFIG 0x01
#define LOTHRESH 0x02
#define HITHRESH 0x03

// Config Register
#define OS 0x0F
#define MUX2 0x0E
#define MUX1 0x0D
#define MUX0 0x0C
#define PGA2 0x0B
#define PGA1 0x0A
#define PGA0 0x09
#define MODE 0x08
#define DR2 0x07
#define DR1 0x06
#define DR0 0x05
#define COMPMODE 0x04
#define COMPPOL 0x03
#define COMPLAT 0x02
#define COMPQUE1 0x01
#define COMPQUE0 0x00

/*
	This is a service application that reads data from the TSL2561 and calculates the Lux.
	Values are written to the application folder under /etc/lux/
*/

#define BDADDR_SERVER (&(bdaddr_t) {{0x23,0x2B,0x50,0x19,0x03,0x00}});

int main()
{
	int file;
	volatile unsigned addr = 0x48;
	char *filename = "/dev/i2c-0";
	unsigned one,two,three,four;
	int Lux;
	int avgLux;
	char str[10];
	int loop;
	int avg;
  	int ads_address = 0x48;
  	uint8_t buf[10];
  	int16_t val;


	avg = 3;
	if((file = open(filename, O_RDWR)) < 0)
	{
		printf("Failed to open\n");
		exit(1);
	}
	if (ioctl(file,I2C_SLAVE_FORCE,addr) < 0)
	{
		printf("Failed\n");
		exit(1);
	}
	
/*
	i2c_smbus_write_byte_data(file, addr, 1);
	i2c_smbus_write_byte(file, CONFIG);
	i2c_smbus_write_byte(file, 0x84);
	i2c_smbus_write_byte(file, 0x83);

	i2c_smbus_write_byte(file, addr);
	i2c_smbus_write_byte(file, CONVERSION);

	one = i2c_smbus_read_byte(file);
	two = i2c_smbus_read_byte(file);
	three = i2c_smbus_read_byte(file);


	printf("Values: %x %x %x\n",one, two, three);
*/

	///////////////////////////////
 	// set config register and start conversion
  	// AIN0 and GND, 4.096v, 128s/s
  	buf[0] = 1;    // config register is 1
  	buf[1] = 0xc3;
  	buf[2] = 0x85;
  	if (write(file, buf, 3) != 3) {
  		perror("Write to register 1");
  	 	exit(-1);
  	}
 	 //////////////////////////////
  	// wait for conversion complete
  	do {
   		if (read(file, buf, 2) != 2) {
     			perror("Read conversion");
     			exit(-1);
   		}
  	} while (buf[0] & 0x80 == 0);
  	//////////////////////////////
  	// read conversion register
  	buf[0] = 0;   // conversion register is 0
  	if (write(file, buf, 1) != 1) {
   	perror("Write register select");
   	exit(-1);
  	}
  	if (read(file, buf, 2) != 2) {
   		perror("Read conversion");
   		exit(-1);
  	}
  	//////////////////////////////
  	// convert output and display results
  	val = (int16_t)buf[0]*256 + (uint16_t)buf[1];
  	printf("Conversion %02x %02x %d %f\n", buf[0], buf[1], val, (float)val*4.096/32768.0);
	

//	while(1)
//	{
//		printf("loop\n");
//		loop = 0;
//		while(loop<avg)
//		{
//			i2c_smbus_write_byte_data(file,COMMAND|CONTROL,ON);
//			usleep(101000);
			
			/*buf2 = i2c_smbus_read_byte_data(file,COMMAND|WORD|LOW0);
			buf = i2c_smbus_read_byte_data(file,COMMAND|WORD|HIGH0);
			buf <<= 8;
			buf |= buf2;
			channel0 = buf;*/
//			channel0 = i2c_smbus_read_word_data(file,COMMAND|WORD|LOW0);

			/*buf2 = i2c_smbus_read_byte_data(file,COMMAND|WORD|LOW1);
			buf = i2c_smbus_read_byte_data(file,COMMAND|WORD|HIGH1);
			buf <<= 8;
			buf |= buf2;
			channel1 = buf;*/
	//		channel1 = i2c_smbus_read_word_data(file,COMMAND|WORD|LOW1);


			//Lux = CalculateLux(0,1,channel0,channel1,0);
			//avgLux += Lux;
			
	//		loop++;
	//		i2c_smbus_write_byte_data(file,COMMAND|CONTROL,OFF);
	//		printf("i2c values: %d %d\n",channel0 ,channel1);
	//	}

		
	//}

	close(file);
}
