REM do rootcint for merit classes
REM
cd ..\src
set MERITINCPATH=%ANALYSISROOT%
echo %MERITINCPATH%
REM
%ROOTSYS%\bin\rootcint -f MeritCint.cxx -c -I%ANALYSISROOT%  -I%MERITROOT%/src -I%MERITROOT%/src/root analysis/Tuple.h analysis/RebinHist.h MultiPSF.h PSFtailCuts.h meritFoM.h MeritPlots.h MeritLinkDef.h
