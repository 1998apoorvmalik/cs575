#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <omp.h>

// Simulation Parameters
const float RYEGRASS_GROWS_PER_MONTH = 30.0;
const float ONE_RABBITS_EATS_PER_MONTH = 1.0;
const float ONE_WOLVES_EATS_PER_MONTH = 1.0;

const float AVG_PRECIP_PER_MONTH = 12.0; // average
const float AMP_PRECIP_PER_MONTH = 4.0;	 // plus or minus
const float RANDOM_PRECIP = 2.0;		 // plus or minus noise

const float AVG_TEMP = 60.0;	// average
const float AMP_TEMP = 20.0;	// plus or minus
const float RANDOM_TEMP = 10.0; // plus or minus noise

const float MIDTEMP = 60.0;
const float MIDPRECIP = 14.0;

unsigned int seed = 32;

// Function Prototypes
void CalculateTemperatureAndPercipitation();
void Rabbits();
void RyeGrass();
void Watcher();
void MyAgent();

// Utility Functions
float Sqr(float x)
{
	return x * x;
}

float Ranf(unsigned int *seedp, float low, float high)
{
	float r = (float)rand_r(seedp); // 0 - RAND_MAX
	return (low + r * (high - low) / (float)RAND_MAX);
}

float ToCelcius(float farenheit)
{
	return (5. / 9.) * (farenheit - 32.);
}

// State Variables
int NowMonth = 0;	// 0 - 11
int NowYear = 2023; // 2023 - 2028

float NowPrecip;	   // inches of rain per month
float NowTemp;		   // temperature this month
int NowNumRabbits = 10; // number of rabbits in the current population
int NowNumWolves = 1;  // number of wolves in the current population
float NowHeight = 5.;  // rye grass height in inches

int main(int argc, char *argv[])
{
#ifdef _OPENMP
	// fprintf( stderr, "OpenMP is supported -- version = %d\n", _OPENMP );
#else
	fprintf(stderr, "No OpenMP support!\n");
	return 1;
#endif

	CalculateTemperatureAndPercipitation();

	omp_set_num_threads(4); // same as # of sections
#pragma omp parallel sections
	{
#pragma omp section
		{
			Rabbits();
		}

#pragma omp section
		{
			RyeGrass();
		}

#pragma omp section
		{
			Watcher();
		}

#pragma omp section
		{
			MyAgent();
		}
	} // implied barrier -- all functions must return in order
	  // to allow any of them to get past here
}

void CalculateTemperatureAndPercipitation()
{
	float ang = (30. * (float)NowMonth + 15.) * (M_PI / 180.);

	float temp = AVG_TEMP - AMP_TEMP * cos(ang);
	NowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

	float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
	NowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
	if (NowPrecip < 0.)
		NowPrecip = 0.;
}

void Rabbits()
{
	while (NowYear < 2029)
	{
		int nextNumRabbits = NowNumRabbits;
		int carryingCapacity = int(NowHeight);
		if (nextNumRabbits < carryingCapacity)
			nextNumRabbits++;
		else if (nextNumRabbits > carryingCapacity)
			nextNumRabbits--;

		// Each wolf contributes to 40% chance that the rabbit population will decrease by 1
		if (Ranf(&seed, 0., 1.) < (float)NowNumWolves * 0.4)
			nextNumRabbits--;

		if (nextNumRabbits < 0)
			nextNumRabbits = 0;

// DoneComputing barrier
#pragma omp barrier
		NowNumRabbits = nextNumRabbits;
// DoneAssigning barrier
#pragma omp barrier
// DonePrinting barrier
#pragma omp barrier
	}
}

void RyeGrass()
{
	while (NowYear < 2029)
	{
		float tempFactor = exp(-Sqr((NowTemp - MIDTEMP) / 10.));
		float precipFactor = exp(-Sqr((NowPrecip - MIDPRECIP) / 10.));
		float nextHeight = NowHeight;
		nextHeight += tempFactor * precipFactor * RYEGRASS_GROWS_PER_MONTH;
		nextHeight -= (float)NowNumRabbits * ONE_RABBITS_EATS_PER_MONTH;

		if (nextHeight < 0.)
			nextHeight = 0.;
// DoneComputing barrier
#pragma omp barrier
		NowHeight = nextHeight;
// DoneAssigning barrier
#pragma omp barrier
// DonePrinting barrier
#pragma omp barrier
	}
}

void MyAgent()
{
	while (NowYear < 2029)
	{
		// Each rabbit contributes to 5% chance that the wolf population will increase by 1
		int nextNumWolves = NowNumWolves;
		if (Ranf(&seed, 0., 1.) < (float)NowNumRabbits * 0.05)
			nextNumWolves++;
		else
			nextNumWolves--;

		if(nextNumWolves < 0)
			nextNumWolves = 0;
			
// DoneComputing barrier
#pragma omp barrier
		NowNumWolves = nextNumWolves;
// DoneAssigning barrier
#pragma omp barrier
// DonePrinting barrier
#pragma omp barrier
	}
}

void Watcher()
{
	while (NowYear < 2029)
	{
// Done Computing barrier
#pragma omp barrier
// Done Assigning barrier
#pragma omp barrier
		printf("%d,%d,%f,%f,%d,%d,%f\n", NowMonth, NowYear, NowPrecip, ToCelcius(NowTemp), NowNumRabbits, NowNumWolves, NowHeight);
		NowMonth++;
		if (NowMonth % 12 == 0)
		{
			NowYear++;
		}
		CalculateTemperatureAndPercipitation();
// Done Printing barrier
#pragma omp barrier
	}
}