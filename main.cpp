#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

#include "CImg.h"

#define ROUND( a ) ( ( (a) < 0 ) ? (int) ( (a) - 0.5 ) : (int) ( (a) + 0.5 ) )
#define BLOC_SIZE 8

using namespace std;
using namespace cimg_library;

double ComputeFullPrecisionDctPoint( const CImg<unsigned char> bloc, int i, int j ) {
	#define C(i) ((i)?1:1/sqrt(2))
	
	double sum = 0;
	
	for ( int x = 0; x < BLOC_SIZE; ++x ) {
		for ( y = 0; y < BLOC_SIZE; ++y ) {
			sum += bloc( x, y )
				* cos( ( 2 * x + 1 ) * i * PI / 2. / N )
				* cos( ( 2 * y + 1 ) * j * PI / 2. / N );
		}
	}
	
	return 2. / N * C( i ) * C( j );
	
	#undef C
}

void ComputeFullPrecisionDctBloc( CImg<unsigned char> bloc, CImg<double>& cbloc ) {
	for ( int x = 0; x < BLOC_SIZE; ++x ) {
		for ( y = 0; y < BLOC_SIZE; ++y ) {
			bloc( x, y ) = bloc( x, y ) - 128;
		}
	}

	for ( int i = 0; i < BLOC_SIZE; ++i ) {
		for ( int j = 0; j < BLOC_SIZE; ++j ) {
			cbloc( i, j ) = ComputeFullPrecisionDctPoint( bloc, i, j );
		}
	}
}

CImg<unsigned char> JPEGEncoder( const CImg<unsigned char> image, const float quality ) {
	CImg<double> fullPrecision(BLOC_SIZE, BLOC_SIZE, 1, 1, 0 );
	CImg<unsigned char> comp( image.width(), image.height(), 1, 1, 0 );
	comp = image;

	// Quantization matrix
	/*
	CImg<> Q(8,8);
	Q(0,0)=16;   Q(0,1)=11;   Q(0,2)=10;   Q(0,3)=16;   Q(0,4)=24;   Q(0,5)=40;   Q(0,6)=51;   Q(0,7)=61;
	Q(1,0)=12;   Q(1,1)=12;   Q(1,2)=14;   Q(1,3)=19;   Q(1,4)=26;   Q(1,5)=58;   Q(1,6)=60;   Q(1,7)=55;
	Q(2,0)=14;   Q(2,1)=13;   Q(2,2)=16;   Q(2,3)=24;   Q(2,4)=40;   Q(2,5)=57;   Q(2,6)=69;   Q(2,7)=56;
	Q(3,0)=14;   Q(3,1)=17;   Q(3,2)=22;   Q(3,3)=29;   Q(3,4)=51;   Q(3,5)=87;   Q(3,6)=80;   Q(3,7)=62;
	Q(4,0)=18;   Q(4,1)=22;   Q(4,2)=37;   Q(4,3)=56;   Q(4,4)=68;   Q(4,5)=109;  Q(4,6)=103;  Q(4,7)=77;
	Q(5,0)=24;   Q(5,1)=35;   Q(5,2)=55;   Q(5,3)=64;   Q(5,4)=81;   Q(5,5)=104;  Q(5,6)=113;  Q(5,7)=92;
	Q(6,0)=49;   Q(6,1)=64;   Q(6,2)=78;   Q(6,3)=87;   Q(6,4)=103;  Q(6,5)=121;  Q(6,6)=120;  Q(6,7)=101;
	Q(7,0)=72;   Q(7,1)=92;   Q(7,2)=95;   Q(7,3)=98;   Q(7,4)=112;  Q(7,5)=100;  Q(7,6)=103;  Q(7,7)=99;
	Q *= quality; 
	*/

	for ( int i = 0; i < image.width(); i += 8 ) {
		for ( int j = 0; i < image.height(); j += 8 ) {
			
		}
	}

	return comp;
}

int main()
{
	// Read the image "lena.bmp"
	CImg<unsigned char> my_image("lena.bmp");

	// Take the luminance information 
	my_image.channel(0);

	float quality=1.;
	CImg<unsigned char> comp_image = JPEGEncoder( my_image, quality );

	// Display the bmp file
	CImgDisplay main_disp( my_image, "Initial Image" );

	// Display the compressed file (by dct)
	CImgDisplay comp_disp( comp_image, "Compressed Image" );

	while ( ! main_disp.is_closed() ) {
		main_disp.wait();
	}

	while ( ! comp_disp.is_closed() ) {
		main_disp.wait();
	}
}

