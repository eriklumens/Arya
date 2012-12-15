#define STB_TRUETYPE_IMPLEMENTATION

#include "Fonts.h"
#include "Files.h"
#include "Resources.h"
#include "common/Logger.h"
#include "../ext/stb_truetype.h"
#include <GL/glfw.h>
#include <iostream>
using namespace std;

namespace Arya
{
    template<> FontManager* Singleton<FontManager>::singleton = 0;
    FontManager::FontManager(){}
    FontManager::~FontManager()
    {
        cleanup();
    }
    int FontManager::initialize()
    {
        loadDefaultFont();
        return 1;
    }
    void FontManager::cleanup(){}

    void FontManager::loadDefaultFont()
    {
        loadResource("courier.ttf");
    }
    Font* FontManager::loadResource(const char* filename)
    {
        File* fontfile = FileSystem::shared().getFile(filename);
        if( fontfile == 0 )
        {
            cout << "Font not found!" << endl;
            return 0;
        }
        Font* font = new Font ;      
        makeImage(fontfile,font);
        addResource(filename, font);
    }
    void FontManager::makeImage(File* file,Font* font)
    {
        unsigned char pixeldata[512*512];
        int width = 512;
        int height = 512;
        stbtt_bakedchar baked[100];
        unsigned char fixedpixeldata[512*512];
        stbtt_BakeFontBitmap((unsigned char*)file->getData(), 0, 20, pixeldata, width, height, 0, 100, font->baked);
        for(int i = 0; i < 512; i++)
        {
            for(int j = 0; j < 512; j++)
            {
                fixedpixeldata[512*i+j] = pixeldata[512 * (511 - i) + j];
            }
        }
        font->baked;
        glGenTextures(1, &font->textureHandle);
        glBindTexture(GL_TEXTURE_2D, font->textureHandle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, fixedpixeldata ); 
    }
}
