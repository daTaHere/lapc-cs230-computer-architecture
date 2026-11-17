/*
    Two-Way Set-Associative Cache Simulation

    This program simulates a Level 1, two-way set-associative write-back
    cache. It uses 32-bit integers, byte-addressed main memory, 16-byte
    cache lines, and two cache ways per set.

    Memory commands are read from a test file and used to simulate processor
    reads and writes. Each address is divided into a tag, set index, and byte
    offset to determine where its data belongs in the cache.

    The program handles cache hits and misses, loads blocks from main memory,
    replaces cache lines when a set is full, and writes dirty cache data back
    into main memory during eviction.

    Concepts demonstrated:
    - Simulating main memory and cache storage
    - Reading commands from an input file
    - Validating memory addresses
    - Dividing addresses into tag, index, and offset fields
    - Using a two-way set-associative cache
    - Detecting cache hits and misses
    - Loading memory blocks into cache lines
    - Tracking tag, valid, and dirty information
    - Performing cache-line eviction
    - Writing dirty cache data back into main memory
    - Displaying cache and main-memory contents
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

// Define operation restristions
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
        cout << "Unable to open CacheInput.txt" << endl;
        return 1;
    }
    
    //while (command != 'X') {
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
    // Find begining of Memory Block
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

            // Update next Way to Evicte
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

    // Copy data from Cache in to Main-Memory
    for (int position = 0; position < INTEGERS_PER_LINE; position++) {
        memory[memoryIndex + position] = cache[set][way][position];
    }
    // Set Dirty flag to False
    cacheLineInfo[set][way].dirty = false;
}
