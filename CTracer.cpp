#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include "CTracer.h"

using namespace std;

CTracer::CTracer() :
   m_bInitialized(false),
   m_bLogToFile(false),
   m_bLogToDbg(false),
   m_dwMaxFiles(3),
   m_ullMaxFileSize(1024 * 1024 * 3), // 3 MB
   m_ullCurrentSize(0)
{
   ::SecureZeroMemory(m_wszLogFileName, sizeof(WCHAR) * MAX_PATH);

   Initialize();

   OverrideDefaults();

   InitializeCriticalSection(&m_cs);

   ULONGLONG ullFileSize = GetFileSize();

   if (ullFileSize > m_ullMaxFileSize)
   {
      RotateLogfile();
   }

}

CTracer::CTracer(const wchar_t* wszFullPath) :
   m_bInitialized(true),
   m_bLogToFile(true),
   m_bLogToDbg(false),
   m_dwMaxFiles(3),
   m_ullMaxFileSize(1024 * 1024 * 3), // 3MB
   m_ullCurrentSize(0)
{
   ::SecureZeroMemory(m_wszLogFileName, sizeof(WCHAR) * MAX_PATH);

   ::StringCchCopyW(m_wszLogFileName, MAX_PATH, wszFullPath);

   OverrideDefaults();

   InitializeCriticalSection(&m_cs);

   ULONGLONG ullFileSize = GetFileSize();

   if (ullFileSize > m_ullMaxFileSize)
   {
      RotateLogfile();
   }

}

CTracer::CTracer(const wchar_t* pwszFolderPath, const wchar_t* pwszFileName) :
   m_bInitialized(true),
   m_bLogToFile(true),
   m_bLogToDbg(false),
   m_dwMaxFiles(3),
   m_ullMaxFileSize(1024 * 1024 * 3), // 3 MB
   m_ullCurrentSize(0)
{
   OverrideDefaults();

   ::SecureZeroMemory(m_wszLogFileName, sizeof(WCHAR) * MAX_PATH);

   wchar_t wszWindowsFolder[MAX_PATH] = {0};

   if (0 == GetWindowsDirectoryW(wszWindowsFolder, MAX_PATH))
   {
      ::StringCchCopyW(wszWindowsFolder, MAX_PATH, L"C:\\Windows");
   }

   if ((NULL != pwszFolderPath) && (wcslen(pwszFolderPath) > 0))
   {
      ::StringCchCopyW(m_wszLogFileName, MAX_PATH, pwszFolderPath);
      ::StringCchCatW(m_wszLogFileName, MAX_PATH, L"\\");
      ::StringCchCatW(m_wszLogFileName, MAX_PATH, pwszFileName);
   }
   else if ((NULL != pwszFolderPath) && (wcslen(pwszFolderPath) == 0))
   {
      DWORD   dwType        = 0;
      DWORD   dwDataSize    = MAX_PATH;
      wchar_t wsz[MAX_PATH] = {0};
      HKEY    hKey          = NULL;

      LONG lResult = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, DF_LOG_REG_KEY, 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey);

      if (ERROR_SUCCESS == lResult)
      {
         lResult = ::RegQueryValueExW(hKey, DF_LOG_REG_VALUE_FOLDER, NULL, &dwType, (LPBYTE)wsz, &dwDataSize);

         if ((ERROR_SUCCESS == lResult) && (REG_SZ == dwType))
         {
            ::StringCchCopyW(m_wszLogFileName, MAX_PATH, wsz);
            ::StringCchCatW(m_wszLogFileName, MAX_PATH, L"\\");
            ::StringCchCatW(m_wszLogFileName, MAX_PATH, pwszFileName);
         }
         else
         {
            ::StringCchCopyW(m_wszLogFileName, MAX_PATH, wszWindowsFolder);
            ::StringCchCatW(m_wszLogFileName, MAX_PATH, L"\\Temp\\");
            ::StringCchCatW(m_wszLogFileName, MAX_PATH, pwszFileName);
         }

         ::RegCloseKey(hKey);
      }
      else
      {
         ::StringCchCopyW(m_wszLogFileName, MAX_PATH, wszWindowsFolder);
         ::StringCchCatW(m_wszLogFileName, MAX_PATH, L"\\Temp\\");
         ::StringCchCatW(m_wszLogFileName, MAX_PATH, pwszFileName);
      }
   }
   else
   {
      ::StringCchCopyW(m_wszLogFileName, MAX_PATH, wszWindowsFolder);
      ::StringCchCatW(m_wszLogFileName, MAX_PATH, L"\\Temp\\");
      ::StringCchCatW(m_wszLogFileName, MAX_PATH, pwszFileName);
   }

   InitializeCriticalSection(&m_cs);

   ULONGLONG ullFileSize = GetFileSize();

   if (ullFileSize > m_ullMaxFileSize)
   {
      RotateLogfile();
   }

}

