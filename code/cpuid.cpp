#include <iostream>
#include <iomanip>
using namespace std;
int main(int argc, char* argv[])
{
	int b[4] = { 0 };
	
	unsigned int start = 0x80000002;
	int count = 5;

	for (int i = 0; i < count; i++) {
		unsigned int a = start + i;
		__cpuid(b, a);
	cout.write(reinterpret_cast<const char*>(b), sizeof(b)); 
	cout << endl;
	cout << "Code:" << hex << a << " gives " << setw(8)  <<
	setfill('0') << b[0] << ' ' << b[1] << ' ' << b[2] << ' ' << b[3] << endl;
}
return 0;
}
