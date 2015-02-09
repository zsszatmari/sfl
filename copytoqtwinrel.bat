@echo off
rem cmake . -DCMAKE_BUILD_TYPE=RELEASE
rem cmake . -DCMAKE_BUILD_TYPE=DEBUG

mkdir ..\g-ear-qt\lib\win
copy release\core.dll ..\g-ear-qt\lib\win
copy release\core.lib ..\g-ear-qt\lib\win
rem copy release\libcore.dll ..\g-ear-qt\lib\win
copy release\libcore.a ..\g-ear-qt\lib\win
copy lib\win\*.dll ..\g-ear-qt\lib\win

rem rsync -av core/public/ ../G-Ear-v2/include/core/
xcopy /s /y core\public ..\g-ear-qt\include\core 
rem rm -r ../G-Ear-v2/include/core
rem cp -r core/public ../G-Ear-v2/include/core


rem rm -r ../G-Ear-v2/lib/assets
rem mkdir ../G-Ear-v2/lib/assets
rem cp core/assets/* ../G-Ear-v2/lib/assets

rem cp core/assets/desktop/* ../G-Ear-v2/lib/assets
