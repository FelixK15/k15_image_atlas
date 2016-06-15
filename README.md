# What problem is this library trying to solve? (ELI5)
This library can be used to generate a single image that contains many, smaller images. The library will try to pack the smaller images
as tightly as possible in a performant manner. This newly created image is often called an 'Atlas'. Hence the name of the library.  

# How do I add this library to my project?
This library is a single header library which means that you just have to #include it into your project. However, you have to add the implementation
of the functions used into *one* of your C/CPP files. To do this you have to add #define K15_IA_IMPLEMENTATION *before* the #include.  

# How does this library work?
This library does implement the 'Skyline Left-Bottom' packing algorithm. I implemented the algorithm roughly using the paper
'A Skyline-Based Heuristic for the 2D Rectangular Strip Packing Problem' by Wei Lijun, Andrew Lim and Wenbin Zhu. 

# License
This software is in the public domain. Where that dedication is not recognized, you are granted a perpetual, irrevocable license to copy
and modify this file however you want.  
