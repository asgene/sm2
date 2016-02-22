/*

Example GF(2^m) Elliptic Curve Diffie-Hellman program for constrained environments. Uses point compression.
Stack-only memory allocation

Use with this mirdef.h header (for a PC using MS C)

#define MR_LITTLE_ENDIAN
#define MIRACL 32
#define mr_utype int
#define MR_IBITS 32
#define MR_LBITS 32
#define mr_unsign32 unsigned int
#define mr_dltype __int64
#define mr_unsign64 unsigned __int64
#define MR_STATIC 6
#define MR_ALWAYS_BINARY
#define MR_NOASM
#define MR_STRIPPED_DOWN
#define MR_GENERIC_MT
#define MAXBASE ((mr_small)1<<(MIRACL-1))
#define MR_BITSINCHAR 8
#define MR_NOKOBLITZ
#define MR_NO_SS
#define MR_SIMPLE_BASE
#define MR_SIMPLE_IO

and also possibly

#define MR_NO_FILE_IO
#define MR_NO_STANDARD_IO

To reduce size further consider using your own random number generator

#define MR_NO_RAND

Also consider NOT using precomputation (use ecurve2_mult(.) instead of mul2_brick(.)), and
go through each of the modules below and carefully delete unused functions..

And do not use point compression??

Build the library from these modules (Example using MS C compiler)

cl /c /O2 /W3 mrcore.c
cl /c /O2 /W3 mrarth0.c
cl /c /O2 /W3 mrarth1.c
cl /c /O2 /W3 mrio1.c
cl /c /O2 /W3 mrbits.c
cl /c /O2 /W3 mrgf2m.c
cl /c /O2 /W3 mrec2m.c

rem
rem Create library 'miracl.lib'
del miracl.lib

lib /OUT:miracl.lib mrbits.obj mrio1.obj
lib /OUT:miracl.lib miracl.lib mrarth0.obj mrarth1.obj mrcore.obj 
lib /OUT:miracl.lib miracl.lib mrec2m.obj mrgf2m.obj
del mr*.obj

rem Create the program

cl /O2 ecdh2m.c miracl.lib

*/

#include <stdio.h>
#include <string.h>
#include "miracl.h"

#define HEXDIGS (MIRACL/4)

/* !!!!!! THIS CODE AND THESE ROMS ARE NOW CREATED AUTOMATICALLY USING THE ROMAKER2.C APPLICATION !!!!!!!! */
/* !!!!!! READ COMMENTS IN ROMAKER2.C !!!!!! */


#define CURVE_M 163
#define CURVE_A 7
#define CURVE_B 6
#define CURVE_C 3

/* NIST b163 bit elliptic curve Y^2+XY=X^3+A.X^2+B (from nist163.ecs). Irreducible polynomial 
   is x^163+x^7+x^6+x^3+1. Here is stored B, the group order q, and the generator G(x,y) */

static const mr_small rom[]=
{0x4A3205FD,0x512F7874,0x1481EB10,0xB8C953CA,0x0A601907,2,    
 0xA4234C33,0x77E70C12,0x000292FE,0,0,4,   
 0xE8343E36,0xD4994637,0xA0991168,0x86A2D57E,0xF0EBA162,3,     
 0x797324F1,0xB11C5C0C,0xA2CDD545,0x71A0094F,0xD51FBC6C,0};  

#define WINDOW 4

/* 2^4 =16 precomputed points based on fixed generator G(x,y) */
/* (created using romaker2.c program with window size of 4)   */

static const mr_small prom[]=
{0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,
0xe8343e36,0xd4994637,0xa0991168,0x86a2d57e,0xf0eba162,0x3,
0x797324f1,0xb11c5c0c,0xa2cdd545,0x71a0094f,0xd51fbc6c,0x0,
0x5fd9372,0xaa17fdf2,0x6648e974,0x31dd5c03,0x75f1f527,0x2,
0xdcf990e0,0x79117c6a,0x5d2ff662,0x9865bdb2,0xb0914960,0x0,
0x78a887a6,0xe478fe58,0x3d3d2364,0x3b85b263,0x34c6885e,0x5,
0xef273598,0xa6d7e436,0x53001bd9,0x75cc731c,0xbf414f74,0x3,
0x4f685f08,0x6f8daf88,0xd1c71b1f,0x6f4913ff,0xbaa8682a,0x2,
0xd5301336,0xb4f6dd1e,0x6454423b,0x4ebaf45,0x65c6e401,0x2,
0xa700c73d,0x6f842a26,0xe8f5f5fd,0x4db6da9,0x48fcdc95,0x6,
0x1db757cb,0xb5bf42ba,0xabfd25b4,0x466e5be3,0x2cf27012,0x7,
0xeb7ad12e,0x47e38e87,0x5f9c4ef1,0x2fcd5483,0x3893822e,0x0,
0x7c98bf09,0xa41f9589,0x680ca2a3,0xdfa83842,0xedd41659,0x1,
0xa3463d19,0xaf8c4bb0,0x2288658a,0x84b3c40d,0x90cd449b,0x2,
0x6bb4bf11,0x5e996afa,0x9b2ae97a,0xff211307,0xfbf58239,0x5,
0x4ab8c649,0x2fcf02ee,0x3c7efb85,0x76f09ad9,0xdbca45da,0x7,
0xeeba93af,0x59adf276,0xd25e3760,0x3292b2c1,0x271d1b84,0x4,
0x37f9391c,0xf423fd60,0xbf079624,0x4c9036e0,0x63075e19,0x4,
0xb726dfb5,0x38c349b3,0x6e0988b7,0x87b54141,0xcb8d73b,0x2,
0xd1acdb88,0x64ca1fcc,0x690dfa73,0xc6708b6f,0xb44b3919,0x2,
0x2b3f67d9,0xc01ccb1e,0x5311dcc8,0xdb9fdd9a,0x9118031e,0x0,
0x9a5c0d6e,0x4e99152f,0xaf853232,0x8db0dccc,0xef0f58d7,0x5,
0x6db63148,0xefdd7e0a,0xcc819e67,0x56a979bc,0x8d1169b4,0x3,
0x8670b917,0x3d61a2ac,0x6ed8588a,0x5cc94655,0x8096413e,0x2,
0xd8715f72,0xc2ce06e6,0xd50c5c77,0x1553a69b,0x8a20fadc,0x2,
0x9dd077b2,0xb2978893,0x7aa617bb,0xadbac172,0xf57da9a2,0x1,
0x6800bbe2,0x837ad74e,0xdac7cd95,0x1082c382,0xb5ec04b2,0x0,
0x43835328,0xa986d58b,0x8d7f7c4e,0xfd642a8d,0x6d4462d4,0x3,
0xf19901e9,0x60993930,0x67c98e9b,0x1ab9c90d,0x5d7f4593,0x6,
0x39e10752,0xde366730,0xa867db73,0x5bcb53b5,0x6b5e56c0,0x3,
0xaf59dcaa,0x8bdbf9ff,0x2221c861,0x52b45c1a,0x221a9b1f,0x6};

