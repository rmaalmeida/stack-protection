
#include <time.h>

#include <stdio.h>


#define TWO(c)     (0x1u << (c))
#define MASK(c)    (((unsigned int)(-1)) / (TWO(TWO(c)) + 1u))
#define COUNT(x,c) ((x) & MASK(c)) + (((x) >> (TWO(c))) & MASK(c))

unsigned char lkt[] = {0,1,1,2,
             1,2,2,3,
             1,2,2,3,
             2,3,3,4};

unsigned char lkt2[] = {0,1,1,0,
             1,0,0,1,
             1,0,0,1,
             0,1,1,0};
//0110 1001 1001 0110

#define bitcount5(n) (lkt[n&0x0f]+lkt[n>>4])
//#define bitcount2(n) (0x6996 >> ((n^(n>>4))&0x0f))

#define paridadee(n) (lkt2[((n^(n>>4))&0x0f)])
#define paridade(n) ((0x6996 >> ((n^(n>>4))&0x0f))&0x01)

unsigned char bitcount2(unsigned char n){return (lkt[n&0x0f]+lkt[n>>4]);}

int bitcount3(unsigned int v){
    //unsigned int v;  // word value to compute the parity of
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
}


#define TWO(c)     (0x1u << (c))
#define MASK(c)    (((unsigned int)(-1)) / (TWO(TWO(c)) + 1u))
#define COUNT(x,c) ((x) & MASK(c)) + (((x) >> (TWO(c))) & MASK(c))


int bitcount (unsigned int n)  {
   n = COUNT(n, 0) ;
   n = COUNT(n, 1) ;
   n = COUNT(n, 2) ;
   n = COUNT(n, 3) ;
   //n = COUNT(n, 4) ;
   // n = COUNT(n, 5) ;    for 64-bit integers
   return n ;
}

#define i_SIZE 10

//2^PARITY_QNT - PARITY_QNT -1  > i_SIZE*8
#define HMM_SIZE 7

unsigned int hamming(unsigned char * data_p, unsigned int length)
{
	unsigned char poolH;
	unsigned char hammingBits = 0;
	unsigned char pBit;
	unsigned char i;


	//clear all hamming bits


	//calculating each of the hamming bits

	poolH = 0;
	pBit = 0;
	for(i=0;i<length;i++){
		poolH += bitcount(*(data_p+i) & 0xAA);
	}
	hammingBits += (poolH&0x01)<<pBit;

	poolH = 0;
	pBit = 1;
	for(i=0;i<length;i++){
		poolH += bitcount(*(data_p+i) & 0xCC);
	}
	hammingBits += (poolH&0x01)<<pBit;

	poolH = 0;
	pBit = 2;
	for(i=0;i<length;i++){
		poolH += bitcount(*(data_p+i) & 0xF0);
	}
	hammingBits += (poolH&0x01)<<pBit;

	for(pBit=3;pBit<HMM_SIZE;pBit++){
		poolH = 0;
		for(i=0;i<length;i++){
			if((i*8+136) & (1<<pBit )){
				poolH += bitcount(*(data_p+i));
			}
		}
		hammingBits += (poolH&0x01)<<pBit;
	}
	return (unsigned int)hammingBits;
}

