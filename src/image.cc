/* image.cc
   Tools to load images at SGI RGB(A) format

   Copyright (C) 2000  Mathias Broxvall
   		       Yannick Perret

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <GL/gl.h>

#include "image.h"

#define IMAGIC      0x01da
#define IMAGIC_SWAP 0xda01

#define SWAP_SHORT_BYTES(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define SWAP_LONG_BYTES(x) (((((x) & 0xff) << 24) | (((x) & 0xff00) << 8)) | \
((((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24)))

typedef struct  
{
  unsigned short imagic;
  unsigned short type;
  unsigned short dim;
  unsigned short sizeX, sizeY, sizeZ;
  unsigned long min, max;
  unsigned long wasteBytes;
  char name[80];
  unsigned long colorMap;
  FILE *file;
  unsigned char *tmp[5];
  unsigned long rleEnd;
  unsigned long *rowStart;
  unsigned long *rowSize;
} Image;

static Image *ImageOpen(char *fileName)
{
  Image *image;
  unsigned long *rowStart, *rowSize, ulTmp;
  int x, i;

  image = (Image *) malloc(sizeof(Image));
  if(image == NULL) 
  {
    fprintf(stderr, "Cannot allocate '%d' bytes.\n", sizeof(Image));
    return(NULL);
  }
  if((image->file = fopen(fileName, "rb")) == NULL) 
  {
    perror(fileName);
    return(NULL);
  }
  /*
   *	Read the image header
   */
  fread(image, 1, 12, image->file);
  /*
   *	Check byte order
   */
  if(image->imagic == IMAGIC_SWAP) 
  {
    image->type = SWAP_SHORT_BYTES(image->type);
    image->dim = SWAP_SHORT_BYTES(image->dim);
    image->sizeX = SWAP_SHORT_BYTES(image->sizeX);
    image->sizeY = SWAP_SHORT_BYTES(image->sizeY);
    image->sizeZ = SWAP_SHORT_BYTES(image->sizeZ);
  }

  for(i = 0; i <= image->sizeZ; i++)
  {
    image->tmp[i] = (unsigned char *) malloc(image->sizeX*256);
    if(image->tmp[i] == NULL) 
    {
      fprintf(stderr, "Cannot allocate '%d' bytes.\n", image->sizeX*256);
      return(NULL);
    }
  }

  if((image->type & 0xFF00) == 0x0100) /* RLE image */
  {
    x = image->sizeY * image->sizeZ * sizeof(long);
    image->rowStart = (unsigned long *)malloc(x);
    image->rowSize = (unsigned long *)malloc(x);
    if(image->rowStart == NULL || image->rowSize == NULL) 
    {
      fprintf(stderr, "Cannot allocate '%d' bytes.\n", x);
      return(NULL);
    }
    image->rleEnd = 512 + (2 * x);
    fseek(image->file, 512, SEEK_SET);
    fread(image->rowStart, 1, x, image->file);
    fread(image->rowSize, 1, x, image->file);
    if(image->imagic == IMAGIC_SWAP) 
    {
      x /= sizeof(long);
      rowStart = image->rowStart;
      rowSize = image->rowSize;
      while(x--) 
      {
	ulTmp = *rowStart;
	*rowStart++ = SWAP_LONG_BYTES(ulTmp);
	ulTmp = *rowSize;
	*rowSize++ = SWAP_LONG_BYTES(ulTmp);
      }
    }
  }
  return image;
}

static void ImageClose(Image *image)
{
  int i;

  if (image == NULL)
    return;

  fclose(image->file);
  for(i = 0; i <= image->sizeZ; i++) {
    if (image->tmp[i] != NULL)
      free(image->tmp[i]);
  }
  free(image);
}

static void ImageGetRow(Image *image, unsigned char *buf, int y, int z)
{
  unsigned char *iPtr, *oPtr, pixel;
  int count;

  if((image->type & 0xFF00) == 0x0100)  /* RLE image */
  {
    fseek(image->file, image->rowStart[y+z*image->sizeY], SEEK_SET);
    fread(image->tmp[0], 1, (unsigned int)image->rowSize[y+z*image->sizeY],
	  image->file);

    iPtr = image->tmp[0];
    oPtr = buf;
    while(1) 
    {
      pixel = *iPtr++;
      count = (int)(pixel & 0x7F);
      if(!count)
	return;
      if(pixel & 0x80) 
      {
	while(count--) 
	{
	  *oPtr++ = *iPtr++;
	}
      } 
      else 
      {
	pixel = *iPtr++;
	while(count--) 
	{
	  *oPtr++ = pixel;
	}
      }
    }
  }
  else /* verbatim image */
  {
    fseek(image->file, 512+(y*image->sizeX)+(z*image->sizeX*image->sizeY),
	  SEEK_SET);
    fread(buf, 1, image->sizeX, image->file);
  }
}

