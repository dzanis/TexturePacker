#ifndef TEXTUREPACKER_H
#define TEXTUREPACKER_H

//NOTE 5.08.16
// найденный TexturePacker здесь https://github.com/bivory/texture-atlas/tree/master/texturepacker
// выдал мне ошибку при запаковки 200-т мелких текстур
// QGM: ../editor/texturepacker/TexturePacker.cpp:417: virtual int TEXTURE_PACKER::MyTexturePacker::packTextures(int&, int&, bool, bool):
// пришлось переписать TexturePaker на свой,на что ушло немало времени ._.
// текстуры пакует без проблем
// но не реализовал поворот текстуры на 90

/* как использовать
  //Создадим обьект пакера
  TexturePacker *tp = new TexturePacker;
  //Установим сколько нам нужно текстур
  tp->setTextureCount(images_count);
 // добавляем размер текстуры в пакер
  tp->addTexture(img_width,img_height);
  //Напишем переменные в которые сохранится размер текстурного атласа
  int atlasWidth,atlasHeight;
  bool forcePowerOfTwo = true; // если нужен атлас кратным двум
  bool witchBorder = true; // если нужен пустой бордюр в один пиксель
  //далее можно вызвать упаковку
  tp->packTextures(atlasWidth,atlasHeight,forcePowerOfTwo,witchBorder);
  // и получаем положения наших текстур
  int x,y,width,height;
  tp->getTextureLocation(index,x,y,width,height);

  ...
  delete tp;
*/

class TexturePacker
{
public:
    TexturePacker();
    ~TexturePacker();
    void setTextureCount(int count);//
    void addTexture(int width,int height); // add textures 0 - n
    void packTextures(int &width,int &height,bool forcePowerOfTwo,bool onePixelBorder);  // pack the textures
    void getTextureLocation(int index,int &x,int &y,int &width,int &height);
private:
    // структура для хранения областей на атласе
    //или любых других объектов у которых есть размер
    struct Image
    {
        int id;
        int x;
        int y;
        int width;
        int height;
    };

    Image *imgArr;// массив
    int size = 0;
    int id = 0;
    int nextPow2(int newSize) const;
    void BubbleSort(Image *&img, bool sortID);
};

#endif // TEXTUREPACKER_H
