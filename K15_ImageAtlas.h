/*
K15 Image Atlas v 1.0
	Single header public domain library

# Author(s):
	Felix Klinge (fklinge dot deck13 dot com)

# Version history
	1.0 | 06/17/2016	-	Intial release 
	1.1 | 10/20/2016	-	Made the source code C89 (so it can be used by C89 only C compiler)

# What problem is this library trying to solve? (ELI5)
	This library can be used to generate a single image that contains
	many, smaller images. The library will try to pack the smaller images
	as tightly as possible in a performant manner. This newly created
	image is often called an 'Atlas'. Hence the name of the library.

# How do I add this library to my project?
	This library is a single header library which means that you just have to
	#include it into your project. However, you have to add the implementation
	of the functions used into *one* of your C/CPP files. To do this you have
	to add #define K15_IA_IMPLEMENTATION *before* the #include.

# How does this library work?
	This library does implement the 'Skyline Left-Bottom' packing
	algorithm. I implemented the algorithm roughly using the paper
	'A Skyline-Based Heuristic for the 2D Rectangular Strip Packing Problem'
	by Wei Lijun, Andrew Lim and Wenbin Zhu.

	To use this library in your project, these are the steps that you should
	you follow:

	1. 	You create a new image atlas and specify how many images you want to add
		(This is used to determine how much memory needs to be allocated)

	Function(s) used:
	K15_IACreateAtlas / K15_IACreateAtlasWithCustomMemory

	Note: 	This will trigger an allocation (the only one in this library)
			if K15_IACreateAtlas is used. If this is not the desired behavior,
			call K15_IACreateAtlasWithCustomMemory with a memory block that is at
			least the size of N bytes where N is an integer value returned by
			the function K15_IACalculateAtlasMemorySizeInBytes .


	2. 	You start adding images to populate the atlas. The library
		will directly place the image at it's respected place according
		to the algorithm implemented and will return the position where the
		image has been placed to the caller.

	Function(s) used:
	K15_IAAddImageToAtlas

	Note: 	K15_IAAddImageToAtlas does take a pixel format paramter to later
			determine if pixel conversion needs to happen when you want to
			get a copy of the image atlas.


	3.	After you added all the images to the atlas, you can 'bake' the atlas
		and get a copy of the pixel data of the finished image atlas.

	Function(s) used:
	K15_IABakeImageAtlasIntoPixelBuffer

	Note: 	K15_IABakeImageAtlasIntoPixelBuffer does take a pixel format paramater.
			Pixel format conversion will happen on the fly if the pixel format
			specified differs from that of individual images (that got added
			by using K15_IAAddImageToAtlas). Currently, the atlas
			will only grow by power of two dimensions.

	4. 	You delete the image atlas to free previously allocated memory during
		K15_IACreateAtlas.

	Function(s) used:
	K15_IAFreeAtlas

	Note: 	If you did no get a copy of the pixel data of the atlas,
			the data will be lost after calling this function. You basically
			only need to call this function if you created the atlas using
			K15_IACreateAtlas and thus triggered a memory allocation.

	If memory was allocated by K15_IA_MALLOC during K15_IACreateAtlas,
	the memory will be freed using K15_IA_FREE.

# Example Usage
{
	const int numImagesToAdd = 256;

	//has already been filled
	unsigned char* imagesToAdd[numImagesToAdd];
	int imagesToAddWidths[numImagesToAdd];
	int imagesToAddHeights[numImagesToAdd];

	K15_IAImageAtlas atlas = {};
	K15_IACreateAtlas(&atlas, numImagesToAdd);

	for (int imageIndex = 0;
		imageIndex < numImagesToAdd;
		++imageIndex)
	{
		unsigned char* imageData = imagesToAdd[imageIndex];
		int imageWidth = imagesToAddWidths[imageIndex];
		int imageHeight = imagesToAddHeights[imageIndex];
		int imagePosX = 0;
		int imagePosY = 0;

		K15_IAAddImageToAtlas(&atlas, KIA_PIXEL_FORMAT_R8G8B8, imageData,
		imageWidth, imageHeight, &imagePosX, &imagePosY);

		//store imagePosX & imagePosY for later use
	}

	int imagePixelDataSizeInBytes = K15_IACalculateAtlasPixelDataSizeInBytes(&atlas);
	void* imagePixelData = malloc(imagePixelDataSizeInBytes);

	int width = 0;
	int height = 0;
	K15_IABakeImageAtlasIntoPixelBuffer(&atlas, KIA_PIXEL_FORMAT_R8G8B8, imagePixelData,
	&width, &height);

	//imagePixelData can be used in combination with imagePosX & imagePosY to
	//identify individual images within the atlas.

	//free memory
	K15_IAFreeAtlas(&atlas);
}

# Hints
	-	If you've got a large amount of images you want to add to an atlas,
		try increasing the amount of wasted spaces rectangles that are getting
		tracked (place '#define K15_IA_MAX_WASTED_SPACE_RECTS N' before 
		including this header - where N is the amount of wasted space rectangles 
		to track. Default is 512). 

	-	Best results can be achieved if the images are sorted prior to adding 
		them to the atlas.

	-	Currently, the library only produces atlases whose width and height 
		are power of two.

# TODO
	- 	Merge wasted space areas
	- 	Allow to create non power of two atlases
	- 	Add border per image (really necessary?)
	- 	Enable automatic mip map creation (really necessary?)

# License:
	This software is in the public domain. Where that dedication is not
	recognized, you are granted a perpetual, irrevocable license to copy
	and modify this file however you want.
*/

#ifndef _K15_ImageAtlas_h_
#define _K15_ImageAtlas_h_

#ifndef K15_IA_STATIC
# define kia_def static
#else
# define kia_def extern
#endif //K15_IA_STATIC

typedef signed int kia_s32;
typedef unsigned int kia_u32;
typedef unsigned short kia_u16;

typedef unsigned char kia_u8;
typedef unsigned char kia_b8;
typedef unsigned char kia_byte;

enum _K15_IAAtlasFlags
{
	KIA_EXTERNAL_MEMORY_FLAG = 0x01,			//<! Memory was provided by the user (K15_IACreateAtlasWithCustomMemory)
	KIA_FORCE_POWER_OF_TWO_DIMENSION = 0x02		//<! Currently used by default
};

typedef enum _K15_IAPixelFormat
{
	KIA_PIXEL_FORMAT_R8 = 1,
	KIA_PIXEL_FORMAT_R8A8 = 2,
	KIA_PIXEL_FORMAT_R8G8B8 = 3,
	KIA_PIXEL_FORMAT_R8G8B8A8 = 4
} K15_IAPixelFormat;

