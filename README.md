# What problem is this library trying to solve? (ELI5) #
This library can be used to generate a single image that contains many, smaller images. The library will try to pack the smaller images
as tightly as possible in a performant manner. This newly created image is often called an 'Atlas'. Hence the name of the library.  

# How do I add this library to my project? #
This library is a single header library which means that you just have to #include it into your project. However, you have to add the implementation
of the functions used into *one* of your C/CPP files. To do this you have to add #define K15_IA_IMPLEMENTATION *before* the #include.  


```
#!c
...
#define K15_IA_IMPLEMENTATION
#include "K15_ImageAtlas.h"
...
```

# How does this library work? #
This library does implement the 'Skyline Left-Bottom' packing algorithm. I implemented the algorithm roughly using the paper
'A Skyline-Based Heuristic for the 2D Rectangular Strip Packing Problem' by Wei Lijun, Andrew Lim and Wenbin Zhu. 

# Example Project #
The example in the 'example' folder is currently Win32 only and currently only used for testing.  
However, if you want to try it out, just execute the build.bat script (visual studio required).
Pressing any key in the example app will add a new rectangle to the atlas.  

# Basic C example #
```
#!c
const int numImagesToAdd = 256;

//has already been filled
unsigned char* imagesToAdd[numImagesToAdd];
int imagesToAddWidths[numImagesToAdd];
int imagesToAddHeights[numImagesToAdd];

K15_IAImageAtlas atlas = {0};
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
```

# Results #
![extended_ascii_table_arial.png](https://bitbucket.org/repo/RL5dzp/images/2549034388-extended_ascii_table_arial.png) ![icons.png](https://bitbucket.org/repo/RL5dzp/images/3521530839-icons.png)
# License #
This software is in the public domain. You are granted a perpetual, irrevocable license to copy
and modify this file however you want.