CTracer::CTracer(bool blnOutputDebugView) :
   m_bInitialized(true),
   m_bLogToFile(false),
   m_bLogToDbg(blnOutputDebugView),
   m_dwMaxFiles(1),
   m_ullMaxFileSize(1024 * 1024 * 3), // 3 MB
   m_ullCurrentSize(0)
{
   ::SecureZeroMemory(m_wszLogFileName, sizeof(WCHAR) * MAX_PATH);

   InitializeCriticalSection(&m_cs);
}

CTracer::~CTracer()
{
   DeleteCriticalSection(&m_cs);
}

void CTracer::OverrideDefaults()
{
   HKEY hKey = NULL;

   LONG lResult = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, DF_LOG_REG_KEY, 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey);

   if (ERROR_SUCCESS == lResult)
   {
      DWORD dwType     = 0;
      DWORD dwSize     = sizeof(DWORD);
      DWORD dwMaxFiles = 0;

      //
      // Maximum number of files (RANGE: 1 to 10)
      //
      lResult = ::RegQueryValueExW(hKey, DF_LOG_REG_VALUE_MAX_FILES, NULL, &dwType, (LPBYTE) & dwMaxFiles, &dwSize);

      if ((ERROR_SUCCESS == lResult) && (REG_DWORD == dwType))
      {
         if ((dwMaxFiles < 1) || (dwMaxFiles > 10))
         {
            m_dwMaxFiles = 3;
         }
         else
         {
            m_dwMaxFiles = dwMaxFiles;
         }
      }
      else
      {
         m_dwMaxFiles = 3;
      }

      //
      // Maximum File Size (in MB) (RANGE: 1 to 10)
      //
      dwType = 0;
      dwSize                = sizeof(DWORD);
      DWORD dwMaxFileSizeMB = 0;

      lResult = ::RegQueryValueExW(hKey, DF_LOG_REG_VALUE_MAX_FILE_SIZE_MB, NULL, &dwType, (LPBYTE) & dwMaxFileSizeMB, &dwSize);

      if ((ERROR_SUCCESS == lResult) && (REG_DWORD == dwType))
      {
         if ((dwMaxFiles < 1) || (dwMaxFiles > 10))
         {
            m_ullMaxFileSize = 1024 * 1024 * 3;
         }
         else
         {
            m_ullMaxFileSize = 1024 * 1024 * dwMaxFileSizeMB;
         }
      }
      else
      {
         m_ullMaxFileSize = 1024 * 1024 * 3;
      }
   }
   ::RegCloseKey(hKey);
}

