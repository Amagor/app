/**
 * @file Defs.h
 * Constants and data types definition header.
 * @author Nezametdinov I.E.
 */

#ifndef __DEFS_H__
#define __DEFS_H__

//#include "../PlatformDefs.h"
#include "../PDL/sbn128/PlatformDefs.h"

/// min number of threads in scheduler
#ifndef MIN_THREADS
#define MIN_THREADS 1
#endif

/// max number of threads in scheduler
#ifndef MAX_THREADS
#define MAX_THREADS 64
#endif
#if MAX_THREADS < MIN_THREADS
#error there are not enaugh threads
#endif
#if MAX_THREADS > 128
#error max number of threads must be less or equal 128
#endif

/// min number of timers
#ifndef MIN_TIMERS
#define MIN_TIMERS 1
#endif

/// max number of timers
#ifndef MAX_TIMERS
#define MAX_TIMERS 32
#endif
#if MAX_TIMERS>32
#error maximum number of timers must be less or equal 32
#endif
#if MAX_TIMERS<MIN_TIMERS
#error there are not enaugh timers
#endif

/// max timeout
#define MAX_TIMEOUT (1UL<<29)

/// min timeout
#define MIN_TIMEOUT 30

/// makes milli seconds out of micro seconds
#define MS(x) ((x)*1000ll)

/// number of leds
#ifndef NUM_LEDS
#define NUM_LEDS 3
#endif
#if NUM_LEDS>8
#error number of leds must be less or equal 8
#endif

/// default channel
#ifndef DEFAULT_CHANNEL
#define DEFAULT_CHANNEL 11
#endif

/// default PAN ID
#ifndef DEFAULT_PAN_ID
#define DEFAULT_PAN_ID 0xAABB
#endif

/// default MAC extended address
#ifndef MAC_DEFAULT_A_EXTENDED_ADDRESS
#define MAC_DEFAULT_A_EXTENDED_ADDRESS 1
#endif

/// number of ports
#ifndef MAX_NUM_PORTS
#define MAX_NUM_PORTS 1
#endif
#if MAX_NUM_PORTS<=0
#error maximum number of ports must be greater than zero
#endif
#if MAX_NUM_PORTS>32
#error maximum number of ports must be less or equal 32
#endif

/// invalid handle
#define INVALID_HANDLE 0xFF

/// validates handle
#define IS_VALID_HANDLE(x) ((x)!=INVALID_HANDLE)

/// validates handle
#define IS_INVALID_HANDLE(x) ((x)==INVALID_HANDLE)

/// signals event
#define SIGNAL_EVENT(x) BEGIN_CRITICAL_SECTION{x;}END_CRITICAL_SECTION

/// NULL
#ifndef NULL
#define NULL 0
#endif

/// PARAM
typedef void* PARAM;

/// RESULT
typedef enum
{
	FAIL    = 0,
	SUCCESS = 1
}RESULT;

/// PROC
typedef void PROC;

/// EVENT
typedef void EVENT;

/// BOOL
enum
{
	FALSE = 0,
	TRUE  = 1
};
typedef uint8_t BOOL;

/// extended MAC address
typedef uint64_t MAC_EXTENDED_ADDR;

/// short MAC address
typedef uint16_t MAC_SHORT_ADDR;

/// TIME
typedef int64_t TIME;

/// PERIOD
typedef int32_t PERIOD;

#endif
