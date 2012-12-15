#include "Overlay.h"
#include "Fonts.h"
#include "Root.h"
#include "Shaders.h"
#include "common/Logger.h"

namespace Arya
{
    Overlay::Overlay() 
    {
        rectVAO = 0;
        overlayProgram = 0;
    }

    Overlay::~Overlay()
    {

    }

    bool Overlay::init()
    {
        if(!initShaders()) return false;

        ww = Root::shared().getWindowWidth();
        wh = Root::shared().getWindowHeight();

        // make 1 by 1 px vertex buffer
        // inititialize shader
        GLfloat vertices[] = {
            0.0,        0.0,
            1.0,        0.0,
            0.0,        1.0,
            1.0,        1.0
        };

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW); 

        glGenVertexArrays(1, &rectVAO);
        glBindVertexArray(rectVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        glBindVertexArray(0);

        return true;
    }

    bool Overlay::initShaders()
    {

        Shader* overlayVertex = new Shader(VERTEX);
        if(!(overlayVertex->addSourceFile("../shaders/overlay.vert"))) return false;
        if(!(overlayVertex->compile())) return false;

        Shader* overlayFragment = new Shader(FRAGMENT);
        if(!(overlayFragment->addSourceFile("../shaders/overlay.frag"))) return false;
        if(!(overlayFragment->compile())) return false;

        overlayProgram = new ShaderProgram("overlay");
        overlayProgram->attach(overlayVertex);
        overlayProgram->attach(overlayFragment);
        if(!(overlayProgram->link())) return false;

        return true;
    }

    void Overlay::addRect(Rect* rect)
    {
        rects.push_back(rect);
    }

    void Overlay::removeRect(Rect* rect)
    {
        for(vector<Rect*>::iterator it = rects.begin(); it != rects.end();)
        {
            if( *it == rect ) it = rects.erase(it);
            else ++it;
        }
    }

    void Overlay::render()
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);

        // bind shader
        overlayProgram->use();
        overlayProgram->setUniform1i("texture1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,FontManager::shared().getFont("courier.ttf")->textureHandle);


        glBindVertexArray(rectVAO);

        // render all rects
        for(int i = 0; i < rects.size(); ++i) {
            // TODO: use dirty flag maybe?
            rects[i]->screenPosition = rects[i]->relative + vec2(2.0 * rects[i]->offsetInPixels.x / ww, 2.0 * rects[i]->offsetInPixels.y / wh);
            rects[i]->screenSize = vec2(2.0 * rects[i]->sizeInPixels.x / ww, 2.0 * rects[i]->sizeInPixels.y / wh);
            overlayProgram->setUniform2fv("screenSize", rects[i]->screenSize);
            overlayProgram->setUniform2fv("screenPosition", rects[i]->screenPosition);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
}
