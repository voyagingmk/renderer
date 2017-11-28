#ifndef RENDERER_TEXTMGR_HPP
#define RENDERER_TEXTMGR_HPP
#include "glutils.hpp"
#include "../com/glcommon.hpp"
#include "../com/color.hpp"
#include "sdf.hpp"
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
    TexRef sdfTexRef;
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
    
    template<typename T>
    void loadSingleChar(T c) {
        TexRef texRef;
        // Load character glyph
        if (FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            return;
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
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
        
    }
    
    void test(char c, int pen_x, int pen_y, const size_t width, const size_t height) {
        FT_Error error;
		FT_GlyphSlot  slot = face->glyph; // 指向当前load进来的char的信息

		unsigned char ** Image = new2DArray<unsigned char>(height, width);
        /*
        vector<vector<unsigned char>> Image(height, vector<unsigned char>(width));
        for(auto r: Image) {
            for(auto it = r.begin(); it != r.end(); it++) {
                *it = 0;
            }
        }*/
        auto my_draw_bitmap = [&]( FT_Bitmap* bitmap, FT_Int x, FT_Int y)
        {
            FT_Int  i, j, p, q;
            FT_Int  x_max = x + bitmap->width;
            FT_Int  y_max = y + bitmap->rows;
            // bitmap->width bitmap->rows是单个字符的实际大小
            // (x, y)是字符在大图里的起始坐标
            printf("my_draw, w:%d, h:%d\n", bitmap->width, bitmap->rows);
            printf("my_draw, x:%d, y:%d\n", x, y);
            printf("x_max:%d, y_max:%d\n", x_max, y_max);
            // 左上角是原点，从上往下画, x, y都是++
            for ( j = y, q = 0; j < y_max; j++, q++ )
            {
                for ( i = x, p = 0; i < x_max; i++, p++ )
                {
                    if ( i<0 || j<0 || i>=width || j>=height )  continue;
                    Image[j][i] |= bitmap->buffer[q * bitmap->width + p];
                    if(Image[j][i]==0) // empty
                        printf("-");
                    else
                        printf("0");
                }
                printf("\n");
            }
        };
            /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char(face, c, FT_LOAD_RENDER );
        if ( error )
            return;
            
        /* now, draw to our target surface */
        printf("bitmap: left: %d, top: %d\n", slot->bitmap_left, slot->bitmap_top);
        my_draw_bitmap( &slot->bitmap,
                       pen_x + slot->bitmap_left,
                       pen_y - slot->bitmap_top );
        
        /* increment pen position */
        pen_x += slot->advance.x >> 6;
		float** buffer = new2DArray<float>(height, width);
		unsigned char** data = new2DArray<unsigned char>(height, width);
        float valMin = 9999, valMax = -9999;
        sdf::SDFBuilder builder(width, height);
        builder.buildSDF([&](int x, int y)->float {
            // 黑色返回0，白色返回1
            return Image[y][x] == 0? 1.0f : 0.0f;
        }, [&](int x, int y, float v) {
            if(v < valMin) {
                valMin = v;
            }
            if(v > valMax) {
                valMax = v;
            }
            buffer[height - y - 1][x] = v;
        });
        printf("result:\n");
        for(int y = height - 1; y >= 0; y--) {
            for(int x = 0; x < width; x++) {
                float v = buffer[y][x];
                printf("%d\t", (int)v);
            }
            printf("\n");
        }
        printf("\n");
        for(int y = height - 1; y >= 0; y--) {
            for(int x = 0; x < width; x++) {
                float v = buffer[y][x];
                //v = (v - valMin) / (valMax - valMin);
                //data[y][x] = v * 255.0f;
                float c = v * 3.0f + 128.0f;
                if ( c < 0.0f ) c = 0.0f;
                if ( c > 255.0f ) c = 255.0f;
                data[y][x] = c;
                printf("%d\t", data[y][x]);
            }
            printf("\n");
        }
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
        glGenTextures(1, &sdfTexRef.texID);
        glBindTexture(GL_TEXTURE_2D, sdfTexRef.texID);
        glTexImage2D(
                     GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     width,
                     height,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     data
                     );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
		del2DArray(buffer, height);
		del2DArray(Image, height);
		del2DArray(data, height);
    }
    
    template<typename T>
    void RenderText(Shader &shader, T text, GLfloat x, GLfloat y, GLfloat scale, Color color)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Activate corresponding render state
        shader.use();
        shader.set3f("textColor", color.r(), color.g(), color.b());
        Matrix4x4 projection = Ortho(0.0f, (float)winWidth, 0.0f, (float)winHeight, 0.01f, 100.0f);
        shader.setMatrix4f("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        
        for (auto c:text) {
            FT_ULong k = static_cast<FT_ULong>(c);
            if(characters.find(k) == characters.end()) {
                loadSingleChar(c);
            }
            Character ch = characters[k];
            GLfloat xpos = x + ch.Bearing.x * scale;
            GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
            // printf("%c, ypos = %.2f - (%d - %d) * %.2f = %.2f\n", c, y, ch.Size.y, ch.Bearing.y, scale, ypos);
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
        
    }
};
    
};

#endif
