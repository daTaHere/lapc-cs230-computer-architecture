/*
    Bresenham Line Drawing with a Bitmap Image

    This program creates a 256 x 256, 8-bit bitmap image and draws a line
    between two user-defined coordinates using Bresenham's line-drawing
    algorithm.

    The line calculations are performed using 32-bit Intel inline assembly.
    The program determines the major direction of the line, calculates each
    pixel position using integer arithmetic, and writes the resulting line
    directly into the bitmap's pixel array.

    Concepts demonstrated:
    - Creating and writing an 8-bit bitmap file
    - Using a color palette
    - Validating coordinate input
    - Using 32-bit Intel inline assembly
    - Calculating pixel locations in a 2D array
    - Implementing Bresenham's line-drawing algorithm
    - Using integer error calculations to draw a line
*/

#include <iostream>
#include <fstream>
#include <string>
#include "windows.h"
using namespace std;
// The following defines the size of the square image in pixels.
#define IMAGE_SIZE 256
#define FILE_NAME "foo.bmp"

// Based on CS230Week5A.cpp
class BitMap {
public:
	// refer to the Wikipedia entry for BMP file format:
	// https://en.wikipedia.org/wiki/BMP_file_format
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	char colorTable[1024];
	char bits[IMAGE_SIZE][IMAGE_SIZE] = { 0 };

	BitMap() {
		int i, j;
		bmfh.bfType = 0x4d42;
		bmfh.bfReserved1 = 0;
		bmfh.bfReserved2 = 0;
		bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih) + sizeof(colorTable);
		bmfh.bfSize = bmfh.bfOffBits + sizeof(bits);

		bmih.biSize = 40;
		bmih.biWidth = IMAGE_SIZE;
		bmih.biHeight = IMAGE_SIZE;
		bmih.biPlanes = 1;
		bmih.biBitCount = 8;
		bmih.biCompression = 0;
		bmih.biSizeImage = 0;
		bmih.biXPelsPerMeter = 2835; // magic number, see Wikipedia entry
		bmih.biYPelsPerMeter = 2835;
		bmih.biClrUsed = 256;
		bmih.biClrImportant = 0;


		// ---------------------------  Build color table.   ------------------------------------
		for (int i = 0; i < 255; i++) {
			j = i * 4;
			colorTable[j] = colorTable[j + 1] = colorTable[j + 2] = colorTable[j + 3] = i;
		}

   
     //==============Set the last palette entry to Cyan.=========================
     //Reference: https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/named-color
     //Cyan: #00FFFF -> Red = 0, Green = 255, Blue = 255

        j = 255 * 4;                                      // Byte offset for palette #255

        colorTable[j] = 255;                         // Blue
        colorTable[j + 1] = 255;                  // Green
        colorTable[j + 2] = 0;                      // Red
        colorTable[j + 3] = 0;                      // Reserved


        // ---------------------------  Build gray scale   ------------------------------------
		for (int i = 0; i < IMAGE_SIZE; i++) {
			for (int j = 0; j < IMAGE_SIZE; j++) {
                bits[i][j] = (j == 255) ? 254 : j; // use color 254 for col  j = 255 ( save color 255 for drawing the line )
			}
		}
	}
};

//==============   Prototypes   ===========================
void plotLine(char bits[][IMAGE_SIZE],int x1, int y1, int x2, int y2);
bool isValid(int x0, int y0, int x1, int y1);

int main() {

    // Moved this into a class because main was too cluttered
    BitMap bm;

    int x0 = 0;
    int y0 = 0;
    int x1 = 0;
    int y1 = 0;

    cout << "Pierce College CS230 Fall 2026 Lab Assignment 2 - Huynh, Adam\n";
    cout << "Enter two pairs of point coordinates in the range of 0 to 255.\n";

    // fetch users input
    cin >> x0 >> y0 >> x1 >> y1;

    // validate input
    if (!isValid(x0, y0, x1, y1))
        return -1;

    // create output object
	ofstream bmpOut(FILE_NAME, ios::out + ios::binary);
	if (!bmpOut) {
		cout << "...could not open file, ending.";
		return -1;
	}



	// draw line
	plotLine(bm.bits, x0, y0, x1, y1);

	// Write out the bit map.
	char* workPtr;
	workPtr = (char*)&bm.bmfh;
	bmpOut.write(workPtr, 14);
	workPtr = (char*)&bm.bmih;
	bmpOut.write(workPtr, 40);
	workPtr = &bm.colorTable[0];
	bmpOut.write(workPtr, sizeof(bm.colorTable));
	workPtr = &bm.bits[0][0];
	bmpOut.write(workPtr, sizeof(bm.bits));
	bmpOut.close();

	// Now let's look at our creation.
    string cmd = "mspaint ";
    cmd += FILE_NAME;
	system(cmd.c_str());
	return 0;
}


