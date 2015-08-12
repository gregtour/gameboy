/*
	Random Inputs
	[no soft or hard a.i. yet]
*/

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>
#include <math.h>

unsigned char ChooseInput();

/*  XORShift128+ Written in 2014 by Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

uint64_t XORShiftNext();

uint64_t xorshift_seed[ 2 ] = {0x021f9c6a70ca0c24ULL, 0xbae766b26645aa22ULL};

uint64_t XORShiftNext() {
    uint64_t s1 = xorshift_seed[ 0 ];
    const uint64_t s0 = xorshift_seed[ 1 ];
    xorshift_seed[ 0 ] = s0;
    s1 ^= s1 << 23; // a
    return ( xorshift_seed[ 1 ] = ( s1 ^ s0 ^ ( s1 >> 17 ) ^ ( s0 >> 26 ) ) ) + s0; // b, c
}


const uint64_t XORSHIFT_MAX = 0xffffffffffffffffULL;


/*Rand.seed((OPTIONAL) seed_num)*/
/*If no seed given, will seed using current UNIX second.*/
/*Returns the seed used.*/
void SeedRand(){
  //TODO: Seeding the XORSHIFT128+ with the built in generator is somewhat questionable.
  int seed = 0;
  seed = (int)time(NULL);

  srand(seed);
  //Seed the XORSHIFT128+ PRNG with whatever junky random number generator was built in.
  xorshift_seed[0] = ((long long)rand() << 32) | rand();
  xorshift_seed[1] = ((long long)rand() << 32) | rand();

}

unsigned int randu()
{
	// Super RNG
	return (unsigned int)((XORShiftNext() & UINT_MAX) % (278499))
		+ rand();
}

typedef unsigned int u32;
typedef unsigned char u8;

u32* moves = NULL;
u32* startmoves = NULL;
u32  movesbytes = 0;

#define SET(d,i, x) (d[(i)/16] = (d[(i)/16] & ~(3 << (2*((i)%16)))) | ((x&3) << (2*((i)%16))))
#define GET(d,i)   	((d[(i)/16] >> (2*((i)%16))) & 3)


u32 movesize = 0;
u32 move_j = 0;
u8 Tumbler()
{
	static u32 mptr = 0;

	u8 dir;
	u8 i;

	if (moves == NULL) {
		movesize = 1;
		movesbytes = 1;
		moves = (u32*)malloc(sizeof(u32) * movesbytes);
		startmoves = (u32*)malloc(sizeof(u32) * movesbytes);
		moves[0] = 0;
		startmoves[0] = 0;
		mptr = 0;
	}

	dir = GET(moves, mptr); //(moves[mptr/16] >> (2*(mptr%16)));
	mptr++;
	if (mptr == movesize) {
		//moves[mptr/16](mptr-1) 
		u8 cur = 0;
		while (cur == 0 && mptr > 0) {
			mptr--;
			cur = GET(moves, mptr);
			cur = (cur + 1) % 4;
			SET(moves, mptr, cur);
			if (cur == GET(startmoves, mptr)) {
				cur = rand()%4;
				SET(moves, mptr, cur);
				SET(startmoves, mptr, cur);
				cur = 0;
			} else
				cur = 1;
		}
		if (mptr == 0 && cur == 0)
		{ 
			//if (movesize == 1) movesize = 23;
			movesize = movesize + 1;
			if (((movesize+15)/16) > movesbytes) {
				movesbytes++;
				moves = (u32*)realloc(moves, sizeof(u32) * movesbytes);
				startmoves = (u32*)realloc(startmoves, sizeof(u32) * movesbytes);
			}
			for (i = 0; i < movesbytes; i++)
			{
				moves[i] = rand(); //randu();
				startmoves[i] = moves[i];
			}
		}
		mptr = 0;
	}  

	move_j++;
	return dir;
}



