#ifndef __LQT_H__
#define __LQT_H__

#define PARAM_MAX_SEND 1600; 
#define PARAM_MIN_SEND 16; 

//#define GOAL_BAT = 3000/3600; //3600 mV max capacity, 3000 mV goal level. 


/* Function for swapping two elements in an array */
void swapElements(int *xp, int *yp);


/* Function for sorting an array */
void sortArray(int arr[], int n);

/* Function for calculating new duty cycle using LQ tracking */
uint16_t get_send_rate(uint16_t bat_level, int16_t param_vector[], 
				int16_t feature_vector[], int16_t init_vector[])

/* Function for printing float values */
void printFloat(float val);

/* Floor function for float values */
float floor(float x);

/* Vector multiplication function, assuming it's a 
	row times column operation with the same amount of elements */
int32_t row_col_mult(int16_t a[], int16_t b[], uint8_t n)


#endif /* __LQT_H__ */