void CTracer::Initialize()
{
   HKEY hKey = NULL;

   LONG lResult = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, DF_LOG_REG_KEY, 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey);

   if (ERROR_SUCCESS == lResult)
   {
      DWORD dwType     = 0;
      DWORD dwValue    = 0;
      DWORD dwDataSize = sizeof(DWORD);

      lResult = ::RegQueryValueExW(hKey, DF_LOG_REG_VALUE_DETAIL, NULL, &dwType, (LPBYTE) & dwValue, &dwDataSize);

      if ((ERROR_SUCCESS == lResult) && (REG_DWORD == dwType))
      {
         if (DF_LOG_TO_DBG & dwValue)
         {
            m_bLogToDbg  = true;
            m_bLogToFile = false;
         }

         //
         // If LogFolder registry key exists, get Logging Folder
         // Else defaults to Temp in Windows Directory
         //
         dwType = 0;
         dwDataSize                      = MAX_PATH;
         wchar_t wszFolderName[MAX_PATH] = {0};
         std::wstring wstrFolderName;

         //
         // If LogFolder registry key exists, get Log folder
         // Else defaults to Temp in Windows Directory
         //
         lResult = ::RegQueryValueExW(hKey, DF_LOG_REG_VALUE_FOLDER, NULL, &dwType, (LPBYTE)wszFolderName, &dwDataSize);

         if ((ERROR_SUCCESS == lResult) && (REG_SZ == dwType))
         {
            wstrFolderName = wszFolderName;
            wstrFolderName += L"\\";
         }
         else
         {
            wchar_t wszWindowsFolder[MAX_PATH] = {0};

            if (0 != GetWindowsDirectoryW(wszWindowsFolder, MAX_PATH))
            {
               wstrFolderName = wszWindowsFolder;
               wstrFolderName += L"\\Temp\\";
            }
            else
            {
               wstrFolderName = L"C:\\Windows\\Temp\\";
            }
         }

         //
         // Construct Log Filename from EITHER the Module name
         // OR defaults to a Temp File name
         //
         wchar_t wszModuleName[MAX_PATH] = {0};
         wchar_t wszFileName[MAX_PATH] = {0};
         std::wstring wstrModuleName;
         std::wstring wstrFileName;

         if (0 != ::GetModuleFileNameW(NULL, wszModuleName, MAX_PATH))
         {
            wstrModuleName = wszModuleName;

            size_t posLastSlash = wstrModuleName.find_last_of(L"\\");
            wstrFileName        = wstrModuleName.substr(posLastSlash + 1, wstrModuleName.size() - posLastSlash - 1);

            size_t posLastDot = wstrFileName.find_last_of(L".");
            wstrFileName.replace(wstrFileName.begin() + posLastDot + 1, wstrFileName.end(), L"log");

            ::StringCchCopyW(m_wszLogFileName, MAX_PATH, wstrFolderName.c_str());
            ::StringCchCatW(m_wszLogFileName, MAX_PATH, wstrFileName.c_str());
         }
         else
         {
            if (0 != ::GetTempFileNameW(wstrFolderName.c_str(), L"FAR", 0, wszFileName))
            {
               wstrFileName = wszFileName;

               size_t posLastDot = wstrFileName.find_last_of(L".");
               wstrFileName.replace(wstrFileName.begin() + posLastDot + 1, wstrFileName.end(), L"log");

               ::StringCchCopyW(m_wszLogFileName, MAX_PATH, wstrFileName.c_str());
            }
         }

         m_bLogToFile = true;
      }

      ::RegCloseKey(hKey);
   }

   m_bInitialized = true;
}

void CTracer::Log(const wchar_t* wszID, ...)
{
   if (m_bLogToDbg || m_bLogToFile)
   {
      wchar_t msg[3 * MAX_PATH] = {0};

      va_list argslist;

      va_start(argslist, wszID);

      wchar_t* wszFormat = va_arg(argslist, wchar_t*);

      ::StringCchVPrintfW(msg, 3*MAX_PATH, wszFormat, argslist);

      va_end(argslist);

      WriteToFile(GetCurrentProcessId(), GetCurrentThreadId(), msg, wszID);
   }
}

void CTracer::LogDebug(const wchar_t* wszID, const char* szName, ...)
{
   if (m_bLogToDbg || m_bLogToFile)
   {
      wchar_t wszFinalMsg[4 * MAX_PATH] = {0};

      if (NULL != wszFinalMsg)
      {
         wchar_t msg[3 * MAX_PATH] = {0};

         va_list argslist;

         va_start(argslist, szName);

         wchar_t* wszFormat = va_arg(argslist, wchar_t*);

         StringCchVPrintfW(msg, 3*MAX_PATH, wszFormat, argslist);

         va_end(argslist);

         ::StringCchPrintfW(wszFinalMsg, 4 * MAX_PATH, L"%S - %s", szName, msg);

         WriteToFile(GetCurrentProcessId(), GetCurrentThreadId(), wszFinalMsg, wszID);
      }
   }
}

