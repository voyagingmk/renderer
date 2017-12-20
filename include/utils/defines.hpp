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

typedef std::string MaterialSettingAlias;
typedef size_t MaterialSettingID;
typedef size_t MaterialID;



typedef std::tuple<size_t, size_t, size_t, size_t> Viewport;


#define L1_CACHE_LINE_SIZE 64


// Memory Allocation Functions
static void *AllocAligned(size_t size) {
#if defined(_MSC_VER)
	return _aligned_malloc(size, L1_CACHE_LINE_SIZE);
#else
	void *ptr;
	if (posix_memalign(&ptr, L1_CACHE_LINE_SIZE, size) != 0) ptr = nullptr;
	return ptr;
//#else
//	return memalign(L1_CACHE_LINE_SIZE, size);
#endif
}

template <typename T>
static T *AllocAligned(size_t count) {
	return (T *)AllocAligned(count * sizeof(T));
}

static void FreeAligned(void *ptr) {
	if (!ptr) return;
#if defined(_MSC_VER)
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}



#endif // RENDERER_DEFINES_HPP
