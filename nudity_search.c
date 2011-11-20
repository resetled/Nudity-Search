/*
 *	"Nudity Searcher" Roman A. Storozhenko, 2011.
 *	"A simple libpng example program"  Guillaume Cottenceau, 2002-2008. http://zarb.org/~gc/html/libpng.html
 *
 *	Make sure, that next dev-packages installed:
 *	libpng12, gtk+-2.0
 *
 *	For compile use next string:
 *
 *	gcc -Wall nudity_search.c -o NuditySearch `sudo pkg-config --cflags gtk+-2.0 --libs gtk+-2.0 --cflags libpng12 --libs libpng12  --cflags gtk+-2.0 --libs gtk+-2.0`
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#define PNG_DEBUG 3
#include <png.h>

int R[256], G[256], B[256];

void abort_(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

int x, y;

int width, height;
int area;
int sto=0;		// The coefficient of coincidence. Maximum - the number of entered parameters (for example 7). 0 - did not match.

png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep * row_pointers;

void read_png_file(char* file_name)
{
	char header[8];	// 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");
	if (!fp)
		abort_("[read_png_file] File %s could not be opened for reading", file_name);
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
		abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);


	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!png_ptr)
		abort_("[read_png_file] png_create_read_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[read_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during init_io");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = info_ptr->width;
	height = info_ptr->height;
	color_type = info_ptr->color_type;
	bit_depth = info_ptr->bit_depth;

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);


	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during read_image");

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (y=0; y<height; y++)
		row_pointers[y] = (png_byte*) malloc(info_ptr->rowbytes);

	png_read_image(png_ptr, row_pointers);

        fclose(fp);
}
void make_status()
{
	printf("[nudity_status]:%d\n",sto);		// Status
	//fclose(fp);
}

void calculate(int min, int max, int red, int green, int blue, float percent_red, float percent_green, float percent_blue)
{
//printf("[nudity_calculate]: min=%d | %d - %d - %d | \n", min, red, green, blue);
int i;
double lo,li,lu;

if(min!=0 && max==0){
	for(i = 0; i <= min;i++){	  
	        //printf("%d\n",i);
	lo=R[red-i];
	li=G[green-i];
	lu=B[blue-i];

	lo=lo*100/area;
	li=li*100/area;
	lu=lu*100/area;

		if(lo>percent_red && li>percent_green && lu>percent_blue){
		//printf("[7] Point was found:\n");
		//printf("Area=%d  usage: RED=%4.2f...GREEN=%4.2f...BLUE=%4.2f\n",area,lo,li,lu);
		sto++;
		}
	}
}

if(max!=0 && min==0){
	for(i = 0; i <= (255-max);i++){	  
	        //printf("%d\n",i);
	lo=R[red+i];
	li=G[green+i];
	lu=B[blue+i];

	lo=lo*100/area;
	li=li*100/area;
	lu=lu*100/area;

		if(lo>percent_red && li>percent_green && lu>percent_blue){
		//printf("[7] Point was found:\n");
		//printf("Area=%d  usage: RED=%4.2f...GREEN=%4.2f...BLUE=%4.2f\n",area,lo,li,lu);
		sto++;
		}
	}

}

}

void minimum_on_channel(int red, int green, int blue, float percent_red, float percent_green, float percent_blue)
{
	int min=0;
	int max=0;
	
if (red < green){
	if(red < blue){
		//minimum - is red
		min=red;
		calculate(min, max, red, green, blue, percent_red, percent_green, percent_blue);
		}
}
if (green < red){
	if(green < blue){
		//minimum - is red
		min=green;
		calculate(min,max,red, green, blue, percent_red, percent_green, percent_blue);
		}
}

if (blue < red){
	if(blue < green){
		//minimum - is red
		min=blue;
		calculate(min,max,red, green, blue, percent_red, percent_green, percent_blue);
		}
}

//--------------------------
if (red > green){
	if(red > blue){
		//minimum - is red
		max=red;
		calculate(min,max,red, green, blue, percent_red, percent_green, percent_blue);
		}
}
if (green > red){
	if(green > blue){
		//minimum - is red
		max=green;
		calculate(min,max,red, green, blue, percent_red, percent_green, percent_blue);
		}
}

if (blue > red){
	if(blue > green){
		//minimum - is red
		max=blue;
		calculate(min,max,red, green, blue, percent_red, percent_green, percent_blue);
		}
}
}

void load_settings()
{
	int first, second, third;
	float percent_red, percent_green, percent_blue;
	
FILE *fp = fopen("config", "r");
	if (!fp)
		abort_("[NuditySearcher] Config file could not be opened for reading");

		while (fscanf(fp, "%d-%d-%d|%f-%f-%f", &first, &second, &third, &percent_red, &percent_green, &percent_blue) != EOF){
			
		//printf("[nudity_config]: %d - %d - %d | %4.2f-%4.2f-%4.2f\n",first, second, third, percent_red, percent_green, percent_blue);		// Reading config
		minimum_on_channel(first, second, third, percent_red, percent_green, percent_blue);
				}

}

void process_file(void)
{
	/*if (info_ptr->color_type != PNG_COLOR_TYPE_RGBA)
		abort_("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGB-A (is %d)",
                       info_ptr->color_type);*/
int red, green, blue;
	for (y=0; y<height; y++) {
		png_byte* row = row_pointers[y];
		for (x=0; x<width; x=x+3) {
			png_byte* ptr = &(row[x]);//4
			//printf("Pixel at position [ %d - %d ] has the following RGB-A values: %d - %d - %d - %d\n",
			       //x, y, ptr[0], ptr[1], ptr[2], ptr[3]);//
			 
			red=ptr[x];		//red channel
			green=ptr[x+1];		//green channel
			blue=ptr[x+2];		//blue channel
						//where x - is the number of horizontal point

			R[red]=R[red]+1;
			G[green]=G[green]+1;
			B[blue]=B[blue]+1;

		}

	}
area=width*height;
//printf("Area...area=%d...\n", area);
}


int main(int argc, char **argv)
{

	if (argc != 2)
		abort_("Usage: program_name <file_in>");

	read_png_file(argv[1]);

	process_file();
	
	load_settings();
	
	make_status();


        return 0;
}