void CTracer::LogError(const wchar_t* wszMessage, DWORD dwError)
{
   if (m_bLogToDbg || m_bLogToFile)
   {
      // Buffer that gets the error message string
      LPWSTR wpszBuf = NULL;
      // HLOCAL hlocal = NULL;

      DWORD dwSystemLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

      DWORD dwOK = ::FormatMessageW(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
         NULL,
         dwError,
         dwSystemLocale,
         (LPWSTR) & wpszBuf,
         NULL,
         NULL);

      if (dwOK == 0)
      {
         // Is it a WMI-related error?
         HMODULE hDll = ::LoadLibraryEx(TEXT("wmiutils.dll"), NULL, DONT_RESOLVE_DLL_REFERENCES);

         if (hDll != NULL)
         {
            dwOK = ::FormatMessageW(
               FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK,
               hDll,
               dwError,
               dwSystemLocale,
               (LPWSTR) & wpszBuf,
               NULL,
               NULL);

            ::FreeLibrary(hDll);
         }
      }

      if (dwOK == 0)
      {
         // Is it a network-related error?
         HMODULE hDll = ::LoadLibraryEx(TEXT("netmsg.dll"), NULL, DONT_RESOLVE_DLL_REFERENCES);

         if (hDll != NULL)
         {
            dwOK = ::FormatMessageW(
               FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK,
               hDll,
               dwError,
               dwSystemLocale,
               (LPWSTR) & wpszBuf,
               NULL,
               NULL);

            ::FreeLibrary(hDll);
         }
      }

      wchar_t wszMsg[3 * MAX_PATH] = {0};

      ::StringCchPrintfW(wszMsg, 3 * MAX_PATH, L"%s 0x%08X - %s", wszMessage, dwError, wpszBuf);

      wchar_t wszTag[] = L"E";

      WriteToFile(GetCurrentProcessId(), GetCurrentThreadId(), wszMsg, wszTag);

      ::LocalFree(wpszBuf);
   }
}

void CTracer::LogBool (const wchar_t* pwszID, const wchar_t* wszMessage, bool blnRet)
{
   if (m_bLogToDbg || m_bLogToFile)
   {
      wchar_t wszMsg[3 * MAX_PATH] = {0};

      ::StringCchPrintfW(wszMsg, 3*MAX_PATH, L"%s - %s", wszMessage, (blnRet ? L"TRUE" : L"FALSE"));

      WriteToFile(GetCurrentProcessId(), GetCurrentThreadId(), wszMsg, pwszID);
   }
}

void CTracer::LogRaw_A(const wchar_t* pwszTitle, const char* pszMessage, ULONG ulLength)
{
   if (m_bLogToFile)
   {
      EnterCriticalSection(&m_cs);

      std::wofstream ofs;
      ofs.open(m_wszLogFileName, std::ios_base::app);
      ofs << pwszTitle << std::endl;
      ofs.close();

      LeaveCriticalSection(&m_cs);
   }

   if (m_bLogToFile)
   {
      EnterCriticalSection(&m_cs);

      std::ofstream ofs;
      ofs.open(m_wszLogFileName, std::ios_base::app);

      for (unsigned int k = 0; k < ulLength; k++)
      {
         BYTE sz = static_cast<unsigned char>(pszMessage[k]);

         // ofs << std::showbase << std::setiosflags(ios::internal) << std::setfill('0') << std::setw(4) << std::hex << (int)sz << std::dec << " ";
         ofs << "0x" << std::setiosflags(std::ios_base::internal) << std::setfill('0') << std::setw(2) << std::hex << (int)sz << std::dec << " ";

         if ((k + 1) % 16 == 0)
         {
            ofs << std::endl;
         }
      }

      ofs << std::endl;

      ofs.close();

      LeaveCriticalSection(&m_cs);
   }
}

void CTracer::LogRaw_W(const wchar_t* pwszTitle, const wchar_t* pwszMessage, ULONG ulLength)
{
   if (m_bLogToFile)
   {
      EnterCriticalSection(&m_cs);

      std::wofstream ofs;
      ofs.open(m_wszLogFileName, std::ios_base::app);
      ofs << pwszTitle << std::endl;
      ofs.close();

      LeaveCriticalSection(&m_cs);
   }

   if (m_bLogToFile)
   {
      EnterCriticalSection(&m_cs);

      std::wofstream ofs;
      ofs.open(m_wszLogFileName, std::ios_base::app);

      for (unsigned int k = 0; k < ulLength; k++)
      {
         BYTE sz = static_cast<unsigned char>(pwszMessage[k]);

         // ofs << std::showbase << std::setiosflags(ios::internal) << std::setfill(L'0') << std::setw(4) << std::hex << (int)sz << std::dec << " ";
         ofs << "0x" << std::setiosflags(std::ios_base::internal) << std::setfill(L'0') << std::setw(2) << std::hex << (int)sz << std::dec << " ";

         if ((k + 1) % 16 == 0)
         {
            ofs << std::endl;
         }
      }

      ofs << std::endl;

      ofs.close();

      LeaveCriticalSection(&m_cs);
   }
}

