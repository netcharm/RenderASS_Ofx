@echo off

set GETTEXT=xgettext
set MSGINIT=msginit
set MSGFMT=msgfmt
set MSGMERGE=msgmerge
set PACKAGE=RenderASS
set BINDING=locale

"%GETTEXT%" --package-name="%PACKAGE%" --package-version=1.0 --default-domain="%PACKAGE%" --c++ -k_ --from-code=utf-8 --output="%PACKAGE%.pot" *.cpp

if exist "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.po" (
  %MSGMERGE% -U -N "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.po" "%PACKAGE%.pot"
  "%MSGFMT%" --check --verbose --output-file "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.mo" "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.po"
) else (
  mkdir "%BINDING%\zh_CN\LC_MESSAGES"
  "%MSGINIT%" --no-translator --locale zh_CN.UTF-8 --output-file "%BINDING%\zh_CN\LC_MESSAGES\%PACKAGE%.po" --input "%PACKAGE%.pot"
)

if exist "%BINDING%\zh_TW\LC_MESSAGES\%PACKAGE%.po" (
  %MSGMERGE% -U -N "%BINDING%\zh_TW\LC_MESSAGES\%PACKAGE%.po" "%PACKAGE%.pot"
  "%MSGFMT%" --check --verbose --output-file "%BINDING%\zh_TW\LC_MESSAGES\%PACKAGE%.mo" "%BINDING%\zh_TW\LC_MESSAGES\%PACKAGE%.po"
) else (
  mkdir "%BINDING%\zh_TW\LC_MESSAGES"
  "%MSGINIT%" --no-translator --locale zh_TW.UTF-8 --output-file "%BINDING%\zh_TW\LC_MESSAGES\%PACKAGE%.po" --input "%PACKAGE%.pot"
)

if exist "%BINDING%\jp\LC_MESSAGES\%PACKAGE%.po" (
  %MSGMERGE% -U -N "%BINDING%\jp\LC_MESSAGES\%PACKAGE%.po" "%PACKAGE%.pot"
  "%MSGFMT%" --check --verbose --output-file "%BINDING%\jp\LC_MESSAGES\%PACKAGE%.mo" "%BINDING%\jp\LC_MESSAGES\%PACKAGE%.po"
) else (
  mkdir "%BINDING%\jp\LC_MESSAGES"
  "%MSGINIT%" --no-translator --locale jp.UTF-8 --output-file "%BINDING%\jp\LC_MESSAGES\%PACKAGE%.po" --input "%PACKAGE%.pot"
)

