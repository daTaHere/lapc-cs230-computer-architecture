/*
    GIVEN PROCESSOR AND CACHE SPECIFICATIONS

    Main memory size: 32,768 bytes
    - This tells us how much simulated main memory to create.
    - Each integer is 4 bytes, so main memory holds:
          32,768 / 4 = 8,192 integers
    - Valid integer addresses are 0, 4, 8, ... 32,764.

    Cache size: 2,048 bytes
    Cache-line size: 16 bytes
    - These values determine the total number of cache lines:
          2,048 / 16 = 128 cache lines

    Cache way size: 2
    - This means the cache is two-way set associative.
    - Each set has two possible cache lines, called ways.
    - The number of cache sets is:
          128 cache lines / 2 ways = 64 sets

    Integer size: 4 bytes
    Cache-line size: 16 bytes
    - These determine how many integers fit in one cache line:
          16 / 4 = 4 integers per cache line

    FINAL CACHE LAYOUT

        64 sets
        2 ways in each set
        4 integers in each cache line

        cache[64][2]
        Each cache entry holds 4 integers plus:
        - tag
        - valid bit
        - dirty bit

    ADDRESS BREAKDOWN

    Total memory is 32,768 bytes:
        32,768 = 2^15, so each address uses 15 bits.

    Cache line is 16 bytes:
        16 = 2^4, so the offset uses 4 bits.
        The offset selects a location inside the cache line.

    There are 64 cache sets:
        64 = 2^6, so the index uses 6 bits.
        The index selects one of the 64 cache sets.

    The remaining bits are the tag:
        15 total bits - 6 index bits - 4 offset bits = 5 tag bits.
        The tag identifies which main-memory block is stored in the set.

    Address format:
        [ 5-bit tag | 6-bit index | 4-bit offset ]


    INPUT COMMAND SCHEMA

    A <address> W <data>
    - Simulate the processor writing an integer to an address.
    - Example: A 8 W 1234

    A <address> R
    - Simulate the processor reading an integer from an address.
    - Example: A 8 R

    B <address>
    - Display the main-memory and cache values for an address.
    - Example: B 8

    C <address>
    - Display both cache ways in the set used by an address.
    - Example: C 1024

    M
    - Display main-memory lines that contain nonzero data.

    X
    - End the program.

    Addresses and data are entered as decimal integers.
*/

#include <iostream>

using namespace std;
const int MEMORY_SIZE = 32768;  // Bytes
const int CACHE_SIZE = 2048;    // Bytes
const int LINE_SIZE = 16;       // Bytes per cache line
const int NUMBER_OF_WAYS = 2;
const int INTEGER_SIZE = 4;     // Bytes per integer

// Values calculated from the given specifications
const int INTEGERS_IN_MEMORY = MEMORY_SIZE / INTEGER_SIZE;
const int INTEGERS_PER_LINE = LINE_SIZE / INTEGER_SIZE;
const int TOTAL_CACHE_LINES = CACHE_SIZE / LINE_SIZE;
const int NUMBER_OF_SETS = TOTAL_CACHE_LINES / NUMBER_OF_WAYS;


// ================  ProtoType  ====================

int validateAddress(int address);

int main() {



    //cout << "Integers in memory: " << INTEGERS_IN_MEMORY << '\n';
    //cout << "Integers per cache line: " << INTEGERS_PER_LINE << '\n';
    //cout << "Total cache lines: " << TOTAL_CACHE_LINES << '\n';
    //cout << "Number of cache sets: " << NUMBER_OF_SETS << '\n';

    char command = ' ';

    cout << "CS230 Lab Assignment 3 - Huynh, Adam\n";
    /*
     test schema  ========
     vaild
     { A 8 W 1234, A 16 R }
     ( B 8,   B 16 }

     invalid
     { A 10 W 1234,  A 40000 R }
     { B 10.   B 40000 }

    */
    

    while (command != 'X') {
        cout << "Enter command: ";
        // extract first char
        cin >> command; // 

        switch (command) {
        case 'A': {  // processor read or write data
            int address;
            char type;
            int data;

            cin >> address >> type;

            if (type == 'W') {
                cin >> data;
                cout << "A " << address << " " << type << " " << data << endl;
            }
            else {
                cout << "A "<< address << " " << type << endl;
            }
            // handle invalid address 
            address = validateAddress(address);

            break;
        }

        case 'B': { // Display the cache and memory values for an address
            // retrive addy
            int address;

            cin >> address;

            cout << "B " << address << endl;

            // handle invalid address 
            address = validateAddress(address);

            break;
        }
        case 'C': // Display the cache lines associated for an address
            // Process C
            break;

        case 'M': // Display main-memory lines if non-zero
            // Process M
            break;

        case 'X':
            // Stop the loop
            cout << "Done." << endl;
            break;
        }
    }

    return 0;
}

int validateAddress(int address) {

    if (address < 0 || address > MEMORY_SIZE - INTEGER_SIZE || address % INTEGER_SIZE != 0) {
        cout << "Input address " << address << " invalid" << endl;
        return 0;
    }

    return address;
}
