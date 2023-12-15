/*
 * Snoolie K / 0xilis
 * 13 December 2023 (EST)
 *
 * Speedtest for AdelaideSky's SkyKitC
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/*
 * i starts at 0 and ends at REPEAT_TIMES
 * We divide it by REPEAT_TIMES_DOUBLE
 * Therefore, when REPEAT_TIMES is 100
 * and REPEAT_TIMES_DOUBLE is 100.0
 * it tests for angles 0.0-1.0, with
 * it incrementing by 0.01 each time
 * (testing every hundredth decimal).
 * Right now, REPEAT_TIMES is 1000
 * and REPEAT_TIMES_DOUBLE is 100.0
 * meaning it currently tests range
 * 0.0-10.0, with it testing every
 * hundredth decimal.
*/

#define REPEAT_TIMES 1000
#define REPEAT_TIMES_DOUBLE 100.0

/*
 * The reason why this function is here is the
 * compiler is *too* smart. In our speed tests,
 * it will see the value of the function we're
 * speed testing is unused, so if we have optimizations
 * turned on, it removes the call, making our
 * speedtest useless. So, we pass the value into
 * a blank function like this, and declare it with
 * optnone to force clang not to optimize it.
 * This does also generate a call to this function
 * in the speed tests, technically making it slightly
 * inaccurate due to it also having to account for
 * a function call, but this function does nothing
 * anyway so the error this creates should be so
 * small that it would be not representable anyway,
 * at least on any modern-ish CPU.
 *
 * I've only tested this trick with clang and not sure
 * if it also works on GCC, but if it does and GCC's
 * compiler optimizations also remove the "unused"
 * function call, I've heard that:
 * __attribute__ ((optimize(0)))
 * should get you the same result... 
*/
__attribute__((optnone)) void blank_call(double x) {
 return;
}

/* 
 * PLEASE compile this with compiler optimizations
 * so these #define's are pre-calculated...
*/
#define PI_SQUARED M_PI*M_PI
#define ONE_POINT_FIVE_PI M_PI_2*3

double cosBhaskara(double angle) {
  if (angle > M_PI_2) {
    /*
     * Bhaskara's formula seems to only approximate the range (-M_PI_2,M_PI_2)
     * So, if we are larger, we mirror it and add angle by PI
     * If we are larger than 1.5 * PI then we add 2 PI to angle
     * Maybe there's a way to do this using only 2 conditions
     * considering we only need the range (0,DOUBLE_PI) but
     * I don't know how unfortunately :P
    */
    if (angle < ONE_POINT_FIVE_PI) {
      /* Mirror and add PI to angle */
      angle += M_PI;
      double angleSquared = angle*angle;
      return -1 * (PI_SQUARED - (4 * angleSquared)) / (PI_SQUARED + angleSquared);
    }
    /* Don't mirror but add 2PI to angle */
    angle += M_PI*2;
  }
  double angleSquared = angle*angle;
  return (PI_SQUARED - (4 * angleSquared)) / (PI_SQUARED + angleSquared);
}

/*
 * The following is an implementation of
 * Bhaskara's cosine formula. This accurately
 * approximates the cosine and is pretty fast.
 * In the context SkyKitC gets the cosine;
 * the *only* time it ever needs the cosine is
 * for the calcPos() function. Therefore, we
 * only ever need to approximate range
 * [0,M_PI*2]. Bhaskara's cosine approximates
 * [-M_PI_2,M_PI_2] which is not enough for us but
 * as explained in the comment below, we
 * simply just mirror and sub angle by M_PI
 * if our angle is in range (M_PI_2,3*M_PI_2)
 * and if our angle is larger than 3*M_PI_2
 * then we subtract our angle by 2*M_PI.
 * This should clear us an accurate range
 * of [-M_PI_2,M_PI_2*5] which covers more
 * than what we need. This logic does however
 * add conditionals to our function which I
 * really am not a fan of; hence why I tried
 * to look for an approximation algorithm
 * that would still be speedy while supporting
 * a wider range, which is implemented in
 * cosBhaskaraAndSnoolieV2/cosBhaskaraAndSnoolie
 * with cosBhaskaraAndSnoolieV2 being slightly
 * more accurate than cosBhaskaraAndSnoolie.
 * This still doesn't handle a range enough
 * that would allow us to use no conditionals,
 * but we do use one less conditional, meaning
 * only one conditional instead of two.
*/

__attribute__((always_inline)) static double cosBhaskara(double angle) {
  if (angle > M_PI_2) {
    /*
     * Bhaskara's formula seems to only approximate the range [-M_PI_2,M_PI_2]
     * So, if we are larger, we mirror it and add angle by PI
     * If we are larger than 1.5 * PI then we add 2 PI to angle
     * Maybe there's a way to do this using only 2 conditions
     * considering we only need the range (0,DOUBLE_PI) but
     * I don't know how unfortunately :P
    */
    if (angle < ONE_POINT_FIVE_PI) {
      /* Mirror and add PI to angle */
      angle -= M_PI;
      double angleSquared = angle*angle;
      return -1 * (PI_SQUARED - (4 * angleSquared)) / (PI_SQUARED + angleSquared);
    }
    /* Don't mirror but add 2PI to angle */
    angle -= M_PI*2;
  }
  double angleSquared = angle*angle;
  return (PI_SQUARED - (4 * angleSquared)) / (PI_SQUARED + angleSquared);
}

