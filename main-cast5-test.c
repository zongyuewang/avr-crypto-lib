/*
 * cast5 test-suit
 * 
*/

#include "config.h"
#include "serial-tools.h"
#include "uart.h"
#include "debug.h"

#include "cast5.h"

#include <stdint.h>
#include <string.h>


/*****************************************************************************
 *  additional validation-functions											 *
 *****************************************************************************/

/*****************************************************************************
 *  self tests																 *
 *****************************************************************************/

void cast5_ctx_dump(cast5_ctx_t *s){
	uint8_t i;
	uart_putstr("\r\n==== cast5_ctx_dump ====\r\n shortkey: ");
	uart_putstr(s->shortkey?"yes":"no");
	for(i=0;i<16;++i){
		uint8_t r;
		uart_putstr("\r\n Km"); uart_hexdump(&i, 1); uart_putc(':');
		uart_hexdump(&(s->mask[i]), 4);
		uart_putstr("\r\n Kr"); uart_hexdump(&i, 1); uart_putc(':');
		r = (s->rotl[i/2]);
		if (i&0x01) r >>= 4;
		r &= 0xf;
		r += (s->roth[i>>3]&(1<<(i&0x7)))?0x10:0x00;
		uart_hexdump(&r, 1);
	}
}


void test_encrypt(uint8_t *block, uint8_t *key, uint8_t keylength, bool print){
	cast5_ctx_t s;
	if (print){
		uart_putstr("\r\nCAST5:\r\n key:\t");
		uart_hexdump(key, keylength/8);
		uart_putstr("\r\n plaintext:\t");
		uart_hexdump(block, 8);
	}
	cast5_init(&s, key, keylength);
//	cast5_ctx_dump(&s);
	cast5_enc(&s, block);
	if (print){
		uart_putstr("\r\n ciphertext:\t");
		uart_hexdump(block, 8);
	}
} 

void test_decrypt(uint8_t *block, uint8_t *key, uint8_t keylength, bool print){
	cast5_ctx_t s;
	if (print){
		uart_putstr("\r\nCAST5:\r\n key:\t");
		uart_hexdump(key, keylength/8);
		uart_putstr("\r\n ciphertext:\t");
		uart_hexdump(block, 8);
	}
	cast5_init(&s, key, keylength);
//	cast5_ctx_dump(&s);
	cast5_dec(&s, block);
	if (print){
		uart_putstr("\r\n plaintext:\t");
		uart_hexdump(block, 8);
	}
} 

void testrun_cast5(void){
	uint8_t block[8];
	uint8_t key[16];
	uint8_t *tda = (uint8_t*)"\x01\x23\x45\x67\x89\xAB\xCD\xEF",
	        *tka = (uint8_t*)"\x01\x23\x45\x67\x12\x34\x56\x78\x23\x45\x67\x89\x34\x56\x78\x9A";
	memcpy(block, tda, 8);
	memcpy(key, tka, 16);
	test_encrypt(block, key, 128, true);
	test_decrypt(block, key, 128, true);
	memcpy(block, tda, 8);
	memcpy(key, tka, 16);
	test_encrypt(block, key, 80, true);
	test_decrypt(block, key, 80, true);
	memcpy(block, tda, 8);
	memcpy(key, tka, 16);
	test_encrypt(block, key, 40, true);
	test_decrypt(block, key, 40, true);
	
/**** long test *****/
	uart_putstr("\r\nmaintance-test");
	uint8_t a[16]= {0x01, 0x23, 0x45, 0x67, 0x12,
				    0x34, 0x56, 0x78, 0x23, 0x45, 
				    0x67, 0x89, 0x34, 0x56, 0x78, 
				    0x9A}, 
			b[16]= {0x01, 0x23, 0x45, 0x67, 0x12,
				    0x34, 0x56, 0x78, 0x23, 0x45, 
				    0x67, 0x89, 0x34, 0x56, 0x78, 
				    0x9A};
	uint32_t i;
	for(i=0;i<1000000; ++i){
		test_encrypt(&(a[0]), &(b[0]), 128, false);
		test_encrypt(&(a[8]), &(b[0]), 128, false);
		test_encrypt(&(b[0]), &(a[0]), 128, false);
		test_encrypt(&(b[8]), &(a[0]), 128, false);
		if ((i&0x000000ff) == 0){
			uart_putstr("\r\n");
			uart_hexdump(&i, 4);
		}
	}
	uart_putstr("\r\na = "); uart_hexdump(a, 16);
	uart_putstr("\r\nb = "); uart_hexdump(b, 16);

}



/*****************************************************************************
 *  main																	 *
 *****************************************************************************/

int main (void){
	char str[20];

	
	DEBUG_INIT();
	uart_putstr("\r\n");

	uart_putstr("\r\n\r\nCrypto-VS\r\nloaded and running\r\n");
restart:
	while(1){ 
		if (!getnextwordn(str,20))  {DEBUG_S("DBG: W1\r\n"); goto error;}
		if (strcmp(str, "test")) {DEBUG_S("DBG: 1b\r\n"); goto error;}
			testrun_cast5();
		goto restart;		
		continue;
	error:
		uart_putstr("ERROR\r\n");
	} /* while (1) */
}
