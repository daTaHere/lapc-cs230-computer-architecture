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
#include <fstream>
#include <iostream>
using namespace std;

struct CacheLineInfo {
    int tag;
    bool valid;
    bool dirty;
};

const int CACHE_SIZE = 2048;    // Bytes
const int INTEGER_SIZE = 4;     // Bytes per integer
const int MEMORY_SIZE = 32768;  // Bytes
const int INTEGERS_IN_MEMORY = MEMORY_SIZE / INTEGER_SIZE;
const int LINE_SIZE = 16;       // Bytes per cache line
const int INTEGERS_PER_LINE = LINE_SIZE / INTEGER_SIZE;
const int NUMBER_OF_WAYS = 2;
const int TOTAL_CACHE_LINES = CACHE_SIZE / LINE_SIZE;
const int NUMBER_OF_SETS = TOTAL_CACHE_LINES / NUMBER_OF_WAYS;

// ================ Storage ====================

// Main memory: 8,192 integers initialized to zero
int memory[INTEGERS_IN_MEMORY] = { 0 };
// Cache: [64 sets] [2 ways] [4 integers per line]
int cache[NUMBER_OF_SETS][NUMBER_OF_WAYS][INTEGERS_PER_LINE] = { 0 };
CacheLineInfo cacheLineInfo[NUMBER_OF_SETS][NUMBER_OF_WAYS] = {};
int evictionWay = 1;


// ================  ProtoType  ====================
int calculateBits(int value);
void displayAddress(int address);
void displayCacheLines(int address);
void displayMainMemory();
int findCacheWay(int set, int tag);
int findEmptyWay(int set);
void loadCacheLine(int address, int set, int way, int tag);
void processRequest(int address, char type, int data);
void splitAddress(int address, int& tag, int& index, int& offset);
int validateAddress(int address);
void writeBackCacheLine(int set, int way);

int main() {

    char command = ' ';

    cout << "CS230 Lab Assignment 3 - Huynh, Adam\n";
    
    while (command != 'X') {
        cout << "Enter command: ";
        cin >> command; 

        switch (command) {
        case 'A': {  // processor read or write data
            int address, data = 0;
            char type ;
            cin >> address >> type;

            if (type == 'W') {
                cin >> data;
                cout << "A " << address << " " << type << " " << data << endl;
            }
            else {
                cout << "A "<< address << " " << type << endl;
            }
         
            address = validateAddress(address);
            processRequest(address, type, data);
            break;
        }

        case 'B': { // Display the cache and memory values for an address
            int address;
            cin >> address;
            cout << "B " << address << endl;

            address = validateAddress(address);
            displayAddress(address);
            break;
        }
        case 'C': { // Display the cache lines associated for an address
            int address;
            cin >> address;
            cout << "C " << address << endl;
            displayCacheLines(address);
            break;
        }

        case 'M': { // Display main-memory lines if non-zero
            cout << "M" << endl;
            displayMainMemory();
            break;
        }
        case 'X': // Exit program
            cout << "Done." << endl;
            break;
        }
    }

    return 0;
}


// ==============    Function Def  ======================

// Compute bit sizes 
int calculateBits(int value) {
    int bits = 0;
    int powerOfTwo = 1;

    while (powerOfTwo < value) {
        powerOfTwo *= 2;
        bits++;
    }

    return bits;
}

// Search both CacheLine for data and output if hit
void displayAddress(int address) {
    // Output ===> Address: <address> memory:<value> cache: <way 0 value> <way 1 value>
    int tag, index, offset;

    splitAddress(address, tag, index, offset);

    int integerPosition = offset / INTEGER_SIZE;
    int memoryValue = memory[address / INTEGER_SIZE];
    int cacheValues[NUMBER_OF_WAYS];

    for (int way = 0; way < NUMBER_OF_WAYS; way++) {
        cacheValues[way] = -1;

        // Check Valid bit and tag
        if (cacheLineInfo[index][way].valid && cacheLineInfo[index][way].tag == tag) {
            cacheValues[way] = cache[index][way][integerPosition];
        }
    }

    cout << "Address: " << address << " memory:" << memoryValue << " cache:";

    for (int way = 0; way < NUMBER_OF_WAYS; way++) {
        cout << " " << cacheValues[way];
    }

    cout << endl;
}

// Output ===> Cache line <idx> set <way> tag <tag> <four integers> V:<valid> D:<dirty>
void displayCacheLines(int address) {
    int tag, index, offset;

    splitAddress(address, tag, index, offset);

    for (int way = 0; way < NUMBER_OF_WAYS; way++) {
        cout << "Cache line " << index
            << " set " << way
            << " tag " << cacheLineInfo[index][way].tag;

        for (int position = 0; position < INTEGERS_PER_LINE; position++) {
            cout << " " << cache[index][way][position];
        }

        cout << " V:" << cacheLineInfo[index][way].valid
            << " D:" << cacheLineInfo[index][way].dirty
            << endl;
    }
}