typedef enum _K15_AtlasResults
{
	K15_IA_RESULT_SUCCESS = 0,				//<! Everything went fine
	K15_IA_RESULT_OUT_OF_MEMORY = 1,		//<! Out of memory
	K15_IA_RESULT_OUT_OF_RANGE = 2,			//<! Out of range (passed wrong index)
	K15_IA_RESULT_INVALID_ARGUMENTS = 3,	//<! Invalid arguments (nullptr, etc)
	K15_IA_RESULT_TOO_FEW_SKYLINES = 4,		//<! K15_IA_MAX_SKYLINES is too small for your atlas
	K15_IA_RESULT_ATLAS_TOO_SMALL = 5,		//<! Only used internally
	K15_IA_RESULT_ATLAS_TOO_LARGE = 6		//<! The atlas has grown too large (Specified by K15_IA_DIMENSION_THRESHOLD)
} kia_result;

struct _K15_IARect;
struct _K15_IAImageNode;
struct _K15_IASkyline;

typedef struct _K15_IARect K15_IARect;
typedef struct _K15_IAImageNode K15_IAImageNode;
typedef struct _K15_IASkyline K15_IASkyline;

typedef struct _K15_ImageAtlas
{
	K15_IASkyline* skylines;		//<! Skylines used to place a new image
	K15_IAImageNode* imageNodes;	//<! Image nodes added to the atlas
	K15_IARect* wastedSpaceRects;	//<! We keep track of wasted space to fill it eventually

	kia_u32 width;					//<! Width of the atlas
	kia_u32 height;					//<! Height of the atlas

	kia_u32 numSkylines;			//<! Number of skylines in the skylines array
	kia_u32 numWastedSpaceRects;	//<! Number of rects in the wastedSpaceRects array
	kia_u32 numImageNodes;			//<! Number of image nodes in the imageNodes array
	kia_u32 numMaxImageNodes;		//<! Maximum number of images supported for the atlas
	kia_u8 flags;					//<! See K15_IAAtlasFlags enum
} K15_ImageAtlas;

//Create a new atlas which is able to store and process p_NumImages of images.
//Note: Triggers an allocation by using K15_IA_MALLOC.
//		Returns one of the following results:
//			- K15_IA_RESULT_INVALID_ARGUMENTS (p_OutImageAtlas is NULL or p_Components is invalid)
//			- K15_IA_RESULT_OUT_OF_MEMORY
//			- K15_IA_RESULT_SUCCESS
kia_def kia_result K15_IACreateAtlas(K15_ImageAtlas* p_OutImageAtlas, kia_u32 p_NumImages);

//Create a new atlas which is able to store and process p_NumImages of images.
//Note: Returns one of the following results:
//			- K15_IA_RESULT_INVALID_ARGUMENTS (p_OutImageAtlas is NULL or p_Components is invalid)
//			- K15_IA_RESULT_SUCCESS
kia_def kia_result K15_IACreateAtlasWithCustomMemory(K15_ImageAtlas* p_OutImageAtlas, kia_u32 p_NumImages,
	void* p_AtlasMemory);

//Calculates the amount of memory needed (in bytes) to store an image atlas which
//is able to store p_NumImages of images.
kia_def kia_u32 K15_IACalculateAtlasMemorySizeInBytes(kia_u32 p_NumImages);

//Calculate the amount of memory needed (in bytes) to store the baked image atlas
//pixel data in a specific pixel format.
kia_def kia_u32 K15_IACalculateAtlasPixelDataSizeInBytes(K15_ImageAtlas* p_ImageAtlas, K15_IAPixelFormat p_PixelFormat);

//Free a previously created atlas (K15_IACreateAtlas). Deallocates all memory associated with 
//an image atlas.
//Note: You don't necessarly need to call this function if you used your own memory using
//		K15_IACreateAtlasWithCustomMemory.
kia_def void K15_IAFreeAtlas(K15_ImageAtlas* p_ImageAtlas);

//Add an image to a specific atlas using a specific pixel format. 
//This will trigger the algorithm to find the best possible position for the image. 
//The position found will be returned to the caller using the p_OutX and p_OutY parameters.
//Note: Returns one of the following results:
//			- K15_IA_RESULT_INVALID_ARGUMENTS (p_ImageAtlas is NULL, p_PixelData is NULL or 
//											   p_PixelDataWith and/or p_PixelDataHeight are invalid or
//											   p_OutX and/or p_OutY are NULL)
//			- K15_IA_RESULT_ATLAS_TOO_SMALL
//			- K15_IA_RESULT_TOO_FEW_SKYLINES
//			- K15_IA_RESULT_OUT_OF_RANGE (Trying to add more images than specified 
//										  in K15_IACreateAtlas / K15_IACreateAtlasWithCustomMemory)
//			- K15_IA_RESULT_SUCCESS
kia_def kia_result K15_IAAddImageToAtlas(K15_ImageAtlas* p_ImageAtlas, K15_IAPixelFormat p_PixelFormat,
	void* p_PixelData, kia_u32 p_PixelDataWidth, kia_u32 p_PixelDataHeight,
	int* p_OutX, int* p_OutY);

//Compose the images in the atlas into a given pixel data buffer using a specific pixel format.
//The width and height of the resulting pixel buffer will be returned to the caller using the
//p_OutWidth and p_OutHeight parameters (can be NULL).
//Note: If there's a mismatch between the pixel format specified (p_PixelFormat) and the 
//		pixel format of individual images (specified in K15_IAAddImageToAtlas), pixel
//		conversion will happen on the fly to match the pixel format specified.
kia_def void K15_IABakeImageAtlasIntoPixelBuffer(K15_ImageAtlas* p_ImageAtlas, K15_IAPixelFormat p_PixelFormat,
	void* p_DestinationPixelDataBuffer, int* p_OutWidth, int* p_OutHeight);

#ifdef K15_IA_IMPLEMENTATION

#define K15_IA_TRUE 1
#define K15_IA_FALSE 0

#ifndef K15_IA_MAX_SKYLINES 
# define K15_IA_MAX_SKYLINES 128
#endif //K15_IA_MAX_SKYLINES 

#ifndef K15_IA_MAX_WASTED_SPACE_RECTS 
# define K15_IA_MAX_WASTED_SPACE_RECTS 512
#endif //K15_IA_MAX_WASTED_SPACE_RECTS 

#ifndef K15_IA_DIMENSION_THRESHOLD
# define K15_IA_DIMENSION_THRESHOLD 8192
#endif //K15_IA_DIMENSION_THRESHOLD

#ifndef K15_IA_DEFAULT_MIN_ATLAS_DIMENSION
# define K15_IA_DEFAULT_MIN_ATLAS_DIMENSION 16
#endif //K15_IA_DEFAULT_MIN_ATLAS_DIMENSION

