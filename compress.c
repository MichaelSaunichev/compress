#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

typedef struct Node{
    int num;
    int frequency;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct SymbolCode {
    int symbol;
    char code[256];
} SymbolCode;

typedef struct {
    int width;
    int height;
    unsigned char* data;
} Image;

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

struct tagBITMAPFILEHEADER
{
WORD bfType; /*specifies the file type*/
DWORD bfSize; /*specifies the size in bytes of the bitmap file*/
WORD bfReserved1; /*reserved; must be 0*/
WORD bfReserved2; /*reserved; must be 0*/
DWORD bfOffBits; /*species the offset in bytes from the bitmapfileheader to the bitmap bits*/
};

struct tagBITMAPINFOHEADER
{
DWORD biSize; /*specifies the number of bytes required by the struct*/
LONG biWidth; /*specifies width in pixels*/
LONG biHeight; /*species height in pixels*/
WORD biPlanes; /*specifies the number of color planes, must be 1*/
WORD biBitCount; /*specifies the number of bit per pixel*/
DWORD biCompression;/*spcifies the type of compression*/
DWORD biSizeImage; /*size of image in bytes*/
LONG biXPelsPerMeter; /*number of pixels per meter in x axis*/
LONG biYPelsPerMeter; /*number of pixels per meter in y axis*/
DWORD biClrUsed; /*number of colors used by th ebitmap*/
DWORD biClrImportant; /*number of colors that are important*/
};

Image *read_bmp(char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file '%s'\n", filename);
        return NULL;
    }

    struct tagBITMAPFILEHEADER file_header;
    struct tagBITMAPINFOHEADER info_header;

    fread(&file_header.bfType, sizeof(WORD), 1, fp);

    if (file_header.bfType != 19778){
        printf("Not BMP file");
        return NULL;
    }

    fread(&file_header.bfSize, sizeof(DWORD), 1, fp);
    fread(&file_header.bfReserved1, sizeof(WORD), 1, fp);
    fread(&file_header.bfReserved2, sizeof(WORD), 1, fp);
    fread(&file_header.bfOffBits, sizeof(DWORD), 1, fp);
    fread(&info_header.biSize, sizeof(DWORD), 1, fp);
    fread(&info_header.biWidth, sizeof(LONG), 1, fp);
    fread(&info_header.biHeight, sizeof(LONG), 1, fp);
    fread(&info_header.biPlanes, sizeof(WORD), 1, fp);
    fread(&info_header.biBitCount, sizeof(WORD), 1, fp);
    fread(&info_header.biCompression, sizeof(DWORD), 1, fp);
    fread(&info_header.biSizeImage, sizeof(DWORD), 1, fp);
    fread(&info_header.biXPelsPerMeter, sizeof(LONG), 1, fp);
    fread(&info_header.biYPelsPerMeter, sizeof(LONG), 1, fp);
    fread(&info_header.biClrUsed, sizeof(DWORD), 1, fp);
    fread(&info_header.biClrImportant, sizeof(DWORD), 1, fp);

    int byte_per_pixel = info_header.biBitCount/8;
    int row_padding = (4 - (info_header.biWidth * byte_per_pixel) % 4) % 4;

    /* Malloc for image data */
    unsigned char* data = (unsigned char*)malloc(info_header.biWidth * info_header.biHeight * byte_per_pixel);

    /* Read from bmp and put into data */
    int i, j;
    for (i = 0; i < info_header.biHeight; i++) {
        for (j = 0; j < info_header.biWidth; j++) {
            fread(&data[((info_header.biHeight - 1 - i) * info_header.biWidth + j) * byte_per_pixel], sizeof(unsigned char), byte_per_pixel, fp);
        }
    fseek(fp, row_padding, SEEK_CUR);
    }

    /* Create and return image pointer */
    Image* image = (Image*)malloc(sizeof(Image));
    image->width = info_header.biWidth;
    image->height = info_header.biHeight;
    image->data = data;
    fclose(fp);
    return image;
}

unsigned char* get_color(Image* image, int x, int y) {
    int byte_per_pixel = 1;
    int index = (image->height - 1 - y) * image->width + x;
    return &image->data[index * byte_per_pixel];
}

