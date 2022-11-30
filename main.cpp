/**
 * 
 * Hello :)
 * 
 * Ultimate end goal of this program is to read existing tif file and add extra pixel information
 * to its extended edges to be used as 'bleed' information required for cutting a printed representation of that file on a sheet cutter (guillotine).
 * 
 * It's a common problem faced in printing industry where files come in to be produced without bleed information defined and
 * it's a solved problem in some commercial applications. For ex. see: https://www.enfocus.com/en/blog/how-to-fix-pdf-bleed-problems-with-pitstop-pro
 * 
 * But, we'll pretend that doesn't exist ;) And I'm not sure if exactly fits our use case, and that is:
 * 
 * - To be used as a command line tool within PHP scripts run trough 'exec()' or 'popen()' functions.
 * - To have some processing progress info would be nice (to be used in conjuction with PHP output buffering disabled).
 * 
 * Method to add that extra information is currently not yet defined. Some ideas are:
 * 
 * - Use edge pixels as a source of information end extend them to a newly defined edge?
 * - Mirror source file information to newly defined edges?
 * - Use magic AI ML crypto diffusion? :)
 * 
 * Down here you'll find source code of an attempt to make such program using 'LibTIFF. A library for anything tiff.
 * Tiff file format was chosen in this case because of it's extensive feature support. Some examples are multipage files, tif2pdf, ICC, CMYK, etc.
 * 
 * More about tiff file format: https://en.wikipedia.org/wiki/TIFF
 * 
 * Current state of this program is the following:
 * 
 * - Accepts input filename, ex. 'foo.tif' (located in root)
 * - Outputs blank file 'new.tif' (it shouldn't be blank but a copy of a source file, a.k.a the program is currently broken)
 * - Profit?
 * 
 * I've put together this source code from 'god knows where' but the basic usage of 'libtiff' library can be found here: http://www.libtiff.org/libtiff.html
 *   
 * !!NOTE!! Current 'libtiff' library used here is downloaded from a hijacked website (http://www.libtiff.org/) and it is outdated.
 * See more here: https://en.wikipedia.org/wiki/LibTIFF
 * New library is maintained at (https://libtiff.gitlab.io/libtiff/) but didn't found pre-built binaries to use it here.
 * 
 * 
 * Building the program
 * 
 * I've compiled and run the program using 'Sublime text' editor and MinGW compiler (installed locally and in PATH on Windows).
 * 
 * https://www.sublimetext.com/
 * https://sourceforge.net/projects/mingw/
 * 
 * In Sublime text you can define a 'build system' by using '.sublime-build' files. That file is provided in the project too.
 * 
 * To configure the Sublime editor for build make sure you have MinGW installed and 'MinGW\bin' path in your PATH system variable.
 * You can create a 'MinGW' build system in Sublime by going to: Tools > Build system > New build system > (copy the contents of 'MinGW.sublime-build' file into it and save)
 * 
 * Build program using 'ctrl+shift+b' -> MinGW or run by 'ctrl+shift+b' -> Mingw - Run
 * 
 * That's it I think....
 * 
 * The real question is... Do you accept the challenge? ;)
 *  
 **/

#include <iostream>
#include <string.h>
#include "tiff-3.8.2-1-lib/include/tiffio.h"

// This is our main program
int main() {

	std::string filename;
	uint32 width 	= 0;
	uint32 height 	= 0;
	float xres;
	float yres;
	uint16 resUnit;

	// Prompt
	std::cout << "Type filename:";

	// Filename to edit
	std::cin >> filename;

	const char * file = filename.c_str();

	// Open specified file
    TIFF* tif = TIFFOpen(file, "r");

    // Some field values
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_XRESOLUTION, &xres);
	TIFFGetField(tif, TIFFTAG_YRESOLUTION, &yres);
	
	/**
	 * 
	 * Display resolution unit
	 * 
	 * 1 = No absolute unit of measurement. Used for images that may have a non-square aspect ratio, but no meaningful absolute dimensions.
	 * 2 = Inch.
	 * 3 = Centimeter.
	 * 
	 **/
	TIFFGetField(tif, TIFFTAG_RESOLUTIONUNIT, &resUnit);
	
	// Print resolution unit
	std::cout << "Res unit: " << resUnit << std::endl;

	// Directories
	if (tif) {
		int dircount = 0;
		do {
		    dircount++;
		} while (TIFFReadDirectory(tif));
		printf("%d directories in %s", dircount, "this file.");
		TIFFClose(tif);
    }

    TIFFClose(tif);

    // Output file
    TIFF *out = TIFFOpen("new.tif", "w");

    int sampleperpixel = 8;

    char *image = new char [width*height*sampleperpixel];

    // Set fields
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);  					// Set the width of the image
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    				// Set the height of the image
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   	// Set number of channels per pixel
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    				// Set the size of the channels
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // Set the origin of the image.

	// Some other essential fields to set that you do not have to understand for now.
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED); 	// Currently CMYK use PHOTOMETRIC_RGB for RGB

	tsize_t linebytes = sampleperpixel * width;     				// Length in memory of one row of pixel in the image.

	unsigned char *buf = NULL;        								// Buffer used to store the row of pixel information for writing to file

	// Allocating memory to store the pixels of current row
	if (TIFFScanlineSize(out), linebytes)
		buf = (unsigned char *)_TIFFmalloc(linebytes);
	else
	    buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, width*sampleperpixel));

	// Now writing image to the file one strip at a time
	for (uint32 row = 0; row < height; row++)
	{
	    memcpy(buf, &image[(height-row-1)*linebytes], linebytes);    // Check the index here, and figure out why not using h*linebytes
	    if (TIFFWriteScanline(out, buf, row, 0) < 0)
	    break;
	}

	(void) TIFFClose(out);

	if (buf)
    	_TIFFfree(buf);

    // Do not close window
    std::cin.get();

    return 0;

}