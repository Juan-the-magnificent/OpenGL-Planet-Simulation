//-----------------------------------------------------------------------------
// Mesh.cpp
// Basic Mesh class with lighting support
//-----------------------------------------------------------------------------
#include "Mesh.h"
#include <iostream>
#include <sstream>
#include <fstream>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
Mesh::Mesh()
    :mLoaded(false)
{
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
}

//-----------------------------------------------------------------------------
// Loads a Wavefront OBJ model with normals support
//-----------------------------------------------------------------------------
bool Mesh::loadOBJ(const std::string& filename)
{
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> tempVertices;
    std::vector<glm::vec2> tempUVs;
    std::vector<glm::vec3> tempNormals;

    if (filename.find(".obj") != std::string::npos)
    {
        std::ifstream fin(filename, std::ios::in);
        if (!fin)
        {
            std::cerr << "Cannot open " << filename << std::endl;
            return false;
        }

        std::cout << "Loading OBJ file " << filename << " ..." << std::endl;

        std::string lineBuffer;
        while (std::getline(fin, lineBuffer))
        {
            if (lineBuffer.substr(0, 2) == "v ")
            {
                std::istringstream v(lineBuffer.substr(2));
                glm::vec3 vertex;
                v >> vertex.x; v >> vertex.y; v >> vertex.z;
                tempVertices.push_back(vertex);
            }
            else if (lineBuffer.substr(0, 2) == "vt")
            {
                std::istringstream vt(lineBuffer.substr(3));
                glm::vec2 uv;
                vt >> uv.s; vt >> uv.t;
                tempUVs.push_back(uv);
            }
            else if (lineBuffer.substr(0, 2) == "vn")
            {
                std::istringstream vn(lineBuffer.substr(3));
                glm::vec3 normal;
                vn >> normal.x; vn >> normal.y; vn >> normal.z;
                tempNormals.push_back(normal);
            }
            else if (lineBuffer.substr(0, 2) == "f ")
            {
                int p1, p2, p3; // to store mesh index
                int t1, t2, t3; // to store texture index
                int n1, n2, n3; // to store normal index
                const char* face = lineBuffer.c_str();
                
                // Try to read face with format: v/vt/vn
                int match = sscanf(face, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                    &p1, &t1, &n1,
                    &p2, &t2, &n2,
                    &p3, &t3, &n3);
                
                if (match != 9) {
                    // Try alternative format without texture coordinates: v//vn
                    match = sscanf(face, "f %d//%d %d//%d %d//%d",
                        &p1, &n1,
                        &p2, &n2,
                        &p3, &n3);
                    if (match != 6) {
                        std::cout << "Failed to parse OBJ face: " << lineBuffer << std::endl;
                        continue;
                    }
                    // For format without texture coordinates, use default UVs
                    t1 = t2 = t3 = 1;
                }

                vertexIndices.push_back(p1);
                vertexIndices.push_back(p2);
                vertexIndices.push_back(p3);

                uvIndices.push_back(t1);
                uvIndices.push_back(t2);
                uvIndices.push_back(t3);

                normalIndices.push_back(n1);
                normalIndices.push_back(n2);
                normalIndices.push_back(n3);
            }
        }

        // Close the file
        fin.close();

        // For each vertex of each triangle
        for (unsigned int i = 0; i < vertexIndices.size(); i++)
        {
            // Get the attributes using the indices
            glm::vec3 vertex = tempVertices[vertexIndices[i] - 1];
            glm::vec2 uv = (uvIndices.size() > i) ? tempUVs[uvIndices[i] - 1] : glm::vec2(0.0f, 0.0f);
            glm::vec3 normal = (normalIndices.size() > i) ? tempNormals[normalIndices[i] - 1] : glm::vec3(0.0f, 1.0f, 0.0f);

            Vertex meshVertex;
            meshVertex.position = vertex;
            meshVertex.texCoords = uv;
            meshVertex.normal = normal;

            mVertices.push_back(meshVertex);
        }

        // If no normals were loaded, calculate them
        if (tempNormals.empty()) {
            calculateNormals();
        }

        // Create and initialize the buffers
        initBuffers();

        return (mLoaded = true);
    }

    // We shouldn't get here so return failure
    return false;
}

//-----------------------------------------------------------------------------
// Calculate normals if OBJ file doesn't contain them
//-----------------------------------------------------------------------------
void Mesh::calculateNormals()
{
    // Initialize all normals to zero
    for (auto& vertex : mVertices) {
        vertex.normal = glm::vec3(0.0f);
    }

    // Calculate normals for each triangle
    for (size_t i = 0; i < mVertices.size(); i += 3) {
        glm::vec3 v0 = mVertices[i].position;
        glm::vec3 v1 = mVertices[i + 1].position;
        glm::vec3 v2 = mVertices[i + 2].position;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        // Add this normal to all three vertices
        mVertices[i].normal += normal;
        mVertices[i + 1].normal += normal;
        mVertices[i + 2].normal += normal;
    }

    // Normalize all normals
    for (auto& vertex : mVertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }
}

//-----------------------------------------------------------------------------
// Create and initialize the vertex buffer and vertex array object
//-----------------------------------------------------------------------------
void Mesh::initBuffers()
{
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);

    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], GL_STATIC_DRAW);

    // Vertex Positions (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

    // Vertex Texture Coords (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));

    // Vertex Normals (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));

    // Unbind to make sure other code does not change it somewhere else
    glBindVertexArray(0);
}

//-----------------------------------------------------------------------------
// Render the mesh
//-----------------------------------------------------------------------------
void Mesh::draw()
{
    if (!mLoaded) return;

    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
    glBindVertexArray(0);
}