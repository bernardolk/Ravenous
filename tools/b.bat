@echo off
call c:\repositories\ravenous\tools\killsymbolserver.bat

IF NOT EXIST "c:\repositories\ravenous\build" mkdir c:\repositories\ravenous\build
pushd c:\repositories\ravenous\build
@set /A _tic=%time:~0,2%*3600^
            +%time:~3,1%*10*60^
            +%time:~4,1%*60^
            +%time:~6,1%*10^
            +%time:~7,1% >nul
			
cl.exe /std:c++20 /MD ^
c:\repositories\ravenous\src\ravenous.cpp ^
c:\repositories\ravenous\src\engine\world\world.cpp ^
c:\repositories\ravenous\src\engine\entity.cpp ^
c:\repositories\ravenous\src\engine\camera.cpp ^
c:\repositories\ravenous\src\engine\shader.cpp ^
c:\repositories\ravenous\src\engine\mesh.cpp ^
c:\repositories\ravenous\src\engine\core\rvn_types.cpp ^
c:\repositories\ravenous\src\engine\globals.cpp ^
c:\repositories\ravenous\src\engine\render\renderer.cpp ^
c:\repositories\ravenous\src\engine\render\text\text_renderer.cpp ^
glfw3.lib glad.lib  IrrXMLd.lib zlibd.lib zlibstaticd.lib freetyped.lib opengl32.lib imgui.lib ^
kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ^
/EHsc /Zi ^
/I c:\repositories\ravenous\include ^
/I c:\repositories\ravenous\src ^
/link /LIBPATH:c:\repositories\ravenous\lib 
popd


@set /A _toc=%time:~0,2%*3600^
		+%time:~3,1%*10*60^
		+%time:~4,1%*60^
		+%time:~6,1%*10^
		+%time:~7,1% >nul

@set /A _elapsed=%_toc%-%_tic
@echo ..
@echo total compilation time: %_elapsed% s.