#define WORDS 6  /* Number of words per big variable 6*32 > 163 */

/* Note that in a real application a source of real random numbers would be required, to
   replace those generated by MIRACL's internal pseudo-random generator "bigbits"  
   Alternatively from a truly random and unguessable seed, use MIRACL's strong random 
   number generator */

/* Elliptic Curve Diffie-Hellman, using point compression to minimize bandwidth, 
   and precomputation to speed up off-line calculation */

int main()
{
    int promptr;
    epoint *PA,*PB;
    big A,B,a,b,q,pa,pb,key,x,y;
    ebrick2 binst;
    miracl instance;      /* create miracl workspace on the stack */

/* Specify base 16 here so that HEX can be read in directly without a base-change */

    miracl *mip=mirsys(&instance,WORDS*HEXDIGS,16); /* size of bigs is fixed */
    char mem_big[MR_BIG_RESERVE(9)];          /* we need 9 bigs... */
    char mem_ecp[MR_ECP_RESERVE(2)];          /* ..and two elliptic curve points */
 	memset(mem_big, 0, MR_BIG_RESERVE(9));    /* clear the memory */
	memset(mem_ecp, 0, MR_ECP_RESERVE(2));

    A=mirvar_mem(mip, mem_big, 0);       /* Initialise big numbers */
    B=mirvar_mem(mip, mem_big, 1);
    pa=mirvar_mem(mip, mem_big, 2);
    pb=mirvar_mem(mip, mem_big, 3);
    key=mirvar_mem(mip, mem_big, 4);
    x=mirvar_mem(mip, mem_big, 5);
    y=mirvar_mem(mip, mem_big, 6);
    a=mirvar_mem(mip, mem_big, 7);
    b=mirvar_mem(mip, mem_big, 8);

    PA=epoint_init_mem(mip, mem_ecp, 0); /* initialise Elliptic Curve points */
    PB=epoint_init_mem(mip, mem_ecp, 1);

    irand(mip, 3L);                      /* change parameter for different random numbers */
    promptr=0;
    init_big_from_rom(B,WORDS,rom,WORDS*4,&promptr);  /* Read in curve parameter B from ROM */
                                                 /* don't need q or G(x,y) (we have precomputed table from it) */

    convert(mip,1,A);                            /* set A=1 */

/* Create precomputation instance from precomputed table in ROM */

    ebrick2_init(&binst,prom,A,B,CURVE_M,CURVE_A,CURVE_B,CURVE_C,WINDOW,CURVE_M);

/* offline calculations */

    bigbits(mip,CURVE_M,a);  /* A's random number */
    mul2_brick(mip,&binst,a,pa,pa);    /* a*G =(pa,ya) */

    bigbits(mip,CURVE_M,b);  /* B's random number */
    mul2_brick(mip,&binst,b,pb,pb);    /* b*G =(pb,yb) */

/* Swap X values */

/* online calculations */
    ecurve2_init(mip,CURVE_M,CURVE_A,CURVE_B,CURVE_C,A,B,FALSE,MR_PROJECTIVE);
    epoint2_set(mip,pb,pb,0,PB); /* decompress PB */
    ecurve2_mult(mip,a,PB,PB);
    epoint2_get(mip,PB,key,key);

/* since internal base is HEX, can use otnum instead of cotnum - avoiding a base change */

#ifndef MR_NO_STANDARD_IO
printf("Alice's Key= ");
otnum(mip,key,stdout);
#endif

    epoint2_set(mip,pa,pa,0,PB); /* decompress PA */
    ecurve2_mult(mip,b,PB,PB);
    epoint2_get(mip,PB,key,key);

#ifndef MR_NO_STANDARD_IO
printf("Bob's Key=   ");
otnum(mip,key,stdout);
#endif

/* clear the memory */

	memset(mem_big, 0, MR_BIG_RESERVE(9));
	memset(mem_ecp, 0, MR_ECP_RESERVE(2));

	return 0;
}