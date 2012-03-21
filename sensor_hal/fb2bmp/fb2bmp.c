#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


typedef struct {
	union {
		unsigned short int stype;                 /* Magic identifier            */
		unsigned char ctype[2];
	} type;
	unsigned int size;                       /* File size in bytes          */
	unsigned short int reserved1, reserved2;
	unsigned int offset;                     /* Offset to image data, bytes */
} __attribute__((packed)) bmp_file_header;



#define ASSERT_EQUAL(a,b) do {									\
		if ((a) != (b)) {										\
			printf("error: %s is not equal to %s\n ", #a,  #b); \
			exit(-1);											\
		} else {												\
			printf("OK: %s is equal to %s\n", #a, #b);			\
		}														\
	} while (0)


#define ASSERT_NOT_EQUAL(a,b) do {							\
		if ((a) == (b)) {									\
			printf("error: %s is equal to %s\n", #a, #b);	\
			exit(-1);										\
		} else {											\
			printf("OK: %s is not equal to %s\n", #a, #b);	\
		}													\
	} while (0)
	
typedef struct {
	unsigned int size;               /* Header size in bytes      */
	int width,height;                /* Width and height of image */
	unsigned short int planes;       /* Number of colour planes   */
	unsigned short int bits;         /* Bits per pixel            */
	unsigned int compression;        /* Compression type          */
	unsigned int imagesize;          /* Image size in bytes       */
	int xresolution,yresolution;     /* Pixels per meter          */
	unsigned int ncolours;           /* Number of colours         */
	unsigned int importantcolours;   /* Important colours         */
} __attribute__((packed)) bmp_info_header;

static void check_formats(struct fb_fix_screeninfo* p_fi, struct fb_var_screeninfo* p_vi)
{
	printf("bpp is %d\n", p_vi->bits_per_pixel);
	ASSERT_EQUAL(p_vi->bits_per_pixel % 8, 0);
	ASSERT_EQUAL(p_fi->visual, FB_VISUAL_TRUECOLOR);
	ASSERT_EQUAL(p_fi->type, FB_TYPE_PACKED_PIXELS);	
	ASSERT_EQUAL(p_vi->xres%4, 0); // because BMP file format will require rounding up to 4 bytes, and we don't want do that shit

	printf("red length is %d\n", p_vi->red.length);
	ASSERT_EQUAL(p_vi->red.msb_right, 0);

	printf("green length is %d\n", p_vi->green.length);
	ASSERT_EQUAL(p_vi->green.msb_right, 0);

	printf("blue length is %d\n", p_vi->blue.length);
	ASSERT_EQUAL(p_vi->blue.msb_right, 0);
}

static void bmp2fb(unsigned char* fb_map_p, struct fb_fix_screeninfo* p_fi, struct fb_var_screeninfo* p_vi, const char *bmp_fname)
{
	check_formats(p_fi, p_vi);
	bmp_file_header file_header;
	memset(&file_header, 0, sizeof file_header);
	bmp_info_header info_header;
	memset(&info_header, 0, sizeof info_header);

	file_header.type.ctype[0] = 'B';
	file_header.type.ctype[1] = 'M';
	file_header.size = sizeof(file_header)+sizeof(info_header) + 3 * p_vi->xres * p_vi->yres;
	file_header.offset = sizeof(file_header)+sizeof(info_header);

	info_header.size = sizeof(info_header);
	info_header.width = p_vi->xres;
	info_header.height = p_vi->yres;
	info_header.planes = 1;
	info_header.bits = 24;
	info_header.compression = 0;
	info_header.imagesize = 3 * p_vi->xres * p_vi->yres;

	bmp_file_header file_header2;
	memset(&file_header2, 0, sizeof file_header2);
	bmp_info_header info_header2;
	memset(&info_header2, 0, sizeof info_header2);
	
	FILE* fp = fopen(bmp_fname, "r");
	ASSERT_NOT_EQUAL(fp, NULL);
	
	ASSERT_EQUAL(fread(&file_header2, sizeof file_header2, 1, fp), 1);
	ASSERT_EQUAL(fread(&info_header2, sizeof info_header2, 1, fp), 1);

	ASSERT_EQUAL(memcmp(&file_header2, &file_header, sizeof file_header), 0);
	ASSERT_EQUAL(memcmp(&info_header2, &info_header, sizeof info_header), 0);

	struct stat st;
	ASSERT_EQUAL(stat(bmp_fname, &st), 0);

	ASSERT_EQUAL(3 * p_vi->xres * p_vi->yres + sizeof info_header2 + sizeof file_header2, st.st_size);
	
	unsigned char * p_bmp_data = (unsigned char *)malloc(3 * p_vi->xres * p_vi->yres);
	ASSERT_NOT_EQUAL(p_bmp_data, NULL);
	
	int fb_bytes_per_pix = p_vi->bits_per_pixel/8;

	int fb_blue_index = p_vi->blue.offset/8;
	int fb_green_index = p_vi->green.offset/8;
	int fb_red_index = p_vi->red.offset/8;

	unsigned char* fb_start = fb_map_p + 
		p_fi->line_length * p_vi->yoffset +
		p_vi->xoffset * fb_bytes_per_pix;
	
	ASSERT_EQUAL(fread(p_bmp_data, 3 * p_vi->xres * p_vi->yres, 1, fp), 1);
	unsigned char* bmp_start = p_bmp_data;
	

	unsigned int x, y;
	for (y=0; y < p_vi->yres; y++) {
		unsigned char * fb_ystart = fb_start + p_fi->line_length * y;

		for (x=0; x < p_vi->xres; x++) {
			bmp_start[0] = fb_ystart[fb_blue_index] = x+y;
			bmp_start[1] = fb_ystart[fb_green_index] = x+y+1;
			bmp_start[2] = fb_ystart[fb_red_index] = x+y+2;
			bmp_start += 3;
			fb_ystart += fb_bytes_per_pix;
		}
	}

	fclose(fp);	
}

static void fb2bmp(unsigned char* fb_map_p, struct fb_fix_screeninfo* p_fi, struct fb_var_screeninfo* p_vi, const char *bmp_fname)
{
	check_formats(p_fi, p_vi);
	bmp_file_header file_header;
	memset(&file_header, 0, sizeof file_header);
	bmp_info_header info_header;
	memset(&info_header, 0, sizeof info_header);

	file_header.type.ctype[0] = 'B';
	file_header.type.ctype[1] = 'M';

	file_header.offset = sizeof(file_header)+sizeof(info_header);

	info_header.size = sizeof(info_header);
	info_header.width = p_vi->xres;
	info_header.height = p_vi->yres;
	info_header.planes = 1;
	info_header.bits = p_vi->bits_per_pixel >= 24 ? 24 : p_vi->bits_per_pixel;
	info_header.compression = 0;
	info_header.imagesize = (info_header.bits / 8) * p_vi->xres * p_vi->yres;

	file_header.size = sizeof(file_header)+sizeof(info_header) + info_header.imagesize;
	
	unsigned char * p_bmp_data = (unsigned char *)malloc(info_header.imagesize);
	ASSERT_NOT_EQUAL(p_bmp_data, NULL);
	printf("red offset is %d\n", p_vi->red.offset);
	printf("green offset is %d\n", p_vi->green.offset);
	printf("blue offset is %d\n", p_vi->blue.offset);
	
	int fb_bytes_per_pix = p_vi->bits_per_pixel/8;
	int bmp_bytes_per_pix = info_header.bits/8;

	int bmp_blue_bit_index = 0;
	int blue_length = p_vi->blue.length;

	int bmp_green_bit_index = bmp_blue_bit_index + blue_length;
	int green_length = p_vi->green.length;

	int bmp_red_bit_index = bmp_green_bit_index + p_vi->green.length;
	int red_length = p_vi->red.length;
	
	int fb_blue_bit_index = p_vi->blue.offset;
	int fb_green_bit_index = p_vi->green.offset;
	int fb_red_bit_index = p_vi->red.offset;

	unsigned char* fb_start = fb_map_p + 
		p_fi->line_length * p_vi->yoffset +
		p_vi->xoffset * fb_bytes_per_pix;
	
	unsigned char* bmp_start = p_bmp_data;
	

	unsigned int x, y;
	for (y=0; y < p_vi->yres; y++) {
		unsigned char * fb_ystart = fb_start + p_fi->line_length * y;

		for (x=0; x < p_vi->xres; x++) {
			memset(fb_ystart, 0xff, bmp_bytes_per_pix);
			bmp_start += bmp_bytes_per_pix;
			fb_ystart += fb_bytes_per_pix;
		}
	}

	for (y=0; y < p_vi->yres; y++) {
		unsigned char * fb_ystart = fb_start + p_fi->line_length * y;

		for (x=0; x < p_vi->xres; x++) {
			memset(fb_ystart, 0xff, bmp_bytes_per_pix);
			bmp_start += bmp_bytes_per_pix;
			fb_ystart += fb_bytes_per_pix;
		}
	}

	FILE *fp = fopen(bmp_fname, "w");
	ASSERT_NOT_EQUAL(fp, NULL);
	fwrite(&file_header, sizeof(file_header), 1, fp);
	fwrite(&info_header, sizeof(info_header), 1, fp);
	fwrite(p_bmp_data, info_header.imagesize, 1, fp);
	fclose(fp);	
}

static void print_memory(void * mem, int size)
{
	int i;
	for (i=0; i < size; i++) {
		unsigned char *p = (unsigned char *)mem;
		printf ("%02x ", p[i]);
		if (i%16 == 15) {
			printf ("\n");
		} else if (i%8 == 7) {
			printf (" ");
		}
	}
	printf("\n\n");
}

int main(int argc, char *argv[])
{
	if (argc !=2 ) {
		printf("Usage: fb2bmp filename.bmp\n");
		exit(-1);
	}
	
	int fd = open("/dev/graphics/fb0", O_RDWR, 0);

	if (fd < 0) { //try /dev/fb0
		fd = open("/dev/fb0", O_RDWR, 0);
	}

	if (fd < 0) {
		printf("error opening fb0\n");
		exit(-1);
	}
	
	struct fb_var_screeninfo var_info;
	if (ioctl(fd, FBIOGET_VSCREENINFO, &var_info) < 0) {
		printf("error ioctl fbioget_vscreeninfo\n");
		exit(-1);
	}

	
	struct fb_fix_screeninfo fix_info;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix_info) < 0) {
		printf("error ioctl fbioget_fscreeninfo\n");
		exit(-1);
	}

	print_memory(&var_info, sizeof(var_info));
	print_memory(&fix_info, sizeof(fix_info));


	void* buffer  = (void*) mmap(
		0, fix_info.smem_len,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fd, 0);

	ASSERT_NOT_EQUAL(buffer, MAP_FAILED);

	if (strstr(argv[0], "fb2bmp")) {
		printf("execute fb2bmp\n");
		fb2bmp((unsigned char *)buffer, &fix_info, &var_info, argv[1]); //argv[1] is the bmp output file name
	} else {
		printf("execute bmp2fb\n");
		bmp2fb((unsigned char *)buffer, &fix_info, &var_info, argv[1]); //argv[1] is the bmp output file name
	}
	
	//we exit here, there's absolutely no need to do munmap/free/close/fclose.
	exit(0);
}
