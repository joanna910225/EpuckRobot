///----------------------------------------------------------------------------------------------------------------
/// Project:	Symbrion + Replicator
/// File:		ethlolmsg.cpp
/// Authors:	Florian Schlachter, IPVS - University of Stuttgart
/// 		Christopher Schwarzer, University of Tübingen
///----------------------------------------------------------------------------------------------------------------


#ifndef ELOLMSGH
#define ELOLMSGH

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "bytequeue.h"

#ifdef __cplusplus
extern "C" {
#endif 

#define ELOLVAROVERHEAD 9
#define ELOLFIXEDDATALENGTH 4

///
///	Low Level Message implementation in C for use on MSP and Blackfin.
/// Struct LolMessage is the container for the message and can be serialized using
/// function lolmsgSerialize(). For iterative deserialization from a byte stream, 
/// a parse context is provided with struct LolParseContext.
///
typedef struct ELolMessage
{
	uint8_t command;
	uint8_t counter;
	uint8_t __pad;
	uint32_t length;
	const uint8_t* data;
} __attribute__((packed)) ELolMessage;

/// Conveniently initialize a LolMessage struct using these functions
void ElolmsgInit(ELolMessage* msg, uint8_t command, const void* data, uint32_t length);

/// 
/// Serialization routine.
/// 
int ElolmsgSerializedSize(ELolMessage* msg);
int ElolmsgSerialize(ELolMessage* msg, void* outbytes);
int ElolmsgSerializeFixed(uint8_t counter, uint8_t command, const void* data, void* outbytes);

typedef enum EParseState
{ 
	ELOLPARSE_COMPLETEBIT = 1,
	ELOLPARSE_HEADER = 2,
	ELOLPARSE_FIXED = 4,
	ELOLPARSE_FIXED_COMPLETE = 5,
	ELOLPARSE_VARIABLE = 6,
	ELOLPARSE_VARIABLE_PL = 8,
	ELOLPARSE_VARIABLE_COMPLETE = 7,
	ELOLPARSE_ERR_NOSTART = 16,
	ELOLPARSE_ERR_BUFTOOSMALL = 18,
	ELOLPARSE_ERR_CHECKSUM = 20
} EParseState;

typedef struct ELolParseContext
{
	uint32_t pos;
	EParseState state;
	uint8_t checksum;
	uint32_t bufLength;
	uint8_t* buf; 
} ELolParseContext;

/// 
/// buf must be minimum of 8 bytes, a buf larger than 8 allows receiving of variable payloads
///
void ElolmsgParseInit(ELolParseContext* ctx, void* buf, uint32_t bufLength);
void ElolmsgParseByte(ELolParseContext* ctx, uint8_t byte);
int ElolmsgParse(ELolParseContext* ctx, void* bytes, uint32_t length);

static inline ELolMessage* ElolmsgParseDone(ELolParseContext* ctx)
{
	if (ctx->state & ELOLPARSE_COMPLETEBIT)
		return (ELolMessage*)ctx->buf;
	else
		return NULL;
}

#ifdef __cplusplus
}
#endif 

#endif
