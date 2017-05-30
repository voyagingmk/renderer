#include "stdafx.h"
#include "buffermgr.hpp"
#include "realtime/glutils.hpp"

using namespace renderer;

BufferMgrBase::~BufferMgrBase() {
    release();
}

#ifdef USE_GL


BufferSet BufferMgrOpenGL::CreateBuffer(const char* aliasname, Mesh& mesh) {
    GLuint VBO, VAO, EBO;
    BufferSet bufferSet;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexes.size() * sizeof(unsigned int), &mesh.indexes[0], GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)3);
    glEnableVertexAttribArray(1);
    //TexCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
    
    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
    checkGLError();
    
    bufferSet.vao = VAO;
    bufferSet.vbo = VBO;
    bufferSet.ebo = EBO;
    assert(VAO > 0 && VBO > 0 && EBO > 0);
    bufferSet.triangles = mesh.indexes.size() / 3;
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
