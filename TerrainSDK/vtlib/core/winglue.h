//
// these definitions let us include gl.h without the entire Windows headers
//

#ifndef WIN_GL_GLUE_H
#define WIN_GL_GLUE_H

#ifndef NULL
#define NULL 0
#endif

   /* XXX This is from Win32's <windef.h> */
#  ifndef APIENTRY
#   define GLUT_APIENTRY_DEFINED
#   if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#    define APIENTRY    __stdcall
#   else
#    define APIENTRY
#   endif
#  endif
   /* XXX This is from Win32's <winnt.h> */
#  ifndef CALLBACK
#   if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#    define CALLBACK __stdcall
#   else
#    define CALLBACK
#   endif
#  endif
   /* XXX This is from Win32's <wingdi.h> and <winnt.h> */
#  ifndef WINGDIAPI
#   define GLUT_WINGDIAPI_DEFINED
#   define WINGDIAPI __declspec(dllimport)
#  endif
   /* XXX This is from Win32's <ctype.h> */
#  ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#   define _WCHAR_T_DEFINED
#  endif

#if 1

//#define WINGDIAPI DECLSPEC_IMPORT
#define DECLSPEC_IMPORT __declspec(dllimport)
#define WINBASEAPI DECLSPEC_IMPORT
#define VOID void

#ifndef _WINDEF_
typedef unsigned long       DWORD;
#define WINAPI      __stdcall
#define APIENTRY    __stdcall
#define CALLBACK    __stdcall
#endif

#endif

#endif // WIN_GL_GLUE_H