void image_to_bmp(Image* image, const char* filename_h, const char* filename) {
    struct tagBITMAPFILEHEADER file_header;
    struct tagBITMAPINFOHEADER info_header;

    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to create file '%s'\n", filename);
        return;
    }

    /* Get and set BMP file header values */
    FILE* fp_h = fopen(filename_h, "rb");
    if (fp_h == NULL) {
        fprintf(stderr, "Failed to open file '%s'\n", filename_h);
        return;
    }

    fread(&file_header.bfType, sizeof(WORD), 1, fp_h);
    fwrite(&file_header.bfType, sizeof(WORD), 1, fp);

    fread(&file_header.bfSize, sizeof(DWORD), 1, fp_h);
    fwrite(&file_header.bfSize, sizeof(DWORD), 1, fp);

    fread(&file_header.bfReserved1, sizeof(WORD), 1, fp_h);
    fwrite(&file_header.bfReserved1, sizeof(WORD), 1, fp);

    fread(&file_header.bfReserved2, sizeof(WORD), 1, fp_h);
    fwrite(&file_header.bfReserved2, sizeof(WORD), 1, fp);

    fread(&file_header.bfOffBits, sizeof(DWORD), 1, fp_h);
    fwrite(&file_header.bfOffBits, sizeof(DWORD), 1, fp);

    fread(&info_header.biSize, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biSize, sizeof(DWORD), 1, fp);

    fread(&info_header.biWidth, sizeof(LONG), 1, fp_h);
    fwrite(&info_header.biWidth, sizeof(LONG), 1, fp);

    fread(&info_header.biHeight, sizeof(LONG), 1, fp_h);
    fwrite(&info_header.biHeight, sizeof(LONG), 1, fp);

    fread(&info_header.biPlanes, sizeof(WORD), 1, fp_h);
    fwrite(&info_header.biPlanes, sizeof(WORD), 1, fp);

    fread(&info_header.biBitCount, sizeof(WORD), 1, fp_h);
    fwrite(&info_header.biBitCount, sizeof(WORD), 1, fp);

    fread(&info_header.biCompression, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biCompression, sizeof(DWORD), 1, fp);

    fread(&info_header.biSizeImage, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biSizeImage, sizeof(DWORD), 1, fp);

    fread(&info_header.biXPelsPerMeter, sizeof(LONG), 1, fp_h);
    fwrite(&info_header.biXPelsPerMeter, sizeof(LONG), 1, fp);

    fread(&info_header.biYPelsPerMeter, sizeof(LONG), 1, fp_h);
    fwrite(&info_header.biYPelsPerMeter, sizeof(LONG), 1, fp);

    fread(&info_header.biClrUsed, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biClrUsed, sizeof(DWORD), 1, fp);

    fread(&info_header.biClrImportant, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biClrImportant, sizeof(DWORD), 1, fp);


    /* Write image data to file */
    int byte_per_pixel = 1;
    int row_padding = (4 - (image->width * byte_per_pixel) % 4) % 4;
    unsigned char padding[3] = {0, 0, 0};
    int i, j;
    for (i = 0; i < image->height; i++) {
        for (j = 0; j < image->width; j++) {
            unsigned char* color_data = get_color(image, j, i);
            fwrite(color_data, sizeof(unsigned char), 1, fp);
            fwrite(color_data, sizeof(unsigned char), 1, fp);
            fwrite(color_data, sizeof(unsigned char), 1, fp);
        }
        fwrite(padding, sizeof(unsigned char), row_padding, fp);
    }
    fclose(fp);
}

Image* turn_grey(Image* image) {
    int i;
    int red;
    int green;
    int blue;
    int size_original = image->width * image->height * 3;
    unsigned char* data = mmap(NULL, (sizeof(unsigned char) * size_original)/3, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);

    pid_t pid = fork();
    if(pid == 0){
        for(i = 0; i<((size_original/3)/2); i++){
            red = image->data[i*3];
            green = image->data[i*3+1];
            blue = image->data[i*3+2];

            data[i] = ((red + green + blue) / 3);
        }
        exit(0);
    }
    else{
        for(i = (size_original/3)/2; i<size_original/3; i++){
            red = image->data[i*3];
            green = image->data[i*3+1];
            blue = image->data[i*3+2];
            data[i] = ((red + green + blue) / 3);
        }
        wait(0);
    }
    Image* new_image = (Image*) malloc(sizeof(Image));
    new_image->height = (image->height);
    new_image->width = (image->width);
    new_image->data = data;

    return new_image;
}

Node* createNode(int num, int frequency) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->num = num;
    newNode->frequency = frequency;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

