/*
	Multithreaded Prime Number Calculation

	This program demonstrates multithreaded processing using the Windows API.
	It detects the number of available logical processors, divides a user-defined
	range among multiple threads, and assigns each thread a portion of the range
	to search for prime numbers.

	Each thread calculates its own prime count and highest prime number. The
	results are then combined and displayed after all threads have completed.

	Concepts demonstrated:
	- Creating and managing multiple threads
	- Dividing work among available processors
	- Passing data to thread functions
	- Waiting for threads to complete
	- Sharing data between threads
	- Using critical sections for synchronization
*/

/* Includes */
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <iostream>

/* prototype for thread routine */
DWORD WINAPI primeCalcFunction(LPVOID lpParam);

//void* primeCalcFunction(void *ptr);

// this structure contains parameters for primeCalcFunction
struct primeParm {
	int threadNumber;
	int beginNumber;
	int limitNumber;
};

int primeCount, lastPrime;
CRITICAL_SECTION myCritSec;

int main() {
	primeParm pDataArray[64];
	DWORD   dwThreadIdArray[64];
	HANDLE  hThreadArray[64];
	//clock_t aTime;
	time_t startTime, endTime;
	//tms time1, time2;
	int primeLimit = 0;
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	long unsigned processorCount = siSysInfo.dwNumberOfProcessors;
	primeParm tP[65]; // entry 0 not a real thread

	std::cout << "number of processors:" << processorCount << std::endl;
	processorCount = processorCount > 64 ? 64 : processorCount;

	do {
		std::cout << "Enter prime number limit:";
		std::cin >> primeLimit;
		std::cout << std::endl;
	} while (primeLimit < 1);

	//aTime = times(&time1);
	//time(&startTime);

	int threadRange = primeLimit / processorCount;
	tP[0].limitNumber = 1; //use entry 0 only for initialization

	for (int i = 1; i <= processorCount; i++) {
		tP[i].threadNumber = i;
		tP[i].beginNumber = tP[i - 1].limitNumber + 1;
		tP[i].limitNumber = tP[i].beginNumber + threadRange;
	}
	tP[processorCount].limitNumber = primeLimit;
	primeCount = 0;
	lastPrime = 2;

	// Initialize critical section
	InitializeCriticalSection((LPCRITICAL_SECTION)&myCritSec);

	/* create threads */
	for (int i = 1; i <= processorCount; i++) {
		hThreadArray[i] = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			primeCalcFunction,       // thread function name
			(LPVOID)&tP[i],          // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadIdArray[i]);   // returns the thread identifier 
	}

	/* Main block now waits for all threads to terminate */
	//Sleep(10000);
	for (int i = 1; i <= processorCount; i++) {
		int j = WaitForSingleObject(hThreadArray[i], INFINITE);
		if (j) std::cout << "WaitForSingleObject " << i << " failed, code:" << j << '\n';
	}
	//WaitForMultipleObjects(processorCount, &hThreadArray[1], TRUE, INFINITE);

	// show results
	//aTime = times(&time2);
	//time(&endTime);
	std::cout << "... threads ended, " << primeCount << " primes, highest was " << lastPrime << std::endl;

	//double userCPUTime = time2.tms_utime - time1.tms_utime;
	// CLOCKS_PER_SEC values:
	// Windows = 1000
	// Mac OS = 100
	// POSIX = 10000000
	//userCPUTime /= 100.0; // Change to 1000 for Windows
	//std::cout << "User mode CPU Time:" << userCPUTime << '\n';
	//double systemCPUTime = time2.tms_stime - time1.tms_stime;
	//systemCPUTime /= 100.0; // Change to 1000 for Windows
	//std::cout << "System mode CPU Time:" << systemCPUTime << '\n';
	//double elapsedDiff = endTime - startTime;

	//std::cout << " elapsed time difference:" << elapsedDiff << " seconds" << std::endl;
	// Destroy critical section
	DeleteCriticalSection((LPCRITICAL_SECTION)&myCritSec);

	// Finally all done.
	exit(0);
}

/* function to calculate prime numbers */
DWORD WINAPI primeCalcFunction(LPVOID lpParam)
{
	primeParm* data;
	int myCount, myLast, i, j;
	bool isPrime;
	data = (primeParm*)lpParam;  /* type cast pointer to prime parms*/

	// serialize starting message.
	EnterCriticalSection((LPCRITICAL_SECTION)&myCritSec);
	std::cout << "Thread " << data->threadNumber << " starting for " << data->beginNumber << " to " << data->limitNumber << std::endl;
	LeaveCriticalSection((LPCRITICAL_SECTION)&myCritSec);

	/* do the work */
	myCount = 0; myLast = 2;
	for (i = data->beginNumber; i < data->limitNumber; i++) {
		isPrime = true;
		for (j = 2; j < i; j++) {
			if (i % j == 0)
				isPrime = false;
		}
		if (isPrime) {
			myCount++;
			myLast = i;
		}
	}
	primeCount += myCount;
	if (lastPrime < myLast)
		lastPrime = myLast;
	std::cout << "Thread " << data->threadNumber << " ending, " << myCount << " primes, highest = " << myLast << std::endl;
	return 0; // pthread_exit(0); /* exit */
}
