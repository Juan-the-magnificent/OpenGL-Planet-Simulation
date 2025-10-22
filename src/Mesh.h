//-----------------------------------------------------------------------------
// Mesh.h
// Basic Mesh class with lighting support
//-----------------------------------------------------------------------------
#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#define GLEW_STATIC
#include "GL/glew.h"
#include "glm/glm.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;  // Added for lighting
};

class Mesh
{
public:
    Mesh();
    ~Mesh();

    bool loadOBJ(const std::string& filename);
    void draw();

private:
    void initBuffers();
    void calculateNormals();  // Calculate normals if OBJ doesn't have them

    bool mLoaded;
    std::vector<Vertex> mVertices;
    GLuint mVBO, mVAO;
};
#endif //MESH_H