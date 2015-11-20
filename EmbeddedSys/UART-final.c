// This technically the UART Driver...
#include "functions.h"

void GPIOF_Handler(void) {
	
	struct buffer *b_buf;
	char *c;
	
	GPIOF -> ICR |= (0x1UL << 4); // Clear interrupt flag
	NVIC -> ICER[0] = (1UL << 30); // Clear pending bit for GPIOF in NVIC
	__set_PRIMASK(1);
	b_buf = get_buffer();
	if (b_buf != NULL) {
		for (c = pressed_str, j = 0; *c != '\0'; c++, j++) {
			b_buf->text[j] = *c;
		}
		Tx_message((char *)b_buf);
		release_buffer(b_buf);
	}
	else {
		// Turn on the LED.
		GPIOF->DATA |=  0x1UL << 1;
		run_status = 0;
	}
	__set_PRIMASK(0);

	// Enable GPIOF Interrupt again
	NVIC -> ICPR[0] = 0x1UL << 30;
	NVIC -> ISER[0] = 0x1UL << 30;
}

void UART0_Handler(void) {
	
	struct buffer *a_buf;
	unsigned char * c;
	__set_PRIMASK(1);
	if (UART0 -> RIS & (0x1UL << 6)) {
		// Rx Timeout Interrupt
		while ((UART0 -> FR & UART_FR_RXFE) == 0) {
			
			// Reads in a char from the Rx FIFO...
			rx_buffer[rx_count] = (unsigned char)(UART0 -> DR & 0xFF);

			if (rx_buffer[rx_count] == 0x0D) {
				
				if (validate_Rx_Msg()) {
					a_buf = get_buffer();
					
					if (a_buf != NULL) {
						
						for (c = rx_buffer, j = 0; *c != '\0'; c++, j++) {
							a_buf->text[j] = *c;
						}
						addRxBufPool(a_buf);
						rx_in_counter++;
						rx_in_counter = rx_in_counter % 10;
					}
					else {
						// Turn on the LED.
						GPIOF->DATA |= 0x1UL << 1;
						run_status = 0;
					}
				}

				// Empty out Rx Buffer...
				for (j = 0; j < BUF_SIZE; j++) {
					rx_buffer[j] = '\0';
				}
				rx_count = 0;
			}
			else {
				rx_count++;
				rx_count = rx_count % BUF_SIZE;

				if (rx_count == 0) {
					// Empty out Rx Buffer...
					for (j = 0; j < BUF_SIZE; j++) {
						rx_buffer[j] = '\0';
					}
				}
			}
		}
	}
	__set_PRIMASK(0);
	UART0 -> ICR |= (0x1UL << 6);	// Clear Interrupt Status Bits
	NVIC -> ICER[0] = (1UL << 5); // Clear pending bit for UART0 in NVIC

	// Enable UART0 Interrupt again
	NVIC -> ICPR[0] = 0x1UL << 5;
	NVIC -> ISER[0] = 0x1UL << 5;
}

void TIMER1A_Handler (void){
	
	TIMER1 -> ICR = 1UL; // Clear Interrupt Status Bit
	NVIC -> ICPR[0] = 1UL << 21; // Clear Pending Bit

	__set_PRIMASK(1);
	if (Rx_message() != NULL) {
		Tx_message((char *)Rx_message());
		releaseRxBufPool(Rx_message());
		rx_out_counter++;
		rx_out_counter = rx_out_counter % 10;
	}

	display_current_time();
	__set_PRIMASK(0);
}

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 16 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
void UART_Init(void){
  	
  	SYSCTL -> RCGCUART |= SYSCTL_RCGCUART_R0; // activate UART0
  	SYSCTL -> RCGCGPIO |= SYSCTL_RCGCGPIO_R0; // activate port A
  	UART0 -> CTL &= ~UART_CTL_UARTEN;        // disable UART
  	UART0 -> IBRD = 8;                       // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.680555) = 8
  	UART0 -> FBRD  = 44;                      // FBRD = int(0.680555 * 64 + 0.5) = 44
                                           // 8 bit word length (no parity bits, one stop bit, enable FIFOs) => 1-byte FIFO?
  	UART0 -> LCRH = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  	GPIOA -> AMSEL &= ~0x03;                // disable analog functionality on PA
  	GPIOA -> AFSEL |= 0x03;                 // enable alt funct on PA1-0
  	GPIOA -> DEN |= 0x03;                    // enable digital I/O on PA1-0
                                           // configure PA1-0 as UART   (just in case)
  	GPIOA -> PCTL = (GPIOA -> PCTL & 0xFFFFFF00)+0x00000011;
	
	UART0 -> IM = (0x1UL << 6);	// Set Interrupt Masks for both Tx/Rx
	UART0 -> ICR |= (0x1UL << 6);	// Clear Interrupt Status Bits

	UART0 -> CTL |= UART_CTL_UARTEN;         // enable UART	
	
	// Setup for UART0 interrupt (Interrupt No. 5)
	NVIC -> ICER[0] = 0x1UL << 5;
	NVIC -> ICPR[0] = 0x1UL << 5;
	NVIC -> ISER[0] = 0x1UL << 5;
}

