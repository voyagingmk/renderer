#ifndef RENDERER_TEXTMGR_HPP
#define RENDERER_TEXTMGR_HPP
#include <codecvt>
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
    std::map<FT_ULong, Character> characters;
    GLuint VAO, VBO;
    size_t winWidth, winHeight;
protected:
    TextMgr():
        VAO(0),
        VBO(0),
        winWidth(0),
        winHeight(0)
    {
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
    
    void setScreenSize(size_t w, size_t h) {
        winWidth = w;
        winHeight = h;
    }
    
    void buildCharaTexture() {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
        
        for (unsigned char c = 0; c < 128; c++)
        {
            loadSingleChar(c);
        }
        loadSingleChar(u'测');
        loadSingleChar(u'试');
    }
    
    template<typename T>
    void loadSingleChar(T c) {
        TexRef texRef;
        // Load character glyph
        if (FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            return;
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
        characters.insert(std::pair<FT_ULong, Character>(static_cast<FT_ULong>(c), character));
        glBindTexture(GL_TEXTURE_2D, 0);
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
    
    template<typename T>
    void RenderText(ShaderProgramHDL shaderHDL, T text, GLfloat x, GLfloat y, GLfloat scale, Color color)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Shader &shader = ShaderMgrOpenGL::getInstance().getShader(shaderHDL);
        // Activate corresponding render state
        shader.use();
        shader.set3f("textColor", color.r(), color.g(), color.b());
        Matrix4x4 projection = Ortho(0.0f, (float)winWidth, 0.0f, (float)winHeight, 0.01f, 100.0f);
        shader.setMatrix4f("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        CheckGLError;
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> codecvt;
        for (auto c:text) {
            Character ch = characters[static_cast<FT_ULong>(c)];
            GLfloat xpos = x + ch.Bearing.x * scale;
            GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
            // std::cout << codecvt.to_bytes(*c) << std::endl;
            printf("%c, ypos = %.2f - (%d - %d) * %.2f = %.2f\n", c, y, ch.Size.y, ch.Bearing.y, scale, ypos);
            GLfloat w = ch.Size.x * scale;
            GLfloat h = ch.Size.y * scale;
            // (xpos, ypos)是字符左下角坐标
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

#endif
