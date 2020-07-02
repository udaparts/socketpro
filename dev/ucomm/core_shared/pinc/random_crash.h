#ifndef __SOCKETPRO_CORE_RANDOM_CRASH_H__
#define __SOCKETPRO_CORE_RANDOM_CRASH_H__

#ifdef BAD_COMM_ENVIRONMENT
static const unsigned int COMM_RANDOM_STRENGTH = 16;
static const unsigned int RANDOM_SENDING_BYTE = 16;
static const unsigned int RANDOM_DELAY_MS = 1;
static const unsigned int RANDOM_CRASH_STRENGTH = 8;

static const int HANDSHAKING_CRASH_CODE = -100;
static const int RECEIVING_CRASH_CODE = -200;
static const int SENDING_CRASH_CODE = -400;

static_assert(RANDOM_SENDING_BYTE >= 8, "RANDOM_SENDING_BYTE cannot be less than 2!");
static_assert(RANDOM_CRASH_STRENGTH >= 2, "RANDOM_CRASH_STRENGTH cannot be less than 2!");
static_assert(COMM_RANDOM_STRENGTH >= 2, "COMM_RANDOM_STRENGTH cannot be less than 2!");

//supported MACROs: ENABLE_RANDOM_HANDSHAKING_CRASH, ENABLE_RANDOM_RECEIVING_CRASH and ENABLE_RANDOM_SENDING_CRASH
#endif

#endif