// Initializes SW1/2 to detect button pressed
void button_Init(void) {
	
	// Enable GPIOF port (used for the on-board button).
	SYSCTL->RCGCGPIO |= (1UL << 5);
  
	// Enable the GPIO pin for the button (SW1).
	GPIOF -> DIR &= ~(0x1UL << 4);       // Set the direction as output
	GPIOF -> DEN |= (0x1UL << 4);       // Enable the GPIO pin for digital function
	GPIOF -> IM |= (0x1UL << 4); 
	GPIOF -> PUR |= (0x1UL << 4);

	// Setup for button interrupt (GPIOF Interrupt)
	NVIC -> ICER[0] = 0x1UL << 30;
	NVIC -> ICPR[0] = 0x1UL << 30;
	NVIC -> ISER[0] = 0x1UL << 30;
}

void led_Init(void) {
	// Enable the GPIO pin for the LEDS (PF1/3).
	GPIOF->DIR |= (0x1UL << 1 | 0x1UL << 3);       // Set the direction as output
	GPIOF->DEN |= (0x1UL << 1 | 0x1UL << 3);       // Enable the GPIO pin for digital function
}

// Set up periodic timer and to cause an interrupt for every
// time-out event...
// Default: 1 second
void periodic_timer_Init(void) {
	// Disable Interrupt at NVIC for Timer1A
	NVIC -> ICER[0] = (1UL << 21);

	SYSCTL -> RCGCTIMER |= (0x1UL << 1);	// Enable GPTM Timer1A
	TIMER1 -> CTL &= ~(0x1UL << 0);		// Disable Timer1A for config.

	// Configure Timer1 to be a concatenated 16-bit Timer
	TIMER1 -> CFG = 0x0UL;
	TIMER1 -> TAMR = 0x2UL;				// Timer1A => Periodic Mode
	TIMER1 -> TAMR &= ~(0x1UL << 4);		// Timer1A => Count-Down
	TIMER1 -> TAILR = PROGRAM_RELOAD * speed;		// Program Reload
	TIMER1 -> IMR = (0x1UL << 0);			// Set Interrupt Mask Reg.
	TIMER1 -> ICR |= 0x1UL;				// Clear Interrupt Status Bit

	// Setup for Timer1A
	NVIC -> ICER[0] = 0x1UL << 21;
	NVIC -> ICPR[0] = 0x1UL << 21;
	NVIC -> ISER[0] = 0x1UL << 21;
	
	TIMER1 ->CTL |= (0x1UL << 0);		// Enable Timer1A
}

// Displays the current time
void display_current_time(void) {
	
	char *c;
	char current_time_str[8];
	uint32_t remainder;
	struct buffer *a_buf;

	// Formatting HH:MM:SS time format for sending...
	current_time_str[0] = 0x30 + (hour / 10);
	current_time_str[1] = 0x30 + (hour % 10);
	current_time_str[2] = ':';
	current_time_str[3] = 0x30 + (minute / 10);
	current_time_str[4] = 0x30 + (minute % 10);
	current_time_str[5] = ':';
	current_time_str[6] = 0x30 + (second / 10);
	current_time_str[7] = 0x30 + (second % 10);

	if (display_time == 1) {
		a_buf = get_buffer();
				
		if (a_buf != NULL) {
			
			for (c = current_time_str, j = 0; j < 8; c++, j++) {
				a_buf->text[j] = *c;
			}
			a_buf->isUsed = 1;
			Tx_message((char *)a_buf);
			release_buffer(a_buf);
		}
		else {
			// Turn on the LED.
			GPIOF->DATA |= 0x1UL << 1;
			run_status = 0;
		}
	}

	second++;
	remainder = second % 60;
	
	if (remainder == 0) {
		second = 0;
		minute++;
		remainder = minute % 60;
		
		if (remainder == 0) {
			minute = 0;
			hour++;
		}
	}
}

