@echo off
rem cmake . -DCMAKE_BUILD_TYPE=RELEASE
rem cmake . -DCMAKE_BUILD_TYPE=DEBUG

mkdir platforms\qt\lib\win
rem copy release\core.dll platforms\qt\lib\win
rem copy release\core.lib  platforms\qt\lib\win
rem copy release\libcore.dll  platforms\qt\lib\win
copy libcore.a  platforms\qt\lib\win
copy lib\win\*.dll  platforms\qt\lib\win

rem rsync -av core/public/ ../G-Ear-v2/include/core/
rem xcopy not seen when invoked from sublime...
c:\windows\system32\xcopy /s /y core\public  platforms\qt\include\core 2>nul
rem rm -r ../G-Ear-v2/include/core
rem cp -r core/public ../G-Ear-v2/include/core


rem rm -r ../G-Ear-v2/lib/assets
rem mkdir ../G-Ear-v2/lib/assets
rem cp core/assets/* ../G-Ear-v2/lib/assets

rem cp core/assets/desktop/* ../G-Ear-v2/lib/assets
