#include "drawable.h"
#include "HalfEdge.h"

#ifndef EDGEPOINT_H
#define EDGEPOINT_H

// this class is used to display the selected edge

class EdgePoint : public Drawable {
protected:
    HalfEdge *representedEdge;

public:
    // Creates VBO data to make a visual
    // representation of the currently selected Vertex
    void create() override;
    // Change which Vertex representedVertex points to
    void updateEdge(HalfEdge* e);
    EdgePoint(OpenGLContext* context, HalfEdge* e);
    EdgePoint(OpenGLContext* context);
    ~EdgePoint();
    GLenum drawMode() override;
};

#endif // EDGEPOINT_H
