///----------------------------------------------------------------------------------------------------------------
/// Project:	Symbrion + Replicator
/// File:		ethlolmsg.c
/// Authors:	Florian Schlachter, IPVS - University of Stuttgart
/// 		Christopher Schwarzer, University of Tübingen
///----------------------------------------------------------------------------------------------------------------


#include "ethlolmsg.h"

#include <stdio.h>
#include <string.h>
//#include "printer.h"
#include "crc8.h"

#define EMSGSTARTFIX 0x55
#define EMSGSTARTVARETH 0x77

#define EFIXEDSIZE 8

#define EFIXEDHEADERSIZE 3
#define EVARHEADERSIZE 5

// Serialized LOLMessage bytes
// byte 0: MSGSTART flag
// byte 1: address
// byte 2: command
// -- fixed type message --
// byte 3-6: data
// byte 7: checksum
// -- variable type message --
// byte 3-6: data length
// byte 7: header checksum
// byte 8-x: data
// byte x+1: data checksum

//work round for 64bit machine
const static uint32_t dataOffsetPose = sizeof(ELolMessage);

void ElolmsgInit(ELolMessage* msg, uint8_t command, const void* data, uint32_t length)
{
	msg->command = command;
	msg->counter = 0;
	msg->length = length;
	msg->data = data;
}

int ElolmsgSerializedSize(ELolMessage* msg)
{
	if (msg->length >= 0)
		return ELOLVAROVERHEAD + msg->length;
	else
		return ELOLVAROVERHEAD - 1;
}

int ElolmsgSerialize(ELolMessage* msg, void* outbytes)
{
	uint8_t* bytes = outbytes;
	uint16_t checksum;
	uint8_t b = EMSGSTARTVARETH;
	*bytes++ = b;
	checksum = crc8_byte(CRC8_INIT, b);
	b = msg->counter;
	*bytes++ = b;
	checksum = crc8_byte(checksum, b);
	
	b = msg->command;
	*bytes++ = b;
	checksum = crc8_byte(checksum, b);
	
	b = (msg->length >> 24) & 0xFF;  // 1. byte
	*bytes++ = b;
	checksum = crc8_byte(checksum, b);
	
	b = (msg->length >> 16) & 0xFF;  // 2. byte
	*bytes++ = b;
	checksum = crc8_byte(checksum, b);
	
	b = (msg->length >> 8) & 0xFF;  // 3. byte
	*bytes++ = b;
	checksum = crc8_byte(checksum, b);
	
	b = msg->length & 0xFF;  // 4. byte
	*bytes++ = b;
	checksum = crc8_byte(checksum, b);
	*bytes++ = checksum;
	if (msg->length >= 0)
	{
		checksum = CRC8_INIT;
		const uint8_t* data = msg->data;
		int i;
		for (i = 0; i < msg->length; i++)
		{
			b = *data++;
			*bytes++ = b;
			checksum = crc8_byte(checksum, b);
		}
		*bytes++ = checksum;
        }
	
	
	return bytes - (uint8_t*)outbytes;
}

int ElolmsgSerializeFixed(uint8_t counter, uint8_t command, const void* data, void* outbytes)
{
	uint8_t* bytes = outbytes;
	uint16_t checksum;
	uint8_t b = EMSGSTARTFIX;
	*bytes++ = b;
	checksum = crc8_byte(CRC8_INIT, b);
	*bytes++ = counter;
	checksum = crc8_byte(checksum, counter);
	*bytes++ = command;
	checksum = crc8_byte(checksum, command);

	const uint8_t* d = data;
	int i;
	for (i = 0; i < ELOLFIXEDDATALENGTH; i++)
	{
		b = *d++;
		*bytes++ = b;
		checksum = crc8_byte(checksum, b);
	}
	*bytes++ = checksum;
	return bytes - (uint8_t*)outbytes;
}

void ElolmsgParseInit(ELolParseContext* ctx, void* buf, uint32_t bufLength)
{
	ctx->buf = buf;
	ctx->bufLength = bufLength;
	ctx->state = 0;
}

void ElolmsgParseByte(ELolParseContext* ctx, uint8_t byte)
{
	ElolmsgParse(ctx, &byte, 1);
}

