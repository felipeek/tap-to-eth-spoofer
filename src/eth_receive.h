#pragma once
#include "common.h"
#include "eth.h"
#include "tap.h"
#include "out.h"

typedef struct {
	Eth_Descriptor* eth;
	Tap_Descriptor* tap;
	Out_Spoofing_Descriptor* osd;
} Eth_Receive_Thread_Args;

void* eth_receive_thread_proc(void* args);