#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

// Décommenter pour avoir un diagnostic des erreurs…
// #define cimg_verbosity 3
#include "cimg/CImg.h"

#define ROUND( a ) ( ( (a) < 0 ) ? (int) ( (a) - 0.5 ) : (int) ( (a) + 0.5 ) )
#define BLOC_SIZE 8
#define C(i) ((i)?1:(1/sqrt(2)))

using namespace std;
using namespace cimg_library;

/***** Commun *****/

inline int D2I( double in ) {
	return in < 0 ? ( in - 0.5 ) : ( in + 0.5 );
}

CImg<double> QuantizationMatrix_( double q ) {
	CImg<double> Q( 8, 8, 1, 1, 25 );
	return Q;
}

CImg<double> QuantizationMatrix( double q ) {
	CImg<double> Q( 8, 8 );
	Q(0,0)=q*16;   Q(0,1)=q*11;   Q(0,2)=q*10;   Q(0,3)=q*16;   Q(0,4)=q*24;   Q(0,5)=q*40;   Q(0,6)=q*51;   Q(0,7)=q*61;
	Q(1,0)=q*12;   Q(1,1)=q*12;   Q(1,2)=q*14;   Q(1,3)=q*19;   Q(1,4)=q*26;   Q(1,5)=q*58;   Q(1,6)=q*60;   Q(1,7)=q*55;
	Q(2,0)=q*14;   Q(2,1)=q*13;   Q(2,2)=q*16;   Q(2,3)=q*24;   Q(2,4)=q*40;   Q(2,5)=q*57;   Q(2,6)=q*69;   Q(2,7)=q*56;
	Q(3,0)=q*14;   Q(3,1)=q*17;   Q(3,2)=q*22;   Q(3,3)=q*29;   Q(3,4)=q*51;   Q(3,5)=q*87;   Q(3,6)=q*80;   Q(3,7)=q*62;
	Q(4,0)=q*18;   Q(4,1)=q*22;   Q(4,2)=q*37;   Q(4,3)=q*56;   Q(4,4)=q*68;   Q(4,5)=q*109;  Q(4,6)=q*103;  Q(4,7)=q*77;
	Q(5,0)=q*24;   Q(5,1)=q*35;   Q(5,2)=q*55;   Q(5,3)=q*64;   Q(5,4)=q*81;   Q(5,5)=q*104;  Q(5,6)=q*113;  Q(5,7)=q*92;
	Q(6,0)=q*49;   Q(6,1)=q*64;   Q(6,2)=q*78;   Q(6,3)=q*87;   Q(6,4)=q*103;  Q(6,5)=q*121;  Q(6,6)=q*120;  Q(6,7)=q*101;
	Q(7,0)=q*72;   Q(7,1)=q*92;   Q(7,2)=q*95;   Q(7,3)=q*98;   Q(7,4)=q*112;  Q(7,5)=q*100;  Q(7,6)=q*103;  Q(7,7)=q*99;
	return Q;
}

/***** Compression *****/

double ComputeFullPrecisionDctPoint( const CImg<signed char>& bloc, int i, int j ) {
	double sum = 0;
	
	for ( int x = 0; x < BLOC_SIZE; ++x ) {
		for ( int y = 0; y < BLOC_SIZE; ++y ) {
			sum += bloc( x, y )
				* cos( ( 2 * x + 1 ) * i * cimg::PI / 2. / BLOC_SIZE )
				* cos( ( 2 * y + 1 ) * j * cimg::PI / 2. / BLOC_SIZE );
		}
	}
	
	return sum * 2 / BLOC_SIZE * C( i ) * C( j );
}

void ComputeFullPrecisionDctBloc( CImg<unsigned char> bloc, CImg<double>& cbloc ) {
	CImg<signed char> centered( BLOC_SIZE, BLOC_SIZE, 1, 1, 0 );
	
	for ( int x = 0; x < BLOC_SIZE; ++x ) {
		for ( int y = 0; y < BLOC_SIZE; ++y ) {
			centered( x, y ) = bloc( x, y ) - 128;
		}
	}

	for ( int i = 0; i < BLOC_SIZE; ++i ) {
		for ( int j = 0; j < BLOC_SIZE; ++j ) {
			cbloc( i, j ) = ComputeFullPrecisionDctPoint( centered, i, j );
		}
	}
}

