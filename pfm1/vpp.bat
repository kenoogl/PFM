setlocal

set path_org=%path%
set VS=c:\Visual_CPP10
set VS_SDK=c:\Visual_CPP10_SDKs
set LIB=%VS%\VC\lib;%VS_SDK%\Lib
path %VS%\VC\bin;%VS%\VC\include;%VS%\common7\IDE;%VS_SDK%\Bin;%VS_SDK%\Include
CL /I%VS%\VC\include /I%VS_SDK%\Include %1 %2 %3 %4 %5 %6 %7 %8 %9 /link kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib
path ;
path %path_org%

endlocal