#include "Overlay.h"
#include "Root.h"
#include <iostream>
#include "Interface.h"
#include "Fonts.h"
#include "Textures.h"
#include "common/Logger.h"
#include <sstream>
#include <vector>
#define STB_TRUETYPE_IMPLEMENTATION
#include "../ext/stb_truetype.h"
using std::vector;

namespace Arya
{
    void Interface::init()
    {
        Font* font = FontManager::shared().getFont("courier.ttf");

        for(int i = 0; i < 9; i++)
        {
            Rect* rect = new Rect;
            rects.push_back(rect);
            rect->offsetInPixels.x = i * 16.0;
            rect->offsetInPixels.y = 0.0;
            rect->sizeInPixels.x = 16.0;
            rect->sizeInPixels.y = 16.0;
            rects[i]->textureHandle = font->textureHandle;
            Root::shared().getOverlay()->addRect(rect);
        }

        float xpos = 0.0f, ypos = 0.0f;      
        stbtt_aligned_quad q;
        string s = "FPS = ";

        for(int i = 0; (i < s.size()); i++)
        {
            stbtt_GetBakedQuad(font->baked, 512, 512, s[i], &xpos ,&ypos,&q,true);
            rects[i]->texOffset = vec2(q.s0, 1 - q.t0 - (q.t1 - q.t0));
            rects[i]->texSize = vec2(q.s1 - q.s0, (q.t1 - q.t0));
        }
    }



    void Interface::onFrame(float elapsedTime)
    {	
        Font* font = FontManager::shared().getFont("courier.ttf");
        time += elapsedTime;
        count += 1;

        if (time >= 1.0)
        {
            float xpos = 0.0f, ypos = 0.0f;      
            std::stringstream myStream;
            myStream.fill(' ');
            myStream.width(3);
            if(count >= 1000) count = 999;
            myStream << count;
            stbtt_aligned_quad q;
            for(int i = 6; i < 9; i++)
            {
                stbtt_GetBakedQuad(font->baked, 512, 512, myStream.str()[i - 6], &xpos ,&ypos,&q,true);
                rects[i]->texOffset = vec2(q.s0, 1 - q.t0 - (q.t1 - q.t0));
                rects[i]->texSize = vec2(q.s1 - q.s0, (q.t1 - q.t0));
            }
            count = 0;
            time = 0.0;
        }

    }
}