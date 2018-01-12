@echo off

set Z=d:\app\7-zip\7z.exe
set TARGET=%~dp0%

:RenderASS
pushd "RenderASS\_out\x64\Debug"
"%Z%" a "%TARGET%RenderASS.ofx.bundle_x64d.7z" "RenderASS.ofx.bundle" 
popd
pushd "RenderASS\_out\x64\Release"
%Z% a "%TARGET%RenderASS.ofx.bundle_x64.7z" "RenderASS.ofx.bundle" 
popd
pushd "RenderASS\_out\x86\Debug"
%Z% a "%TARGET%RenderASS.ofx.bundle_x86d.7z" "RenderASS.ofx.bundle" 
popd
pushd "RenderASS\_out\x86\Release"
%Z% a "%TARGET%RenderASS.ofx.bundle_x86.7z" "RenderASS.ofx.bundle" 
popd

:TawawaBlue
pushd "TawawaBlue\_out\x64\Debug"
dir
%Z% a "%TARGET%TawawaBlue.ofx.bundle_x64d.7z" "TawawaBlue.ofx.bundle" 
popd
pushd "TawawaBlue\_out\x64\Release"
%Z% a "%TARGET%TawawaBlue.ofx.bundle_x64.7z" "TawawaBlue.ofx.bundle" 
popd
pushd "TawawaBlue\_out\x86\Debug"
%Z% a "%TARGET%TawawaBlue.ofx.bundle_x86d.7z" "TawawaBlue.ofx.bundle" 
popd
pushd "TawawaBlue\_out\x86\Release"
%Z% a "%TARGET%TawawaBlue.ofx.bundle_x86.7z" "TawawaBlue.ofx.bundle" 
popd

:end