Node* buildHuffmanTree(Image* image) {
    int arr[256] = {0};
    int i;
    int j;
    int size = image->height*image->width*3;
    for(i=0; i<size/3; i++){
        arr[image->data[i]] +=1 ;
    }

    Node *nodes[256];
    for (i = 0; i < 256; i++) {
        if (arr[i] > 0) {
            nodes[i] = createNode(i, arr[i]);
        } else {
            nodes[i] = NULL;
        }
    }

    int min1;
    int min2;
    
    while (1) {
        min1 = -1;
        min2 = -1;
        for (i = 0; i < 256; i++) {
            if (nodes[i] != NULL) {
                if ((min1 == -1) || (nodes[i]->frequency) < (nodes[min1]->frequency)){
                    min2 = min1;
                    min1 = i;
                } else if ((min2 == -1) || (nodes[i]->frequency) < (nodes[min2]->frequency)) {
                    min2 = i;
                }
            }
        }
        if (min2 == -1)
            break;
        
        Node* newNode = createNode(-1, nodes[min1]->frequency + nodes[min2]->frequency);
        newNode->left = nodes[min1];
        newNode->right = nodes[min2];
        
        nodes[min1] = newNode;
        nodes[min2] = NULL;
    }
    return nodes[min1];
}

void createHuffmanCodes(Node* root, int code[], int top, SymbolCode symbolCodes[], int* numCodes) {
    if (root->left) {
        code[top] = 0;
        createHuffmanCodes(root->left, code, top + 1, symbolCodes, numCodes);
    }
    
    if (root->right) {
        code[top] = 1;
        createHuffmanCodes(root->right, code, top + 1, symbolCodes, numCodes);
    }
    
    if (root->num != -1) {
        SymbolCode codePair;
        codePair.symbol = root->num;
        
        int i;
        for (i = 0; i < top; i++) {
            codePair.code[i] = code[i] == 0 ? '0' : '1';
        }
        codePair.code[top] = '\0';
        
        symbolCodes[*numCodes] = codePair;
        (*numCodes)++;
    }
}

void compressImage(Image* image, SymbolCode symbolCodes[], int numCodes, unsigned char** compressedData, int* compressedLength) {
    int maxCompressedLength = (image->width * image->height) * 8;
    *compressedData = (unsigned char*)malloc(maxCompressedLength);
    
    int compressedIndex = 0;
    unsigned char currentByte = 0;
    
    int i, j;
    for (i = 0; i < (image->height); i++) {
        for (j = 0; j < image->width; j++) {
            int pixelValue = image->data[i * image->width + j];
            SymbolCode symbolCode;
            int found = 0;
            
            int k;
            for (k = 0; k < numCodes; k++) {
                if (symbolCodes[k].symbol == pixelValue) {
                    symbolCode = symbolCodes[k];
                    found = 1;
                    break;
                }
            }
            
            if (!found) {
                printf("Symbol code not found for pixel value: %d\n", pixelValue);
                continue;
            }
            
            int codeLength = strlen(symbolCode.code);
            for (k = 0; k < codeLength; k++) {
                currentByte = (currentByte << 1) | (symbolCode.code[k] - '0');
                if (++compressedIndex % 8 == 0) {
                    (*compressedData)[compressedIndex / 8 - 1] = currentByte;
                    currentByte = 0;
                }
            }
        }
    }
    
    if (compressedIndex % 8 != 0) {
        currentByte = currentByte << (8 - (compressedIndex % 8));
        (*compressedData)[compressedIndex / 8] = currentByte;
    }
    
    *compressedLength = (compressedIndex + 7) / 8;
}

