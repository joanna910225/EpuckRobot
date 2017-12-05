#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>

#include "spicomm.h"
#include "crc16.h"

//#define TEST

void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev1.0";
static uint8_t mode = 1;
static uint8_t bits = 16;
static uint32_t speed =20000000;

struct txbuf_t msgTX;
struct rxbuf_t msgRX;

int init_spi()
{
	int fd;
	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 *          * spi mode
	 *                   */
	int ret;
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 *          * bits per word
	 *                   */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 *          * max speed hz
	 *                   */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	return fd;
}

/*send message and read data from dsPIC*/
void do_msg(int fd, int16_t *txbuf, int16_t *rxbuf, int16_t len)
{
	struct spi_ioc_transfer xfer;

	int   status;

	memset(&xfer, 0, sizeof xfer);
	memset(rxbuf, 0, sizeof rxbuf);

	xfer.rx_buf = (__u64) rxbuf;
	xfer.tx_buf = (__u64) txbuf;
	xfer.len = 2*len; //size in bytes

	status = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
	if (status < 0)
	{
		perror("SPI_IOC_MESSAGE");
		memset(rxbuf, 0, sizeof rxbuf);
		return;
	}

#ifdef TESTSPI   
	int16_t *bp;
	printf("response(%2d, %2d): ",2* len, status);
	for (bp = txbuf; len; len--)
		printf(" %d", *bp++);
	printf("\n");
#endif
}

void setSpeed(int lspeed, int rspeed)
{
	msgTX.cmd.set_motor=1;
	//msgTX.left_motor = 0;
	//msgTX.right_motor = 0;
	//printf(" %d %d \n", lspeed,rspeed);
	msgTX.left_motor = lspeed;
	msgTX.right_motor = rspeed;
}

//get ir sensor data from the data stream
void get_ir_data(int*ir, int size)
{
	int i;
	for(i=0;i<size;i++)
	{
		ir[i]=msgRX.ir[i];
	}
}

// Filtered proximity sensing, signals at 32Hz
void get_proximity_filtered_data(int *ir, int size)
{
  int i;
  for(i=0;i<size;i++)
    {
      ir[i]=msgRX.prox_filtered[i];
    }

}

//get acc sensor data from the data stream
void get_acc_data(int*acc, int size)
{
  int i=0;
  for(i=0;i<size;i++)
    {
      acc[i]=msgRX.acc[i];
    }
}

//get mic sensor data from the data stream
void get_mic_data(int*mic, int size)
{
  int i=0;
  for(i=0;i<size;i++)
    {
      mic[i]=msgRX.mic[i];
    }
}

//get motor step data from the data stream
void get_tac_data(int*tac)
{
	//tac[0]=300;
	//tac[1]=300;

    tac[0]=msgRX.tacl;
    tac[1]=msgRX.tacr;
    //printf(" %f %f \n", msgRX.tacl,msgRX.tacr);
}

//send cmd and get data from dsPIC
void update(int fd)
{

	msgTX.crc = crc16((uint8_t*)&msgTX, sizeof(msgTX) - 2, CRC16_INIT);
	do_msg(fd,(int16_t *)&msgTX,(int16_t *)&msgRX, sizeof(msgRX)/sizeof(int16_t));
	//uint16_t checksum = crc16((uint8_t*)msgRX.ir, sizeof(msgRX) - 4, CRC16_INIT);
	//if(checksum != msgRX.crc)
	//	printf("crc error, required: %#x, received: %#x\n", msgRX.crc, checksum);
	//may need to reset the cmd array
	memset((int16_t*)&msgTX, 0, sizeof(msgTX)/sizeof(int16_t));
}

//reset robot
void reset_robot(int fd)
{
	msgTX.cmd.reset = 1;
	update(fd);
}

void setLED(int id, int on)
{
	int16_t temp;
	memcpy(&temp, &(msgTX.led_cmd),2);
	if(on)
		temp |=1<<id;
	else
		temp &= ~(1<<id);
	memcpy(&(msgTX.led_cmd),&temp,2);
	msgTX.cmd.set_led = 1;
}

void setLEDs(int led, int on)
{
	int16_t temp;
	memcpy(&temp, &(msgTX.led_cmd),2);
        temp &= ~0x3FF;// clear all led bits
	if(on)
		temp |= led & 0x3FF;
	else
		temp &= ~(led & 0x3FF);
	memcpy(&(msgTX.led_cmd),&temp,2);
	msgTX.cmd.set_led = 1;
}

void setIRPulse(int id, int on)
{
    	int16_t temp;
	memcpy(&temp, &(msgTX.led_cmd),2);
	if(on)
		temp |=1<<(id+10);
	else
		temp &= ~(1<<(id+10));
	memcpy(&(msgTX.led_cmd),&temp,2);
	msgTX.cmd.set_ir_pulse = 1;
}

void setIRPulses(int irs, int on)
{
	int16_t temp;
	memcpy(&temp, &(msgTX.led_cmd),2);
        temp &= ~0x3C00;// clear all ir pulse bits
	if(on)
		temp |= (irs << 10) & 0x3C00;
	else
		temp &= ~((irs<<10) & 0x3C00);
	memcpy(&(msgTX.led_cmd),&temp,2);
	msgTX.cmd.set_ir_pulse = 1;
        printf("%d %d %d %d\n", msgTX.led_cmd.ir0, msgTX.led_cmd.ir1, msgTX.led_cmd.ir2, msgTX.led_cmd.ir3);
}

//don't use this function at moment
void blinkLED(int id, int cycles)
{
	setLED(id, 1);
	msgTX.cmd.blinking_led = 1;
	msgTX.led_cycle = cycles;
}

void blinkLEDs(int leds, int cycles)
{
	if(cycles ==0)
		setLEDs(leds, 0);
	else
		setLEDs(leds, 1);
	msgTX.cmd.blinking_led = 1;
	msgTX.led_cycle = cycles;
}

void playSound(int index)
{
	msgTX.cmd.play_sound = 1;
	msgTX.sound_num = index;
}
int16_t get_battery()
{
	return msgRX.batt;
}
