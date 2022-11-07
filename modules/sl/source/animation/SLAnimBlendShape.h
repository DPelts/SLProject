#ifndef SLANIMBLENDSHAPE_H
#define SLANIMMBLENDSHAPE_H

#include <SL.h>
#include <SLVec3.h>

class SLAnimBlendShape
{
public:
    SLAnimBlendShape();
    SLAnimBlendShape(SLint size);
    SLAnimBlendShape(vector<SLVec3f> vertices);
    ~SLAnimBlendShape();

    void createBlendShape(SLint size);

    // Setter
    void addVertex(SLVec3f vertex);
    void addName(SLstring name);

private:
    SLstring       _name;
    SLVVec3f _vertices;
};

typedef vector<SLAnimBlendShape*> SLVAnimBlendShape;


#endif