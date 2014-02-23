@ECHO OFF
CLS

IF NOT "%1" == "" GOTO dragndrop

REM Show arguments
ColladaConv
ECHO.

SET cmdLine=
SET /P cmdLine=Enter arguments: 
ECHO.

ColladaConv %cmdLine%

ECHO.
GOTO exit


:dragndrop

SET outputDir=%~dp0..\Content
SET baseDir=%~dp0..\Content
IF NOT EXIST %outputDir%\models\%~n1 mkdir %outputDir%\models\%~n1
copy %1 %outputDir%\models\%~n1

%~d0 
cd %~dp0
ColladaConv models\%~n1\%~nx1 -base %baseDir% -dest %outputDir%

del %outputDir%\models\%~n1\%~nx1

:exit

PAUSE