#if 1
enum {BTN_RIGHT = 0, BTN_LEFT = 1, BTN_UP = 2, BTN_DOWN = 3, BTN_A = 4, BTN_B = 5, BTN_SELECT = 6, BTN_START = 7, NO_BUTTON = 255};
unsigned char ChooseInput(unsigned char prev)
{
	static int buttonCount = 0;
	static unsigned int buttonOn = 0;
	static unsigned int buttonOff = 0;
	static unsigned char lastkey = NO_BUTTON;

	if (buttonOn > 0)
	{
		buttonOn--;
		return lastkey;
	}
	if (buttonOff > 0)
	{
		buttonOff--;
		return NO_BUTTON;
	}
	
	buttonCount++;
	// A B direction
	if (buttonCount % 4 == 3 /*|| buttonCount % 3 == 1*/)
	{
		lastkey = BTN_A + (randu() % 3) % 2;
		buttonOn = 4;
		buttonOff = 2;
	}
	else
	{
		lastkey = Tumbler() + BTN_RIGHT;
		//lastkey = randu() % 4;
		buttonOn = 6 * (randu() % 10) + 6;
		/*buttonOn = 12;//8;//6;
		buttonOff = 0; //12; //32;//4;*/
		buttonOff = 4;
	}

	return lastkey;
}

unsigned char ChooseInputBattle(unsigned int time)
{
	static int buttonCount = 0;
	static unsigned int buttonOn = 0;
	static unsigned int buttonOff = 0;
	static unsigned char lastkey = NO_BUTTON;

	if (buttonOn > 0)
	{
		buttonOn--;
		return lastkey;
	}
	if (buttonOff > 0)
	{
		buttonOff--;
		return NO_BUTTON;
	}

	buttonCount++;
	lastkey = BTN_A;
	buttonOn = 4;
	buttonOff = 2;

	if (time > 2000)
	{
		lastkey = (randu() % 6);
		buttonOn = 8;
		buttonOff = 8;
	}

	return lastkey;
}
#endif

#if 0
// 0, 1, 2, 3 right left up down
// 4, 5 A B
// 6, 7 select start
enum {BTN_RIGHT = 0, BTN_LEFT = 1, BTN_UP = 2, BTN_DOWN = 3, BTN_A = 4, BTN_B = 5, BTN_SELECT = 6, BTN_START = 7, NO_BUTTON = 255};
unsigned char ChooseInput(unsigned char prev)
{
	unsigned char frame12;
	unsigned char frame16;
	static int frame = 0;
	static int buttonCount = 0;
	static unsigned int direction = 0;
	static unsigned int steps = 0;
	frame = frame + 1;
	frame12 = frame % 12;
	frame16 = frame % 16;

	// alternate 6 on 6 off
	if (frame12 && frame12 < 6) return prev;
	if (frame12) return NO_BUTTON;

	// scrupulously pick a button to press
	buttonCount = (buttonCount + 1) % 16;

	// A A B A A B A A B _ direction direction direction direction direction direction
	if (buttonCount % 4 == 0) return BTN_A;
	if (buttonCount % 4 == 1) return BTN_A;
	if (buttonCount % 4 == 2) return BTN_B;

	// take up to 10 steps in a random direction
	if (steps == 0)
	{
		direction = randu() % 4;
		steps = (randu() % 10) + 2;
	}
	steps--;
	return direction;
}
#endif

#if 0
/*Rand.rand((OPTIONAL) first_bound, (OPTIONAL) second_bound)*/
/*Returns a random primitive*/
unsigned char ChooseInput(unsigned char prev)
{
	int num;
	unsigned char key;

	if (rand() % 2 == 1) { return prev; }

	// 0, 1, 2, 3 right left up down
	// 4, 5 A B
	// 6, 7 select start
    //TODO: Fix this hack.
    num = (int)((XORShiftNext() & UINT_MAX) % (278499));
	
	num = (num % 100);
	//if (num == 0) return 6;
	//if (num < 2) return 7;
	if (num == 0) return 7;
	if (num < 33) return 4;
	if (num < 50) return 5;
	if ((num/2) % 2 == 0) return ( (num/4)%2 );
	//return (num%4);
	return (num)%2 + 2;

	/*// 85% // 75% //50% A, B
	if (num % 8 != 7)
	{
		num = num >> 5;
		if (num % 2) return 4;
		return 5;
	}
	// 33% Movement
	else if (num % 3 == 0 || num % 3 == 1)
	{
		num = (num/12);
		return (num % 4);
	}
	// 17% Start or Select
	else
	{
		if (num & 0x0100) return 6;
		else return 7;
	}

    return key;*/
}
#endif
