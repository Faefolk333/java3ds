#include "../native/motion.h"
#include <cassert>
#include <cstdio>
int main(){assert(motionAcceleration(512)==9806);assert(motionAcceleration(-512)==-9806);assert(motionRotation(14375,14.375f)==1000000);assert(motionRotation(-14375,14.375f)==-1000000);assert(motionRotation(100,0)==0);puts("MOTION_UNITS_PASS one_g rotation_counts_per_degree zero_coefficient");}
