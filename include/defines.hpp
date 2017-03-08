#ifndef RENDERER_DEFINES_HPP
#define RENDERER_DEFINES_HPP

extern const bool Enable_GammaCorrect;
extern const bool Enable_HDR;
extern const bool Enable_IndirectDiffuse;
extern const int SampleNum_IndirectDiffuse;

#define RenderType_Default 0
#define RenderType_DepthMap 1
#define RenderType_NormalMap 2

extern const int RenderType;

#endif // RENDERER_DEFINES_HPP
