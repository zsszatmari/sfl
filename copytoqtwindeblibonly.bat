@echo off
rem cmake . -DCMAKE_BUILD_TYPE=RELEASE
rem cmake . -DCMAKE_BUILD_TYPE=DEBUG

mkdir ..\g-ear-qt\lib\win
copy debug\core.dll ..\g-ear-qt\lib\win
copy debug\core.lib ..\g-ear-qt\lib\win
rem copy debug\libcore.dll ..\g-ear-qt\lib\win
copy debug\libcore.a ..\g-ear-qt\lib\win