// Accept then msg for transmitting
void Tx_message(char* msg_buffer) {

	char * c;
	for (c = msg_buffer; *c != '\0'; c++) {
		UART0 -> DR = *c;
	}
	UART0 -> DR = (unsigned char) '\n';
	UART0 -> DR = (unsigned char) 0x0D;
}

struct buffer* Rx_message(void) {

	uint32_t itr;

	for (itr = 0; itr < NUM_BUFS; itr++) {
		
		if (rx_pool_ptr[itr]->isUsed == 1)
			return rx_pool_ptr[itr];
	}
	return NULL;
}

unsigned char* validate_Rx_Msg(void) {
	
	// Receiver Commands from console...	
	if ((rx_buffer[0] == 'P') && (rx_buffer[1] == 0x0D)) {
		display_time = 0;
		return rx_buffer;
	}
	else if ((rx_buffer[0] == 'C') && (rx_buffer[1] == 0x0D)) {
		
		display_time = 1;
		return rx_buffer;
	}
	else if ((rx_buffer[0] == 'F') && (rx_buffer[1] == 0x0D)) {
		
		speed = speed / 2;
		periodic_timer_Init();
		return rx_buffer;
	}
	else if ((rx_buffer[0] == 'S') && (rx_buffer[1] == 0x0D)) {
		
		speed = speed * 2;
		periodic_timer_Init();
		return rx_buffer;
	}
	else if ((rx_buffer[0] == 'Q') && (rx_buffer[1] == 0x0D)) {
		
		// Turn on the LED.
		GPIOF->DATA |= 0x1UL << 3;
		for(j=0;j<0xffff;j++);
		GPIOF->DATA &= ~(0x1UL << 3);
		run_status = 0;
		return rx_buffer;
	}
	else if ((rx_buffer[0] == 'T') &&
		(rx_buffer[1] >= 0x30 && rx_buffer[1] <= 0x35) &&
		(rx_buffer[2] >= 0x30 && rx_buffer[2] <= 0x39) &&
		(rx_buffer[3] == ':') &&
		(rx_buffer[4] >= 0x30 && rx_buffer[4] <= 0x35) &&
		(rx_buffer[5] >= 0x30 && rx_buffer[5] <= 0x39) &&
		(rx_buffer[6] == 0x0D)) {
								
		hour = ((rx_buffer[1] - 0x30) * 10) + (rx_buffer[2] - 0x30);
		minute = ((rx_buffer[4] - 0x30) * 10) + (rx_buffer[5] - 0x30);
		second = 0;
		
		return rx_buffer;
	}
	
	return NULL;
} 

struct buffer* addRxBufPool(struct buffer * a_buffer) {

	if (rx_pool_ptr[rx_in_counter] == NULL) {
		
		rx_pool_ptr[rx_in_counter] = a_buffer;
		return rx_pool_ptr[rx_in_counter];
	}
		
	return NULL;
}

// Free buffer back to the pool...
struct buffer* releaseRxBufPool(struct buffer * a_buffer){
	
	release_buffer(a_buffer);		
	rx_pool_ptr[rx_out_counter] = NULL;

	return rx_pool_ptr[rx_out_counter];
}

// Allocates pool of buffers....
void buffer_Init(void) {

	buf_ptr = buf;
}

// Retrieve available buffer...
struct buffer* get_buffer(void) {
	
	uint32_t itr;
	buf_ptr = buf;
	// Find free buffer...
	for (itr = 0; itr < NUM_BUFS; itr++, buf_ptr++) {
		
		if (buf_ptr->isUsed == 0) {

			buf_ptr->isUsed = 1;
			return buf_ptr;
		}
	}
	return NULL;
}

// Free buffer back to the pool...
void release_buffer(struct buffer * a_buffer) {
	
	uint32_t index;
	
	a_buffer->isUsed = 0;
	
	for (index = 0; index < BUF_SIZE; index++) {
		a_buffer->text[index] = '\0';
	}
}

int main(void)
{
	// Component Initializations...
	buffer_Init();
	button_Init();
	led_Init();
	UART_Init();
	periodic_timer_Init();
	
	while(run_status){}
}
