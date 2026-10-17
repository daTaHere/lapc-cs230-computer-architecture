// CS230Week1.cpp : This file contains the 'main' function. 
// Program execution begins and ends there.


#include <iostream>
#include <cmath>
#include <ctime>

int primeCount, lastPrime;

int main()

{
	clock_t startTime, endTime;
	time_t time1, time2;
	int i = 0, j = 0, primeLimit = 0;
	bool isPrime = true;

	do {
		std::cout << "Enter prime number limit:";
		std::cin >> primeLimit;
		std::cout << std::endl;
	} while (primeLimit < 1);

	//aTime = times(&time1);
	startTime = clock();
	time1 = time(NULL);

	primeCount = 0;
	lastPrime = 2;

	/* do the work */
	// improved algo
	int myCount = 0, myLast = 2;
	// iterate through odd nums only
	for (i = 3; i < primeLimit; i+=2) {
		isPrime = true;
		// break if number is not Prime
		for (j = 2; j < i && isPrime; j++) {
			if (i % j == 0)
				isPrime = false;
		}
		if (isPrime) {
			primeCount++;
			lastPrime = i;
		}
	}

	/* do the work */
	// ---- brute force ----
	//for (i = 2; i < primeLimit; i++) {
	//	isPrime = true;
	//	for (j = 2; j < i; j++) {
	//		if (i % j == 0)
	//			isPrime = false;
	//	}
	//	if (isPrime) {
	//		primeCount++;
	//		lastPrime = i;
	//	}
	//}

	// show results
	endTime = clock(); // CPU time
	clock_t timeUsed = endTime - startTime;
	float fCPUtime = (timeUsed * 1.0) / CLOCKS_PER_SEC;
	// CLOCKS_PER_SEC values:
	// Windows = 1000
	// Mac OS = 100
	// POSIX = 10000000
	time2 = time(NULL); // elapsed time
	std::cout << "... ended, " << primeCount << " primes, highest was " << lastPrime << std::endl;

	std::cout << "Clock ticks:" << timeUsed << '\n';
	std::cout << "CPU Time : " << fCPUtime  << '\n';
	std::cout << "Elapsed time:" << difftime(time2, time1) << " seconds\n";

	// Finally all done.
	return 0;
}