__attribute__((always_inline)) static double cosBhaskaraAndSnoolie(double angle) {
  if (angle > M_PI_2) {
    /*
     * Bhaskara's formula seems to only approximate the range (-M_PI_2,M_PI_2)
     * So, if we are larger, we use another formula I thought of.
     * Perhaps someone else thought of this before me, but I haven't
     * seen anyone else come up with this cosine approximation...
     * unlike bhaskara's it *does* have some inaccuracy but tbh we
     * shouldn't need to worry about the margin of error in this
     * context since it's small enough to not matter. I'm not a
     * mathematician however so this probably isn't perfect, if
     * someone more knowledgable than me could tweak it a bit to
     * be faster or more accurate that would be great :P.
    */
    double c = (angle - ONE_POINT_FIVE_PI) / M_PI;
    double preWarp = (c - c * fabs(c));
    return preWarp * (3 + (fabs(preWarp) * 4));
  }
  double angleSquared = angle*angle;
  return (PI_SQUARED - (4 * angleSquared)) / (PI_SQUARED + angleSquared);
}

__attribute__((always_inline)) static double sinBhaskara(double angle) {
  double doublePi = M_PI*2;
  return (16 * angle * (M_PI - angle)) / (5 * doublePi - (4 * angle * (M_PI - angle)));
}

/* this is terribly inaccurate... */
double atanSnoolie(double angle) {
 /* find formula to use */
 if (angle > 2.038) {
  if (angle > 4.0) {
   return 1.43; /* cap at 1.43 */
  }
  return (0.16 * angle) + 0.789;
 } else if (angle < -1.735) {
  if (angle < -3.506) {
   return -1.35; /* cap at -1.35 */
  }
  return (0.16 * angle) - 0.789;
 } else {
  /* main formula */
  return M_PI_4 * angle - angle * (fabs(angle) - 1) * (0.2447 - (0.00722 * fabs(angle)));
 }
}

#define M_PI_10 M_PI/10 /* PLEASE compile with compiler optimizations to save a div... */

__attribute__((always_inline)) static double cosBhaskaraAndSnoolieV2(double angle) {
  if (angle > M_PI_2) {
    /*
     * Bhaskara's formula seems to only approximate the range (-M_PI_2,M_PI_2)
     * So, if we are larger, we use another formula I thought of.
     * Perhaps someone else thought of this before me, but I haven't
     * seen anyone else come up with this cosine approximation...
     * unlike bhaskara's it *does* have some inaccuracy but tbh we
     * shouldn't need to worry about the margin of error in this
     * context since it's small enough to not matter. I'm not a
     * mathematician however so this probably isn't perfect, if
     * someone more knowledgable than me could tweak it a bit to
     * be faster or more accurate that would be great :P.
    */
    double b = (angle - ONE_POINT_FIVE_PI);
    double preWarp = (b * (fabs(b) - M_PI));
    return preWarp * (-M_PI_10 - (fabs(preWarp) * 0.03693172));
  }
  double angleSquared = angle*angle;
  return (PI_SQUARED - (4 * angleSquared)) / (PI_SQUARED + angleSquared);
}

double speedtest_libc_cos(void) {
 clock_t startTime = clock();

 /* do program */

 for (int i = 0; i < REPEAT_TIMES; i++) {
  double inp = (double)i / REPEAT_TIMES_DOUBLE;
  double res = cos(inp);
  blank_call(res);
 }

 /* get time */
 return (double)(clock() - startTime) / CLOCKS_PER_SEC;
}

double speedtest_bhaskara_cos(void) {
 clock_t startTime = clock();

 /* do program */

 for (int i = 0; i < REPEAT_TIMES; i++) {
  double inp = (double)i / REPEAT_TIMES_DOUBLE;
  double res = cosBhaskaraAndSnoolie(inp);
  blank_call(res);
 }

 /* get time */
 double elapsed_time = (double)(clock() - startTime) / CLOCKS_PER_SEC;

 //printf("elapsed_time: %f\n", elapsed_time);
 return elapsed_time;
}

double speedtest_libc_sin(void) {
 clock_t startTime = clock();

 /* do program */

 for (int i = 0; i < REPEAT_TIMES; i++) {
  double inp = (double)i / REPEAT_TIMES_DOUBLE;
  inp *= (M_PI * 2);
  double res = sin(inp);
  blank_call(res);
 }

 /* get time */
 return (double)(clock() - startTime) / CLOCKS_PER_SEC;
}

