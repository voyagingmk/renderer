#ifndef RENDERER_TEXTMGR_HPP
#define RENDERER_TEXTMGR_HPP

#include "glutils.hpp"
#include "glcommon.hpp"
#include "texturemgr.hpp"
#include "shadermgr.hpp"
#include "../color.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace renderer {

class TextMgr {
public:
    FT_Library ft;
    FT_Face face;
    std::map<GLchar, Character> characters;
    GLuint VAO, VBO;
protected:
    TextMgr() {
        if (FT_Init_FreeType(&ft)) {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        }
    }
    ~TextMgr() {
        FT_Done_FreeType(ft);
    }
public:
    static TextMgr& getInstance() {
        static TextMgr mgr;
        return mgr;
    }
    void loadTTF(const char* path, size_t fontSize) {
        if (FT_New_Face(ft, path, 0, &face)) {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return;
        }
        FT_Set_Pixel_Sizes(face, 0, fontSize);
    }
    
    void releaseFace() {
        FT_Done_Face(face);
    }
    
    void buildCharaTexture() {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
        TexRef texRef;
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            glGenTextures(1, &texRef.texID);
            glBindTexture(GL_TEXTURE_2D, texRef.texID);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
            // Set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // Now store character for later use
            Character character = {
                texRef, 
                Point2dI(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                Point2dI(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<int>(face->glyph->advance.x)
            };
            characters.insert(std::pair<GLchar, Character>(c, character));
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    void setupBuffer() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        CheckGLError;
    }
    
    void RenderText(ShaderProgramHDL shaderHDL, std::string text, GLfloat x, GLfloat y, GLfloat scale, Color color)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Shader &shader = ShaderMgrOpenGL::getInstance().getShader(shaderHDL);
        // Activate corresponding render state
        shader.use();
        shader.set3f("textColor", color.r(), color.g(), color.b());
        Matrix4x4 projection = Ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.01f, 100.0f);
        shader.setMatrix4f("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        CheckGLError;
        
        // Iterate through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            Character ch = characters[*c];
            
            GLfloat xpos = x + ch.Bearing.x * scale;
            GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
            
            GLfloat w = ch.Size.x * scale;
            GLfloat h = ch.Size.y * scale;
            // Update VBO for each character
            GLfloat vertices[6][4] = {
                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos,     ypos,       0.0, 1.0 },
                { xpos + w, ypos,       1.0, 1.0 },
                
                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos + w, ypos,       1.0, 1.0 },
                { xpos + w, ypos + h,   1.0, 0.0 }
            };
            // Render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.texRef.texID);
            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        CheckGLError;
    }
};
    
};
// std::map<GLchar, Character> Characters;


#endif
