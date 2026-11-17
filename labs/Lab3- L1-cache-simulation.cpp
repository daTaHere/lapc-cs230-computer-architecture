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
#include <cctype>
#include <fstream>
#include <iostream>
using namespace std;

// Cache Object 
struct CacheLineInfo {
    int tag;
    bool valid;
    bool dirty;
};

// Define operation restrictions
const int CACHE_SIZE = 2048;
const int INTEGER_SIZE = 4;
const int MEMORY_SIZE = 32768;
const int INTEGERS_IN_MEMORY = MEMORY_SIZE / INTEGER_SIZE;
const int LINE_SIZE = 16; 
const int INTEGERS_PER_LINE = LINE_SIZE / INTEGER_SIZE;
const int NUMBER_OF_WAYS = 2;
const int TOTAL_CACHE_LINES = CACHE_SIZE / LINE_SIZE;
const int NUMBER_OF_SETS = TOTAL_CACHE_LINES / NUMBER_OF_WAYS;

// ================ Storage Arrays  ====================

// Machine Secondary Memeory
int memory[INTEGERS_IN_MEMORY] = { 0 };
// Create Cache Object
int cache[NUMBER_OF_SETS][NUMBER_OF_WAYS][INTEGERS_PER_LINE] = { 0 };
// Array of cache sets [ way0, way1] 
CacheLineInfo cacheLineInfo[NUMBER_OF_SETS][NUMBER_OF_WAYS] = {};
// Track next Eviction
int evictionWay = 1;


// ================  ProtoType  ====================
void displayAddress(int address);
void displayCacheLines(int address);
void displayMainMemory();
int findCacheWay(int set, int tag);
int findEmptyWay(int set);
void loadCacheLine(int address, int set, int way, int tag);
void processRequest(int address, char type, int data);
void splitAddress(int address, int& tag, int& index, int& offset);
int splitAddressHelper(int value);
int validateAddress(int address);
void writeBackCacheLine(int set, int way);

int main() {

    char command = ' ';

    cout << "CS230 Lab Assignment 3 - Huynh, Adam\n";

    ifstream inputFile("testInput.txt");

    if (!inputFile.is_open()) {
        cout << "Unable to open testInput.txt" << endl;
        return 1;
    }
    
    while (inputFile >> command ) {
        command = static_cast<char>(toupper(command));

        cout << "Enter command: ";

        switch (command) {
        case 'A': {  // processor read or write data
            int address, data = 0;
            char type ;
            inputFile >> address >> type;
            type = static_cast<char>(toupper(type));

            if (type == 'W') {
                inputFile >> data;
                cout << "A " << address << " " << type << " " << data << endl;
            }
            else {
                cout << "A "<< address << " " << type << endl;
            }

            cout << endl;
         
            address = validateAddress(address);
            processRequest(address, type, data);
            break;
        }

        case 'B': {  // Display the cache and memory values for an address
            int address;
            inputFile >> address;
            cout << "B " << address << endl;

            address = validateAddress(address);
            displayAddress(address);
            cout << endl;
            break;
        }
        case 'C': {   // Display the cache lines for an address
            int address;
            inputFile >> address;
            cout << "C " << address << endl;
            displayCacheLines(address);
            cout << endl;
            break;
        }

        case 'M': { // Display main-memory lines if non-zero
            cout << "M" << endl;
            displayMainMemory();
            cout << endl;
            break;
        }
        case 'X': // Exit program
            cout << "X" << endl;
            cout << "Done." << endl;
            break;
        }
    }

    return 0;
}


// ==============    Function Def  ======================

