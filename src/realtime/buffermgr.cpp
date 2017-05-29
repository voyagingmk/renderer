#include "stdafx.h"
#include "buffermgr.hpp"
#include "realtime/glutils.hpp"

using namespace renderer;

BufferMgrBase::~BufferMgrBase() {
    release();
}

#ifdef USE_GL


BufferSet BufferMgrOpenGL::CreateBuffer(const char* aliasname, std::vector<float>& vertices,std::vector<float>& texcoords, std::vector<unsigned int>& indices) {
    GLuint VBO, VAO, EBO;
    BufferSet bufferSet;
    size_t verticesNum = vertices.size() / 3;
    std::vector<GLfloat> array(5 * verticesNum);
    for(uint32_t v = 0; v < verticesNum; v++) {
        uint32_t aidx = v * 5;
        uint32_t vidx = v * 3;
        array[aidx + 0] = vertices[vidx + 0];
        array[aidx + 1] = vertices[vidx + 1];
        array[aidx + 2] = vertices[vidx + 2];
        uint32_t tidx = v * 2;
        array[aidx + 3] = texcoords[tidx + 0];
        array[aidx + 4] = texcoords[tidx + 1];
    }
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // TexCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    /*
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // TexCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    */
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
    
    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
    checkGLError();
    
    bufferSet.vao = VAO;
    bufferSet.vbo = VBO;
    bufferSet.ebo = EBO;
    assert(VAO > 0 && VBO > 0 && EBO > 0);
    bufferSet.triangles = indices.size() / 3;
    printf("triangles:%d\n", bufferSet.triangles);
    bufferDict[aliasname] = bufferSet;
    return bufferSet;
}

void BufferMgrOpenGL::DrawBuffer(const char* aliasname) {
    BufferSet bufferSet = bufferDict[aliasname];
    glBindVertexArray(bufferSet.vao);
    //glDrawArrays(GL_TRIANGLES, 0, bufferSet.triangles);
    glDrawElements(GL_TRIANGLES, bufferSet.triangles * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    checkGLError();
}

#endif
