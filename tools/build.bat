@echo off
IF NOT EXIST "w:\build" mkdir w:\build
pushd w:\build
@set /A _tic=%time:~0,2%*3600^
            +%time:~3,1%*10*60^
            +%time:~4,1%*60^
            +%time:~6,1%*10^
            +%time:~7,1% >nul
			
cl.exe /MD w:\src\Ravenous.cpp ^
w:\include\imgui\imgui.cpp w:\include\imgui\imgui_impl_glfw.cpp w:\include\imgui\imgui_impl_opengl3.cpp w:\include\imgui\imgui_draw.cpp w:\include\imgui\imgui_widgets.cpp ^
glfw3.lib glad.lib  IrrXMLd.lib zlibd.lib zlibstaticd.lib freetyped.lib opengl32.lib ^
kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ^
/EHsc /Zi /I w:\include /I w:\src /link /LIBPATH:w:\lib
popd


@set /A _toc=%time:~0,2%*3600^
		+%time:~3,1%*10*60^
		+%time:~4,1%*60^
		+%time:~6,1%*10^
		+%time:~7,1% >nul

@set /A _elapsed=%_toc%-%_tic
@echo ..
@echo total compilation time: %_elapsed% s.




