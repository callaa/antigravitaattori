#ifndef _EXTENSIONS_H_
#define _EXTENSIONS_H_

#if 0
#define HAVE_MULTITEX
#ifdef WIN32
#define MAPIENTRY __stdcall
#else
#define MAPIENTRY
#endif

typedef void (MAPIENTRY *MFNGLMULTITEXCOORD2FVPROC) (GLenum target, const GLfloat *v);
typedef void (MAPIENTRY *MFNGLACTIVETEXTUREARBPROC) (GLenum texture);

extern MFNGLMULTITEXCOORD2FVPROC mglMultiTexCoord2fv;
extern MFNGLACTIVETEXTUREARBPROC mglActiveTextureARB;
#endif

#endif

