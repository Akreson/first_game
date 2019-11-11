@echo Start Font Asset Build

@echo off

REM Set as Post-Build Event
set PathToBinary=..\Debug\bin\
set FontPath=DevData\LiberationMono-Regular.ttf

IF EXIST data\font.edg del data\font.edg

%PathToBinary%FontAssetBuild.exe %FontPath%
