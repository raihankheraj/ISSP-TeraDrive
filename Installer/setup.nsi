!include MUI2.nsh


; The name of the installer
Name "TeraDrive Hard Disk Drive S.M.A.R.T Attribute Collector"

; The file to write
OutFile "setup.exe"

; Request application privileges for Windows Vista and higher
RequestExecutionLevel admin

; Build Unicode installer
Unicode True

; The default installation directory
InstallDir $PROGRAMFILES\TeraDrive

!define MUI_HEADERIMAGE

!define MUI_HEADERIMAGE_BITMAP "TeraDrive.bmp"

!define MUI_HEADERIMAGE_BITMAP_STRETCH "NoStretchNoCrop"

!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_LICENSE "License.rtf"

!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES
;--------------------------------

; Pages

;Page directory
;Page instfiles

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File libcrypto-1_1.dll
  File libssl-1_1.dll
  File mysqlcppconn-9-vs14.dll
  File SmartHDDGUI.exe
  File SmartHDDGUI.ini
  
SectionEnd

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"