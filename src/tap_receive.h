#pragma once
#include "common.h"
#include "eth.h"
#include "tap.h"
#include "in.h"

typedef struct {
	Eth_Descriptor* eth;
	Tap_Descriptor* tap;
	In_Spoofing_Descriptor* isd;
} Tap_Receive_Thread_Args;

void* tap_receive_thread_proc(void* args);