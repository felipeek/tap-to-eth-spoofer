#pragma once
#include <stdatomic.h>
#include "common.h"
#include "eth.h"
#include "tap.h"
#include "tap_spoof.h"

typedef struct {
	Eth_Descriptor* eth;
	Tap_Descriptor* tap;
	Tap_Spoofing_Descriptor* tsd;
	atomic_int* stop;
} Eth_Receive_Thread_Args;

void* eth_receive_thread_proc(void* args);