#ifndef __glad_h_
#define __glad_h_

#ifdef __gl_h_
#error OpenGL header already included, remove this include, glad already provides it
#endif

#define __gl_h_

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

typedef void* (*GLADloadproc)(const char *name);
typedef void* GLADapiproc;

#ifdef __cplusplus
extern "C" {
#endif

struct gladGLversionStruct {
    int major;
    int minor;
};

typedef void* (* GLADloadproc)(const char *name);

#ifndef GLEXT_64_TYPES_DEFINED
/* This code block is duplicated in glxext.h, so must be protected */
#define GLEXT_64_TYPES_DEFINED
/* Define int32_t, int64_t, and uint64_t types for UST/MSC */
/* (as used in the GL_EXT_timer_query extension). */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <inttypes.h>
#elif defined(__sun__) || defined(__digital__)
#include <inttypes.h>
#if defined(__STDC__)
#if defined(__arch64__) || defined(_LP64)
typedef long int int64_t;
typedef unsigned long int uint64_t;
#else
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#endif /* __arch64__ */
#endif /* __STDC__ */
#elif defined( __VMS ) || defined(__sgi)
#include <inttypes.h>
#elif defined(__SCO__) || defined(__USLC__)
#include <stdint.h>
#elif defined(__UNIXOS2__) || defined(__SOL64__)
typedef long int int32_t;
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#elif defined(_WIN32) && defined(__GNUC__)
#include <stdint.h>
#elif defined(_WIN32)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
/* Fallback if nothing above works */
#include <inttypes.h>
#endif
#endif

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;

#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_TRIANGLES                      0x0004
#define GL_ONE                            1
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRONT_AND_BACK                 0x0408
#define GL_POLYGON_MODE                   0x0B40
#define GL_CULL_FACE                      0x0B44
#define GL_DEPTH_TEST                     0x0B71
#define GL_STENCIL_TEST                   0x0B90
#define GL_VIEWPORT                       0x0BA2
#define GL_BLEND                          0x0BE2
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_UNPACK_SKIP_ROWS               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_TEXTURE_2D                     0x0DE1
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_RGBA                           0x1908
#define GL_FILL                           0x1B02
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_LINEAR                         0x2601
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_NUM_EXTENSIONS                 0x821D
#define GL_DEPTH_TEST                     0x0B71
#define GL_LESS                           0x0201
#define GL_VERSION                        0x1F02
#define GL_RENDERER                       0x1F01

typedef void (APIENTRYP PFNGLCLEARPROC) (GLbitfield mask);
typedef void (APIENTRYP PFNGLCLEARCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRYP PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef const GLubyte *(APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
typedef const GLubyte *(APIENTRYP PFNGLGETSTRINGIPROC) (GLenum name, GLuint index);
typedef void (APIENTRYP PFNGLGETINTEGERVPROC) (GLenum pname, GLint *data);
typedef void (APIENTRYP PFNGLENABLEPROC) (GLenum cap);
typedef void (APIENTRYP PFNGLDEPTHFUNCPROC) (GLenum func);

GLAPI int gladLoadGL(void);
GLAPI int gladLoadGLLoader(GLADloadproc);

GLAPI struct gladGLversionStruct GLVersion;

GLAPI PFNGLCLEARPROC glad_glClear;
#define glClear glad_glClear
GLAPI PFNGLCLEARCOLORPROC glad_glClearColor;
#define glClearColor glad_glClearColor
GLAPI PFNGLVIEWPORTPROC glad_glViewport;
#define glViewport glad_glViewport
GLAPI PFNGLGETSTRINGPROC glad_glGetString;
#define glGetString glad_glGetString
GLAPI PFNGLENABLEPROC glad_glEnable;
#define glEnable glad_glEnable
GLAPI PFNGLDEPTHFUNCPROC glad_glDepthFunc;
#define glDepthFunc glad_glDepthFunc

#ifdef __cplusplus
}
#endif

#endif