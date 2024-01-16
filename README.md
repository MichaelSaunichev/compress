Description:

Accepts bmp images and compresses them to cbmp images using huffman encoding. The compression process additionally turns the images grey. Cbmps contain pixel frequency data and the program accepts those cbmps and uses that frequncy data to decompress the images

Usage:

First run with ./compress lion.bmp to create lion.cbmp, which has image data for a grey version of lion.bmp

Next run with ./compress lion.cbmp to create lion_uncompressed.bmp, which will be a grey image of the original lion.bmp
