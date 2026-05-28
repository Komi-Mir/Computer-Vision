# Image Filter App
Приложение для обработки изображений на C с использованием GTK3 и SDL2.

**Функции**
- Медиальный фильтр
- Фильтр Гаусса
- Детекция границ
- Свертка
- Пикселизация

**Зависимости**
- GTK3
- SDL2
- make
- GCC

**Как установить?**
``` bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-pkg-config
pacman -S mingw-w64-x86_64-gtk3
pacman -S mingw-w64-x86_64-SDL2
pacman -S make
```
**Как собрать?**
``` bash
cd "папка в котором лежит проект"
make
```
После в папке появится app.exe 