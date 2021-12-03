#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

double Sum = 0;
double Step = 0;
unsigned long long CountIterations = 0;

void* calculate_part (void* data) {

	double value = *(double*)data;
	double subSum = 0;

	for (unsigned long long i = 0; i < CountIterations; i++) {

		subSum += (value / sin(value)) * Step;
		value += Step;
	
    }
	
	Sum += subSum;

    return data;
    
}

int main (int argc, char ** argv) {

	if (argc != 3) {

		printf("Wrong number of arguments\n");
		return 0;
	
    }

	unsigned long long N = atoll(argv[1]);
	unsigned long long K = atoll(argv[2]);

	double down_border = M_PI / 4;
	double high_border = M_PI / 2;
	double between = high_border - down_border;
	Step = between / K;

	pthread_t * tid = calloc (N, sizeof(pthread_t));

	CountIterations = K / N;
	unsigned long long number_step = 0;

	double * data = calloc (N, sizeof(double));

	double position = down_border;

	int trash;
	unsigned long long number_of_thread = 0;

	for (unsigned long long j = 0; j < N; j++) {

		if (number_step == K)
			break;
		
		data[j] = position;
		position += between / N;
		int ret_val = pthread_create(tid + j, NULL, calculate_part, data + j);
		
		if (ret_val < 0)
			perror("OH NO!!!");
		
		number_of_thread++;
		number_step++;

	}
	
	for (unsigned long long j = 0; j < number_of_thread; j++)
		pthread_join(tid[j], (void**)&trash);
	
	number_of_thread = 0;
	
	printf("res = %f\n", Sum);	

	free (tid);
    free (data);

	return 0;

}