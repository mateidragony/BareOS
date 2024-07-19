#include <barelib.h>
#include <interrupts.h>
#include <tty.h>
#include <semaphore.h>

volatile uint32 tty_in_sem;         /* Semaphore used to lock `tty_getc` if waiting for data                                            */
volatile uint32 tty_out_sem;        /* Semaphore used to lock `tty_putc` if waiting for space in the queue                              */
char tty_in[TTY_BUFFLEN];  /* Circular buffer for storing characters read from the UART until requested by a thread            */
char tty_out[TTY_BUFFLEN]; /* Circular buffer for storing character to be written to the UART while waiting for it to be ready */
uint32 tty_in_head = 0;    /* Index of the first character in `tty_in`                                                         */
uint32 tty_in_count = 0;   /* Number of characters in `tty_in`                                                                 */
uint32 tty_out_head = 0;   /* Index of the first character in `tty_out`                                                        */
uint32 tty_out_count = 0;  /* Number of characters in `tty_out`                                                                */

bool tty_in_sem_enabled = true;
bool tty_out_sem_enabled = true;


void uart_handler(void);
int set_uart_interrupt(byte);

/*  Initialize the `tty_in_sem` and `tty_out_sem` semaphores  *
 *  for later TTY calls.                                      */
void tty_init(void) {
  if(tty_in_sem_enabled) tty_in_sem = sem_create(0);
  else tty_in_sem = 0;
  if(tty_out_sem_enabled) tty_out_sem = sem_create(TTY_BUFFLEN);
  else tty_out_sem = 0;
}

/*  Get a character  from the `tty_in`  buffer and remove  *
 *  it from the circular buffer.  If the buffer is empty,  * 
 *  wait on  the semaphore  for data to be  placed in the  *
 *  buffer by the UART.                                    */
char tty_getc(void) {
  char mask;


  if(tty_in_sem_enabled){
    sem_wait(tty_in_sem);
    mask = disable_interrupts();  
  } else {
    mask = disable_interrupts();                              
    if(tty_in_count == 0) tty_in_sem = 1;
    else tty_in_sem = 0;
    while(tty_in_sem); // sorry musser. I kinda do want to implement semaphores now tho...
    tty_in_sem = 0;
  }

  char c = tty_in[tty_in_head];
  tty_in_head = (tty_in_head+1) % TTY_BUFFLEN;
  tty_in_count--;

  restore_interrupts(mask);
  return c;
}

/*  Place a character into the `tty_out` buffer and enable  *
 *  uart interrupts.   If the buffer is  full, wait on the  *
 *  semaphore  until notified  that there  space has  been  *
 *  made in the  buffer by the UART. */
void tty_putc(char ch) {

  char mask;

  if(tty_out_sem_enabled){
    sem_wait(tty_out_sem);
    mask = disable_interrupts();
  } else {
    mask = disable_interrupts();                               /*  Prevent interruption while char is being written  */
    if(tty_out_count == TTY_BUFFLEN) tty_out_sem = 1;
    else tty_out_sem = 0;
    while(tty_out_sem); // sorry again...
  }

  tty_out[(tty_out_head + tty_out_count) % TTY_BUFFLEN] = ch;
  tty_out_count++;
  
  restore_interrupts(mask);
  set_uart_interrupt(1);
}