void huffmanImageToBMP(unsigned char* data, const char* filename_h, const char* filename, int frequencies[], int compressed_length) {
    struct tagBITMAPFILEHEADER file_header;
    struct tagBITMAPINFOHEADER info_header;

    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to create file '%s'\n", filename);
        return;
    }

    /* Get and set BMP file header values */
    FILE* fp_h = fopen(filename_h, "rb");
    if (fp_h == NULL) {
        fprintf(stderr, "Failed to open file '%s'\n", filename_h);
        return;
    }

    fread(&file_header.bfType, sizeof(WORD), 1, fp_h);
    fwrite(&file_header.bfType, sizeof(WORD), 1, fp);

    fread(&file_header.bfSize, sizeof(DWORD), 1, fp_h);
    fwrite(&file_header.bfSize, sizeof(DWORD), 1, fp);

    fread(&file_header.bfReserved1, sizeof(WORD), 1, fp_h);
    fwrite(&file_header.bfReserved1, sizeof(WORD), 1, fp);

    fread(&file_header.bfReserved2, sizeof(WORD), 1, fp_h);
    fwrite(&file_header.bfReserved2, sizeof(WORD), 1, fp);

    fread(&file_header.bfOffBits, sizeof(DWORD), 1, fp_h);
    fwrite(&file_header.bfOffBits, sizeof(DWORD), 1, fp);

    fread(&info_header.biSize, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biSize, sizeof(DWORD), 1, fp);

    fread(&info_header.biWidth, sizeof(LONG), 1, fp_h);
    fwrite(&info_header.biWidth, sizeof(LONG), 1, fp);

    fread(&info_header.biHeight, sizeof(LONG), 1, fp_h);
    fwrite(&info_header.biHeight, sizeof(LONG), 1, fp);

    fread(&info_header.biPlanes, sizeof(WORD), 1, fp_h);
    fwrite(&info_header.biPlanes, sizeof(WORD), 1, fp);

    fread(&info_header.biBitCount, sizeof(WORD), 1, fp_h);
    fwrite(&info_header.biBitCount, sizeof(WORD), 1, fp);

    fread(&info_header.biCompression, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biCompression, sizeof(DWORD), 1, fp);

    fread(&info_header.biSizeImage, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biSizeImage, sizeof(DWORD), 1, fp);

    fread(&info_header.biXPelsPerMeter, sizeof(LONG), 1, fp_h);
    fwrite(&info_header.biXPelsPerMeter, sizeof(LONG), 1, fp);

    fread(&info_header.biYPelsPerMeter, sizeof(LONG), 1, fp_h);
    fwrite(&info_header.biYPelsPerMeter, sizeof(LONG), 1, fp);

    fread(&info_header.biClrUsed, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biClrUsed, sizeof(DWORD), 1, fp);

    fread(&info_header.biClrImportant, sizeof(DWORD), 1, fp_h);
    fwrite(&info_header.biClrImportant, sizeof(DWORD), 1, fp);

    /* Write frequency information */
    fwrite(frequencies, sizeof(int), 256, fp);
    fwrite(data, 1, compressed_length, fp);

    fclose(fp_h);
    fclose(fp);
}



Node* rebuild_huffman_tree(int frequency_data[]){
    int i;
    Node *nodes[256];
    for (i = 0; i < 256; i++) {
        if (frequency_data[i] > 0) {
            nodes[i] = createNode(i, frequency_data[i]);
        } else {
            nodes[i] = NULL;
        }
    }

    int min1;
    int min2;
    
    while (1) {
        min1 = -1;
        min2 = -1;
        for (i = 0; i < 256; i++) {
            if (nodes[i] != NULL) {
                if (min1 == -1 || nodes[i]->frequency < nodes[min1]->frequency) {
                    min2 = min1;
                    min1 = i;
                } else if (min2 == -1 || nodes[i]->frequency < nodes[min2]->frequency) {
                    min2 = i;
                }
            }
        }
        if (min2 == -1)
            break;
        
        Node* newNode = createNode(-1, nodes[min1]->frequency + nodes[min2]->frequency);
        newNode->left = nodes[min1];
        newNode->right = nodes[min2];
        
        nodes[min1] = newNode;
        nodes[min2] = NULL;
    }
    return nodes[min1];
}

void reconstructImageData(FILE* fp, int* frequency_data, Image* image) {
    int width = image->width;
    int height = image->height;
    int num_pixels = width * height;
    image->data = (unsigned char*)malloc(num_pixels * sizeof(unsigned char));
    Node* root = rebuild_huffman_tree(frequency_data);
    SymbolCode symbolCodes[256];
    int code[256];
    int numCodes = 0;
    createHuffmanCodes(root, code, 0, symbolCodes, &numCodes);


    int current_bit = 0;
    unsigned char current_byte = 0;
    int pixel_index = 0;

    Node* currentNode = root;

    while (pixel_index < num_pixels) {
        if (current_bit == 0) {
            fread(&current_byte, sizeof(unsigned char), 1, fp);
            current_bit = 8;
        }

        int bit = (current_byte >> (current_bit - 1)) & 1;
        current_bit--;

        currentNode = (bit == 0) ? currentNode->left : currentNode->right;

        if (currentNode->left == NULL && currentNode->right == NULL) {
            image->data[pixel_index] = currentNode->num;
            pixel_index++;
            currentNode = root;
        }
    }
}

