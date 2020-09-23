#pragma once

#include <glm/glm.hpp>
#include <QListWidgetItem>
#include <QObject>
#include "drawable.h"
#include "vertex.h"

// class represents vertex display when selected
class VertPoint: public Drawable {
    protected:
        Vertex *representedVertex;
        OpenGLContext* cont;

    public:
        // Creates VBO data to make a visual
        // representation of the currently selected Vertex
        void create() override;
        // Change which Vertex representedVertex points to
        void updateVertex(Vertex* v);
        ~VertPoint() override;
        VertPoint(OpenGLContext* context, Vertex *v);
        VertPoint(OpenGLContext* context);
        VertPoint();
        void destroy();
        GLenum drawMode() override;
};
