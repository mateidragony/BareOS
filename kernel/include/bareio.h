/*
 *  This header includes function prototypes for IO procedures
 *  used by the UART and other print/scan related functions.
 */
#ifndef H_BAREIO
#define H_BAREIO

#define LINE_BUF_LEN 1024

char uart_putc(char);
char uart_getc(void);
void printf(const char*, ...);
void debug_printf(int, const char*, ...);
unsigned char get_line(char**, int*, int*, unsigned char, unsigned char(*)(char, char**, int*, int*, int*, unsigned char));

unsigned char shitty_get_line_for_ag(char *line_buf, int *line_len, int line_buf_sz);

#endif // bareio.h