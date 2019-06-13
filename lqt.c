/* T e s t */

#include "contiki.h"
#include "lqt.h"
#include <stdio.h>


/* Function for element swaps in array */
void swapElements(int *xp, int *yp)
{
	int temp = *xp; 
	*xp = *yp; 
	*yp = temp; 
}

/* Function for sorting array */
void sortArray(int arr[], int n)
{
	int i, j, temp; 
	for (i=0;i< n-1; i++) {
		for(j=0; j < n-i-1; j++) {	
			if (arr[j] > arr[j+1]) swapElements(&arr[j], &arr[j+1]);
		}
	}	
}

/* Vector mult for row x col */
int32_t row_col_mult(int16_t a[], int16_t b[], uint8_t n) 
{
	uint8_t i; 
	int32_t x = 0;
	int32_t tempA, tempB;
	
	for (i=0; i<n; i++)
	{
		tempA = a[i]; 
		tempB = b[i];
		x += tempA*tempB;
		
		//printf("X: %ld, tempA: %ld, tempB: %ld \n", x, tempA, tempB); 
	}	
	return x;
}


/* LQ track with integers */
uint16_t get_send_rate(uint16_t bat_level, int16_t param_vector[], int16_t feature_vector[], int16_t init_vector[])
{
	// Vars and params for calculations 
	int32_t fac1, fac2, temp = 0;
	uint8_t i;
	int8_t flag = 1; 
	int8_t alpha_inv = 10; // equal to alpha = 0.1. Using division instead of multiplying with floats since floats are not allowed.
	
	int16_t p = 1000; //scaling factor 
	uint32_t step_scaled = 1e7; // 0.001*p^3*10. 

	//Bat vars 
	int32_t B_current;
	int32_t B_tgt = 650; //650 equals 65% 

	//DC vars
	int32_t dc = 300;
	int32_t dc_smooth = 300;
	int32_t dc_real = 0;

	

	//Bat level b/w [0,1000], assume 3400=0%, 3600=100%
	B_current = (bat_level-3400)/2; 
	B_current *= 10; 
	if (B_current >1000) B_current = 1000; 
	else if (B_current <0) B_current = 0; 


	//Compute fac1, fac2 to add to new param vector
	 /* Compute the parameter vector.
	  *
	  *  		10*step*p^3
	  *		   ------------- = fac1
	  *			feat*feat'
	  * 
	  * Using step = 0.001, p = 1000, denominator = 1e7
	  * The denominator is multiplied with p^3 in order to fit fac1 b/w [0,100]
	  *
	  *				feat*param'
	  *  B_cur -   ------------ = fac2
	  *				     p 
	  *
	  *	 The vector multiplication is divided with p to keep the scaling "correct"
	  *
	  *  Finally, the temporary term is fac1*fac2 divided with 10 to keep the result
	  *   within reasonable values. 
	  *  
	  */

	fac1= step_scaled;
	fac2 = (row_col_mult(feature_vector, feature_vector, 3));
	fac1 = fac1/fac2; 	

	fac2= B_current - (row_col_mult(feature_vector, param_vector, 3)/p);
	temp = (fac1*fac2)/10; 
	
	/*DEBUG PRINTS 
	*	printf("fac1: %ld, fac2: %ld, temp: %ld \n", fac1, fac2, temp); 
	*	printf("Feat_Vec:  ");
	*	for (i=0; i<3; i++)
	*	{ 
	*	   printf("%d   ", feature_vector[i]);
	*	}
	*	printf("\n"); 
	*	*/

	/* Update the parameter vector. If the new parameter element is out of bounds,
	*  reset it to the initial value. 
	*
	*  @param flag is used to change the sign for the elements according to the algorithm
	*/

	for (i=0; i<3; i++)
	{
		param_vector[i] = param_vector[i] + (feature_vector[i]*temp)/(1	0*p);
		if (param_vector[i]*flag <= 0) param_vector[i] = init_vector[i]; 
		flag *= -1; 
	}
	

 	// Compute the duty cycle. If out of bounds, set to limit. 
	dc = (B_tgt*p - param_vector[0]*B_current + param_vector[2]*B_tgt) / param_vector[1]; 
	if (dc<0) dc = 0;
	else if (dc >1000) dc = 1000; 

	//Update feature vector 
	feature_vector[0] = B_current; 
	feature_vector[1] = dc; 
	feature_vector[2] = -B_tgt; //Static lmAu


    /* Smoothing of the change of the duty cycle using past values
    *
	* dc_smooth = dc_smooth + alpha*(dc-dc_smooth) --> divide with alpha_inv. 
	*
	*/
	dc_smooth = dc_smooth + (dc-dc_smooth)/alpha_inv;

	// Control the variance of the duty cycle by using 50% of dc and 50% of dc_smooth (beta=0.5)
	dc_real = dc/2 + dc_smooth/2;
	dc_real = dc_real/10; 
	
	//printf("DC: %d, DC_SMOOTH: %d, param[0]: %d \n", dc, dc_smooth, param_vector[0]);
	if (dc_real < 1 && dc_real => 0) return = 4; //32 packets/sec as max 
	if (dc_real <0 || dc_real >100)
	{
		printf("ERROR invalid send interval, setting to 100 seconds. DC=%d \n", dc_real);
		return 12800; //12800 ticks --> 1 packet every 100 sec
	}
	return (100-dc_real)*8; //8 --> from 16 packets/sec to 1 packet every 6.25 sec  
}




	