#if K15_IA_DEFAULT_MIN_ATLAS_DIMENSION <= 8
# error "'K15_IA_DEFAULT_MIN_ATLAS_DIMENSION' needs to be at least 8"
#endif

#if K15_IA_DIMENSION_THRESHOLD <= 0
# error "'K15_IA_DIMENSION_THRESHOLD' can not be negative or zero"
#endif

#if K15_IA_DEFAULT_MIN_ATLAS_DIMENSION > K15_IA_DIMENSION_THRESHOLD
# error "'K15_IA_DEFAULT_MIN_ATLAS_DIMENSION' is greater than 'K15_IA_DIMENSION_THRESHOLD'"
#endif

#ifndef K15_IA_MALLOC
# include <stdlib.h>
# define K15_IA_MALLOC malloc
# define K15_IA_FREE free
#endif //K15_IA_MALLOC

#ifndef K15_IA_MEMCPY
# include <string.h>
# define K15_IA_MEMCPY memcpy
#endif //K15_IA_MEMCPY

#ifndef K15_IA_MEMSET
# include <string.h>
# define K15_IA_MEMSET memset
#endif //K15_IA_MEMSET

#ifndef K15_IA_MEMMOVE
# include <string.h>
# define K15_IA_MEMMOVE memmove
#endif //K15_IA_MEMMOVE

#ifndef K15_IA_QSORT
# include <search.h>
# define K15_IA_QSORT qsort
#endif //K15_IA_QSORT

#ifndef K15_IA_MIN
# define K15_IA_MIN(a,b) ((a) < (b) ? (a) : (b))
#endif //K15_IA_MIN

#ifndef K15_IA_MAX
# define K15_IA_MAX(a,b) ((a) > (b) ? (a) : (b))
#endif //K15_IA_MAX

#ifndef kia_internal
# define kia_internal static
#endif //kia_internal

typedef struct _K15_IARect
{
	kia_u16 posX;
	kia_u16 posY;
	kia_u16 width;
	kia_u16 height;
} K15_IARect;

typedef struct _K15_IAImageNode
{
	K15_IAPixelFormat pixelDataFormat;
	K15_IARect rect;
	kia_byte* pixelData;
} K15_IAImageNode;

typedef struct _K15_IASkyline
{
	kia_u16 baseLinePosX;
	kia_u16 baseLinePosY;
	kia_u32 baseLineWidth;
} K15_IASkyline;

/*********************************************************************************/
kia_internal int K15_IASortSkylineByXPos(const void* p_SkylineA, const void* p_SkylineB)
{
	K15_IASkyline* skylineA = (K15_IASkyline*)p_SkylineA;
	K15_IASkyline* skylineB = (K15_IASkyline*)p_SkylineB;

	return skylineA->baseLinePosX - skylineB->baseLinePosX;
}
/*********************************************************************************/



