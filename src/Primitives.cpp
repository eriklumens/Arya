#include "Primitives.h"
#include "common/Logger.h"

namespace Arya
{
    //-----------------------------
    // Triangle
    //-----------------------------

    Triangle::Triangle() : Mesh()
    {
        init();
    }

    void Triangle::init()
    {
        // Vertices
        GLfloat triangleVertices[] = {
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f
        };

        vertexCount = 3;
        primitiveType = GL_TRIANGLES;

        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                vertexCount * 3 * sizeof(GLfloat),
                triangleVertices,
                GL_STATIC_DRAW);

        // Colors
        GLfloat triangleColors[] = {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };

        glGenBuffers(1, &colorBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                vertexCount * 3 * sizeof(GLfloat),
                triangleColors,
                GL_STATIC_DRAW);

        hasColor = true;
    }

    //-----------------------------
    // Colored Triangle
    //-----------------------------

    ColoredTriangle::ColoredTriangle() : StaticObject()
    {
        vaoHandle = 0;
        Triangle* tri = new Triangle;
        meshes.push_back(tri);
        makeVAO();
    }

    void ColoredTriangle::makeVAO()
    {
        LOG_INFO("Making VAO");

        glGenVertexArrays(1, &vaoHandle);

        LOG_INFO("Finished making VAO");

        glBindVertexArray(vaoHandle);


        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, meshes[0]->getVertexBuffer());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

        glBindBuffer(GL_ARRAY_BUFFER, meshes[0]->getColorBuffer());
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);   

    }
}
