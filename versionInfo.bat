@ echo off
rem %1 is expected to be the SVN repo root
if exist "%SVNDIR%\SubWCRev.exe" ( "%SVNDIR%\SubWCRev.exe" %1 versionInfo.tags .\versionInfo.inc )
if not exist "%SVNDIR%\SubWCRev.exe" ( 
  echo "SVN Commandline Tool not installed! Please, download from https://tortoisesvn.net/ Using DUMMY Version Info..."
  copy /Y versionInfo.dummy .\versionInfo.inc
)