/*********************************************************************************/
kia_internal void K15_IAConvertPixel(kia_u8* p_SourcePixel, kia_u8* p_DestinationPixel,
	K15_IAPixelFormat p_SourcePixelFormat, K15_IAPixelFormat p_DestinationPixelFormat)
{
	if (p_SourcePixelFormat == KIA_PIXEL_FORMAT_R8)
	{
		kia_s32 colorIndex = 0;
		for (colorIndex = 0;
			colorIndex < p_DestinationPixelFormat;
			++colorIndex)
		{
			p_DestinationPixel[colorIndex] = *p_SourcePixel;
		}
	}
	else if (p_SourcePixelFormat == KIA_PIXEL_FORMAT_R8A8)
	{
		kia_u8 sourcePixel = (kia_u8)((float)p_SourcePixel[0] * (float)((p_SourcePixel[1]) / 255));

		if (p_DestinationPixelFormat == KIA_PIXEL_FORMAT_R8)
			p_DestinationPixel[0] = sourcePixel;
		else if (p_DestinationPixelFormat == KIA_PIXEL_FORMAT_R8G8B8)
		{
			p_DestinationPixel[0] = sourcePixel;
			p_DestinationPixel[1] = sourcePixel;
			p_DestinationPixel[2] = sourcePixel;
		}
		else if (p_DestinationPixelFormat == KIA_PIXEL_FORMAT_R8G8B8A8)
		{
			p_DestinationPixel[0] = p_SourcePixel[0];
			p_DestinationPixel[1] = p_SourcePixel[0];
			p_DestinationPixel[2] = p_SourcePixel[0];
			p_DestinationPixel[3] = p_SourcePixel[1];
		}
	}
	else if (p_SourcePixelFormat == KIA_PIXEL_FORMAT_R8G8B8)
	{
		kia_u8 greyscale = (kia_u8)((float)(p_SourcePixel[0]) * 0.21f +
			(float)(p_SourcePixel[1]) * 0.72f +
			(float)(p_SourcePixel[2]) * 0.07f);

		if (p_DestinationPixelFormat == KIA_PIXEL_FORMAT_R8)
			p_DestinationPixel[0] = greyscale;
		else if (p_DestinationPixelFormat == KIA_PIXEL_FORMAT_R8A8)
		{
			p_DestinationPixel[0] = greyscale;
			p_DestinationPixel[1] = 255;
		}
		else if (p_DestinationPixelFormat == KIA_PIXEL_FORMAT_R8G8B8A8)
		{
			p_DestinationPixel[0] = p_SourcePixel[0];
			p_DestinationPixel[1] = p_SourcePixel[1];
			p_DestinationPixel[2] = p_SourcePixel[2];
			p_DestinationPixel[3] = 255;
		}
	}
	else if (p_SourcePixelFormat == KIA_PIXEL_FORMAT_R8G8B8A8)
	{
		float greyscale = (float)(p_SourcePixel[0]) * 0.21f +
			(float)(p_SourcePixel[1]) * 0.72f +
			(float)(p_SourcePixel[2]) * 0.07f;

		float alpha = (float)(p_SourcePixel[3] / 255.f);
		float greyscaleWithAlpha = greyscale * alpha;

		if (p_DestinationPixelFormat == KIA_PIXEL_FORMAT_R8)
			p_DestinationPixel[0] = (kia_u8)(greyscaleWithAlpha + 0.5f);
		else if (p_DestinationPixelFormat == KIA_PIXEL_FORMAT_R8A8)
		{
			p_DestinationPixel[0] = (kia_u8)(greyscale + 0.5f);
			p_DestinationPixel[1] = p_SourcePixel[3];
		}
		else if (p_DestinationPixelFormat == KIA_PIXEL_FORMAT_R8G8B8)
		{
			p_DestinationPixel[0] = (kia_u8)((float)(p_SourcePixel[0]) * alpha);
			p_DestinationPixel[1] = (kia_u8)((float)(p_SourcePixel[1]) * alpha);
			p_DestinationPixel[2] = (kia_u8)((float)(p_SourcePixel[2]) * alpha);
		}
	}
}
/*********************************************************************************/
kia_internal kia_result K15_IAConvertPixelData(kia_byte* p_DestinationPixelData, kia_byte* p_SourcePixelData,
	K15_IAPixelFormat p_DestinationPixelFormat, K15_IAPixelFormat p_SourcePixelFormat,
	kia_u32 p_PixelDataStride)
{
	kia_u32 sourcePixelIndex = 0;
	kia_u32 destinationPixelIndex = 0;
	kia_u32 pixelIndex = 0;
	if (!p_SourcePixelData || !p_DestinationPixelData || p_PixelDataStride == 0)
	{
		return K15_IA_RESULT_INVALID_ARGUMENTS;
	}

	//convert pixel by pixel
	for (pixelIndex = 0;
		pixelIndex < p_PixelDataStride;
		++pixelIndex)
	{
		sourcePixelIndex = pixelIndex * p_SourcePixelFormat;
		destinationPixelIndex = pixelIndex * p_DestinationPixelFormat;

		K15_IAConvertPixel(p_SourcePixelData + sourcePixelIndex,
			p_DestinationPixelData + destinationPixelIndex,
			p_SourcePixelFormat, p_DestinationPixelFormat);
	}

	return K15_IA_RESULT_SUCCESS;
}
/*********************************************************************************/
kia_internal kia_u32 K15_IARemoveSkylineByIndex(K15_IASkyline* p_Skylines, kia_u32 p_NumSkylines,
	kia_u32 p_SkylineIndex)
{
	kia_u32 numSkylines = p_NumSkylines;
	kia_u32 numSkylinesToMove = 0;

	if (p_SkylineIndex + 1 < numSkylines)
	{
		numSkylinesToMove = numSkylines - p_SkylineIndex;
		K15_IA_MEMMOVE(p_Skylines + p_SkylineIndex, p_Skylines + p_SkylineIndex + 1,
			numSkylinesToMove * sizeof(K15_IASkyline));
	}

	return --numSkylines;
}
/*********************************************************************************/
kia_internal kia_u32 K15_IAAddWastedSpaceRect(K15_IARect* p_WastedSpaceRects, kia_u32 p_NumWastedSpaceRects,
	kia_u32 p_PosX, kia_u32 p_PosY, kia_u32 p_Width, kia_u32 p_Height)
{
	if (p_NumWastedSpaceRects == K15_IA_MAX_WASTED_SPACE_RECTS)
		return p_NumWastedSpaceRects;

	p_WastedSpaceRects[p_NumWastedSpaceRects].posX = p_PosX;
	p_WastedSpaceRects[p_NumWastedSpaceRects].posY = p_PosY;
	p_WastedSpaceRects[p_NumWastedSpaceRects].width = p_Width;
	p_WastedSpaceRects[p_NumWastedSpaceRects].height = p_Height;

	return p_NumWastedSpaceRects + 1;
}
/*********************************************************************************/
kia_internal void K15_IAFindWastedSpaceAndRemoveObscuredSkylines(K15_IASkyline* p_Skylines,
	kia_u32* p_NumSkylinesOutIn, K15_IARect* p_WastedSpaceRects, kia_u32* p_NumWastedSpaceRectsOutIn,
	kia_u32 p_PosX, kia_u32 p_PosY, kia_u32 p_Width)
{
	kia_u32 baseLinePosX = 0;
	kia_u32 baseLinePosY = 0;
	kia_u32 baseLineWidth = 0;
	kia_u32 rightPos = p_PosX + p_Width;
	kia_u32 baseLineRightPos = 0;
	kia_u32 numSkylines = *p_NumSkylinesOutIn;
	kia_u32 numWastedSpaceRects = *p_NumWastedSpaceRectsOutIn;
	kia_u32 skylineIndex = 0;
	K15_IASkyline* skyline = 0;

	for (skylineIndex = 0;
		skylineIndex < numSkylines;
		++skylineIndex)
	{
		skyline = p_Skylines + skylineIndex;
		baseLinePosX = skyline->baseLinePosX;
		baseLinePosY = skyline->baseLinePosY;
		baseLineWidth = skyline->baseLineWidth;

		if (p_PosX < baseLinePosX && rightPos > baseLinePosX && p_PosY >= baseLinePosY)
		{
			//Split if we 'reach' into another skyline
			//check first if the current skyline is fully obscured
			baseLineRightPos = baseLinePosX + baseLineWidth;

			if (rightPos < baseLineRightPos)
			{
				numWastedSpaceRects = K15_IAAddWastedSpaceRect(p_WastedSpaceRects, numWastedSpaceRects,
					baseLinePosX, baseLinePosY, rightPos - baseLinePosX, p_PosY - baseLinePosY);

				skyline->baseLineWidth = baseLineRightPos - rightPos;
				skyline->baseLinePosX = rightPos;
				continue;
			}

			numSkylines = K15_IARemoveSkylineByIndex(p_Skylines, numSkylines, skylineIndex);
			numWastedSpaceRects = K15_IAAddWastedSpaceRect(p_WastedSpaceRects, numWastedSpaceRects,
				baseLinePosX, baseLinePosY, baseLineWidth, p_PosY - baseLinePosY);

			--skylineIndex;
		}
	}

	*p_NumSkylinesOutIn = numSkylines;
	*p_NumWastedSpaceRectsOutIn = numWastedSpaceRects;
}
/*********************************************************************************/
kia_internal kia_u32 K15_IAMergeSkylines(K15_IASkyline* p_Skylines, kia_u32 p_NumSkylines)
{
	K15_IASkyline* skyline = 0;
	K15_IASkyline* previousSkyline = 0;

	kia_u32 baselineY = 0;
	kia_u32 previousBaseLineY = 0;
	kia_u32 skylineIndex = 1;
	kia_u32 numSkylines = p_NumSkylines;

	if (numSkylines > 1)
	{
		for (skylineIndex = 1;
			skylineIndex < numSkylines;
			++skylineIndex)
		{
			skyline = p_Skylines + skylineIndex;
			previousSkyline = p_Skylines + skylineIndex - 1;

			if (skyline->baseLinePosY == previousSkyline->baseLinePosY)
			{
				previousSkyline->baseLineWidth += skyline->baseLineWidth;

				numSkylines = K15_IARemoveSkylineByIndex(p_Skylines, numSkylines, skylineIndex);
				--skylineIndex;
			}
		}
	}

	return numSkylines;
}
/*********************************************************************************/
kia_internal kia_result K15_IATryToInsertSkyline(K15_ImageAtlas* p_ImageAtlas, kia_u32 p_BaseLineY,
	kia_u32 p_BaseLineX, kia_u32 p_BaseLineWidth)
{
	K15_IASkyline* skylines = p_ImageAtlas->skylines;
	K15_IASkyline* newSkyline = 0;
	K15_IARect* wastedSpaceRects = p_ImageAtlas->wastedSpaceRects;

	kia_u32 numSkylines = p_ImageAtlas->numSkylines;
	kia_u32 numWastedSpaceRects = p_ImageAtlas->numWastedSpaceRects;

	if (numSkylines == K15_IA_MAX_SKYLINES)
		return K15_IA_RESULT_TOO_FEW_SKYLINES;

	newSkyline = skylines + numSkylines++;
	newSkyline->baseLinePosX = p_BaseLineX;
	newSkyline->baseLinePosY = p_BaseLineY;
	newSkyline->baseLineWidth = p_BaseLineWidth;

	//Sort by x position
	K15_IA_QSORT(skylines, numSkylines, sizeof(K15_IASkyline), K15_IASortSkylineByXPos);

	//try to merge neighbor skylines with the same baseline (y pos)
	p_ImageAtlas->numSkylines = K15_IAMergeSkylines(skylines, numSkylines);

	return K15_IA_RESULT_SUCCESS;
}
/*********************************************************************************/
kia_internal kia_result K15_IATryToGrowAtlasSize(K15_ImageAtlas* p_ImageAtlas)
{
	kia_u32 width = p_ImageAtlas->width;
	kia_u32 height = p_ImageAtlas->height;
	kia_u32 numSkylines = p_ImageAtlas->numSkylines;
	kia_u32 widthExtend = 0;
	kia_u32 skylineIndex = 0;
	kia_u32 oldWidth = width;
	kia_b8 foundSkyline = K15_IA_FALSE;

	K15_IASkyline* skylines = p_ImageAtlas->skylines;
	K15_IASkyline* skyline = 0;

	if (width > height)
		height = height << 1;
	else
		width = width << 1;

	if (width > K15_IA_DIMENSION_THRESHOLD || height > K15_IA_DIMENSION_THRESHOLD)
		return K15_IA_RESULT_ATLAS_TOO_SMALL;

	widthExtend = width - oldWidth;

	p_ImageAtlas->width = width;
	p_ImageAtlas->height = height;

	//find skylines with pos == 0 (at the very bottom and extend their width)
	for (skylineIndex = 0;
		skylineIndex < numSkylines;
		++skylineIndex)
	{
		skyline = skylines + skylineIndex;

		if (skyline->baseLinePosY == 0)
		{
			skyline->baseLineWidth += widthExtend;
			foundSkyline = K15_IA_TRUE;
		}
	}

	if (!foundSkyline)
		K15_IATryToInsertSkyline(p_ImageAtlas, 0, oldWidth, widthExtend);

	return K15_IA_RESULT_SUCCESS;
}
/*********************************************************************************/
kia_internal kia_result K15_IATryToGrowAtlasSizeToFit(K15_ImageAtlas* p_ImageAtlas,
	kia_u32 p_MinWidth, kia_u32 p_MinHeight)
{
	kia_u32 width = p_ImageAtlas->width;
	kia_u32 height = p_ImageAtlas->height;

	if (height >= p_MinHeight &&
		width >= p_MinWidth)
	{
		return K15_IA_RESULT_SUCCESS;
	}

	while (height < p_MinHeight || width < p_MinWidth)
	{
		kia_result result = K15_IATryToGrowAtlasSize(p_ImageAtlas);

		if (result != K15_IA_RESULT_SUCCESS)
			return result;

		width = p_ImageAtlas->width;
		height = p_ImageAtlas->height;
	}

	return K15_IA_RESULT_SUCCESS;
}
/*********************************************************************************/
kia_u32 K15_IACalculatePlacementHeuristic(kia_u32 p_BaseLinePosX, kia_u32 p_BaseLinePosY, kia_u32 p_NodeWidth,
	kia_u32 p_NodeHeight, K15_IASkyline* p_Skylines, kia_u32 p_NumSkylines)
{
	kia_u32 heuristic = 0;
	kia_u32 skylineIndex = 0;
	K15_IASkyline* skyline = 0;

	for (skylineIndex = 0;
		skylineIndex < p_NumSkylines;
		++skylineIndex)
	{
		skyline = p_Skylines + skylineIndex;

		if (skyline->baseLinePosX >= skyline->baseLinePosX && skyline->baseLinePosX < p_BaseLinePosX + p_NodeWidth)
		{
			kia_u32 right = K15_IA_MIN(skyline->baseLinePosX + skyline->baseLineWidth, p_BaseLinePosX + p_NodeWidth);
			kia_u32 left = K15_IA_MAX(skyline->baseLinePosX, p_BaseLinePosX);

			kia_u32 width = right - left;
			kia_u32 height = p_BaseLinePosY - skyline->baseLinePosY;

			heuristic += width * height;
		}
	}

	return heuristic;
}
/*********************************************************************************/
kia_internal kia_u32 K15_IARemoveOrTrimWastedSpaceRect(K15_IARect* p_WastedSpaceRects,
	kia_u32 p_NumWastedSpaceRects, kia_u32 p_Index, kia_u32 p_Width, kia_u32 p_Height)
{
	K15_IARect* wastedSpaceRect = p_WastedSpaceRects + p_Index;

	if (wastedSpaceRect->width == p_Width &&
		wastedSpaceRect->height > p_Height)
	{
		wastedSpaceRect->posY += p_Height;
		wastedSpaceRect->height -= p_Height;
	}
	else if (wastedSpaceRect->height == p_Height &&
		wastedSpaceRect->width > p_Width)
	{
		wastedSpaceRect->posX += p_Width;
		wastedSpaceRect->width -= p_Width;
	}
	else
	{
		kia_u32 rectWidth = wastedSpaceRect->width;
		kia_u32 rectHeight = wastedSpaceRect->height;

		kia_u32 restHeight = wastedSpaceRect->height - p_Height;
		kia_u32 restWidth = wastedSpaceRect->width - p_Width;

		kia_u32 posLowerX = wastedSpaceRect->posX;
		kia_u32 posLowerY = wastedSpaceRect->posY + p_Height;
		kia_u32 posRightX = wastedSpaceRect->posX + p_Width;
		kia_u32 posRightY = wastedSpaceRect->posY;

		if (p_NumWastedSpaceRects > 1)
		{
			//Remove
			kia_u32 numElementsToShift = p_NumWastedSpaceRects - p_Index;
			K15_IA_MEMMOVE(p_WastedSpaceRects + p_Index, p_WastedSpaceRects + p_Index + 1,
				sizeof(K15_IARect) * numElementsToShift);
		}

		--p_NumWastedSpaceRects;

		if (restWidth != 0 && restHeight != 0)
		{
			if (restWidth > restHeight)
			{
				p_NumWastedSpaceRects = K15_IAAddWastedSpaceRect(p_WastedSpaceRects, p_NumWastedSpaceRects, posRightX, posRightY, restWidth, rectHeight);
				p_NumWastedSpaceRects = K15_IAAddWastedSpaceRect(p_WastedSpaceRects, p_NumWastedSpaceRects, posLowerX, posLowerY, p_Width, restHeight);
			}
			else
			{
				p_NumWastedSpaceRects = K15_IAAddWastedSpaceRect(p_WastedSpaceRects, p_NumWastedSpaceRects, posLowerX, posLowerY, rectWidth, restHeight);
				p_NumWastedSpaceRects = K15_IAAddWastedSpaceRect(p_WastedSpaceRects, p_NumWastedSpaceRects, posRightX, posRightY, restWidth, p_Height);
			}
		}
	}

	return p_NumWastedSpaceRects;
}
/*********************************************************************************/
kia_internal kia_b8 K15_IATryToFitInWastedSpace(K15_IARect* p_WastedSpaceRects, kia_u32* p_NumWastedSpaceRectsInOut,
	K15_IAImageNode* p_NodeToInsert)
{
	kia_u32 wastedRectWidth = 0;
	kia_u32 wastedRectHeight = 0;
	kia_u32 nodeWidth = p_NodeToInsert->rect.width;
	kia_u32 nodeHeight = p_NodeToInsert->rect.height;
	kia_u32 numWastedSpaceRects = *p_NumWastedSpaceRectsInOut;
	kia_u32 heuristic = 0;
	kia_u32 bestHeuristic = ~0;
	kia_u32 bestFitIndex = ~0;
	kia_u32 rectIndex = 0;
	K15_IARect* wastedSpaceRect = 0;

	for (rectIndex = 0;
		rectIndex < numWastedSpaceRects;
		++rectIndex)
	{
		wastedSpaceRect = p_WastedSpaceRects + rectIndex;
		wastedRectWidth = wastedSpaceRect->width;
		wastedRectHeight = wastedSpaceRect->height;

		if (wastedRectWidth >= nodeWidth &&
			wastedRectHeight >= nodeHeight)
		{
			heuristic = wastedRectWidth * wastedRectHeight;

			if (heuristic < bestHeuristic)
			{
				bestHeuristic = heuristic;
				bestFitIndex = rectIndex;
			}
			
			//we can't get better than this
			if (bestHeuristic == 0)
				break;
		}
	}

	if (bestFitIndex != ~0)
	{
		//copy position
		wastedSpaceRect = p_WastedSpaceRects + bestFitIndex;
		p_NodeToInsert->rect.posX = wastedSpaceRect->posX;
		p_NodeToInsert->rect.posY = wastedSpaceRect->posY;

		numWastedSpaceRects = K15_IARemoveOrTrimWastedSpaceRect(p_WastedSpaceRects,
			numWastedSpaceRects, bestFitIndex, nodeWidth, nodeHeight);
	}

	*p_NumWastedSpaceRectsInOut = numWastedSpaceRects;

	return (bestFitIndex != ~0);
}
/*********************************************************************************/
kia_internal kia_b8 K15_IACheckCollision(K15_IASkyline* p_Skylines, kia_u32 p_NumSkylines, kia_u32 p_BaseLinePosY,
	kia_u32 p_BaseLinePosX, kia_u32 p_Width)
{
	kia_u32 baseLinePosRight = p_BaseLinePosX + p_Width;
	kia_u32 skylineIndex = 0;
	K15_IASkyline* skyline = 0;
	kia_b8 collision = K15_IA_FALSE;

	for (skylineIndex = 0;
		skylineIndex < p_NumSkylines;
		++skylineIndex)
	{
		skyline = p_Skylines + skylineIndex;

		if (skyline->baseLinePosX > baseLinePosRight)
			break;

		if (skyline->baseLinePosY > p_BaseLinePosY)
		{
			collision = K15_IA_TRUE;
			break;
		}
	}

	return collision;
}
/*********************************************************************************/
kia_internal kia_result K15_IAAddImageToAtlasSkyline(K15_ImageAtlas* p_ImageAtlas, K15_IAImageNode* p_NodeToInsert,
	int* p_OutX, int* p_OutY)
{
	kia_result result = K15_IA_RESULT_ATLAS_TOO_SMALL;
	kia_u32 numSkylines = p_ImageAtlas->numSkylines;
	kia_u32 numImageNodes = p_ImageAtlas->numImageNodes;
	kia_u32 numWastedSpaceRects = p_ImageAtlas->numWastedSpaceRects;
	kia_u32 height = p_ImageAtlas->height;
	kia_u32 width = p_ImageAtlas->width;
	kia_u32 lowerPixelSpace = 0;
	kia_u32 nodeWidth = p_NodeToInsert->rect.width;
	kia_u32 nodeHeight = p_NodeToInsert->rect.height;
	kia_u32 baseLinePosY = 0;
	kia_u32 baseLinePosX = 0;
	kia_u32 baseLineWidth = 0;
	kia_u32 heuristic = 0;
	kia_u32 bestHeuristic = ~0;
	kia_u32 bestFitIndex = ~0;
	kia_u32 skylineIndex = 0;
	kia_u32 numSkylinesLeft = 0;
	K15_IASkyline* skyline = 0;
	K15_IASkyline* skylines = p_ImageAtlas->skylines;
	K15_IASkyline* nextSkyline = 0;
	K15_IAImageNode* imageNodes = p_ImageAtlas->imageNodes;
	K15_IARect* wastedSpaceRects = p_ImageAtlas->wastedSpaceRects;

	kia_b8 fitsInWastedSpace = K15_IATryToFitInWastedSpace(wastedSpaceRects,
		&p_ImageAtlas->numWastedSpaceRects, p_NodeToInsert);

	kia_b8 nodeCollides = K15_IA_FALSE;
	if (!fitsInWastedSpace)
	{
		for (skylineIndex = 0;
			skylineIndex < numSkylines;
			++skylineIndex)
		{
			skyline = skylines + skylineIndex;
			baseLinePosY = skyline->baseLinePosY;
			baseLinePosX = skyline->baseLinePosX;
			baseLineWidth = skyline->baseLineWidth;
			lowerPixelSpace = height - baseLinePosY;

			nodeCollides = K15_IA_FALSE;

			//check if image fits vertically and does not go out of the atlas if placed
			//on this skyline
			if (lowerPixelSpace >= nodeHeight &&
				baseLinePosX + nodeWidth <= width)
			{
				//we need additional checks if the image is wider than the current skyline
				if (baseLineWidth < nodeWidth)
				{
					//due to the nature of the algorithm we just need to check the next skylines.
					//if they're vertically smaller than the current skyline, we can safely place
					//the image at this position. However, this would introduce wasted space
					//(which we track)

					//No need to check. If this would be the last skyline (and we therefore would 
					//access out of bounds here), we would never end in this code as the node
					//would exceed the atlas width.
					nextSkyline = skyline + 1;
					numSkylinesLeft = (numSkylines - skylineIndex - 1);

					nodeCollides = K15_IACheckCollision(nextSkyline, numSkylinesLeft,
						baseLinePosY, baseLinePosX, nodeWidth);
				}

				if (!nodeCollides)
				{
					//node potentially fits. Calculate and save heuristic
					heuristic = K15_IACalculatePlacementHeuristic(baseLinePosX, baseLinePosY, 
						nodeWidth, nodeHeight, skylines, numSkylines);

					if (heuristic < bestHeuristic)
					{
						bestHeuristic = heuristic;
						bestFitIndex = skylineIndex;
					}

					//we can't get better than this
					if (bestHeuristic == 0)
						break;
				}
			}
		}

		if (bestFitIndex != ~0)
		{
			skyline = skylines + bestFitIndex;
			p_NodeToInsert->rect.posX = skyline->baseLinePosX;
			p_NodeToInsert->rect.posY = skyline->baseLinePosY;

			if (skyline->baseLineWidth > p_NodeToInsert->rect.width)
			{
				skyline->baseLinePosX += p_NodeToInsert->rect.width;
				skyline->baseLineWidth -= p_NodeToInsert->rect.width;
			}
			else
			{
				p_ImageAtlas->numSkylines = K15_IARemoveSkylineByIndex(skylines, numSkylines,
					bestFitIndex);
			}

			result = K15_IATryToInsertSkyline(p_ImageAtlas, p_NodeToInsert->rect.posY + p_NodeToInsert->rect.height,
				p_NodeToInsert->rect.posX, p_NodeToInsert->rect.width);

			numSkylines = p_ImageAtlas->numSkylines;
		}
	}
	else
	{
		result = K15_IA_RESULT_SUCCESS;
	}

	if (result == K15_IA_RESULT_SUCCESS)
	{
		if (p_OutX)
			*p_OutX = p_NodeToInsert->rect.posX;

		if (p_OutY)
			*p_OutY = p_NodeToInsert->rect.posY;

		//remove/trim any skylines that would be obscured by the new skyline
		K15_IAFindWastedSpaceAndRemoveObscuredSkylines(skylines, &numSkylines,
			wastedSpaceRects, &p_ImageAtlas->numWastedSpaceRects,
			p_NodeToInsert->rect.posX, p_NodeToInsert->rect.posY,
			p_NodeToInsert->rect.width);

		p_ImageAtlas->numSkylines = numSkylines;
	}

	return result;
}
/*********************************************************************************/




