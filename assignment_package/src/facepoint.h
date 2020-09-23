#include "drawable.h"
#include "face.h"
#ifndef FACEPOINT_H
#define FACEPOINT_H

// this class is used to display the selected face (ringed by lines)
class FacePoint: public Drawable {
    protected:
        Face *representedFace;

    public:
        // Creates VBO data to make a visual
        // representation of the currently selected face
        void create() override;
        // Change which face represented face points to
        void updateFace(Face* f);
        FacePoint(OpenGLContext* context, Face* f);
        FacePoint(OpenGLContext* context);
        ~FacePoint();
        GLenum drawMode() override;
};

#endif // FACEPOINT_H
