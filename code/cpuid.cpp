#include <iostream>
#include <iomanip>
#include <intrin.h>
#include <cctype>
#include <string>

/*
    Accepts a hexadecimal CPUID request code and a request count.
    Displays EAX, EBX, ECX, and EDX values in hexadecimal.
    Decodes vendor and processor-name responses as ASCII text.
*/

/*
ASCII table (hex → digits and letters)

Hex | Char || Hex | Char || Hex | Char || Hex | Char|| Hex | Char
----|------||-------|------||-----|-------||------|------||-----|-----
30  | 0      ||    41  | A      || 4E  | N       || 61   | a      || 6E  |   n
31  | 1      ||    42  | B      || 4F  | O       || 62   | b      || 6F  |   o
32  | 2      ||    43  | C      || 50  | P       || 63   | c       || 70  |   p
33  | 3      ||    44  | D      || 51  | Q      || 64   | d      || 71  |   q
34  | 4      ||    45  | E       || 52  | R      || 65   | e      || 72   |   r
35  | 5      ||    46  | F       || 53  | S      || 66   | f       || 73   |   s
36  | 6      ||    47  | G      || 54  | T       || 67  | g      || 74   |    t
37  | 7      ||    48  | H      || 55  | U      || 68   | h      || 75  |    u
38  | 8      ||    49  | I        || 56  | V      || 69   | i       || 76  |    v
39  | 9      ||    4A  | J       || 57  | W     || 6A   | j      || 77   |   w
     |          ||    4B  | K      || 58   | X      || 6B   | k     || 78   |   x
     |          ||    4C  | L      || 59   | Y      || 6C   | l      || 79   |   y
     |          ||   4D  | M     || 5A   | Z      || 6D  | m    || 7A   |   z
*/

using namespace std;

// Appends ASCII text from vendor and processor name responses
void displayMessage(const int b[], unsigned int code, string& message)
{
    int order[4] = { 0, 1, 2, 3 };
    int count = 4;

    if (code == 0 || code == 0x80000000) {
        order[0] = 1;  // EBX
        order[1] = 3;  // EDX
        order[2] = 2;  // ECX
        count = 3;
    }
    else if (code < 0x80000002 || code > 0x80000004) {
        return;  
    }

    for (int i = 0; i < count; ++i) {
        const char* characters =
            reinterpret_cast<const char*>(&b[order[i]]);

        for (int j = 0; j < sizeof(int); ++j) {
            if (characters[j] == '\0')
                return;

            message += characters[j];
        }
    }
}



int main()
{

    bool exit = false;
    char choice;

    while (!exit) {

        int b[4] = {};
        unsigned int start;
        int count;
        string message;

        cout << "Input CPUID code: ";
        cin >> hex >> start;

        cout << "Input request count: ";
        cin >> dec >> count;

        for (int i = 0; i < count; ++i) {
            unsigned int code = start + i;
            __cpuidex(b, static_cast<int>(code), 0);

            displayMessage(b, code, message);

            cout << "Code: " << hex << code << " gives ";

            for (unsigned int value : b) {
                cout << setfill('0')
                    << setw(8) << value << ' ';
            }

            cout << '\n';
        }

        cout << "\nMessage: " << message << '\n';

        message.clear();

        cout << "\nExit? (Y/N): ";
        cin >> choice;

        exit = (toupper(static_cast<unsigned char>(choice)) == 'Y');
    }
}
