// This program accepts a binary16 value in character hexadecimal, converts that into a binary 16-bit value 
// (C++ unsigned short int), then converts the short int into a C++ float and displays the result.
#include <iostream>
#include <string>
#include <bit>
#include <cstdint>
#include <cctype>
using namespace std;

//============== Function Prototype ============== 
bool isValidChoice();
void handleBits( const unsigned short int& value, const int numberOfBits);
void handle32BitFloat(const unsigned short int& sign, const unsigned short int& exponent,const unsigned short int& significand);
void displayBitFields(const unsigned short int& sign, const unsigned short int& exponent, const unsigned short int& significand);
void handleHexInput(const string& userInput, unsigned short int& binValue, bool& isValid);
void handleBinary( const unsigned short int& sign, const unsigned short int& exponent, const unsigned short int& significand);

int main() {
    string userInput;
    cout << "CS230 Fall 2026 Lab 1 - Huynh, Adam\n";
    bool choice = NULL;

    do{
        unsigned short int binValue = 0;
        bool isValid = true;

        cout << "Enter four hexadecimal digits:";
        getline(cin >> ws, userInput);

        if (userInput.length() != 4) {
            cout << "Input characters must be in the ranges of 0-9, a-z, or A-Z\n";
        }
        else {
            handleHexInput(userInput, binValue, isValid);
            
            if (isValid) {       // process valid user inputs
                unsigned short int sign;
                unsigned short int exponent;
                unsigned short int significand;

                sign = binValue >> 15;
                exponent = (binValue >> 10) & 0x1F;
                significand = binValue & 0x3FF;

                handleBinary(
                    sign,
                    exponent,
                    significand
                );
            }
        }
        choice = isValidChoice();
    } while (choice);

    cout << "Done, program ending.\n";

    return 0;
}

//============== Function Declarations ==============
bool isValidChoice() {
    string input;
    // validate Y/N input non-case sensitive
    while (true) {
        cout << "Continue?  enter Y or N:";
        getline(cin >> ws, input);

        if (input.length() == 1) {
            char validInput = static_cast<char>(
                toupper(static_cast<unsigned char>(input[0]))
                );

            if (validInput == 'Y' || validInput == 'N') {
                return (validInput == 'Y') ? true : false;
            }
        }
    }
}

void handleBits(const unsigned short int& value, const int numberOfBits) {
    // // process bits
    for (int bit = numberOfBits - 1; bit >= 0; --bit) {
        cout << ((value >> bit) & 1);
    }
}

void handle32BitFloat(
    const unsigned short int& sign,
    const unsigned short int& exponent,
    const unsigned short int& significand) {
    // convert 16bit to 32bit float

    uint32_t floatBits = 0; // container for 32 bit float

    floatBits |= static_cast<uint32_t>(sign) << 31;  // shift the sign into bit 31 and OR bitwise op
    unsigned int floatExponent = exponent + 112; // // adjust bias
    floatBits |= floatExponent << 23; // shift exponent  23 bits  left and OR bitwise op 
    floatBits |= static_cast<uint32_t>(significand) << 13; // shift signficate 13 bits left and OR bitwise op

    float floatValue = bit_cast<float>(floatBits); // assign bit value to float

    cout << "Value:" << floatValue << '\n';
}

void displayBitFields(
    const unsigned short int& sign,
    const unsigned short int& exponent,
    const unsigned short int& significand) {

    cout << "Sign: ";
    handleBits(sign, 1);

    cout << " Exponent:";
    handleBits(exponent, 5);

    cout << " Significand:";
    handleBits(significand, 10);
    cout << '\n';
}

void handleHexInput(const string& userInput, unsigned short int& binValue, bool& isValid) {
    // validate users  hex input
    int validHex = 0;
    binValue = 0;
    
    for (int i = 0; i < 4; ++i) {
        char hexChar = userInput[i];

        if (hexChar >= '0' && hexChar <= '9')
            validHex = hexChar - '0';
        else if (hexChar >= 'a' && hexChar <= 'f')
            validHex = hexChar - 'a' + 10;
        else if (hexChar >= 'A' && hexChar <= 'F')
            validHex = hexChar - 'A' + 10;
        else {
            cout << "Input characters must be in the ranges of 0-9, a-z, or A-Z\n";
            isValid = false;
            return;
        }
        binValue = static_cast<unsigned short int>(binValue << 4);
        binValue = static_cast<unsigned short int>(binValue | validHex);
    }
}

void handleBinary(  
    const unsigned short int& sign, 
    const unsigned short int& exponent,
    const unsigned short int& significand) {
    // output bit field and ==> message or value
    displayBitFields(sign, exponent, significand);

    if (exponent == 0 && significand == 0) {
        cout << "Zero\n";
    }
    else if (exponent == 0) {
        cout << "Subnormal numbers are not allowed\n";
    }
    else if (exponent == 0x1F) {
        if (sign == 0)
            cout << "Positive infinity\n";
        else
            cout << "Negative infinity\n";
        if (significand != 0)
            cout << "...significand should be zero.\n";
    }
    else {
        handle32BitFloat(sign, exponent, significand);
    }
}
