#include <stdlib.h>

#define ALPHA 0.875
#define RHO 0.25

static double est_rtt = 2.0;
static double est_var = 0.0;
static double rto = 1.0;

double calculate_rto(double sample_rtt);