// The parse function FROM HELL!
// Beware all kinds of evil performance hacks.
int ElolmsgParse(ELolParseContext* ctx, void* bytes, uint32_t inlength)
{
	uint8_t* outbytes = ctx->buf;
	ELolMessage* outmsg = (ELolMessage*) ctx->buf;
	uint8_t* inbytes = (uint8_t*)bytes;
	
	// Local copies of object variables
	// Must be written back on return!
	uint32_t pos = ctx->pos;
	uint8_t checksum = ctx->checksum;
	EParseState state = ctx->state;
	
	uint32_t parsed = 0;
	switch (state)
	{
	default:
		state = ELOLPARSE_HEADER;
		// intentional fall through!
	case ELOLPARSE_HEADER:
	{
		// Parse the first byte. This tells us which message type we have.
		if (inlength <= 0)
			goto end;
		
		uint8_t firstbyte = *inbytes++;
		checksum = crc8_byte(CRC8_INIT, firstbyte);
		parsed++;
		pos = 0;
		
		if (firstbyte == EMSGSTARTFIX)
		{
			state = ELOLPARSE_FIXED;
			goto case_fixed;		
		}
		else if(firstbyte == EMSGSTARTVARETH)
		{
			state = ELOLPARSE_VARIABLE;
			goto case_variable;			
		}
		else
		{
			state = ELOLPARSE_ERR_NOSTART;
			goto end;		
		}
	}
		break;
	case ELOLPARSE_FIXED: case_fixed:
		// Here we parse the complete fixed type message
		while (parsed < inlength)
		{
			uint8_t inbyte = *inbytes++;
			parsed++;
			switch (pos)
			{
				case 0:
					outmsg->counter = inbyte;
					break;
				case 1:
					outmsg->command = inbyte;
					outmsg->length = 4;
					outmsg->data = outbytes + 8;
					pos = 7;
					break;
				// after this pos is increased to 8
				case 12:
					if (inbyte != checksum)
						state = ELOLPARSE_ERR_CHECKSUM;
					else
						state = ELOLPARSE_FIXED_COMPLETE;

					goto end;
				default:	// this happens for pos 8 through 11
					outbytes[pos] = inbyte;	// the 4 fixed payload bytes
			}
			checksum = crc8_byte(checksum, inbyte);
			pos++;
		}
		break;
	case ELOLPARSE_VARIABLE: case_variable:
		// Here we parse the header of a variable type message
		while (parsed < inlength)
		{
			uint8_t inbyte = *inbytes++;
			parsed++;
			switch (pos)
			{
				case 0:
					outmsg->counter = inbyte;
					break;
				case 1:
					outmsg->command = inbyte;
					break;
				case 2:
					outmsg->length = inbyte; // 1.byte
					break;
				case 3:
					outmsg->length = outmsg->length << 8 | inbyte;  // 2. byte
					break;	
				case 4:
					outmsg->length = outmsg->length << 8 | inbyte;  // 2. byte
					break;	
				case 5:
					outmsg->length = outmsg->length << 8 | inbyte;  // 2. byte
					break;	
				case 6:
					if (inbyte != checksum)
					{
						state = ELOLPARSE_ERR_CHECKSUM;
						goto end;
					}
					else if (outmsg->length > ctx->bufLength - 8)
					{
						state = ELOLPARSE_ERR_BUFTOOSMALL;
						goto end;
					}
					else if (outmsg->length == 0)
					{
						state = ELOLPARSE_VARIABLE_COMPLETE;
						goto end;
					}
					else
					{
						outmsg->data = outbytes + dataOffsetPose;
						pos = dataOffsetPose;
						checksum = CRC8_INIT;
						state = ELOLPARSE_VARIABLE_PL;
						goto case_variable_pl;
					}
			}
			checksum = crc8_byte(checksum, inbyte);
			pos++;
		}
		break;
	case ELOLPARSE_VARIABLE_PL : case_variable_pl:
	// Here we parse the payload of a variable type message
	{
		uint32_t end = dataOffsetPose + outmsg->length;
		while (parsed < inlength)
		{
			uint8_t inbyte = *inbytes++;
			parsed++;
			
			if (pos >= end)
			{
				if (inbyte != checksum)
					state = ELOLPARSE_ERR_CHECKSUM;
				else
					state = ELOLPARSE_VARIABLE_COMPLETE;
				goto end;
			}
			
			outbytes[pos++] = inbyte;
			checksum = crc8_byte(checksum, inbyte);
		}
	}
		break;
	}
	
	end:
	ctx->pos = pos;
	ctx->checksum = checksum;
	ctx->state = state;
	return parsed;
}

