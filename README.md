Description:
Accepts bmp images and compresses them to cbmp images using huffman encoding. These cbmps contain the 
  *The compression process additionally turns the image grey
Additionally accepts those cbmp images and decompresses them.

Usage:
First run with ./compress lion.bmp to create lion.cbmp, which has image data for a grey version of lion.bmp
Next run with ./compress lion.cbmp to create lion_uncompressed.bmp, which will be a grey image of the original lion.bmp
