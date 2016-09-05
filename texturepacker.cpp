#include "texturepacker.h"

#define NULL 0


//Rect взят отсюда
//https://code.woboq.org/qt5/qtbase/src/corelib/tools/qrect.h.html#_ZN5QRectC1Ev
struct Rect
{
private:
    int x1, y1, x2 ,y2;

public:
    Rect()	{ x1 = y1 = 0; x2 = y2 = -1; }
    inline Rect(int aleft, int atop, int awidth, int aheight):
     x1(aleft), y1(atop), x2(aleft + awidth - 1), y2(atop + aheight - 1) {}

    inline int x()    {return x1;}
    inline int y()    {return y1;}
    inline int left()    {return x1;}
    inline int top()    {return y1;}
    inline int width()    {return x2 - x1 + 1;}
    inline int height()    {return y2 - y1 + 1;}
    void setWidth(int w){ x2 = (x1 + w - 1);}
    void setHeight(int h){ y2 = (y1 + h - 1);}
};

// алгоритм взят отсюда
//http://www.blackpawn.com/texts/lightmaps/default.html
// а также спасибо
//http://gamedevblogs.ru/blog/actionscript/906.html
class Node
{
public:
    Node* child[2];
    Rect rect;
    int imgID = -1;

    ~Node()
     {
        delete child[0];
        delete child[1];
     }

    Node* Insert(int width , int height, int img){
        if (child[0] != NULL) {//we're not a leaf then
            // (try inserting into first child)
            Node *newNode = child[0]->Insert(width ,height, img );
            if (newNode != NULL) return newNode;

            // (no room, insert into second)
            return child[1]->Insert(width ,height, img );
        } else {
            // (if there's already a lightmap here, return)
            if (imgID != -1 )return NULL;
            // (if we're too small, return)
            if (width > rect.width() || height > rect.height()) //img doesn't fit in pnode->rect
                return NULL;
            // (if we're just right, accept)
            if (width == rect.width() && height == rect.height()){//img fits perfectly in pnode->rect
                this->imgID = img;
                return this;
            }

            //(otherwise, gotta split this node and create some kids)
            child[0] = (new Node());
            child[1] = (new Node());

            // (decide which way to split)
            int dw = rect.width() - width;
            int dh = rect.height() - height;

            if (dw > dh) {
                child[0]->rect =  Rect(rect.left(), rect.top(), width,
                                        rect.height());
                child[1]->rect =  Rect(rect.left() + width, rect.top(),
                                        rect.width()-width, rect.height());
            } else {
                child[0]->rect =  Rect(rect.left(), rect.top(),
                                        rect.width(), height);
                child[1]->rect =  Rect(rect.left(), rect.top() + height,
                                        rect.width(), rect.height()-height);
            }

            // (insert into first child we created)
            return child[0]->Insert( width ,height, img );
        }

    }

};




TexturePacker::TexturePacker()
{

}


TexturePacker::~TexturePacker()
{
   // очишяю ненужный массив
    delete[] imgArr;
}



int TexturePacker::nextPow2(int newSize) const
{
  int pixel = 1;
  while ( pixel < newSize )
  {
    pixel = pixel*2;
  }
  return pixel;
}

//http://mathbits.com/MathBits/CompSci/Arrays/Bubble.htm
void TexturePacker::BubbleSort(Image *&img, bool sortID)
{
    int i, j, flag = 1;    // set flag to 1 to start first pass
    Image temp;             // holding variable
    int numLength = size;
    for(i = 1; (i <= numLength) && flag; i++)
    {
        flag = 0;
        for (j=0; j < (numLength -1); j++)
        {

            if(sortID)
            {
                if (img[j+1].id < img[j].id)     // ascending order simply changes to <
                {
                    temp = img[j];             // swap elements
                    img[j] = img[j+1];
                    img[j+1] = temp;
                    flag = 1;               // indicates that a swap occurred.
                }
            }else{
                if (img[j+1].width > img[j].width ||
                        img[j+1].height > img[j].height)     // ascending order simply changes to <
                {
                    temp = img[j];             // swap elements
                    img[j] = img[j+1];
                    img[j+1] = temp;
                    flag = 1;               // indicates that a swap occurred.
                }
            }


        }
    }
    return;   //arrays are passed to functions by address; nothing is returned
}







void TexturePacker::setTextureCount(int count)
{
    imgArr = new Image[count];
    size = count;
}



void TexturePacker::addTexture(int width, int height)
{
    Image img;
    img.id = id;
    img.width = width;
    img.height = height;

    imgArr[id] = img;
    id++;
}

void TexturePacker::packTextures(int &width, int &height, bool forcePowerOfTwo, bool onePixelBorder)
{

    // сортируем для лучшего качества "упаковки",поэтому этим пренебрегать не желательно
    // вначале пойдут самые большие текстуры (но порядок индексов премешается,в конце отсортирую и индексы)
    BubbleSort(imgArr,false);


    if ( onePixelBorder )// если нужен бордюр
    {
      for (int i=0; i < size; i++)
      {
        Image &img = imgArr[i];
        img.width+=2;
        img.height+=2;
      }

    }

    // задаём размер исходя из самой большой тектуры
    int atlasWidth = imgArr[0].width;
    int atlasHeight = imgArr[0].height;

    if ( forcePowerOfTwo )
    {
      atlasWidth = nextPow2(atlasWidth);
      atlasHeight = nextPow2(atlasHeight);
    }


     Node *atlas; // наш атлас
     int i = 0;
     while( i < size){
         if(i == 0){
            atlas = new Node(); // создаём наш атлас
             // устанавливаю размер атласа
             atlas->rect.setWidth(atlasWidth);
             atlas->rect.setHeight(atlasHeight);
         }
       // пытаемся вставить картинку в атлас
       Node *n = atlas->Insert(imgArr[i].width, imgArr[i].height, imgArr[i].id);

       if(n != NULL) {
         //нам это удалось!  сохраняем положение текстуры
         imgArr[i].x = n->rect.x();
         imgArr[i].y = n->rect.y();
         i++;
       } else {
            // места не хватает  увеличиваем размер атласа
           if ( forcePowerOfTwo )
           {
             atlasWidth = nextPow2(atlasWidth+1);
             atlasHeight = nextPow2(atlasHeight+1);
           }
           else
           {
              atlasWidth  += imgArr[i].width;
              atlasHeight += imgArr[i].height;
           }
           //NOTE OpenGLES 2.0 Max Texture Size 2048x2048
           delete atlas;// удаляю атлас чтоб его снова пересобрать
           i = 0;//сбрасываю итерацию на начало
       }




     }

    delete atlas;
    //сортирую индексы текстур по возрастанию (т.е в том же порядке как пришли )
    BubbleSort(imgArr,true);



     if ( onePixelBorder )// если нужен был бордюр,сдвигаю на центр и возврашяю размеры
     {
       for (int i=0; i < size; i++)
       {
         Image &img = imgArr[i];
         img.x +=1;
         img.y +=1;
         img.width-=2;
         img.height-=2;
       }

     }

     width = atlasWidth;
     height = atlasHeight;



}

void TexturePacker::getTextureLocation(int index, int &x, int &y, int &width, int &height)
{

       Image img = imgArr[index];
       x = img.x;
       y = img.y;
       width = img.width;
       height= img.height;
}