unsigned int hammingUltra(unsigned char * data_p, unsigned int length)
{
	unsigned char poolH;
	unsigned char hammingBits = 0;
	unsigned char pBit;
	unsigned char i;


	//clear all hamming bits


	//calculating each of the hamming bits

	poolH = 0;
	pBit = 0;
	for(i=0;i<length;i++){
		poolH ^= (*(data_p+i) & 0xAA);
	}

	hammingBits += (bitcount2(poolH)&0x01)<<pBit;

	poolH = 0;
	pBit = 1;
	for(i=0;i<i_SIZE;i++){
		poolH ^= (*(data_p+i) & 0xCC);
	}
	hammingBits += (bitcount2(poolH)&0x01)<<pBit;

	poolH = 0;
	pBit = 2;
	for(i=0;i<i_SIZE;i++){
		poolH ^= (*(data_p+i) & 0xF0);
	}
	hammingBits += (bitcount2(poolH)&0x01)<<pBit;

//this is the map of needed bytes to add to bit count

//i 0 1 2 3 4 5 6 7 8 9
//  3 1	0	1	0	1	0	1	0	1	0
//  4 0	1	1	0	0	1	1	0	0	1
//  5 0	0	0	1	1	1	1	0	0	0
//  6 0	0	0	0	0	0	0	1	1	1

  pBit = 3;
	poolH = 0;
	for(i=0;i<i_SIZE;i+=2){
			poolH ^= (*(data_p+i));
	}
	hammingBits += (bitcount2(poolH)&0x01)<<pBit;

  pBit = 4;
	poolH = 0;
	poolH ^= (*(data_p+1));
	poolH ^= (*(data_p+2));
	poolH ^= (*(data_p+5));
	poolH ^= (*(data_p+6));
	poolH ^= (*(data_p+9));
	hammingBits += (bitcount2(poolH)&0x01)<<pBit;

  pBit = 5;
	poolH = 0;
	for(i=3;i<7;i++){
			poolH ^= (*(data_p+i));
	}
	hammingBits += (bitcount2(poolH)&0x01)<<pBit;


  pBit = 6;
	poolH = 0;
	for(i=7;i<i_SIZE;i++){
			poolH ^= (*(data_p+i));
	}
	hammingBits += (bitcount2(poolH)&0x01)<<pBit;

	return (unsigned int)hammingBits;
}


unsigned int hammingFast(unsigned char * data_p, unsigned int length)
{
	unsigned char poolH;
	unsigned int hammingBits = 0;
	unsigned int pBit;
	unsigned int i;
	unsigned int posbit;



	//clear all hamming bits


	//calculating each of the hamming bits

	poolH = 0;
	pBit = 0;
	for(i=0;i<length;i++){
		poolH ^= (*(data_p+i) & 0xAA);
	}

	hammingBits += (paridade(poolH))<<pBit;

	poolH = 0;
	pBit = 1;
	for(i=0;i<length;i++){
		poolH ^= (*(data_p+i) & 0xCC);
	}
	hammingBits += (paridade(poolH))<<pBit;

	poolH = 0;
	pBit = 2;
	for(i=0;i<length;i++){
		poolH ^= (*(data_p+i) & 0xF0);
	}
	hammingBits += (paridade(poolH))<<pBit;

//2^n = x
//1<<n = x
//x =
//    pBit = length;
//    hammBits = lenght/8;
//
//    while (1<<pBit != 0) {
//        pBit>>=1;
//
//        hammBits <<= 1;
//    }
//   // printf("%i %i\n",length, hammBits);
//2^pBit = (length*8);
//1<<pBit = (length*8);

	for(pBit=3;(1<<pBit)<(length *8);pBit++){
		poolH = 0;
		posbit = (1<<(pBit-3));
		//printf("pattern %i\n", pBit);
		for(i=0;i<length;i++){
			if((i+1) & (posbit)){
				poolH ^= (*(data_p+i));
				//printf("1");
			}
			//else{printf("0");}

		}
		//printf("\n");
		hammingBits += (paridade(poolH))<<pBit;
	}


	//system("pause");
	return (unsigned int)hammingBits;
}


int main(int argc, char *argv[]){
    unsigned char data [100000] = {};
    unsigned long int i;
    clock_t begin, end;
    double time_spent;
    unsigned int original, falha,datasize;
    datasize = 10;
    for (i=0;i<datasize;i++){
        data[i] = rand()%256;
    }




    begin = clock();
    for(i=0;i<2000000;i++)
    {
        hamming(data,10);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("HamNo %f\n",time_spent);




    begin = clock();

    for(i=0;i<2000000;i++)
    {
        hammingFast(data,10);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("HamFa %f\n",time_spent);


   begin = clock();

    for(i=0;i<2000000;i++)
    {
        hammingUltra(data,10);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("HamUl %f\n",time_spent);



    return 0;
}//end main



