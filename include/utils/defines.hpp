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

extern bool Enable_DebugPixcel;

static int logDebug(char const* const _Format, ...) {
	if(Enable_DebugPixcel){
        #if defined(_MSC_VER)
		int _Result;
		va_list _ArgList;
		__crt_va_start(_ArgList, _Format);
		_Result = _vfprintf_l(stdout, _Format, NULL, _ArgList);
		__crt_va_end(_ArgList);
		return _Result;
        #endif
	}
    return 0;
}

typedef size_t MaterialSettingID;
typedef size_t MaterialID;



typedef std::tuple<size_t, size_t, size_t, size_t> Viewport;

#endif // RENDERER_DEFINES_HPP
