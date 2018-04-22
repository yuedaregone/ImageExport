#include <stdio.h>
#include <string.h>
#include <iostream>
#include <map>
#include <string>

#define MAX_COUNT 1024

typedef struct _Tex
{
    int x = 0;
    int y = 0;
    int sx = 0;
    int sy = 0;
}Tex;

int ParseFile(const char *filePath)
{
    FILE *fp = fopen(filePath, "rb");
    if (fp == NULL)
    {
        printf("Not find file:");
        printf(filePath);
        printf("\n");
        return -1;
    }
    int head = 5;
    char buff[MAX_COUNT] = {'\0'};
    for (int i = 0; i < head; ++i)
    {
        fgets(buff, MAX_COUNT, fp);
    }

    std::map<std::string, std::string> maps;
    std::string key;
    std::string value;
    while (1)
    {       
        memset(buff, 0, MAX_COUNT);
        char* ch = fgets(buff, MAX_COUNT, fp);
        if (ch == NULL)
        {
            break;
        }

        if (buff[0] != ' ')
        {
            key = buff;
            key.pop_back();
        }
        else
        {
            char* pos = strstr(buff, "xy: ");
            if (pos != NULL)
            {
                pos = pos + 4;
                std::string value = pos;
                value.pop_back();
                size_t p = value.find(',');
                int x = atoi(value.substr(0, p).c_str());
                int y = atoi(value.substr(p+1, value.length() - p - 1).c_str());
            }
        }        
    }
    return 0;
}

int main()
{
    ParseFile(".\\cards.atlas");
    printf("HelloWorld!");
    getchar();
    return 0;
}