#pragma once
#include <stdatomic.h>
#include "common.h"
#include "eth.h"
#include "tap.h"
#include "eth_spoof.h"

typedef struct {
	Eth_Descriptor* eth;
	Tap_Descriptor* tap;
	Eth_Spoofing_Descriptor* esd;
	atomic_int* stop;
} Tap_Receive_Thread_Args;

void* tap_receive_thread_proc(void* args);