void CTracer::LogFileRaw_A (const wchar_t* pwszFilename, const char* pszMessage, ULONG ulLength, bool blnBinary)
{
   if (m_bLogToFile)
   {
      WCHAR wszUniqueFileName[1024] = {0};
      SYSTEMTIME systemTime;
      GetLocalTime(&systemTime);

      std::wstring wstrLogFileName = m_wszLogFileName;
      size_t       posLastSlash    = wstrLogFileName.find_last_of(L"\\");
      std::wstring wstrFolder      = wstrLogFileName.substr(0, posLastSlash + 1);

      HRESULT hr = ::StringCchPrintfW(wszUniqueFileName, 1024,
         L"%s%04d%02d%02d%02d%02d%02d_%lu_%s",
         wstrFolder.c_str(),
         systemTime.wYear,
         systemTime.wMonth,
         systemTime.wDay,
         systemTime.wHour,
         systemTime.wMinute,
         systemTime.wSecond,
         ulLength,
         pwszFilename);

      if (blnBinary)
      {
         std::ofstream ofs;
         ofs.open(wszUniqueFileName, std::ios_base::out | std::ios_base::binary);
         // ofs.write(reinterpret_cast< const char*>(pszMessage), ulLength);
         ofs.write(pszMessage, ulLength);
         ofs.close();
      }
      else
      {
         std::ofstream ofs;
         ofs.open(wszUniqueFileName, std::ios_base::out);
         ofs << pszMessage;
         ofs.close();
      }
   }
}

void CTracer::LogFileRaw_W (const wchar_t* pwszFilename, const wchar_t* pwszMessage, ULONG ulLength, bool blnBinary)
{
   if (m_bLogToFile)
   {
      WCHAR wszUniqueFileName[1024] = {0};
      SYSTEMTIME systemTime;
      GetLocalTime(&systemTime);

      std::wstring wstrLogFileName = m_wszLogFileName;
      size_t       posLastSlash    = wstrLogFileName.find_last_of(L"\\");
      std::wstring wstrFolder      = wstrLogFileName.substr(0, posLastSlash + 1);

      HRESULT hr = ::StringCchPrintfW(wszUniqueFileName, 1024,
         L"%s%04d%02d%02d%02d%02d%02d_%lu_%s",
         wstrFolder.c_str(),
         systemTime.wYear,
         systemTime.wMonth,
         systemTime.wDay,
         systemTime.wHour,
         systemTime.wMinute,
         systemTime.wSecond,
         ulLength,
         pwszFilename);

      if (blnBinary)
      {
         std::wofstream ofs;
         ofs.open(wszUniqueFileName, std::ios_base::out | std::ios_base::binary);
         ofs.write(pwszMessage, ulLength);
         ofs.close();
      }
      else
      {
         std::wofstream ofs;
         ofs.open(wszUniqueFileName, std::ios_base::out);
         ofs << pwszMessage;
         ofs.close();
      }
   }
}

