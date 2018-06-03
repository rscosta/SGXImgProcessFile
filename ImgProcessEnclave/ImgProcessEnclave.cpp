#include "ImgProcessEnclave_t.h"

#include "sgx_trts.h"
#include "stdlib.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    printDebug(buf);
}

void sgxRotateImageRight(unsigned char *inBuffer, size_t inBufferSize, unsigned char *outBuffer, size_t outBufferSize)
{

	printf("ROTATE IMAGE RIGHT INIT...");

	int i = 0, j = 0, last_offset = 0, offset = 0;
	unsigned char _outBuffer[outBufferSize];
	
	for(i=0;i<54;i++)
	{									
		_outBuffer[i] = inBuffer[i];								
	}

	offset = i;
	
	// extract image height, width and bitDepth from imageHeader 
	int height = *(int*)&_outBuffer[18];
	int width = *(int*)&_outBuffer[22];
	int bitDepth = *(int*)&_outBuffer[28];
	unsigned char _imgInBuffer[width][height];

	printf("Width=%d Height=%d bitDepth=%d\n\n", width, height, bitDepth);

	//if ColorTable present, extract it.
	if(bitDepth <= 8) 
	{
	  	for(i=0;i<1024;i++)
		{
		  _outBuffer[offset] = inBuffer[offset];
		  offset++;
		}
	}

	last_offset = offset;
	
	// copying 1D to 2D array
	for(i=0;i<width;i++)
	{
	  for(j=0;j<height;j++)
	  {
            _imgInBuffer[i][j]=inBuffer[offset++];  
	  }
	}

	offset = last_offset;
	
        //to rotate right
	for(i=0;i<width;i++)
	{
		for(j=0;j<height;j++)
		{
			_outBuffer[offset++] = _imgInBuffer[j][height-1-i];
		}
	}

	memcpy(outBuffer, _outBuffer, outBufferSize);

	printf("ROTATE IMAGE RIGHT DONE!");
}

void sgxRotateImage180(unsigned char *inBuffer, size_t inBufferSize, unsigned char *outBuffer, size_t outBufferSize)
{

	printf("ROTATE IMAGE 180 DEGREE INIT...");

	int i = 0, j = 0, last_offset = 0, offset = 0;
	unsigned char _outBuffer[outBufferSize];
	
	for(i=0;i<54;i++)
	{									
		_outBuffer[i] = inBuffer[i];								
	}

	offset = i;
	
	// extract image height, width and bitDepth from imageHeader 
	int height = *(int*)&_outBuffer[18];
	int width = *(int*)&_outBuffer[22];
	int bitDepth = *(int*)&_outBuffer[28];
	unsigned char _imgInBuffer[width][height];

	printf("Width=%d Height=%d bitDepth=%d\n\n", width, height, bitDepth);

	//if ColorTable present, extract it.
	if(bitDepth <= 8) 
	{
	  	for(i=0;i<1024;i++)
		{
		  _outBuffer[offset] = inBuffer[offset];
		  offset++;
		}
	}

	last_offset = offset;
	
	// copying 1D to 2D array
	for(i=0;i<width;i++)
	{
	  for(j=0;j<height;j++)
	  {
            _imgInBuffer[i][j]=inBuffer[offset++];  
	  }
	}

	offset = last_offset;
	
        //to rotate left
	for(i=0;i<width;i++)
	{
		for(j=0;j<height;j++)
		{
			_outBuffer[offset++] = _imgInBuffer[width-i][j];
		}
	}

	memcpy(outBuffer, _outBuffer, outBufferSize);

	printf("ROTATE IMAGE 180 DEGREE DONE!");
}

void sgxMirrorImage(unsigned char *inBuffer, size_t inBufferSize, unsigned char *outBuffer, size_t outBufferSize)
{

	printf("MIRROR IMAGE INIT...");

	int i = 0, j = 0, last_offset = 0, offset = 0;
	unsigned char _outBuffer[outBufferSize];
	
	for(i=0;i<54;i++)
	{									
		_outBuffer[i] = inBuffer[i];								
	}

	offset = i;
	
	// extract image height, width and bitDepth from imageHeader 
	int height = *(int*)&_outBuffer[18];
	int width = *(int*)&_outBuffer[22];
	int bitDepth = *(int*)&_outBuffer[28];
	unsigned char _imgInBuffer[width][height];

	printf("Width=%d Height=%d bitDepth=%d\n\n", width, height, bitDepth);

	//if ColorTable present, extract it.
	if(bitDepth <= 8) 
	{
	  	for(i=0;i<1024;i++)
		{
		  _outBuffer[offset] = inBuffer[offset];
		  offset++;
		}
	}

	last_offset = offset;
	
	// copying 1D to 2D array
	for(i=0;i<width;i++)
	{
	  for(j=0;j<height;j++)
	  {
            _imgInBuffer[i][j]=inBuffer[offset++];  
	  }
	}

	offset = last_offset;
	
        //to rotate left
	for(i=0;i<width;i++)
	{
		for(j=0;j<height;j++)
		{
			_outBuffer[offset++] = _imgInBuffer[i][width-1-j];
		}
	}

	memcpy(outBuffer, _outBuffer, outBufferSize);

	printf("MIRROR IMAGE DONE!");
}