// Display Main-Memory and Cache values for requested Address
void displayAddress(int address) {
    // Output ===> Address: <address> memory:<value> cache: <way 0 value> <way 1 value>
    int tag, index, offset;

    splitAddress(address, tag, index, offset);

    int integerPosition = offset / INTEGER_SIZE;
    int memoryValue = memory[address / INTEGER_SIZE];
    int cacheValues[NUMBER_OF_WAYS];

    for (int way = 0; way < NUMBER_OF_WAYS; way++) {
        cacheValues[way] = -1;

        // Check Valid bit and Tag
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

// Display both ways of an Address's Cache Set
void displayCacheLines(int address) {
    // Output ===> Cache line <idx> set <way> tag <tag> <four integers> V:<valid> D:<dirty>
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

// Display NONZERO data in Main-Memory
void displayMainMemory(){
    // Output ===> Memory: <four integers>
    for (int memoryIndex = 0;  memoryIndex < INTEGERS_IN_MEMORY; memoryIndex += INTEGERS_PER_LINE) {
        bool hasNonzeroValue = false;

        // Check for a NONZERO value
        for (int position = 0; position < INTEGERS_PER_LINE; position++) {
            if (memory[memoryIndex + position] != 0)
                hasNonzeroValue = true;
        }

        // Display Main-Memory data
        if (hasNonzeroValue) {
            cout << "Memory:";

            for (int position = 0; position < INTEGERS_PER_LINE; position++) {
                cout << " " << memory[memoryIndex + position];
            }
            cout << endl;
        }
    }
}

//  Check for data in Cache
int findCacheWay(int set, int tag) {
    for (int way = 0; way < NUMBER_OF_WAYS; way++) {
        if (cacheLineInfo[set][way].valid && cacheLineInfo[set][way].tag == tag)
            return way;
    }
    return -1;
}

// Find unused Way in Cache Set
int findEmptyWay(int set) {
    for (int way = 0; way < NUMBER_OF_WAYS; way++) {
        if (!cacheLineInfo[set][way].valid)
            return way;
    }
    return -1;
}

// Copy data to Cache from Main-Memory
void loadCacheLine(int address, int set, int way, int tag) {
    // Find beginning of Memory Block
    int blockStartAddress = (address / LINE_SIZE) * LINE_SIZE;

    //  Find Block location in Memory
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

// Handle Read & Write for requested Address
void processRequest(int address, char type, int data) {
    int tag, index, offset;

    splitAddress(address, tag, index, offset);
    int way = findCacheWay(index, tag);

    //========== Cache Miss =============
    if (way == -1) {
        // First look for an empty way
        way = findEmptyWay(index);

        // Cache Set is full
        if (way == -1) {
            way = evictionWay;
            // Output acknowledgment of eviction
            cout << "Evicting cache[" << index
                << "," << way << "]" << endl;

            // Write to memory if Dirty flag == True
            if (cacheLineInfo[index][way].dirty)
                writeBackCacheLine(index, way);

            // Update next Way to Evict
            evictionWay = (evictionWay + 1) % NUMBER_OF_WAYS;
        }

        // Load Cache from Main-Memory
        loadCacheLine(address, index, way, tag);
    }
    int integerPosition = offset / INTEGER_SIZE;

    //============ Process Write Request ==============
    if (type == 'W') {
        // Update Cache and set Dirty flag == True
        cache[index][way][integerPosition] = data;
        cacheLineInfo[index][way].dirty = true;
    }
}

// Split Address into Cache Object Fields ==> CacheLine( tag, index, offset ) 
void splitAddress(int address, int& tag, int& index, int& offset) {
    int offsetBits = splitAddressHelper(LINE_SIZE);
    int indexBits = splitAddressHelper(NUMBER_OF_SETS);

    int offsetMask = (1 << offsetBits) - 1;
    int indexMask = (1 << indexBits) - 1;

    offset = address & offsetMask;
    index = (address >> offsetBits) & indexMask;
    tag = address >> (offsetBits + indexBits);
}
// Calculate bits for an Address field
int splitAddressHelper(int value) {
    // Base 2 
    int bits = 0;
    int powerOfTwo = 1;

    while (powerOfTwo < value) {
        powerOfTwo *= 2;
        bits++;
    }

    return bits;
}

// Check requested Address is a Valid block
int validateAddress(int address) {
    // Make sure Address is valid and does not OVERFLOW
    if (address < 0 || address > MEMORY_SIZE - INTEGER_SIZE || address % INTEGER_SIZE != 0) {
        cout << "Input address " << address << " invalid" << endl;
        return 0;
    }

    return address;
}

// Wirte Cache data into Main-Memory
void writeBackCacheLine(int set, int way) {
    int tag = cacheLineInfo[set][way].tag;

    // Find Block in Main-Memory
    int memoryBlock = (tag * NUMBER_OF_SETS) + set;

    // Find location in Block
    int memoryIndex = memoryBlock * INTEGERS_PER_LINE;

    // Copy data from Cache into Main-Memory
    for (int position = 0; position < INTEGERS_PER_LINE; position++) {
        memory[memoryIndex + position] = cache[set][way][position];
    }
    // Set Dirty flag to False
    cacheLineInfo[set][way].dirty = false;
}
