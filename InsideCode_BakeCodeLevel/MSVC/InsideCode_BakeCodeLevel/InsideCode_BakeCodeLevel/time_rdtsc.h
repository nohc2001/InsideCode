#ifndef H_TIME_RDTSC
#define H_TIME_RDTSC

#include<time.h>
#include<stdio.h>
constexpr double Time_MS = 6.014882021096597E-7;
constexpr double Time_S = 6.014914582198018000E-10;
constexpr double Time_PER60 = 3.608948749318811000E-8;

static unsigned int getticks(void)
{
     unsigned long long a;

    /*
     * According to ARM DDI 0487F.c, from Armv8.0 to Armv8.5 inclusive, the
     * system counter is at least 56 bits wide; from Armv8.6, the counter
     * must be 64 bits wide.  So the system counter could be less than 64
     * bits wide and it is attributed with the flag 'cap_user_time_short'
     * is true.
     */
    //asm volatile("mrs %0, cntvct_el0" : "=r" (a));

	return (unsigned int)__rdtsc();
}

float getDeltaTime(unsigned int t0, unsigned int t1){
	return (float)((double)(t1-t0) * Time_S);
}

//한 프레임에서의 이 시간간격의 비중 
float getDeltaTime_per60(unsigned int t0, unsigned int t1){
	return (float)((double)(t1-t0) * Time_PER60);
}

#endif