static void ImageGetRawData(Image *image, unsigned char *data)
{
  int i, j, k;
  int remain = 0;

  switch( image->sizeZ )
  {
  case 1:
    remain = image->sizeX % 4;
    break;
  case 2:
    remain = image->sizeX % 2;
    break;
  case 3:
    remain = (image->sizeX * 3) & 0x3;
    if(remain)
      remain = 4 - remain;
    break;
  case 4:
    remain = 0;
    break;
  }

  for(i = 0; i < image->sizeY; i++) 
  {
    for ( k = 0; k < image->sizeZ ; k++ )
      ImageGetRow(image, image->tmp[k+1], i, k);
    for (j = 0; j < image->sizeX; j++) 
      for ( k = 1; k <= image->sizeZ ; k++ )
	*data++ = *(image->tmp[k] + j);
    data += remain;
  }
}

IMAGE *ImageLoad(char *fileName)
{
  Image *image;
  IMAGE *final;
  int sx;

  image = ImageOpen(fileName);
  if (image == NULL)
    return(NULL);

  final = (IMAGE *) malloc(sizeof(IMAGE));
  if(final == NULL) 
  {
    fprintf(stderr, "Cannot allocate '%d' bytes.\n", sizeof(IMAGE));
    ImageClose(image);
    return(NULL);
  }
  final->imagic = image->imagic;
  final->type = image->type;
  final->dim = image->dim;
  final->sizeX = image->sizeX; 
  final->sizeY = image->sizeY;
  final->sizeZ = image->sizeZ;

  /* 
   * Round up so rows are long-word aligned 
   */
  sx = ((image->sizeX) * (image->sizeZ) + 3) >> 2;

  final->data 
    = (unsigned char *) malloc(sx * image->sizeY * sizeof(unsigned int));

  if(final->data == NULL) 
  {
    ImageClose(image);
    free(final);
    fprintf(stderr, "Cannot allocate '%d' bytes.\n", sx * image->sizeY * sizeof(unsigned int));
    return(NULL);
  }

  ImageGetRawData(image, final->data);
  ImageClose(image);
  return final;
}



/* read a RGB file and returns a data array at R,G,B,A format */
void read_2d_image_rgb(char *image, unsigned char **rbuffer,
                       int *tx, int *ty)
{
  int w, h, c, pad = 0;

  *rbuffer = NULL;
  *tx = *ty = 0;
  IMAGE *base_image = ImageLoad((char *)image);

  if(base_image == NULL)
  {
    fprintf(stderr, "Cannot load image \"%s\".\n", image);
    return;
  }

  *rbuffer = (unsigned char*)malloc(sizeof(unsigned char)*base_image->sizeX*base_image->sizeY*4);
  if (*rbuffer == NULL)
  {
    fprintf(stderr, "Cannot allocate '%d' bytes for image '%s'\n", sizeof(unsigned char)*base_image->sizeX*base_image->sizeY*4, image);
    /* Free the image */
    free(base_image->data);
    free(base_image);
    return;
  }

  *tx = base_image->sizeX;
  *ty = base_image->sizeY;

  /* Copy the data over */
  for(w = 0; w < *tx; w++)
  {
    for(h = 0; h < *ty; h++)
    {
      for(c = 0; c < 3; c++)
      {
        (*rbuffer)[(w+h*(*tx))*4 + c] = base_image->data[(w + base_image->sizeX * h)
			      * base_image->sizeZ + pad + c];
      }
    if (base_image->sizeZ == 4)
      (*rbuffer)[(w+h*(*tx))*4 + 3] = base_image->data[(w + base_image->sizeX * h)
			      * base_image->sizeZ + pad + 3];
    else
      (*rbuffer)[(w+h*(*tx))*4 + 3] = (unsigned int)255;
    }
    pad += (base_image->sizeX * base_image->sizeZ) % 4;
  }

  /* Free the image */
  free(base_image->data);
  free(base_image);

  return;
}
