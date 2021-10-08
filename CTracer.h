#pragma once

#include <windows.h>

#define DF_LOG_REG_KEY                    L"SOFTWARE\\Company\\Product"
#define DF_LOG_REG_VALUE_PATH             L"LogPath"
#define DF_LOG_REG_VALUE_DETAIL           L"LogDetails"
#define DF_LOG_REG_VALUE_FOLDER           L"LogFolder"
#define DF_LOG_REG_VALUE_MAX_FILES        L"LogMaxFiles"
#define DF_LOG_REG_VALUE_MAX_FILE_SIZE_MB L"LogMaxFileSizeMB"
#define DF_LOG_REG_VALUE_TRUNCATE         L"LogTruncate"

#define DF_LOG_TO_FILE                    1
#define DF_LOG_TO_DBG                     2

class CTracer
{
private:
   bool      m_bInitialized;
   bool      m_bLogToFile;
   bool      m_bLogToDbg;
   DWORD     m_dwMaxFiles;
   ULONGLONG m_ullMaxFileSize;
   ULONGLONG m_ullCurrentSize;

   wchar_t m_wszLogFileName[MAX_PATH];

   CRITICAL_SECTION m_cs;

   void Initialize();
   void WriteLastError();
   void RotateLogfile();
   ULONGLONG GetFileSize();
   void OverrideDefaults();

public:
   CTracer();
   CTracer(bool blnOutputDebugView);
   CTracer(const wchar_t* pwszFullPath);
   CTracer(const wchar_t* pwszFolderPath, const wchar_t* pwszFileName);

   ~CTracer();

   void Log(const wchar_t* pwszID, ...);
   void LogBool (const wchar_t* pwszID, const wchar_t* pwszMessage, bool blnRet);
   void LogDebug(const wchar_t* pwszID, const char* pszFunction, ...);

   void LogError (const wchar_t* pwszMessage, DWORD dwError);

   void LogRaw_A(const wchar_t* pwszTitle, const char* pszMessage, ULONG ulLength);
   void LogRaw_W(const wchar_t* pwszTitle, const wchar_t* pwszMessage, ULONG ulLength);

   void LogFileRaw_A (const wchar_t* pwszFilename, const char* pszMessage, ULONG ulLength, bool blnBinary);
   void LogFileRaw_W (const wchar_t* pwszFilename, const wchar_t* pwszMessage, ULONG ulLength, bool blnBinary);

   void WriteToFile(DWORD dwProcessID, DWORD dwThreadId, const wchar_t* pwszMsg, const wchar_t* pwszTag);
   void WriteToFileRaw(const wchar_t* pwszMsg);

   bool IsFileExisting(const wchar_t* pwszFile);

   inline bool IsEnabled() {return (m_bLogToDbg || m_bLogToFile); }
};