//***************************    Function definitions   *****************************

// Validate input range        ---------------------------------------------------------
bool isValid(int x0, int y0, int x1, int y1) {
    bool result = true;

    int inputs[4] = { x0, y0, x1, y1 };

    for (int value : inputs) {
        if (value < 0 || value >= IMAGE_SIZE) {
            cout << "Value " << value << " out of range, ending.\n";
            result = false;
        }
    }

    return result;
}

// Pixel plotting logic in Intel assembly -------------------------------------------------
void plotLine( char bits[][IMAGE_SIZE], int x0, int y0,int x1, int y1) {
    
    //Based on Intel assembler at: "Michael Abrash's Graphics Programming Black Book Special Edition"
    // https://www.phatcode.net/res/224/files/html/ch35/35-01.html

// Initialize Vars
    int deltaX, deltaY, stepX,  stepY,  error;

    __asm {
        /*==================================================
        1. Find the x-axis displacement and magnitude
            deltaX = displacement of x
            (direction)
            stepX  = flag move right +1
            stepX  = flag move left -1
        ==================================================*/

        mov eax, x1
        sub eax, x0

        mov stepX, 1 // move right
        jge DeltaXCalculated

        neg eax
        mov stepX, -1 // move left

        DeltaXCalculated:
            mov deltaX, eax

        /*==================================================
         2. Find the y-axis displacement and direction.
            deltaY = displacement of y
            (direction)
            stepY  = flag move up +1
            stepY  = -flag move down -1
        ==================================================*/

        mov eax, y1
        sub eax, y0

        mov stepY, 1 // move up

        jge DeltaYCalculated

        neg eax
        mov stepY, -1 // move down

        DeltaYCalculated:
            mov deltaY, eax

         /*==================================================
         3. Decide next direction X or Y
            If deltaX >= deltaY, move through X each loop.
            Otherwise, move through Y each loop.
            Line1 -> X direction 
            Line2 -> Y direction
        ==================================================*/

        mov eax, deltaX
        cmp eax, deltaY
        jl Line2YError

        Line1XError :
            mov eax, deltaY
                shl eax, 1
                sub eax, deltaX
                mov error, eax

        // loop X-axis
        Line1Loop :
            mov eax, y0
                shl eax, 8     
                add eax, x0

        mov edi, bits
        mov byte ptr[edi + eax], 255 // reserved line color

        mov eax, x0
        cmp eax, x1
        jne Line1Continue

        mov eax, y0
        cmp eax, y1
        je LineFinished

        Line1Continue :

            // if (error >= 0)
            cmp error, 0
            jl Line1MoveX

            // y0 += (+-) 1
            mov eax, stepY
            add y0, eax

            // Adjust Error 
            mov eax, deltaX
            shl eax, 1
            sub error, eax

        Line1MoveX :

            // x0 += (+/-)1
            mov eax, stepX
            add x0, eax

            // Adjust Error
            mov eax, deltaY
            shl eax, 1
            add error, eax

            jmp Line1Loop

        Line2YError :
            mov eax, deltaX
                shl eax, 1
                sub eax, deltaY
                mov error, eax
        // Loop Y-axis
        Line2Loop :
            mov eax, y0
            shl eax, 8          
            add eax, x0

            mov edi, bits
            mov byte ptr[edi + eax], 255 // reserved line color

           
            mov eax, x0
            cmp eax, x1
            jne Line2Continue

            mov eax, y0
            cmp eax, y1
            je LineFinished
        
          Line2Continue :
             // if (error >= 0) 
            cmp error, 0
            jl Line2MoveY

            // x0 += stepX
            mov eax, stepX
            add x0, eax

            // error -= 2 * deltaY
            mov eax, deltaY
            shl eax, 1
            sub error, eax

            Line2MoveY :
                mov eax, stepY
                add y0, eax

                mov eax, deltaX
                shl eax, 1
                add error, eax

                jmp Line2Loop

            // exit and return to caller
            LineFinished :
    }
}
