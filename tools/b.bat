@echo off
call %~dp0\killsymbolserver.bat

IF NOT EXIST "%~dp0..\build" mkdir %~dp0..\build
pushd %~dp0..\build
@set /A _tic=%time:~0,2%*3600^
            +%time:~3,1%*10*60^
            +%time:~4,1%*60^
            +%time:~6,1%*10^
            +%time:~7,1% >nul
			
clang-cl.exe /std:c++20 /MD /Fe:Ravenous.exe ^
%~dp0..\src\Editor\Console\Console.cpp ^
%~dp0..\src\Editor\EditorCollisionLogPanel.cpp ^
%~dp0..\src\Editor\EditorContext.cpp ^
%~dp0..\src\Editor\EditorEntityPanel.cpp ^
%~dp0..\src\Editor\EditorInput.cpp ^
%~dp0..\src\Editor\EditorInputRecorderPanel.cpp ^
%~dp0..\src\Editor\EditorLightsPanel.cpp ^
%~dp0..\src\Editor\EditorMain.cpp ^
%~dp0..\src\Editor\EditorPalettePanel.cpp ^
%~dp0..\src\Editor\EditorPlayerPanel.cpp ^
%~dp0..\src\Editor\EditorSceneObjectsPanel.cpp ^
%~dp0..\src\Editor\EditorState.cpp ^
%~dp0..\src\Editor\EditorToolbar.cpp ^
%~dp0..\src\Editor\EditorUndo.cpp ^
%~dp0..\src\Editor\EditorWorldPanel.cpp ^
%~dp0..\src\Editor\EntityState.cpp ^
%~dp0..\src\Editor\Reflection\Reflection.cpp ^
%~dp0..\src\Editor\Reflection\Serialization.cpp ^
%~dp0..\src\Editor\Tools\EditorTools.cpp ^
%~dp0..\src\Editor\Tools\InputRecorder.cpp ^
%~dp0..\src\Engine\Camera\Camera.cpp ^
%~dp0..\src\Engine\Catalogues.cpp ^
%~dp0..\src\Engine\Collision\ClController.cpp ^
%~dp0..\src\Engine\Collision\ClEdgeDetection.cpp ^
%~dp0..\src\Engine\Collision\ClEpa.cpp ^
%~dp0..\src\Engine\Collision\ClGjk.cpp ^
%~dp0..\src\Engine\Collision\ClResolvers.cpp ^
%~dp0..\src\Engine\Collision\CollisionMesh.cpp ^
%~dp0..\src\Engine\Collision\CollisionTest.cpp ^
%~dp0..\src\Engine\Collision\Raycast.cpp ^
%~dp0..\src\Engine\Collision\Simplex.cpp ^
%~dp0..\src\Engine\Core\Logging.cpp ^
%~dp0..\src\Engine\Core\RView.cpp ^
%~dp0..\src\Engine\Core\UUIDGenerator.cpp ^
%~dp0..\src\Engine\Entities\EHandle.cpp ^
%~dp0..\src\Engine\Entities\Entity.cpp ^
%~dp0..\src\Engine\Entities\StaticMesh.cpp ^
%~dp0..\src\Engine\Entities\Traits\EntityTraits.cpp ^
%~dp0..\src\Engine\Entities\Traits\EntityTraitsManager.cpp ^
%~dp0..\src\Engine\Geometry\Cylinder.cpp ^
%~dp0..\src\Engine\Geometry\Mesh.cpp ^
%~dp0..\src\Engine\Geometry\Plane.cpp ^
%~dp0..\src\Engine\Geometry\Quad.cpp ^
%~dp0..\src\Engine\IO\Display.cpp ^
%~dp0..\src\Engine\IO\Input.cpp ^
%~dp0..\src\Engine\IO\InputPhase.cpp ^
%~dp0..\src\Engine\IO\Loaders.cpp ^
%~dp0..\src\Engine\MainLoop.cpp ^
%~dp0..\src\Engine\Platform\GlWindow.cpp ^
%~dp0..\src\Engine\Platform\Platform.cpp ^
%~dp0..\src\Engine\Platform\PlatformLayerWin.cpp ^
%~dp0..\src\Engine\RavenousEngine.cpp ^
%~dp0..\src\Engine\Render\ImRender.cpp ^
%~dp0..\src\Engine\Render\Renderer.cpp ^
%~dp0..\src\Engine\Render\Shader.cpp ^
%~dp0..\src\Engine\Render\Text\TextRenderer.cpp ^
%~dp0..\src\Engine\Rvn.cpp ^
%~dp0..\src\Engine\Serialization\Parsing\Parser.cpp ^
%~dp0..\src\Engine\Serialization\sr_config.cpp ^
%~dp0..\src\Engine\Utils\Utils.cpp ^
%~dp0..\src\Engine\World\EntitySlot.cpp ^
%~dp0..\src\Engine\World\World.cpp ^
%~dp0..\src\Engine\World\WorldChunk.cpp ^
%~dp0..\src\Engine\World\WorldChunkStorage.cpp ^
%~dp0..\src\Game\Animation\AnPlayer.cpp ^
%~dp0..\src\Game\Animation\AnUpdate.cpp ^
%~dp0..\src\Game\Entities\Checkpoint.cpp ^
%~dp0..\src\Game\Entities\Player.cpp ^
%~dp0..\src\Game\Entities\Traits\Interactable.cpp ^
%~dp0..\src\Game\Gameplay\GameState.cpp ^
%~dp0..\src\Game\Input\PlayerInput.cpp ^
%~dp0..\src\Ravenous.cpp ^
%~dp0..\src\Test\TestSerialization.cpp ^
glfw3.lib glad.lib  IrrXMLd.lib zlibd.lib zlibstaticd.lib freetyped.lib opengl32.lib imgui.lib ^
kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ^
/EHsc /Zi ^
/I %~dp0..\include ^
/I %~dp0..\src ^
/I %~dp0..\include\GLFW ^
/I %~dp0..\include\dearIMGUI ^
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




