#include "CU_TM4C123.h"
#include <stdlib.h>

// Global Variables
const uint32_t NUM_BUFS = 10;
const uint32_t BUF_SIZE = 16;
const uint32_t PROGRAM_RELOAD = 15999999UL;
//const uint32_t PRESCALAR_RELOAD = 15UL;
//const uint32_t PROGRAM_RELOAD = 39999UL;

uint32_t second = 0;
uint32_t minute = 0;
uint32_t hour = 0;

uint32_t display_time = 1;
uint32_t run_status = 1;
double speed = 1;	// Default to 1 => 1 sec./time report

unsigned char pressed_str[] = "Button Pressed";
unsigned char *c;

// Variables for buffer management...
unsigned char rx_buffer[BUF_SIZE];
uint32_t rx_count = 0;
uint32_t rx_in_counter = 0;
uint32_t rx_out_counter = 0;
uint32_t itr;

struct buffer buf[NUM_BUFS];
struct buffer *rx_pool_ptr[NUM_BUFS];
struct buffer *buf_ptr;

//	Buffer Structure...
struct buffer {
	unsigned char text[BUF_SIZE];
	uint8_t isUsed;
};

// UART Driver Functions
void UART_Init(void);
void button_Init(void);
void led_Init(void);
void periodic_timer_Init(void);

void display_current_time(void);
unsigned char* validate_Rx_Msg(void);
void empty_buffer(void);
//void copy_string(unsigned char *src, unsigned char *dest);
void Tx_message(unsigned char* msg_buffer);
struct buffer* Rx_message(void);

// Buffer Manager Functions
void buffer_Init(void);
struct buffer* get_buffer(void);
void release_buffer(struct buffer * a_buffer);
void notify_full_buffer(void);
struct buffer* addRxBufPool(struct buffer * a_buffer);
struct buffer* releaseRxBufPool(struct buffer * a_buffer);

extern void GPIOF_Handler(void);
extern void UART0_Handler(void);
extern void TIMER1A_Handler(void);
