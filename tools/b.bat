@echo off
call %~dp0\killsymbolserver.bat

IF NOT EXIST "%~dp0..\build" mkdir %~dp0..\build
pushd %~dp0..\build
@set /A _tic=%time:~0,2%*3600^
            +%time:~3,1%*10*60^
            +%time:~4,1%*60^
            +%time:~6,1%*10^
            +%time:~7,1% >nul
			
cl.exe /std:c++20 /MD ^
%~dp0..\src\engine\world\world.cpp ^
%~dp0..\src\engine\collision\cl_controller.cpp ^
%~dp0..\src\game\collision\cl_edge_detection.cpp ^
%~dp0..\src\engine\collision\cl_resolvers.cpp ^
%~dp0..\src\engine\collision\raycast.cpp ^
%~dp0..\src\engine\render\im_render.cpp ^
%~dp0..\src\engine\collision\simplex.cpp ^
%~dp0..\src\engine\collision\cl_epa.cpp ^
%~dp0..\src\engine\collision\cl_gjk.cpp ^
%~dp0..\src\engine\entity.cpp ^
%~dp0..\src\engine\camera.cpp ^
%~dp0..\src\engine\shader.cpp ^
%~dp0..\src\engine\mesh.cpp ^
%~dp0..\src\engine\core\rvn_types.cpp ^
%~dp0..\src\engine\render\renderer.cpp ^
%~dp0..\src\engine\render\text\text_renderer.cpp ^
%~dp0..\src\engine\rvn.cpp ^
%~dp0..\src\ravenous.cpp ^
glfw3.lib glad.lib  IrrXMLd.lib zlibd.lib zlibstaticd.lib freetyped.lib opengl32.lib imgui.lib ^
kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ^
/EHsc /Zi ^
/I %~dp0..\include ^
/I %~dp0..\src ^
/link /LIBPATH:%~dp0..\lib


popd


@set /A _toc=%time:~0,2%*3600^
		+%time:~3,1%*10*60^
		+%time:~4,1%*60^
		+%time:~6,1%*10^
		+%time:~7,1% >nul

@set /A _elapsed=%_toc%-%_tic
@echo total compilation time: %_elapsed% s.
@echo ..




