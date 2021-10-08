#ifndef _COMMON_H
#define _COMMON_H

#include <windows.h>
#include <wtsapi32.h>

#include <stdio.h>
#include <errno.h>

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#define BUFSIZE             256

//
// set ENABLE_TRACELOG here (below) for universal logging
//
#define ENABLE_TRACELOG     1

#define DF_NULL_LOG(...)

#define DF_INFO_LOG(...)                     \
{                                            \
   if (g_objTracer.IsEnabled())              \
   {                                         \
      g_objTracer.Log(L"I", __VA_ARGS__); \
   }                                         \
}

#define DF_INFO_BOOL_LOG(wszMessage, blnRet)            \
{                                                       \
   if (g_objTracer.IsEnabled())                         \
   {                                                    \
      g_objTracer.LogBool(L"I", wszMessage, blnRet); \
   }                                                    \
}

#define DF_ERROR_LOG(wszMessage, dwError)        \
{                                                \
   if (g_objTracer.IsEnabled())                  \
   {                                             \
      g_objTracer.LogError(wszMessage, dwError); \
   }                                             \
}

#if (ENABLE_TRACELOG != 0)

#define DF_TRACE_LOG(...)                     \
{                                             \
   if (g_objTracer.IsEnabled())               \
   {                                          \
      g_objTracer.Log(L"D", __VA_ARGS__); \
   }                                          \
}

#define DF_BOOL_LOG(wszMessage, blnRet)                  \
{                                                        \
   if (g_objTracer.IsEnabled())                          \
   {                                                     \
      g_objTracer.LogBool(L"D", wszMessage, blnRet); \
   }                                                     \
}

#define DF_DEBUG_LOG(...)                                        \
{                                                                \
   if (g_objTracer.IsEnabled())                                  \
   {                                                             \
      g_objTracer.LogDebug(L"D", __FUNCTION__, __VA_ARGS__); \
   }                                                             \
}

#define DF_TRACE_LOG_RAW_A(pszTitle, pszMessage, iLength) \
{                                                         \
   if (g_objTracer.IsEnabled())                           \
   {                                                      \
     g_objTracer.LogRaw_A(pszTitle, pszMessage, iLength); \
   }                                                      \
}

#define DF_TRACE_LOG_RAW_W(pwszTitle, pwszMessage, iLength) \
{                                                           \
   if (g_objTracer.IsEnabled())                             \
   {                                                        \
     g_objTracer.LogRaw_W(pwszTitle, pwszMessage, iLength); \
   }                                                        \
}

#define DF_TRACE_TEXT_FILE_A(pwszFileName, pszMessage, iLength)          \
{                                                                        \
   if (g_objTracer.IsEnabled())                                          \
   {                                                                     \
     g_objTracer.LogFileRaw_A(pwszFileName, pszMessage, iLength, false); \
   }                                                                     \
}

#define DF_TRACE_BINARY_FILE_A(pwszFileName, pszMessage, iLength)       \
{                                                                       \
   if (g_objTracer.IsEnabled())                                         \
   {                                                                    \
     g_objTracer.LogFileRaw_A(pwszFileName, pszMessage, iLength, true); \
   }                                                                    \
}

#define DF_TRACE_TEXT_FILE_W(pwszFileName, pwszMessage, iLength)          \
{                                                                         \
   if (g_objTracer.IsEnabled())                                           \
   {                                                                      \
     g_objTracer.LogFileRaw_W(pwszFileName, pwszMessage, iLength, false); \
   }                                                                      \
}

#define DF_TRACE_BINARY_FILE_W(pwszFileName, pwszMessage, iLength)       \
{                                                                        \
   if (g_objTracer.IsEnabled())                                          \
   {                                                                     \
     g_objTracer.LogFileRaw_W(pwszFileName, pwszMessage, iLength, true); \
   }                                                                     \
}

#else

#define DF_TRACE_LOG(...)
#define DF_BOOL_LOG(...)
#define DF_DEBUG_LOG(...)

#define DF_TRACE_LOG_RAW_A(...)
#define DF_TRACE_LOG_RAW_W(...)

#define DF_TRACE_TEXT_FILE_A(...)
#define DF_TRACE_BINARY_FILE_A(...)

#define DF_TRACE_TEXT_FILE_W(...)
#define DF_TRACE_BINARY_FILE_W(...)

#define df_trace_log(pwszFormat, pwszMessage)

#endif

#define __DEBUG_LOG( format, ... )
#define __DEBUG_DF_ASSERT(assertion)
#define __DEBUG_BREAKPOINT()

// DEBUG builds only, override __DEBUG_LOG and __DEBUG_DF_ASSERT to no-ops
#ifdef _DEBUG
// DF_ALLOWDEBUGGER_BREAK controls whether or not the assert macros
// call DebugBreak() [i.e. Debugger break]

// If the file including us has already define DF_ALLOWDEBUGGER_BREAK then we do not
// override it, only if DF_ALLOWDEBUGGER_BREAK is not defined do we define it.

// g_objTracer.WriteTrace(GetCurrentProcessId(), GetCurrentThreadId(), msg );

#ifndef DF_ALLOWDEBUGGER_BREAK
#define  DF_ALLOWDEBUGGER_BREAK   0
#endif

#undef __DEBUG_LOG
#define __DEBUG_LOG(...)                                         \
{                                                                \
   if (g_objTracer.IsEnabled())                                  \
   {                                                             \
      g_objTracer.LogDebug(L"D", __FUNCTION__, __VA_ARGS__); \
   }                                                             \
}

#if (DF_ALLOWDEBUGGER_BREAK > 0)
#undef __DEBUG_BREAKPOINT
#define __DEBUG_BREAKPOINT()     DebugBreak()
#endif

#undef __DEBUG_DF_ASSERT
#define __DEBUG_DF_ASSERT(assertion)                                                                            \
do                                                                                                              \
{                                                                                                               \
   if ( !((assertion)) )                                                                                        \
   {                                                                                                            \
      __DEBUG_LOG(L"#####  File %S (line %d): Soft assertion \"%S\" failed\n", __FILE__, __LINE__, #assertion); \
      __DEBUG_BREAKPOINT();                                                                                     \
   }                                                                                                            \
} while ( 0 )

#endif // DBG

#endif //_COMMON_H
