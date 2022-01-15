@echo off
call c:\repositories\ravenous\tools\killsymbolserver.bat

IF NOT EXIST "c:\repositories\ravenous\lib\build" mkdir c:\repositories\ravenous\lib\build

pushd c:\repositories\ravenous\lib\build
@set /A _tic=%time:~0,2%*3600^
            +%time:~3,1%*10*60^
            +%time:~4,1%*60^
            +%time:~6,1%*10^
            +%time:~7,1% >nul
			
cl.exe /std:c++17 /MD ^
c:\repositories\ravenous\include\dearIMGUI\imgui.cpp ^
c:\repositories\ravenous\include\dearIMGUI\imgui_impl_glfw.cpp ^
c:\repositories\ravenous\include\dearIMGUI\imgui_impl_opengl3.cpp ^
c:\repositories\ravenous\include\dearIMGUI\imgui_draw.cpp ^
c:\repositories\ravenous\include\dearIMGUI\imgui_widgets.cpp ^
c:\repositories\ravenous\include\dearIMGUI\imgui_tables.cpp ^
c:\repositories\ravenous\include\dearIMGUI\imgui_demo.cpp ^
c:\repositories\ravenous\include\dearIMGUI\imgui_stdlib.cpp ^
/EHsc /Zi /c /I c:\repositories\ravenous\include\

lib.exe imgui.obj imgui_impl_glfw.obj imgui_impl_opengl3.obj imgui_draw.obj imgui_widgets.obj imgui_tables.obj imgui_demo.obj imgui_stdlib.obj

popd

@set /A _toc=%time:~0,2%*3600^
		+%time:~3,1%*10*60^
		+%time:~4,1%*60^
		+%time:~6,1%*10^
		+%time:~7,1% >nul

@set /A _elapsed=%_toc%-%_tic
@echo ..
@echo total compilation time: %_elapsed% s.




