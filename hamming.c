
#define MSG_SIZE 10

//2^PARITY_QNT - PARITY_QNT -1  > MSG_SIZE*8
#define HMM_SIZE 8

#define FULL_MSG MSG_SIZE+PARITY_QNT

#define TWO(c)     (0x1u << (c))
#define MASK(c)    (((unsigned int)(-1)) / (TWO(TWO(c)) + 1u))
#define COUNT(x,c) ((x) & MASK(c)) + (((x) >> (TWO(c))) & MASK(c))

int bitcount (unsigned int n)  {
   n = COUNT(n, 0) ;
   n = COUNT(n, 1) ;
   n = COUNT(n, 2) ;
   n = COUNT(n, 3) ;
   //n = COUNT(n, 4) ;    //for 32-bit integers
   //n = COUNT(n, 5) ;    //for 64-bit integers
   return n ;
}

unsigned int hamming(unsigned char * data_p, unsigned int length)
{
	unsigned char poolH;
	unsigned char hammingBits = 0;
	unsigned char pBit;
	unsigned char msg;
   
	
	//calculating each of the hamming bits
	
	poolH = 0;
	pBit = 0;
	for(msg=0;msg<length;msg++){
		poolH += bitcount(*(data_p+msg) & 0xAA);
	}
	hammingBits += (poolH&0x01)<<pBit;
	
	poolH = 0;
	pBit = 1;
	for(msg=0;msg<MSG_SIZE;msg++){
		poolH += bitcount(*(data_p+msg) & 0xCC);
	}
	hammingBits += (poolH&0x01)<<pBit;
	
	poolH = 0;
	pBit = 2;
	for(msg=0;msg<MSG_SIZE;msg++){
		poolH += bitcount(*(data_p+msg) & 0xF0);
	}
	hammingBits += (poolH&0x01)<<pBit;
	
	for(pBit=3;pBit<HMM_SIZE;pBit++){
		poolH = 0;
		for(msg=0;msg<MSG_SIZE;msg++){
			if((msg*8+136) & (1<<pBit )){
				poolH += bitcount(*(data_p+msg));
			}
		}
		hammingBits += (poolH&0x01)<<pBit;
	}
	return (unsigned int)hammingBits;
}