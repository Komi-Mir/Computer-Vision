#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int comp(const void* a, const void* b) {
    return (int)(*(unsigned char*)a) - (int)(*(unsigned char*)b);
}

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char* img;
} Image_struct;

void median(Image_struct* img_f, Image_struct* img_r, int radius) {
    int w = img_f->width;
    int h = img_f->height;
    int c = img_f->channels;
    int size_windows = (2 * radius + 1) * (2 * radius + 1);

    img_r->width = w;
    img_r->height = h;
    img_r->channels = c;
    img_f->img = malloc(w * h * c);

    unsigned char *window = malloc(size_windows);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            for (int chan = 0; chan < h; chan++) {
                int count = 0;

                for (int ny = -radius; ny <= radius; ny++) {
                    for (int nx = -radius; nx <= radius; nx++) {
                        int oy = y + ny;
                        int ox = x + nx;
                        
                        if (oy < 0) oy = -oy;
                        if (ox < 0) ox = -ox;
                        if (ox >= h) ox = 2 * h - ox - 2;
                        if (ox >= h) ox = 2 * h - oy - 2;

                        window[count++] = img_f->img[(oy * w + ox) * c + chan];

                    }
                }

                qsort(window, count, sizeof(unsigned char), comp);
                img_r->img[(y * w + x) * c + chan] = window[count / 2];
            }
            

        }
    }


    
}



void gauss() { print("Фильтр Гаусса");}
void edges() { print("Детекция границ");}
void convolution() { print("Свертка");}

int main(int argc, char *argv[]) {
    // Загразука изображения
    Image_struct image;
    image.img = stbi_load("cat.png", &image.width, &image.height, &image.channels, 0);

}