void CTracer::WriteToFile(DWORD dwProcessID, DWORD dwThreadId, const wchar_t* pwszMsg, const wchar_t* pwszTag)
{
   if (m_bLogToDbg)
   {
      WCHAR wszMsg[1024] = {0};

      HRESULT hr = ::StringCchPrintfW(wszMsg, 1024,
         L"[%5d][%5d] [%s] :: %s\n",
         dwProcessID,
         dwThreadId,
         pwszTag,
         pwszMsg);

      if (S_OK != hr)
      {
         if (STRSAFE_E_INSUFFICIENT_BUFFER == hr)
         {
            // copy operation failed due to insufficient buffer space
            // destination buffer contains a truncated, null-terminated version of the intended result
            wszMsg[1022] = L'\n';
            wszMsg[1023] = L'\0';
         }
      }

      ::OutputDebugStringW(wszMsg);
   }

   if (m_bLogToFile)
   {
      WCHAR wszMsg[1024] = {0};
      SYSTEMTIME systemTime;
      GetLocalTime(&systemTime);

      HRESULT hr = ::StringCchPrintfW(wszMsg, 1024,
         L"%04d-%02d-%02d %02d:%02d:%02d [%5d][%5d] [%s] :: %s\n",
         systemTime.wYear,
         systemTime.wMonth,
         systemTime.wDay,
         systemTime.wHour,
         systemTime.wMinute,
         systemTime.wSecond,
         dwProcessID,
         dwThreadId,
         pwszTag,
         pwszMsg);

      if (S_OK != hr)
      {
         if (STRSAFE_E_INSUFFICIENT_BUFFER == hr)
         {
            // copy operation failed due to insufficient buffer space
            // destination buffer contains a truncated, null-terminated version of the intended result
            wszMsg[1022] = L'\n';
            wszMsg[1023] = L'\0';
         }
      }

      std::wofstream ofs;

      EnterCriticalSection(&m_cs);

      ofs.open(m_wszLogFileName, std::ios_base::app);
      ofs << wszMsg;
      ofs.close();

      LeaveCriticalSection(&m_cs);
   }

}

void CTracer::WriteToFileRaw(const wchar_t* pwszMsg)
{
   std::wofstream ofs;

   EnterCriticalSection(&m_cs);

   ofs.open(m_wszLogFileName, std::ios_base::app);
   ofs << pwszMsg << std::endl;
   ofs.close();

   LeaveCriticalSection(&m_cs);
}

bool CTracer::IsFileExisting(const wchar_t* pwszFile)
{
   if (pwszFile == NULL)
   {
      return false;
   }

   WIN32_FIND_DATAW FindData;
   HANDLE hFind;

   hFind = FindFirstFileW(pwszFile, &FindData);

   if (INVALID_HANDLE_VALUE == hFind)
   {
      return false;
   }
   else
   {
      return true;
   }
}

// Rotate files:
// log.txt -> log.txt.1
// log.txt.1 -> log.txt.2
// log.txt.2 -> log.txt.3
// log.txt.3 -> delete
void CTracer::RotateLogfile()
{
   for (int i = m_dwMaxFiles; i > 0; --i)
   {
      wchar_t wszSource[MAX_PATH] = {0};
      wchar_t wszTarget[MAX_PATH] = {0};

      if (i > 1)
      {
         ::StringCchPrintfW(wszSource, MAX_PATH, L"%s.%d", m_wszLogFileName, i - 1);
         ::StringCchPrintfW(wszTarget, MAX_PATH, L"%s.%d", m_wszLogFileName, i);
      }
      else
      {
         ::StringCchPrintfW(wszSource, MAX_PATH, L"%s", m_wszLogFileName);
         ::StringCchPrintfW(wszTarget, MAX_PATH, L"%s.%d", m_wszLogFileName, i);
      }

      if (IsFileExisting(wszTarget))
      {
         _wremove(wszTarget);
      }

      if (IsFileExisting(wszSource))
      {
         _wrename(wszSource, wszTarget);
      }
   }
}

ULONGLONG CTracer::GetFileSize()
{
   // ULONG ulSize = 0;
   //
   // WIN32_FIND_DATAW FindData;
   // HANDLE hFind;
   //
   // hFind = FindFirstFileW(m_wszLogFileName, &FindData);
   //
   // if (INVALID_HANDLE_VALUE == hFind)
   // {
   // ulSize = 0;
   // }
   // else
   // {
   // ulSize = (FindData.nFileSizeHigh * (MAXDWORD + 1)) + FindData.nFileSizeLow;
   // }
   //
   // return ulSize;

   /*
    __int64 FileSize(const wchar_t* name)
    {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
    return -1; // error condition, could call GetLastError to find out more
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
    }
    */

   HANDLE hFile = CreateFileW(
      m_wszLogFileName,
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      NULL);

   if (hFile == INVALID_HANDLE_VALUE)
   {
      return -1; // error condition, could call GetLastError to find out more
   }

   LARGE_INTEGER size;

   if (0 == GetFileSizeEx(hFile, &size))
   {
      CloseHandle(hFile);
      return -1; // error condition, could call GetLastError to find out more
   }

   CloseHandle(hFile);

   return size.QuadPart;
}
