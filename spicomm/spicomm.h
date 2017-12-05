#ifndef _SPICOMM_H
#define _SPICOMM_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

    struct __attribute__((__packed__)) cmd_t{
        uint16_t set_motor:1;
        uint16_t set_led:1;
        uint16_t reset:1;
        uint16_t cal_ir:1;
        uint16_t cal_acc:1;
        uint16_t read_ir:1;
        uint16_t read_light:1;
        uint16_t read_mic:1;
        uint16_t read_acc:1;
        uint16_t play_sound:1;
        uint16_t blinking_led:1;
        unsigned int set_ir_pulse:1;
	unsigned int reseverd:4;
    };
    struct __attribute__((__packed__)) led_cmd_t{
        uint16_t led0:1;
        uint16_t led1:1;
        uint16_t led2:1;
        uint16_t led3:1;
        uint16_t led4:1;
        uint16_t led5:1;
        uint16_t led6:1;
        uint16_t led7:1;
        uint16_t bodyled:1;
        uint16_t frontled:1;
        uint16_t ir0:1;
        uint16_t ir1:1;
        uint16_t ir2:1;
        uint16_t ir3:1;
	uint16_t reserved:2;
    };

    //from arm9 to dspic
    struct __attribute__((__packed__)) txbuf_t
    {
        struct cmd_t cmd;		//first two bytes for commands
        int16_t left_motor;	//speed of left motor
        int16_t right_motor;	//speed of right motor
        struct led_cmd_t led_cmd;	//command for leds
        int16_t led_cycle;		// blinking rate of LEDS
        int16_t sound_num;		// sound index [0--8]
        int16_t reserved[20];	//reserved, in order to makde the txbuf_t and rxbuf_t are in the same size, now 25 16bits words
        uint16_t	crc;	//crc checksum
    };

    //from dspic to arm9
    struct __attribute__((__packed__)) rxbuf_t
    {
        int16_t dummy;		// leave it empty
        int16_t ir[8];			// IR Ranges
        int16_t acc[3];			// Accelerometer x/y/z
        int16_t mic[3];			// microphones 1,2,3
        int16_t prox_filtered[8];      	// Filtered proximity sensing, signals at 32Hz
        int16_t tacl;			// steps made on l/r motors
        int16_t tacr;
        int16_t batt;			// battery level
        uint16_t crc;			//crc checksum
    };

  int init_spi();
  void do_msg(int fd, int16_t *txbuf, int16_t *rxbuf, int16_t len);
  void setSpeed(int lspeed, int rspeed);
  void get_ir_data(int *ir, int size);
  void get_proximity_filtered_data(int *ir, int size);
  void get_acc_data(int *acc, int size);
  void get_tac_data(int *tac);
  void get_mic_data(int *mic, int size);
  int16_t get_battery();
  void setLED(int id, int on);
  void setLEDs(int leds, int on);
  void blinkLEDs(int leds, int cycle); //cycles in ms/10
  void blinkLED(int id, int cycle);
  void setIRPulse(int id, int on);
  void setIRPulses(int irs, int on);
  void playSound(int index);
  void reset_robot(int fd);

  void update(int fd);
  void pabort(const char *s);

#ifdef __cplusplus
} 
#endif


#endif