// Display main-memory content
void displayMainMemory(){
    for (int memoryIndex = 0;
        memoryIndex < INTEGERS_IN_MEMORY;
        memoryIndex += INTEGERS_PER_LINE)
    {
        bool hasNonzeroValue = false;

        // Check whether this memory line contains any nonzero value
        for (int position = 0; position < INTEGERS_PER_LINE; position++) {
            if (memory[memoryIndex + position] != 0) {
                hasNonzeroValue = true;
            }
        }

        // Display only nonzero memory lines
        if (hasNonzeroValue) {
            cout << "Memory:";

            for (int position = 0; position < INTEGERS_PER_LINE; position++) {
                cout << " " << memory[memoryIndex + position];
            }
            cout << endl;
        }
    }
}

//  Locate empty way for update
int findCacheWay(int set, int tag) {
    for (int way = 0; way < NUMBER_OF_WAYS; way++) {
        if (cacheLineInfo[set][way].valid && cacheLineInfo[set][way].tag == tag) {
            return way;
        }
    }
    return -1;
}

// Find free way for update
int findEmptyWay(int set) {
    for (int way = 0; way < NUMBER_OF_WAYS; way++) {
        if (!cacheLineInfo[set][way].valid) {
            return way;
        }
    }
    return -1;
}

// Load cache from memory
void loadCacheLine(int address, int set, int way, int tag) {
    // Find the first address of the memory
    int blockStartAddress = (address / LINE_SIZE) * LINE_SIZE;

    // // Set index
    int memoryIndex = blockStartAddress / INTEGER_SIZE;

    // Copy data into cache
    for (int position = 0; position < INTEGERS_PER_LINE; position++) {
        cache[set][way][position] = memory[memoryIndex + position];
    }

    // Update cache line info
    cacheLineInfo[set][way].tag = tag;
    cacheLineInfo[set][way].valid = true;
    cacheLineInfo[set][way].dirty = false;
}

// cache logic handle read or write request to and from cache.
void processRequest(int address, char type, int data) {
    int tag, index, offset;

    splitAddress(address, tag, index, offset);
    int way = findCacheWay(index, tag);

    //==========  Missed =============
    if (way == -1) {
        // First look for an empty way
        way = findEmptyWay(index);

        // CacheLine is full
        if (way == -1) {
            way = evictionWay;
            // Output ===> Input address <address> invalid
            cout << "Evicting cache[" << index
                << "," << way << "]" << endl;

            // Write to memory
            if (cacheLineInfo[index][way].dirty) {
                writeBackCacheLine(index, way);
            }

            // Determin next way to evicted
            evictionWay = (evictionWay + 1) % NUMBER_OF_WAYS;
        }

        // Load from main memory into cache line
        loadCacheLine(address, index, way, tag);
    }
    int integerPosition = offset / INTEGER_SIZE;

    //============ Hit ==============
    if (type == 'W') {
        // Update cache and mark the line dirty
        cache[index][way][integerPosition] = data;
        cacheLineInfo[index][way].dirty = true;
    }
}

// Split the address into its cache fields
void splitAddress(int address, int& tag, int& index, int& offset) {
    int offsetBits = calculateBits(LINE_SIZE);
    int indexBits = calculateBits(NUMBER_OF_SETS);

    int offsetMask = (1 << offsetBits) - 1;
    int indexMask = (1 << indexBits) - 1;

    offset = address & offsetMask;
    index = (address >> offsetBits) & indexMask;
    tag = address >> (offsetBits + indexBits);
}

// Check address location size is within expected offset
int validateAddress(int address) {

    if (address < 0 || address > MEMORY_SIZE - INTEGER_SIZE || address % INTEGER_SIZE != 0) {
        cout << "Input address " << address << " invalid" << endl;
        return 0;
    }

    return address;
}

// Wirte data into memory before replacment
void writeBackCacheLine(int set, int way) {
    int tag = cacheLineInfo[set][way].tag;

    // Set main-memory address
    int memoryBlock = (tag * NUMBER_OF_SETS) + set;

    // Set index
    int memoryIndex = memoryBlock * INTEGERS_PER_LINE;

    // Write data from cache to main memory
    for (int position = 0; position < INTEGERS_PER_LINE; position++) {
        memory[memoryIndex + position] = cache[set][way][position];
    }

    cacheLineInfo[set][way].dirty = false;
}