CImg<signed char> JPEGEncoder( const CImg<unsigned char>& image, const float q ) {
	CImg<double> fullPrecision( BLOC_SIZE, BLOC_SIZE, 1, 1, 0 );
	CImg<signed char> comp( image.width(), image.height(), 1, 1, 0 );
	comp = image;

	CImg<double> Q = QuantizationMatrix( q );

	for ( int i = 0; i < image.width(); i += BLOC_SIZE ) {
		for ( int j = 0; j < image.height(); j += BLOC_SIZE ) {
			ComputeFullPrecisionDctBloc( image.get_crop( i, j, i + BLOC_SIZE, j + BLOC_SIZE ), fullPrecision );
			
			for ( int x = 0; x < BLOC_SIZE; ++x ) {
				for ( int y = 0; y < BLOC_SIZE; ++y ) {
					comp( i + x, j + y ) = D2I( fullPrecision( x, y ) / Q( x, y ) );
				}
			}
		}
	}

	return comp;
}

/***** Décompression *****/

double ComputeInverseDctPoint( const CImg<double>& cbloc, int x, int y ) {
	double sum = 0;
	
	
	
	for ( int i = 0; i < BLOC_SIZE; ++i ) {
		for ( int j = 0; j < BLOC_SIZE; ++j ) {
			sum += cbloc( i, j )
				* C(i)
				* C(j)
				* cos( ( 2 * x + 1 ) * i * cimg::PI / 2. / BLOC_SIZE )
				* cos( ( 2 * y + 1 ) * j * cimg::PI / 2. / BLOC_SIZE );
		}
	}

	return sum * 2 / BLOC_SIZE;
}

void ComputeInverseDctBloc( const CImg<double>& cbloc, CImg<unsigned char>& bloc ){
	for ( int x = 0; x < BLOC_SIZE; ++x ) {
		for ( int y = 0; y < BLOC_SIZE; ++y ){
			bloc( x, y ) = D2I( ComputeInverseDctPoint( cbloc, x, y ) ) + 128;
		}
	}
}

CImg <unsigned char> JPEGDecoder( const CImg<signed char>& compressedImage, const float q ) {
	CImg<unsigned char> decompressImage(compressedImage.width(),compressedImage.height(),1,1,0);
	CImg<double> dctBloc( BLOC_SIZE, BLOC_SIZE, 1, 1, 0 );
	CImg<unsigned char> realBloc( BLOC_SIZE, BLOC_SIZE, 1, 1, 0 );
	
	CImg<double> Q = QuantizationMatrix( q );

	for ( int i = 0; i < compressedImage.width(); i += BLOC_SIZE ) {
		for ( int j = 0; j < compressedImage.height(); j += BLOC_SIZE ) {
			for ( int x = 0; x < BLOC_SIZE; ++x ) {
				for ( int y = 0; y < BLOC_SIZE; ++y ) {
					dctBloc( x, y ) = compressedImage( x + i, y + j ) * Q( x, y );
				}
			}
			
			ComputeInverseDctBloc( dctBloc, realBloc );
			for ( int x = 0; x < BLOC_SIZE; ++x ) {
				for ( int y = 0; y < BLOC_SIZE; ++y ) {
					decompressImage( x + i, y +j ) = realBloc( x, y );
				}
			}
		}
	
	}
	return decompressImage;
}

/***** Évaluation *****/

double Distorsion( const CImg<unsigned char>& img1, const CImg<unsigned char>& img2 ) {
	double sum = 0;
	
	for ( int x = 0; x < img1.width(); ++x ) {
		for ( int y = 0; y < img1.height(); ++y ) {
			sum += pow( img1( x, y ) - img2( x, y ), 2 );
		}
	}
	
	return sum / img1.width() / img1.height();
}

/***** Exécution *****/

void WaitWindow( CImgDisplay& win ) {
	while ( ! win.is_closed() ) {
		win.wait();
	}
}

int main()
{
	// Read the image "lena.bmp"
	CImg<unsigned char> my_image("lena.bmp");

	// Take the luminance information 
	my_image.channel(0);

	float quality = 1;
	CImg<unsigned char> comp_image = JPEGEncoder( my_image, quality );
	CImg<unsigned char> decomp_image = JPEGDecoder(comp_image,quality);
	
	cout << "Taux de distorsion : " << Distorsion( my_image, decomp_image ) << endl;

	// Display the bmp file
	CImgDisplay main_disp( my_image, "Initial Image" );

	// Display the compressed file (by dct)
	//CImgDisplay comp_disp( comp_image, "Compressed Image" );
	CImgDisplay decomp_disp( decomp_image, "Decompressed Image" );

	WaitWindow( main_disp );
	//WaitWindow( comp_disp );
	WaitWindow( decomp_disp );
}