double speedtest_bhaskara_sin(void) {
 clock_t startTime = clock();

 /* do program */

 for (int i = 0; i < REPEAT_TIMES; i++) {
  double inp = (double)i / REPEAT_TIMES_DOUBLE;
  inp *= (M_PI * 2);
  double res = sinBhaskara(inp);
  blank_call(res);
 }

 /* get time */
 return (double)(clock() - startTime) / CLOCKS_PER_SEC;
}

double speedtest_libc_atan(void) {
 clock_t startTime = clock();

 /* do program */

 for (int i = 0; i < REPEAT_TIMES; i++) {
  double inp = (double)i / REPEAT_TIMES_DOUBLE;
  double res = atan(inp);
  blank_call(res);
 }

 /* get time */
 return (double)(clock() - startTime) / CLOCKS_PER_SEC;
}

double speedtest_snoolie_atan(void) {
 clock_t startTime = clock();

 /* do program */

 for (int i = 0; i < REPEAT_TIMES; i++) {
  double inp = (double)i / REPEAT_TIMES_DOUBLE;
  double res = atanSnoolie(inp);
  blank_call(res);
 }

 /* get time */
 return (double)(clock() - startTime) / CLOCKS_PER_SEC;
}

double speedtest_snoolie_cosv1(void) {
 clock_t startTime = clock();
 for (int i = 0; i < REPEAT_TIMES; i++) {
  double inp = (double)i / REPEAT_TIMES_DOUBLE;
  double res = cosBhaskaraAndSnoolie(inp);
  blank_call(res);
 }
 return (double)(clock() - startTime) / CLOCKS_PER_SEC;
}

double speedtest_snoolie_cosv2(void) {
 clock_t startTime = clock();
 for (int i = 0; i < REPEAT_TIMES; i++) {
  double inp = (double)i / REPEAT_TIMES_DOUBLE;
  double res = cosBhaskaraAndSnoolieV2(inp);
  blank_call(res);
 }
 return (double)(clock() - startTime) / CLOCKS_PER_SEC;
}

void test_speedcases(void) {
 double bhaskara_max_wait = 0;
 double libc_max_wait = 0;

 double bhaskara_sin_time = 0;
 double libc_sin_time = 0;

 
 double averageTimeBhaskara = 0;
 double averageTimeLibc = 0;
 for (int i = 0; i < 100; i++) {
  bhaskara_sin_time = speedtest_snoolie_cosv2();
  libc_sin_time = speedtest_libc_cos();
  
  /* detect max wait */
  if (bhaskara_sin_time > bhaskara_max_wait) {
   bhaskara_max_wait = bhaskara_sin_time;
  }
  if (libc_sin_time > libc_max_wait) {
   libc_max_wait = libc_sin_time;
  }

  /* add to average */
  averageTimeBhaskara += bhaskara_sin_time;
  averageTimeLibc += libc_sin_time;
 }
 printf("snoolie total: %f\n", averageTimeBhaskara);
 printf("libc total: %f\n", averageTimeLibc);
 averageTimeBhaskara /= 100.0;
 averageTimeLibc /= 100.0;
 printf("snoolie average: %f\n", averageTimeBhaskara);
 printf("libc average: %f\n", averageTimeLibc);
 printf("snoolie max wait: %f\n", bhaskara_max_wait);
 printf("libc max wait: %f\n", libc_max_wait);
}

/* I just copied and pasted the above function, which is why the variables are bhaskara and libc */
void test_speedcases_snoolie(void) {
 double bhaskara_max_wait = 0;
 double libc_max_wait = 0;

 double bhaskara_sin_time = 0;
 double libc_sin_time = 0;
 double averageTimeBhaskara = 0;
 double averageTimeLibc = 0;

 for (int i = 0; i < 100; i++) {
  bhaskara_sin_time = speedtest_snoolie_cosv2();
  libc_sin_time = speedtest_snoolie_cosv1();
  
  /* detect max wait */
  if (bhaskara_sin_time > bhaskara_max_wait) {
   bhaskara_max_wait = bhaskara_sin_time;
  }
  if (libc_sin_time > libc_max_wait) {
   libc_max_wait = libc_sin_time;
  }

  /* add to average */
  averageTimeBhaskara += bhaskara_sin_time;
  averageTimeLibc += libc_sin_time;
 }
 printf("new total: %f\n", averageTimeBhaskara);
 printf("old total: %f\n", averageTimeLibc);
 averageTimeBhaskara /= 100.0;
 averageTimeLibc /= 100.0;
 printf("new average: %f\n", averageTimeBhaskara);
 printf("old average: %f\n", averageTimeLibc);
 printf("new max wait: %f\n", bhaskara_max_wait);
 printf("old max wait: %f\n", libc_max_wait);
}

int main(void) {
 printf("start\n");
 test_speedcases_snoolie();
 printf("end\n");
 return 0;
}
