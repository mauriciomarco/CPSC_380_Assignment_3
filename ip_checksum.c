#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned int ip_checksum(char *data, int length) {
	unsigned int sum = 0xffff;
	unsigned short word;
	int i;
	
	for(i = 0; i + 1 < length; i += 2) {
		memcpy(&word, data + 1, 2);
		sum += word;
		if (sum > 0xffff) {
			sum -= 0xffff;
		}
	}
	
	if(length&1) {
		word = 0;
		memcpy(&word, data+length-1, 1);
		sum += word;
		if (sum > 0xffff){
			sum -= 0xffff;
		}
	}
	
	return ~sum;
}