Image *read_compressed(char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file '%s'\n", filename);
        return NULL;
    }

    struct tagBITMAPFILEHEADER file_header;
    struct tagBITMAPINFOHEADER info_header;

    fread(&file_header.bfType, sizeof(WORD), 1, fp);

    if (file_header.bfType != 19778){
        printf("Not BMP file");
        return NULL;
    }

    fread(&file_header.bfSize, sizeof(DWORD), 1, fp);
    fread(&file_header.bfReserved1, sizeof(WORD), 1, fp);
    fread(&file_header.bfReserved2, sizeof(WORD), 1, fp);
    fread(&file_header.bfOffBits, sizeof(DWORD), 1, fp);
    fread(&info_header.biSize, sizeof(DWORD), 1, fp);
    fread(&info_header.biWidth, sizeof(LONG), 1, fp);
    fread(&info_header.biHeight, sizeof(LONG), 1, fp);
    fread(&info_header.biPlanes, sizeof(WORD), 1, fp);
    fread(&info_header.biBitCount, sizeof(WORD), 1, fp);
    fread(&info_header.biCompression, sizeof(DWORD), 1, fp);
    fread(&info_header.biSizeImage, sizeof(DWORD), 1, fp);
    fread(&info_header.biXPelsPerMeter, sizeof(LONG), 1, fp);
    fread(&info_header.biYPelsPerMeter, sizeof(LONG), 1, fp);
    fread(&info_header.biClrUsed, sizeof(DWORD), 1, fp);
    fread(&info_header.biClrImportant, sizeof(DWORD), 1, fp);

    int frequency_data[256] = {0};
    int j;

    for(j = 0; j<256; j++){
        fread(&frequency_data[j], sizeof(int), 1, fp);
    }

    Image* image = (Image*)malloc(sizeof(Image));
    image->width = info_header.biWidth;
    image->height = info_header.biHeight;
    reconstructImageData(fp, frequency_data, image);

    fclose(fp);
    return image;
}

int isBMP(const char* filename) {
    const char* extension = strrchr(filename, '.');
    if (extension == NULL) {
        return 0;
    }
    return (strcmp(extension, ".bmp") == 0);
}

int isCBMP(const char* filename) {
    const char* extension = strrchr(filename, '.');
    if (extension == NULL) {
        return 0;
    }

    return (strcmp(extension, ".cbmp") == 0);
}

char* getFileNameWithoutExtension(const char* filename) {
    char* name = strdup(filename);
    char* extension = strrchr(name, '.');
    if (extension != NULL) {
        *extension = '\0';
    }
    return name;
}

int main(int argc, char *argv[]) { 
    if(argc != 2){
        printf("Invalid input, expected:\n[programname] [IMAGEFILE]");
        return 1;
    }

    if(isBMP(argv[1])){
        clock_t start = clock();
        Image *image = read_bmp(argv[1]);
        if (image == NULL) {
            return 1;
        }
        Image* grey_image = turn_grey(image);
        if (grey_image == NULL) {
            return 1;
        }

        char* outputFileName = getFileNameWithoutExtension(argv[1]);
        char cbmpFileName[256];
        sprintf(cbmpFileName, "%s.cbmp", outputFileName);

        Node* root = buildHuffmanTree(grey_image);
        SymbolCode symbolCodes[256];
        int code[256];
        int numCodes = 0;
        createHuffmanCodes(root, code, 0, symbolCodes, &numCodes);
        unsigned char* compressedData;
        int compressedLength;
        compressImage(grey_image, symbolCodes, numCodes, &compressedData, &compressedLength);
        int i;
        int frequencies[256] = {0};
        int size = grey_image->height * image->width * 3;
        for(i = 0; i < size / 3; i++){
            frequencies[grey_image->data[i]] += 1;
        }
        huffmanImageToBMP(compressedData, argv[1], cbmpFileName, frequencies, compressedLength);
        clock_t end = clock();
        double total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Time for compression: %f seconds \n", total_time);

        free(image->data);
        free(image);
        free(compressedData);
        free(outputFileName);
    }
    else if(isCBMP(argv[1])){
        clock_t start1 = clock();
        Image* uncompressed_image = read_compressed(argv[1]);

        char* outputFileName = getFileNameWithoutExtension(argv[1]);
        char bmpFileName[256];
        sprintf(bmpFileName, "%s_uncompressed.bmp", outputFileName);

        image_to_bmp(uncompressed_image, argv[1], bmpFileName);
        clock_t end1 = clock();
        double total_time1 = ((double)(end1 - start1)) / CLOCKS_PER_SEC;
        printf("Time for decompression: %f seconds \n", total_time1);
        
        free(uncompressed_image->data);
        free(uncompressed_image);
        free(outputFileName);
    }
    else {
        printf("Input is not bmp or cbmp");
        return 1;
    }

    return 0;
}