/*********************************************************************************/
kia_def kia_result K15_IACreateAtlas(K15_ImageAtlas* p_OutTextureAtlas, kia_u32 p_NumImages)
{
	kia_u32 memoryBufferSizeInBytes = 0;
	kia_byte* memoryBuffer = 0;
	kia_result result;

	if (p_NumImages == 0)
		return K15_IA_RESULT_INVALID_ARGUMENTS;

	memoryBufferSizeInBytes = K15_IACalculateAtlasMemorySizeInBytes(p_NumImages);
	memoryBuffer = (kia_byte*)K15_IA_MALLOC(memoryBufferSizeInBytes);

	if (!memoryBuffer)
		return K15_IA_RESULT_OUT_OF_MEMORY;

	result = K15_IACreateAtlasWithCustomMemory(p_OutTextureAtlas, p_NumImages, memoryBuffer);

	if (result != K15_IA_RESULT_SUCCESS)
		K15_IA_FREE(memoryBuffer);
	else
		p_OutTextureAtlas->flags &= ~KIA_EXTERNAL_MEMORY_FLAG; //Erase 'KIA_EXTERNAL_MEMORY_FLAG' flag

	return result;
}
/*********************************************************************************/
kia_def kia_result K15_IACreateAtlasWithCustomMemory(K15_ImageAtlas* p_OutImageAtlas, kia_u32 p_NumImages,
	void* p_MemoryBuffer)
{
	K15_ImageAtlas atlas = {0};

	kia_byte* memoryBuffer = (kia_byte*)p_MemoryBuffer;

	kia_u32 memoryBufferSizeInBytes = K15_IACalculateAtlasMemorySizeInBytes(p_NumImages);
	kia_u32 skylineMemoryBufferOffset = sizeof(K15_IAImageNode) * p_NumImages;
	kia_u32 wastedSpaceMemoryBufferOffset = skylineMemoryBufferOffset + sizeof(K15_IASkyline) * K15_IA_MAX_SKYLINES;

	if (!p_OutImageAtlas || p_NumImages == 0 || !p_MemoryBuffer)
	{
		return K15_IA_RESULT_INVALID_ARGUMENTS;
	}

	//clear memory
	K15_IA_MEMSET(p_MemoryBuffer, 0, memoryBufferSizeInBytes);

	atlas.height = K15_IA_DEFAULT_MIN_ATLAS_DIMENSION;
	atlas.width = K15_IA_DEFAULT_MIN_ATLAS_DIMENSION;
	atlas.numWastedSpaceRects = 0;
	atlas.numMaxImageNodes = p_NumImages;
	atlas.numImageNodes = 0;
	atlas.numSkylines = 0;
	atlas.imageNodes = (K15_IAImageNode*)(memoryBuffer);
	atlas.skylines = (K15_IASkyline*)(memoryBuffer + skylineMemoryBufferOffset);
	atlas.wastedSpaceRects = (K15_IARect*)(memoryBuffer + wastedSpaceMemoryBufferOffset);
	atlas.flags = KIA_EXTERNAL_MEMORY_FLAG;

	K15_IATryToInsertSkyline(&atlas, 0, 0, K15_IA_DEFAULT_MIN_ATLAS_DIMENSION);

	*p_OutImageAtlas = atlas;

	return K15_IA_RESULT_SUCCESS;
}
/*********************************************************************************/
kia_def kia_u32 K15_IACalculateAtlasMemorySizeInBytes(kia_u32 p_NumImages)
{
	kia_u32 imageNodeDataSizeInBytes = p_NumImages * sizeof(K15_IAImageNode);
	kia_u32 skylineDataSizeInBytes = K15_IA_MAX_SKYLINES * sizeof(K15_IASkyline);
	kia_u32 wastedSpaceRectsSizeInBytes = K15_IA_MAX_WASTED_SPACE_RECTS * sizeof(K15_IARect);

	return imageNodeDataSizeInBytes + skylineDataSizeInBytes + wastedSpaceRectsSizeInBytes;
}
/*********************************************************************************/
kia_def kia_u32 K15_IACalculateAtlasPixelDataSizeInBytes(K15_ImageAtlas* p_ImageAtlas,
	K15_IAPixelFormat p_PixelFormat)
{
	kia_u32 numPixels = p_ImageAtlas->width * p_ImageAtlas->height;
	kia_u32 pixelDataSizeInBytes = numPixels * p_PixelFormat;

	return pixelDataSizeInBytes;
}
/*********************************************************************************/
kia_def void K15_IAFreeAtlas(K15_ImageAtlas* p_ImageAtlas)
{
	if (!p_ImageAtlas)
		return;

	if ((p_ImageAtlas->flags & KIA_EXTERNAL_MEMORY_FLAG) == 0)
		K15_IA_FREE(p_ImageAtlas->imageNodes); //points to the start of the memory buffer
}
/*********************************************************************************/
kia_def kia_result K15_IAAddImageToAtlas(K15_ImageAtlas* p_ImageAtlas, K15_IAPixelFormat p_PixelFormat,
	void* p_PixelData, kia_u32 p_PixelDataWidth, kia_u32 p_PixelDataHeight,
	int* p_OutX, int* p_OutY)
{
	kia_result result = K15_IA_RESULT_ATLAS_TOO_SMALL;
	kia_result growResult = K15_IA_RESULT_SUCCESS;

	kia_u32 imageNodeIndex = 0;
	K15_IAImageNode* imageNode = 0;

	if (!p_ImageAtlas || !p_PixelData || p_PixelDataWidth == 0 || p_PixelDataHeight == 0 ||
		!p_OutX || !p_OutY)
	{
		return K15_IA_RESULT_INVALID_ARGUMENTS;
	}

	if (p_ImageAtlas->numImageNodes == p_ImageAtlas->numMaxImageNodes)
		return K15_IA_RESULT_OUT_OF_RANGE;

	imageNodeIndex = p_ImageAtlas->numImageNodes;
	imageNode = p_ImageAtlas->imageNodes + imageNodeIndex;

	imageNode->pixelData = (kia_byte*)p_PixelData;
	imageNode->pixelDataFormat = p_PixelFormat;
	imageNode->rect.height = p_PixelDataHeight;
	imageNode->rect.width = p_PixelDataWidth;

	while (result != K15_IA_RESULT_SUCCESS)
	{
		result = K15_IAAddImageToAtlasSkyline(p_ImageAtlas, imageNode, p_OutX, p_OutY);

		if (result == K15_IA_RESULT_ATLAS_TOO_SMALL)
			growResult = K15_IATryToGrowAtlasSize(p_ImageAtlas);
		else if (growResult != K15_IA_RESULT_SUCCESS)
		{
			result = growResult;
			break;
		}
	}

	if (result == K15_IA_RESULT_SUCCESS)
		++p_ImageAtlas->numImageNodes;

	return result;
}
/*********************************************************************************/
kia_def void K15_IABakeImageAtlasIntoPixelBuffer(K15_ImageAtlas* p_ImageAtlas,
	K15_IAPixelFormat p_DestinationPixelFormat, void* p_DestinationPixelData,
	int* p_OutWidth, int* p_OutHeight)
{

	kia_u32 atlasHeight = 0;
	kia_u32 atlasStride = 0;
	kia_u32 atlasPixelDataIndex = 0;
	kia_u32 destinationPixelDataIndex = 0;
	kia_u32 destinationPixelDataOffset = 0;
	kia_u32 imageNodePixelDataOffset = 0;
	kia_u32 numImageNodes = 0;
	kia_u32 imageNodeWidth = 0;
	kia_u32 imageNodeHeight = 0;
	kia_u32 imageNodePosX = 0;
	kia_u32 imageNodePosY = 0;
	kia_u32 nodeIndex = 0;
	kia_u32 strideIndex = 0;
	kia_byte* imageNodePixelData = 0;
	kia_byte* destinationPixelData = 0;

	K15_IAPixelFormat imageNodePixelFormat = KIA_PIXEL_FORMAT_R8;
	K15_IAImageNode* imageNodes = 0;
	K15_IAImageNode* imageNode = 0;

	if (!p_ImageAtlas || !p_DestinationPixelData)
		return;

	destinationPixelData = (kia_byte*)p_DestinationPixelData;

	atlasHeight = p_ImageAtlas->height;
	atlasStride = p_ImageAtlas->width;
	numImageNodes = p_ImageAtlas->numImageNodes;

	K15_IA_MEMSET(destinationPixelData, 0, p_DestinationPixelFormat * atlasHeight * atlasStride);

	imageNodes = p_ImageAtlas->imageNodes;

	for (nodeIndex = 0;
		nodeIndex < numImageNodes;
		++nodeIndex)
	{
		imageNode = imageNodes + nodeIndex;

		imageNodeWidth = imageNode->rect.width;
		imageNodeHeight = imageNode->rect.height;
		imageNodePosX = imageNode->rect.posX;
		imageNodePosY = imageNode->rect.posY;
		imageNodePixelFormat = imageNode->pixelDataFormat;
		imageNodePixelData = imageNode->pixelData;

		destinationPixelDataOffset = (imageNodePosX + (imageNodePosY * atlasStride)) * p_DestinationPixelFormat;
		imageNodePixelDataOffset = 0;

		//Convert pixels if formats mismatch
		if (imageNodePixelFormat != p_DestinationPixelFormat)
		{
			for (strideIndex = 0;
				strideIndex < imageNodeHeight;
				++strideIndex)
			{
				K15_IAConvertPixelData(destinationPixelData + destinationPixelDataOffset,
					imageNodePixelData + imageNodePixelDataOffset, p_DestinationPixelFormat,
					imageNodePixelFormat, imageNodeWidth);

				destinationPixelDataOffset += p_DestinationPixelFormat * atlasStride;
				imageNodePixelDataOffset += imageNodePixelFormat * imageNodeWidth;
			}
		}
		else
		{
			for (strideIndex = 0;
				strideIndex < imageNodeHeight;
				++strideIndex)
			{
				K15_IA_MEMCPY(destinationPixelData + destinationPixelDataOffset,
					imageNodePixelData + imageNodePixelDataOffset, imageNodeWidth * imageNodePixelFormat);

				destinationPixelDataOffset += p_DestinationPixelFormat * atlasStride;
				imageNodePixelDataOffset += p_DestinationPixelFormat * imageNodeWidth;
			}
		}
	}

	if (p_OutWidth)
		*p_OutWidth = atlasStride;

	if (p_OutHeight)
		*p_OutHeight = atlasHeight;
}
/*********************************************************************************/
#endif //K15_IMAGE_ATLAS_IMPLEMENTATION
#endif //_K15_ImageAtlas_h_