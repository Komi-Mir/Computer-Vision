#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

    free(window);


    
}



void gauss(Image_struct* img_f, Image_struct* img_r, int radius, float sigma) {
    int w = img_f->width;
    int h = img_f->height;
    int c = img_f->channels;
    int size_windows = 2 * radius + 1;

    img_r->width = w;
    img_r->height = h;
    img_r->channels = c;
    img_f->img = malloc(w * h * c);

    unsigned char *window = malloc(size_windows);
    // Ядра Гаусса
    float *ker = malloc(size_windows * size_windows * sizeof(float));
    float summ = 0.0f;
    for (int ny = -radius; ny <= radius; ny++) {
        for (int nx = -radius; nx <= radius; nx++) {
            float a = expf(-(nx * nx + ny * ny) / (2.0f * sigma * sigma));
            ker[(ny + radius) * size_windows + (nx + radius)] = a;
            summ += a;

            }
    }
    // нормализует сумма весов = 1
    for (int i = 0; i < size_windows * size_windows; i++) {
        ker[i] /= summ;
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            for (int chan = 0; chan < h; chan++) {
                float res = 0.0f;

                for (int ny = -radius; ny <= radius; ny++) {
                    for (int nx = -radius; nx <= radius; nx++) {
                        int oy = y + ny;
                        int ox = x + nx;
                        
                        if (oy < 0) oy = -oy;
                        if (ox < 0) ox = -ox;
                        if (ox >= h) ox = 2 * h - ox - 2;
                        if (ox >= h) ox = 2 * h - oy - 2;

                        float weight = ker[(ny + radius) * size_windows + (nx + radius)];
                        res += weight * img_f->img[(oy * w + ox) * c + chan];

                    }
                }
                img_r->img[(y * w + x) * c + chan] = (unsigned char)res;
                
            }
            

        }
    }

    free(ker);


}
void detector(Image_struct* img_f, Image_struct* img_r) {
    int w = img_f->width;
    int h = img_f->height;
    int c = img_f->channels;

    img_r->width = w;
    img_r->height = h;
    img_r->channels = c;
    img_f->img = malloc(w * h * c);

    // Ядра Собеля
    int Gx[3][3] = { 
        {-1, 0, 1}, 
        {-2, 0, 2}, 
        {-1, 0, 1} 
    };
    int Gy[3][3] = { 
        {-1,-2,-1}, 
        { 0, 0, 0}, 
        { 1, 2, 1} 
    };


    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            for (int chan = 0; chan < h; chan++) {
                float gx = 0.0f;
                float gy = 0.0f;

                for (int ny = -1; ny <= 1; ny++) {
                    for (int nx = -1; nx <= 1; nx++) {
                        int oy = y + ny;
                        int ox = x + nx;
                        
                        if (oy < 0) oy = -oy;
                        if (ox < 0) ox = -ox;
                        if (ox >= h) ox = 2 * h - ox - 2;
                        if (ox >= h) ox = 2 * h - oy - 2;

                        float pixel = img_f->img[(oy * w + ox) * c + chan];
                        gx += Gx[ny + 1][nx + 1] * pixel;
                        gy += Gy[ny + 1][nx + 1] * pixel;

                    }
                }

                float power = sqrtf(gx * gx + gy * gy);
                if (power > 255.0f) power = 255.0f;
                img_r->img[(y * w + x) * c + chan] = (unsigned char)power;
                
            }
            

        }
    }
}
void convolution() { print("Свертка");}

int main(int argc, char *argv[]) {
    // Загразука изображения
    Image_struct image;
    image.img = stbi_load("cat.png", &image.width, &image.height, &image.channels, 0);

}