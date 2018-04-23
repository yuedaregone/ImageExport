#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include "..\MImage.h"
#define MAX_COUNT 1024

typedef struct _TexItem
{
    std::string name;
    int x = 0;
    int y = 0;
    int sx = 0;
    int sy = 0;
}TexItem;

typedef struct _Tex
{
    std::string pngname;
    std::vector<TexItem*> texs;    
}Tex;

int ParseFile(const char *filePath, std::vector<Tex*>& texs)
{
    FILE *fp = fopen(filePath, "rb");
    if (fp == NULL)
    {
        printf("Not find file:");
        printf(filePath);
        printf("\n");
        return -1;
    }
    
    char buff[MAX_COUNT] = {'\0'};            
    Tex* tex = NULL;   
    TexItem* texitem = NULL;
    while (1)
    {       
        memset(buff, 0, MAX_COUNT);
        char* ch = fgets(buff, MAX_COUNT, fp);
        if (ch == NULL)
        {      
            if (texitem != NULL)
            {
                tex->texs.push_back(texitem);
                texitem = NULL;
            }
            if (tex != NULL)
            {
                texs.push_back(tex);
                tex = NULL;
            }      
            break;
        }

        if (strlen(buff) == 1 && buff[0] == '\n')
        {
            if (texitem != NULL)
            {
                tex->texs.push_back(texitem);
                texitem = NULL;
            }
            if (tex != NULL)
            {
                texs.push_back(tex);
                tex = NULL;
            }

            memset(buff, 0, MAX_COUNT);
            char* nextLine = fgets(buff, MAX_COUNT, fp);
            if (nextLine == NULL)
            {                
                break;
            }            
            tex = new Tex();
            tex->pngname = buff;  
            tex->pngname.pop_back();

            for (int i = 0; i < 3; ++i)
            {
                fgets(buff, MAX_COUNT, fp);
            }  
            continue;        
        }

        if (buff[0] != ' ')
        {
            if (texitem != NULL)
            {
                tex->texs.push_back(texitem);
            }

            texitem = new TexItem();
            texitem->name = buff;
            texitem->name.pop_back();
        }
        else
        {
            if (texitem == NULL)
                continue;

            char* pos = strstr(buff, "xy: ");
            if (pos != NULL)
            {
                pos = pos + 4;
                std::string value = pos;
                value.pop_back();
                size_t p = value.find(',');
                texitem->x = atoi(value.substr(0, p).c_str());
                texitem->y = atoi(value.substr(p+1, value.length() - p - 1).c_str());            
            }
            char* size = strstr(buff, "size: ");
            if (size != NULL)
            {
                size = size + 6;
                std::string value = size;
                value.pop_back();
                size_t p = value.find(',');
                texitem->sx = atoi(value.substr(0, p).c_str());
                texitem->sy = atoi(value.substr(p+1, value.length() - p - 1).c_str());   
            }
        }        
    }    
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[])
{         
    if (argc != 3)
    {
        printf("*.exe inText outFolder\n");
        return 0;
    }

    std::vector<Tex*> texs;
    ParseFile(argv[1], texs);       	

    for (int i = 0; i < texs.size(); ++i)
    {
        Tex* t = texs[i];
        if (t == NULL) continue;

        MImage img;        
	    img.initWithFile(("./" + t->pngname).c_str(), MImage::typePNG);       

        unsigned char* data = img.getData();  
        unsigned short width = img.getWidth();
        unsigned short height = img.getHeight();
        size_t size = img.getDataSize();        
        
        for (int j = 0; j < t->texs.size(); ++j)
        {
            TexItem* tItem = t->texs[j];
            printf("processing:%s\n", tItem->name.c_str());            

            int w = tItem->sx;
            int h = tItem->sy;
            int x = tItem->x;
            int y = tItem->y;

            unsigned char* out = new unsigned char[w * h * 4];                     
            for (int k = 0; k < h; ++k)
            {
                memcpy(out + k * w * 4, data + (width * (k + y) + x) * 4, w * 4);
            }
            std::string outName = argv[2];
            if (argv[2][outName.length() - 1] != '\\' && argv[2][outName.length() - 1] != '/')
            {
                outName += "\\" + tItem->name + ".png";
            }
            else
            {
                outName += tItem->name + ".png";
            }
            write_png(outName.c_str(), w, h, out);      
            delete[] out;
        }     
    }
    return 0;
}