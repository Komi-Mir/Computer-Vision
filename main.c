#include <stdio.h>

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char* im;
} Image;

void median() { print("Меддианный фильтр");}
void gauss() { print("Фильтр Гаусса");}
void edges() { print("Детекция границ");}
void convolution() { print("Свертка");}

int main(int argc, char *argv[]) {}