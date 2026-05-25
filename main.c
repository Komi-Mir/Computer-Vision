#include <stdio.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    int size = (2 * radius + 1) * (2 * radius + 1);

    img_r->width = w;
    img_r->height = h;
    img_r->channels = c;
    img_f->img = malloc(w * h * c);
    
}
void gauss() { print("Фильтр Гаусса");}
void edges() { print("Детекция границ");}
void convolution() { print("Свертка");}

int main(int argc, char *argv[]) {
    // Загразука изображения
    Image_struct image;
    image.img = stbi_load("cat.png", &image.width, &image.height, &image.channels, 0);

}