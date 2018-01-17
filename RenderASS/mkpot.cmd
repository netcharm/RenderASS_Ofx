@echo off

set GETTEXT=xgettext
set MSGINIT=msginit
set MSGFMT=msgfmt
set MSGMERGE=msgmerge
set PACKAGE=RenderASS
set BINDING=locale

"%GETTEXT%" --package-name="%PACKAGE%" --package-version=1.0 --default-domain="%PACKAGE%" --c++ -k_ --from-code=utf-8 --output="%PACKAGE%.pot" *.cpp

mkdir -p zh_CN/LC_MESSAGES
rem "%MSGINIT%" --no-translator --locale zh_CN.UTF-8 --output-file "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.po" --input "%PACKAGE%.pot"

rem %MSGMERGE% -N "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.po" messages.po > "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.po"
%MSGMERGE% -U -N "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.po" "%PACKAGE%.pot"

"%MSGFMT%" --check --verbose --output-file "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.mo" "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.po"
