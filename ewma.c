
#include "ewma.h"

double calculate_rto(double samp_rtt) {
	est_rtt = ALPHA * samp_rtt + (1 - ALPHA) * est_rtt;
	est_var = (1 - RHO) * est_var + RHO * abs((samp_rtt - est_var));
	rto = est_rtt + 4.0 * est_var; 
	return rto;
}

