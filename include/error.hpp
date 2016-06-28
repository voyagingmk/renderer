
/*
    pbrt source code Copyright(c) 1998-2010 Matt Pharr and Greg Humphreys.

    This file is part of pbrt.

    pbrt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.  Note that the text contents of
    the book "Physically Based Rendering" are *not* licensed under the
    GNU GPL.

    pbrt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef RENDERER_ERROR_HPP
#define RENDERER_ERROR_HPP

// core/error.h*

// Error Reporting Declarations

// Setup printf format
#ifdef __GNUG__
#define PRINTF_FUNC __attribute__ \
    ((__format__ (__printf__, 1, 2)))
#else
#define PRINTF_FUNC
#endif // __GNUG__
void Info(const char *, ...) PRINTF_FUNC;
void Warning(const char *, ...) PRINTF_FUNC;
void Error(const char *, ...) PRINTF_FUNC;
void Severe(const char *, ...) PRINTF_FUNC;



// Error Reporting Includes
#include <stdarg.h>
#include<string>
#include<algorithm>

// Error Reporting Definitions
#define PBRT_ERROR_IGNORE 0
#define PBRT_ERROR_CONTINUE 1
#define PBRT_ERROR_ABORT 2

static const char *findWordEnd(const char *buf) {
	while (*buf != '\0' && !isspace(*buf))
		++buf;
	return buf;
}

// Error Reporting Functions
static void processError(const char *format, va_list args,
	const char *message, int disposition) {
#if !defined(_MSC_VER)
	char *errorBuf;
	if (vasprintf(&errorBuf, format, args) == -1) {
		fprintf(stderr, "vasprintf() unable to allocate memory!\n");
		abort();
	}
#else
	char errorBuf[2048];
	vsnprintf_s(errorBuf, sizeof(errorBuf), _TRUNCATE, format, args);
#endif
	// Report error
	if (disposition == PBRT_ERROR_IGNORE)
		return;
	else {
		// PBRT_ERROR_CONTINUE, PBRT_ERROR_ABORT
		// Print formatted error message
		extern int line_num;
		int column = 0;
		int width = std::max(20, 80 - 2);
		if (line_num != 0) {
			extern std::string current_file;
			column += fprintf(stderr, "%s(%d): ", current_file.c_str(), line_num);
		}
		fputs(message, stderr);
		fputs(": ", stderr);
		column += strlen(message) + 2;
		const char *msgPos = errorBuf;
		while (true) {
			while (*msgPos != '\0' && isspace(*msgPos))
				++msgPos;
			if (*msgPos == '\0')
				break;

			const char *wordEnd = findWordEnd(msgPos);
			if (column + wordEnd - msgPos > width)
				column = fprintf(stderr, "\n    ");
			while (msgPos != wordEnd) {
				fputc(*msgPos++, stderr);
				++column;
			}
			fputc(' ', stderr);
			++column;
		}
		fputs("\n", stderr);
	}
	if (disposition == PBRT_ERROR_ABORT) {
#if !defined(_MSC_VER)
		__debugbreak();
#else
		abort();
#endif
	}
#if !defined(_MSC_VER)
	free(errorBuf);
#endif
}


static void Info(const char *format, ...) {
	//if (!PbrtOptions.verbose || PbrtOptions.quiet) return;
	va_list args;
	va_start(args, format);
	processError(format, args, "Notice", PBRT_ERROR_CONTINUE);
	va_end(args);
}


static void Warning(const char *format, ...) {
	//if (PbrtOptions.quiet) return;
	va_list args;
	va_start(args, format);
	processError(format, args, "Warning", PBRT_ERROR_CONTINUE);
	va_end(args);
}


static void Error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	processError(format, args, "Error", PBRT_ERROR_CONTINUE);
	va_end(args);
}


static void Severe(const char *format, ...) {
	va_list args;
	va_start(args, format);
	processError(format, args, "Fatal Error", PBRT_ERROR_ABORT);
	va_end(args);
}


#endif // RENDERER_ERROR_HPP
