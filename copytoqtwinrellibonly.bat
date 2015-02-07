@echo off
rem cmake . -DCMAKE_BUILD_TYPE=RELEASE
rem cmake . -DCMAKE_BUILD_TYPE=DEBUG

mkdir ..\g-ear-qt\lib\win
copy release\core.dll ..\g-ear-qt\lib\win
copy release\core.lib ..\g-ear-qt\lib\win
rem copy release\libcore.dll ..\g-ear-qt\lib\win
copy release\libcore.a ..\g-ear-qt\lib\win

