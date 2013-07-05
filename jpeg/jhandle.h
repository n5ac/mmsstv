#ifndef JHANDLE_H
#define JHANDLE_H
extern BYTE *image_buffer;	/* Points to large array of R,G,B-order data */
extern int image_height;	/* Number of rows in image */
extern int image_width;		/* Number of columns in image */

extern "C" int write_JPEG_file (const char *filename, int quality);
extern "C" int read_JPEG_file (const char * filename);

extern "C" void SetJPEGSize(int height, int width, int plane);
extern "C" void SetJPEGImage(BYTE *p, int line);
#endif
