#include "stdafx.h"
#include "defines.hpp"

const bool Enable_GammaCorrect = false;
const bool Enable_HDR = true;
const bool Enable_IndirectDiffuse = false;
const int SampleNum_IndirectDiffuse = 300;


int line_num = 0;
std::string current_file = "";

const int RenderType = RenderType_Default;

bool Enable_DebugPixcel = false;