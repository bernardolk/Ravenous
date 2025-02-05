#include <glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "EditorMain.h"

#include "tools/EditorTools.h"
#include "editor/EditorToolbar.h"
#include "editor/EditorEntityPanel.h"
#include "editor/EditorPlayerPanel.h"
#include "editor/EditorWorldPanel.h"
#include "editor/EditorPalettePanel.h"
#include "editor/EditorLightsPanel.h"
#include "editor/EditorCollisionLogPanel.h"
#include "editor/EditorInputRecorderPanel.h"
#include "editor/EditorSceneObjectsPanel.h"
#include "editor/EditorColors.h"
#include "engine/utils/colors.h"
#include "..\Game\Entities\Player.h"
#include "engine/render/renderer.h"
#include "engine/utils/utils.h"
#include "engine/camera/camera.h"
#include "engine/entities/lights.h"
#include "Engine/Entities/StaticMesh.h"
#include "Reflection/Serialization.h"
#include "engine/io/loaders.h"
#include "engine/geometry/vertex.h"
#include "engine/io/display.h"
#include "engine/io/input.h"
#include "engine/render/ImRender.h"
#include "engine/render/Shader.h"
#include "engine/render/text/face.h"
#include "engine/render/text/TextRenderer.h"
#include "engine/serialization/sr_config.h"
#include "engine/world/World.h"

namespace Editor
{
	void StartDearImguiFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void EndDearImguiFrame()
	{
		ImGui::EndFrame();
	}

	//------------------
	// > UPDATE EDITOR
	//------------------
	void Update(EPlayer* Player, RWorld* World, RCamera* Camera)
	{
		auto& EdContext = *GetContext();

		string& SceneName = RWorld::Get()->SceneName;

		if (EdContext.LastFrameScene != SceneName)
		{
			EdContext.EntityPanel.Active = false;
			EdContext.WorldPanel.Active = false;
		}

		EdContext.LastFrameScene = SceneName;

		// check for asset changes
		// CheckForAssetChanges();
		UpdateTriaxisGizmo();

		// ENTITY PANEL
		if (!EdContext.EntityPanel.Active) {
			EdContext.EntityPanel.RenameBuffer[0] = 0;
			EdContext.SnapMode = false;
			EdContext.StretchMode = false;
			EdContext.SnapReference = {};
		}

		// unselect lights when not panel is not active
		if (!EdContext.LightsPanel.Active) {
			EdContext.LightsPanel.SelectedLight = -1;
			EdContext.LightsPanel.SelectedLightType = "";
		}
		else if (EdContext.LightsPanel.SelectedLight != -1 && EdContext.LightsPanel.SelectedLightType != "") {
			EdContext.ShowLightbulbs = true;
		}


		// set editor mode values to initial if not active
		if (!EdContext.MeasureMode) {
			EdContext.FirstPointFound = false;
			EdContext.SecondPointFound = false;
		}
		if (!EdContext.SnapMode) {
			EdContext.SnapCycle = 0;
			EdContext.SnapAxis = 1;
			EdContext.SnapReference = {};
		}

		// respond to mouse if necessary
		if (EdContext.MoveMode)
		{
			if (EdContext.MouseClick)
			{
				if (EdContext.SelectedLight > -1)
					PlaceLight();
				else
					PlaceEntity(World);
			}
			else
			{
				if (EdContext.SelectedLight > -1) {
					MoveLightWithMouse(EdContext.SelectedLightType, EdContext.SelectedLight, World);
				}
				else {
					MoveEntityWithMouse(*EdContext.SelectedEntity);
				}
			}
		}

		if (EdContext.SelectEntityAuxMode)
		{
			if (EdContext.MouseClick)
			{
				CheckSelectionToSelectRelatedEntity(World, Camera);
			}
		}

		if (EdContext.MoveEntityByArrows)
		{
			if (EdContext.MouseDragging) {
				MoveEntityByArrows(*EdContext.SelectedEntity);
			}
			// the below condition is to prevent from deactivating too early
			else if (!EdContext.MouseClick) {
				PlaceEntity(World);
			}
		}

		if (EdContext.RotateEntityWithMouse)
		{
			if (EdContext.MouseDragging) {
				RotateEntityWithMouse(*EdContext.SelectedEntity);
			}
			// the below condition is to prevent from deactivating too early
			else if (!EdContext.MouseClick) {
				PlaceEntity(World);
			}
		}


		if (EdContext.PlaceMode)
		{
			if (EdContext.MouseClick) {
				PlaceEntity(World);
			}
			else {
				SelectEntityPlacingWithMouseMove(*EdContext.SelectedEntity, World);
			}
		}

		if (EdContext.ScaleEntityWithMouse) {
			ScaleEntityWithMouse(*EdContext.SelectedEntity);
		}
		
		// resets mouse click event
		EdContext.MouseClick = false;
		EdContext.CtrlIsPressed = false;
		
		// check for debug flags
		if (EdContext.DebugLedgeDetection)
		{
			ClPerformLedgeDetection(Player, World);
		}
	}

	void UpdateTriaxisGizmo()
	{
		auto& EdContext = *GetContext();

		for (int I = 0; I < 3; I++)
		{
			auto Entity = EdContext.TriAxis[I];
			glm::mat4 Model = Mat4Identity;
			Model = rotate(Model, glm::radians(Entity->Rotation.x), vec3(1.0f, 0.0f, 0.0f));
			Model = rotate(Model, glm::radians(Entity->Rotation.y), vec3(0.0f, 1.0f, 0.0f));
			Model = rotate(Model, glm::radians(Entity->Rotation.z), vec3(0.0f, 0.0f, 1.0f));
			Model = scale(Model, Entity->Scale);
			Entity->MatModel = Model;
		}
	}

	//---------------------
	// > RENDER EDITOR UI
	//---------------------

	void Render(EPlayer* Player, RWorld* World, RCamera* Camera)
	{
		auto& EdContext = *GetContext();

		// render world objs if toggled
		if (EdContext.ShowEventTriggers)
		{
			RenderEventTriggers(Camera, World);
		}

		if (EdContext.ShowWorldCells)
		{
			RenderWorldCells(Camera, World);
		}

		if (EdContext.ShowLightbulbs)
		{
			RenderLightbulbs(Camera, World);
		}

		// render triaxis
		auto TriaxisView = lookAt(vec3(0.0f), Camera->Front, -1.0f * Camera->Up);
		float DisplacementX[3] = {0.3f, 0.0f, 0.0f};
		float DisplacementY[3] = {0.0f, 0.3f, 0.0f};
		for (int I = 0; I < 3; I++)
		{
			// ref. axis
			auto Axis = EdContext.TriAxis[I];
			Axis->Shader->Use();
			Axis->Shader->SetMatrix4("model", Axis->MatModel);
			Axis->Shader->SetMatrix4("view", TriaxisView);
			Axis->Shader->SetFloat2("screenPos", TriaxisScreenposX, TriaxisScreenposY);
			RenderEntity(Axis);
		}

		// Entity panel special render calls
		if (EdContext.EntityPanel.Active)
		{
			// Render glowing pink wireframe on top of selected entity
			{
				// update
				auto State = GetEntityState(EdContext.SelectedEntity);
				auto Model = MatModelFromEntityState(State);

				// compute color intensity based on time
				float TimeValue = glfwGetTime();
				float Intensity = sin(TimeValue) * 2;
				if (Intensity < 0)
					Intensity *= -1.0f;
				Intensity += 1.0f;

				// render
				auto GlowingLine = ShaderCatalogue.find("color")->second;
				GlowingLine->Use();
				GlowingLine->SetMatrix4("model", Model);
				GlowingLine->SetFloat3("color", Intensity * 0.890f, Intensity * 0.168f, Intensity * 0.6f);
				GlowingLine->SetFloat("opacity", 1);
				RenderMesh(EdContext.SelectedEntity->Mesh, RRenderOptions{true, false, 3});
			}

			// Render glowing yellow wireframe on top of an arbitrary related entity
			if (EdContext.EntityPanel.ShowRelatedEntity)
			{
				// update
				auto State = GetEntityState(EdContext.EntityPanel.RelatedEntity);
				auto Model = MatModelFromEntityState(State);

				// compute color intensity based on time
				float TimeValue = glfwGetTime();
				float Intensity = sin(TimeValue) * 2;
				if (Intensity < 0) {
					Intensity *= -1.0f;
				}
				Intensity += 1.0f;

				// render
				auto GlowingLine = ShaderCatalogue.find("color")->second;
				GlowingLine->Use();
				GlowingLine->SetMatrix4("model", Model);
				GlowingLine->SetFloat3("color", Intensity * 0.941f, Intensity * 0.776f, Intensity * 0);
				GlowingLine->SetFloat("opacity", 1);
				RenderMesh(EdContext.EntityPanel.RelatedEntity->Mesh, RRenderOptions{true, false, 3});
			}

			// if (EdContext.bGizmoPositionsDirty) {
				UpdateEntityControlArrows(&EdContext.EntityPanel);
				UpdateEntityRotationGizmo(&EdContext.EntityPanel);
				EdContext.bGizmoPositionsDirty = false;
			// }
		}

		// render glowing wireframe on top of snap reference entity
		if (EdContext.SnapMode && EdContext.SnapReference.IsValid())
		{
			// update
			auto State = GetEntityState(EdContext.SnapReference);
			auto Model = MatModelFromEntityState(State);

			// compute color intensity based on time
			float TimeValue = glfwGetTime();
			float Intensity = sin(TimeValue) * 2;
			if (Intensity < 0)
				Intensity *= -1.0f;
			Intensity += 1.0f;

			// render
			auto GlowingLine = ShaderCatalogue.find("color")->second;
			GlowingLine->Use();
			GlowingLine->SetMatrix4("model", Model);
			GlowingLine->SetFloat3("color", Intensity * 0.952f, Intensity * 0.843f, Intensity * 0.105f);
			GlowingLine->SetFloat("opacity", 1);
			RenderMesh(EdContext.SnapReference->Mesh, RRenderOptions{true, false, 3});
		}

		// --------------
		// render panels
		// --------------
		if (EdContext.SceneObjectsPanel.Active)
			RenderSceneObjectsPanel(World, &EdContext.SceneObjectsPanel);

		if (EdContext.WorldPanel.Active)
			RenderWorldPanel(&EdContext.WorldPanel, World, Player);

		if (EdContext.EntityPanel.Active)
		{
			auto& Panel = EdContext.EntityPanel;

			RenderEntityPanel(&Panel, World);
			
			if (Panel.ShowBoundingBox) {
				RImDraw::AddCollisionMesh(IMHASH, &Panel.Entity->Collider); 
			}
		}

		if (EdContext.PlayerPanel.Active)
		{
			RenderPlayerPanel(&EdContext.PlayerPanel);
		}

		if (EdContext.PalettePanel.Active)
			RenderPalettePanel(&EdContext.PalettePanel);

		if (EdContext.LightsPanel.Active)
			RenderLightsPanel(&EdContext.LightsPanel, World);

		if (EdContext.InputRecorderPanel.Active)
			RenderInputRecorderPanel(&EdContext.InputRecorderPanel);

		if (EdContext.CollisionLogPanel.Active)
			RenderCollisionLogPanel(&EdContext.CollisionLogPanel);

		// -----------------------
		// render gizmos inscreen
		// -----------------------
		glClear(GL_DEPTH_BUFFER_BIT);
		if (EdContext.MeasureMode && EdContext.FirstPointFound && EdContext.SecondPointFound)
		{
			RRenderOptions RenderOpts;
			RenderOpts.AlwaysOnTop = true;
			RenderOpts.LineWidth = 2.0;
			RenderOpts.Color = EdRed;

			vec3 SecondPoint;
			if (EdContext.MeasureAxis == 0)
				SecondPoint = vec3(EdContext.MeasureTo, EdContext.MeasureFrom.y, EdContext.MeasureFrom.z);
			if (EdContext.MeasureAxis == 1)
				SecondPoint = vec3(EdContext.MeasureFrom.x, EdContext.MeasureTo, EdContext.MeasureFrom.z);
			if (EdContext.MeasureAxis == 2)
				SecondPoint = vec3(EdContext.MeasureFrom.x, EdContext.MeasureFrom.y, EdContext.MeasureTo);

			auto VertexList = vector<RVertex>{RVertex{EdContext.MeasureFrom}, RVertex{SecondPoint}};
			RImDraw::AddVertexList(IMHASH, VertexList, 0, RenderOpts, GL_LINE_LOOP);
		}

		if (EdContext.LocateCoordsMode && EdContext.LocateCoordsFoundPoint)
		{
			RImDraw::AddPoint(IMHASH, EdContext.LocateCoordsPosition, 2.0);
		}
		
		if (EdContext.EntityPanel.Active)
		{
			auto& Panel = EdContext.EntityPanel;
			if (EdContext.UsingTranslationGizmo) {
				RenderEntityControlArrows(&Panel, World, Camera);
				RImDraw::AddCollisionMesh(IMHASH, Panel.XArrow->CollisionMesh);
			}
			if (EdContext.UsingRotationGizmo) {
				RenderEntityRotationGizmo(&Panel, World, Camera);
			}
		}

		RenderToolbar(World);

		RenderTextOverlay(Player, Camera);
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Terminate()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Initialize()
	{
		auto& EdContext = *GetContext();

		ImGui::CreateContext();
		auto& Io = ImGui::GetIO();
		ImGui_ImplGlfw_InitForOpenGL(GlobalDisplayState::Get()->GetWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 330");

		ImGui::StyleColorsDark();
		EdContext.ImStyle = &ImGui::GetStyle();
		EdContext.ImStyle->WindowRounding = 1.0f;

		// load tri axis gizmo
		const auto AxisMesh = LoadWavefrontObjAsMesh("axis");

		auto XAxis = new EStaticMesh;
		auto YAxis = new EStaticMesh;
		auto ZAxis = new EStaticMesh;

		XAxis->Mesh = AxisMesh;
		YAxis->Mesh = AxisMesh;
		ZAxis->Mesh = AxisMesh;

		const auto BlueTex = LoadTextureFromFile("blue.jpg", Paths::Textures);
		const auto GreenTex = LoadTextureFromFile("green.jpg", Paths::Textures);
		const auto RedTex = LoadTextureFromFile("red.jpg", Paths::Textures);

		YAxis->TextureDiffuse = RTexture{GreenTex, "texture_diffuse", "green.jpg", "green axis"};
		XAxis->TextureDiffuse = RTexture{BlueTex, "texture_diffuse", "blue.jpg", "blue axis"};
		ZAxis->TextureDiffuse = RTexture{RedTex, "texture_diffuse", "red.jpg", "red axis"};

		const auto Shader = ShaderCatalogue.find("ortho_gui")->second;
		XAxis->Shader = Shader;
		XAxis->Scale = vec3{0.1, 0.1, 0.1};
		XAxis->Rotation = vec3{90, 0, 90};

		YAxis->Shader = Shader;
		YAxis->Scale = vec3{0.1, 0.1, 0.1};
		YAxis->Rotation = vec3{180, 0, 0};

		ZAxis->Shader = Shader;
		ZAxis->Scale = vec3{0.1, 0.1, 0.1};
		ZAxis->Rotation = vec3{90, 0, 180};

		EdContext.TriAxis[0] = XAxis;
		EdContext.TriAxis[1] = YAxis;
		EdContext.TriAxis[2] = ZAxis;


		// load entity panel axis arrows
		// @todo: refactor this to use the entity_manager
		auto XArrow = new EStaticMesh;
		auto YArrow = new EStaticMesh;
		auto ZArrow = new EStaticMesh;

		XArrow->Mesh = AxisMesh;
		YArrow->Mesh = AxisMesh;
		ZArrow->Mesh = AxisMesh;

		const auto ArrowShader = ShaderCatalogue.find("ed_entity_arrow_shader")->second;
		XArrow->Shader = ArrowShader;
		XArrow->Scale = vec3(0.5, 0.5, 0.5);
		XArrow->Rotation = vec3(0);

		YArrow->Shader = ArrowShader;
		YArrow->Scale = vec3(0.5, 0.5, 0.5);
		YArrow->Rotation = vec3(0);

		ZArrow->Shader = ArrowShader;
		ZArrow->Scale = vec3(0.5, 0.5, 0.5);
		ZArrow->Rotation = vec3(0);

		YArrow->TextureDiffuse = RTexture{GreenTex, "texture_diffuse", "green.jpg", "green arrow"};
		XArrow->TextureDiffuse = RTexture{BlueTex, "texture_diffuse", "blue.jpg", "blue arrow"};
		ZArrow->TextureDiffuse = RTexture{RedTex, "texture_diffuse", "red.jpg", "red arrow"};

		// CollisionMesh
		const auto ArrowCollider = new RCollisionMesh;

		for(auto& Vertex : AxisMesh->Vertices) {
			ArrowCollider->Vertices.push_back(Vertex.Position);
		}
		for(auto& Index : AxisMesh->Indices) {
			ArrowCollider->Indices.push_back(Index);
		}

		XArrow->CollisionMesh = ArrowCollider;
		XArrow->Collider = *ArrowCollider;

		YArrow->CollisionMesh = ArrowCollider;
		YArrow->Collider = *ArrowCollider;

		ZArrow->CollisionMesh = ArrowCollider;
		ZArrow->Collider = *ArrowCollider;

		EdContext.EntityPanel.XArrow = XArrow;
		EdContext.EntityPanel.YArrow = YArrow;
		EdContext.EntityPanel.ZArrow = ZArrow;

		// creates entity rotation gizmos
		EdContext.EntityPanel.RotationGizmoX = new EStaticMesh;
		SetEntityAssets(EdContext.EntityPanel.RotationGizmoX, {
		.Name = "rotation_gizmo_x",
		.Mesh = "rotation_gizmo",
		.Shader = "ed_entity_arrow_shader",
		.Texture = "red",
		.CollisionMesh = "rotation_gizmo_collision"});

		EdContext.EntityPanel.RotationGizmoY = new EStaticMesh;
		SetEntityAssets(EdContext.EntityPanel.RotationGizmoY, {
		.Name = "rotation_gizmo_y",
		.Mesh = "rotation_gizmo",
		.Shader = "ed_entity_arrow_shader",
		.Texture = "green",
		.CollisionMesh = "rotation_gizmo_collision"});

		EdContext.EntityPanel.RotationGizmoZ = new EStaticMesh;
		SetEntityAssets(EdContext.EntityPanel.RotationGizmoZ, {
		.Name = "rotation_gizmo_z",
		.Mesh = "rotation_gizmo",
		.Shader = "ed_entity_arrow_shader",
		.Texture = "blue",
		.CollisionMesh = "rotation_gizmo_collision"});


		// palette panel
		InitializePalette(&EdContext.PalettePanel);

		EdContext.LastFrameScene = RWorld::Get()->SceneName;
	}

	EEntity* CopyEntity(EEntity* Entity)
	{
		auto TypeMetadata = Reflection::TypeMetadataManager::Get()->FindTypeMetadata(Entity->TypeID);
		auto* NewEntity = TypeMetadata->NewFunction();
		NewEntity->Mesh = Entity->Mesh;
		NewEntity->TextureDiffuse = Entity->TextureDiffuse;
		NewEntity->TextureNormal = Entity->TextureNormal;
		NewEntity->TextureSpecular = Entity->TextureSpecular;
		NewEntity->Shader = Entity->Shader;
		NewEntity->Position = Entity->Position;
		NewEntity->Rotation = Entity->Rotation;
		NewEntity->Scale = Entity->Scale;
		NewEntity->Collider = Entity->Collider;
		NewEntity->CollisionMesh = Entity->CollisionMesh;
		NewEntity->Collider = Entity->Collider;
		NewEntity->Flags = Entity->Flags;
		NewEntity->BoundingBox = Entity->BoundingBox;
		FindNameForNewEntity(NewEntity);
		return NewEntity;
	}

	void FindNameForNewEntity(EEntity* NewEntity)
	{
		//todo: This is not really great because iterating over all entities is expensive, plus string operations for each, but I am really not sure what's a good solve. We can't cache names because they could change, unless we invalidate the cache on every possible name change. Would need to encapsulate get/set name to ensure that. In editor builds could add the invalidation logic in the GetName call and then for game builds we dont do anything, we can forceinline each call.
		string MeshName = NewEntity->Mesh->Name;
		int N = 1;
		REntityIterator It;
		while (auto* Entity = It())
		{
			if (Entity->Name.starts_with(MeshName))
			{
				auto i = Entity->Name.find_last_of('_');
				if (i != string::npos && i < Entity->Name.size() - 1)
				{
					string NumberString = Entity->Name.substr(i + 1);
					Parser p{NumberString, (int)NumberString.size()};
					p.ParseInt();
					if (p.HasToken()) {
						int EntityN = GetParsed<int>(p);
						N = EntityN >= N ? EntityN + 1 : N;  	
					}
				}	
			}
		}

		NewEntity->Name = MeshName + "_" + to_string(N);
	}

	void RenderTextOverlay(EPlayer* Player, RCamera* Camera)
	{
		float GuiY = GlobalDisplayState::ViewportHeight - 60;
		float ScreenHeight = GlobalDisplayState::ViewportHeight;

		string Font = "consola18";
		string FontCenter = "swanseait38";
		string FontCenterSmall = "swanseait20";
		float CenteredTextHeight = ScreenHeight - 120;
		float CenteredTextHeightSmall = CenteredTextHeight - 40;
		auto ToolTextColorYellow = vec3(0.8, 0.8, 0.2);
		auto ToolTextColorGreen = vec3(0.6, 1.0, 0.3);
		
		// CAMERA POSITION
		string CamP[3]{
		FormatFloatTostr(Camera->Position.x, 2),
		FormatFloatTostr(Camera->Position.y, 2),
		FormatFloatTostr(Camera->Position.z, 2),
		};
		string CameraPosition = "camera:   x: " + CamP[0] + " y:" + CamP[1] + " z:" + CamP[2];
		RenderText(Font, 235, 45, CameraPosition);

		// PLAYER POSITION
		vec3 PlayerFeet = Player->GetFeetPosition();
		string PlayerP[3]{FormatFloatTostr(PlayerFeet.x, 1), FormatFloatTostr(PlayerFeet.y, 1), FormatFloatTostr(PlayerFeet.z, 1)};
		string PlayerPos = "player:   x: " + PlayerP[0] + " y: " + PlayerP[1] + " z: " + PlayerP[2];
		RenderText(Font, 235, 70, PlayerPos);

		// PLAYER LIVES
		string Lives = std::to_string(Player->Lives);
		RenderText(Font, GlobalDisplayState::ViewportWidth - 400, 90, Player->Lives == 2 ? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1}, Lives);

		// PLAYER STATE
		auto PlayerStateTextColor = vec3(0, 0, 0);
		string PlayerStateText;
		switch (Player->PlayerState)
		{
			case NPlayerState::Standing:
				PlayerStateText = "PLAYER PlayerState::Standing";
				break;
			case NPlayerState::Falling:
				PlayerStateText = "PLAYER FALLING";
				break;
			case NPlayerState::Jumping:
				PlayerStateText = "PLAYER JUMPING";
				break;
			case NPlayerState::Sliding:
				PlayerStateText = "PLAYER SLIDING";
				break;
			case NPlayerState::SlideFalling:
				PlayerStateText = "PLAYER SLIDE FALLING";
				break;
		}
		RenderText("consola18", GlobalDisplayState::ViewportWidth - 400, 30, PlayerStateTextColor, PlayerStateText);

		// FPS
		string Fps = std::to_string(RavenousEngine::GetFrame().Fps);
		string FpsGui = "FPS: " + Fps;
		RenderText(Font, GlobalDisplayState::ViewportWidth - 110, 40, FpsGui);


		// EDITOR TOOLS INDICATORS

		// ----------
		// SNAP MODE
		// ----------
		auto& EdContext = *GetContext();

		if (EdContext.SnapMode)
		{
			string SnapCycle;
			switch (EdContext.SnapCycle)
			{
				case 0:
					SnapCycle = "top";
					break;
				case 1:
					SnapCycle = "mid";
					break;
				case 2:
					SnapCycle = "bottom";
					break;
			}

			string SnapAxis;
			switch (EdContext.SnapAxis)
			{
				case 0:
					SnapAxis = "X";
					break;
				case 1:
					SnapAxis = "Y";
					break;
				case 2:
					SnapAxis = "Z";
					break;
			}

			// if position is changed and not commited, render text yellow
			vec3 SnapModeSubtextColor;
			if (EdContext.SnapReference.IsValid())
			{
				SnapModeSubtextColor = ToolTextColorYellow;
			}
			else
			{
				auto StateChange = EdContext.UndoStack.GetStateChangeAtPosition();
				if (StateChange.State.Entity.IsValid() && StateChange.State.Position != EdContext.EntityPanel.Entity->Position) {
					SnapModeSubtextColor = ToolTextColorYellow;
				}
				else {
					SnapModeSubtextColor = ToolTextColorGreen;
				}
			}

			// selects text based on situation of snap tool
			string SubText;
			if (EdContext.SnapReference.IsValid()) {
				SubText = "select another entity to snap to.";
			}
			else {
				SubText = "press Enter to commit position. x/y/z to change axis.";
			}

			RenderText(FontCenter, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeight, ToolTextColorYellow, true, "SNAP MODE (" + SnapAxis + "-" + SnapCycle + ")");
			RenderText(FontCenterSmall, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeightSmall, SnapModeSubtextColor, true, SubText);
		}

		// -------------
		// MEASURE MODE
		// -------------
		if (EdContext.MeasureMode)
		{
			string Axis =
				EdContext.MeasureAxis == 0 ? "x" :
				EdContext.MeasureAxis == 1 ? "y" :
				"z";

			RenderText(FontCenter, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeight, vec3(0.8, 0.8, 0.2), true, "MEASURE MODE (" + Axis + ")"
			);

			if (EdContext.SecondPointFound)
			{
				float DistRef = (
				EdContext.MeasureAxis == 0 ? EdContext.MeasureFrom.x :
				EdContext.MeasureAxis == 1 ? EdContext.MeasureFrom.y :
				EdContext.MeasureFrom.z);

				RenderText(FontCenter, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeightSmall, vec3(0.8, 0.8, 0.2), true, "(" + FormatFloatTostr(abs(EdContext.MeasureTo - DistRef), 2) + " m)"
				);
			}
		}

		// ----------
		// MOVE MODE
		// ----------
		if (EdContext.MoveMode)
		{
			string MoveAxis;
			switch (EdContext.MoveAxis)
			{
				case 0:
					MoveAxis = "XZ";
					break;
				case 1:
					MoveAxis = "X";
					break;
				case 2:
					MoveAxis = "Y";
					break;
				case 3:
					MoveAxis = "Z";
					break;
			}

			RenderText(FontCenter, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeight, vec3(0.8, 0.8, 0.2), true, "MOVE MODE (" + MoveAxis + ")");
			RenderText(FontCenter, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeightSmall, vec3(0.8, 0.8, 0.2), true, "press M to alternate between move and place modes");
		}

		// ----------
		// PLACE MODE
		// ----------
		if (EdContext.PlaceMode)
		{
			RenderText(FontCenter, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeight, vec3(0.8, 0.8, 0.2), true, "PLACE MODE");
			RenderText(FontCenterSmall, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeightSmall, vec3(0.8, 0.8, 0.2), true, "press M to alternate between move and place modes");
		}

		// -------------------
		// LOCATE COORDS MODE
		// -------------------
		if (EdContext.LocateCoordsMode)
		{
			RenderText(FontCenter, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeight, vec3(0.8, 0.8, 0.2), true, "LOCATE COORDS MODE");

			string LocateCoordsSubtext;
			if (!EdContext.LocateCoordsFoundPoint)
			{
				LocateCoordsSubtext = "Please select a world position to get coordinates.";
			}
			else
			{
				LocateCoordsSubtext =
				"(x: " + FormatFloatTostr(EdContext.LocateCoordsPosition[0], 2) +
				", y: " + FormatFloatTostr(EdContext.LocateCoordsPosition[1], 2) +
				", z: " + FormatFloatTostr(EdContext.LocateCoordsPosition[2], 2) + ")";
			}

			RenderText(FontCenterSmall, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeight - 40, ToolTextColorGreen, true, LocateCoordsSubtext);
		}

		// -------------
		// STRETCH MODE
		// -------------
		if (EdContext.StretchMode)
		{
			RenderText(FontCenter, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeight, vec3(0.8, 0.8, 0.2), true, "STRETCH MODE");
		}

		// --------------------------
		// ENTITY SELECTION AUX MODE
		// --------------------------
		if (EdContext.SelectEntityAuxMode)
		{
			RenderText(FontCenter, GlobalDisplayState::ViewportWidth / 2, CenteredTextHeight, vec3(0.8, 0.8, 0.2), true, "SELECT RELATED ENTITY");
		}
	}

	//TODO: Reimplement
	void RenderEventTriggers(RCamera* Camera, RWorld* World)
	{
		/*
		 if (World->interactables.size() == 0)
			 return;

		auto find = ShaderCatalogue.find("color");
		auto shader = find->second;

		Shader->Use();
		Shader->SetMatrix4("view", camera->mat_view);
		Shader->SetMatrix4("projection", camera->mat_projection);

		for (int i = 0; i < World->interactables.size(); i++)
		{
			auto checkpoint = World->interactables[i];
			Shader->SetMatrix4("model", checkpoint->trigger_mat_model);
			Shader->SetFloat3("color", 0.5, 0.5, 0.3);
			Shader->SetFloat("opacity", 0.6);
			RenderMesh(checkpoint->trigger, RRenderOptions{});
		}
		*/
	}

	void RenderWorldCells(RCamera* Camera, RWorld* World)
	{
		auto Shader = ShaderCatalogue.find("color")->second;
		auto CellMesh = GeometryCatalogue.find("aabb")->second;
		auto& EdContext = *GetContext();

		auto ChunkIterator = World->GetChunkIterator();
		while (auto* Chunk = ChunkIterator())
		{
			RRenderOptions Opts;
			Opts.Wireframe = true;

			vec3 Color = vec3(0.27, 0.55, 0.65);
			if ((int) EdContext.WorldPanel.ChunkPositionVec.x == Chunk->i &&
				(int) EdContext.WorldPanel.ChunkPositionVec.y == Chunk->j &&
				(int) EdContext.WorldPanel.ChunkPositionVec.z == Chunk->k)
			{
				Opts.LineWidth = 1.5;
				Color = vec3(0.8, 0.4, 0.2);
			}
			
			else if (
				(Chunk->i == WorldChunkNumX || Chunk->i == 0) ||
				(Chunk->j == WorldChunkNumY || Chunk->j == 0) ||
				(Chunk->k == WorldChunkNumZ || Chunk->k == 0))
			{
				Color = vec3(0.0, 0.0, 0.0);
			}
			
			// creates model matrix
			vec3 Position = GetWorldCoordinatesFromWorldCellCoordinates(Chunk->i, Chunk->j, Chunk->k);
			glm::mat4 Model = translate(Mat4Identity, Position);
			Model = scale(Model, vec3{WorldChunkLengthMeters, WorldChunkLengthMeters, WorldChunkLengthMeters});

			//render
			Shader->Use();
			Shader->SetFloat3("color", Color);
			Shader->SetFloat("opacity", 0.85f);
			Shader->SetMatrix4("model", Model);
			Shader->SetMatrix4("view", Camera->MatView);
			Shader->SetMatrix4("projection", Camera->MatProjection);
			glDisable(GL_CULL_FACE);
			RenderMesh(CellMesh, Opts);
			glEnable(GL_CULL_FACE);
		}
	}

	void RenderLightbulbs(RCamera* Camera, RWorld* World)
	{
		auto& EdContext = *GetContext();

		auto Mesh = GeometryCatalogue.find("lightbulb")->second;
		auto Shader = ShaderCatalogue.find("color")->second;

		Shader->SetMatrix4("view", Camera->MatView);
		Shader->SetMatrix4("projection", Camera->MatProjection);

		auto SelectedLight = EdContext.LightsPanel.SelectedLight;
		auto SelectedLightType = EdContext.LightsPanel.SelectedLightType;

		// point lights
		int PointLightsCount = 0;
		for (const auto& Light : World->PointLights)
		{
			auto Model = translate(Mat4Identity, Light->Position + vec3{0, 0.5, 0});
			Model = scale(Model, vec3{0.1f});
			RRenderOptions Opts;
			//opts.wireframe = true;
			//render
			Shader->Use();
			Shader->SetMatrix4("model", Model);
			Shader->SetFloat3("color", Light->Diffuse);
			Shader->SetFloat("opacity", 1.0);

			RenderMesh(Mesh, Opts);

			PointLightsCount++;
		}

		// spot lights
		int SpotLightsCount = 0;
		for (const auto& Light : World->SpotLights)
		{
			auto Model = translate(Mat4Identity, Light->Position + vec3{0, 0.5, 0});
			Model = scale(Model, vec3{0.1f});
			RRenderOptions Opts;
			//opts.wireframe = true;
			//render
			Shader->Use();
			Shader->SetMatrix4("model", Model);
			Shader->SetFloat3("color", Light->Diffuse);
			Shader->SetFloat("opacity", 1.0);
			RenderMesh(Mesh, Opts);
			SpotLightsCount++;
		}

		// render selection box and dir arrow for selected lightbulb
		if (SelectedLight >= 0)
		{
			vec3 LightPosition;
			vec3 LightDirection;
			if (SelectedLightType == "point")
			{
				assert(SelectedLight <= PointLightsCount);
				auto& Light = *World->PointLights[SelectedLight];
				LightPosition = Light.Position;
			}
			else if (SelectedLightType == "spot")
			{
				assert(SelectedLight <= SpotLightsCount);
				auto& Light = *World->SpotLights[SelectedLight];
				LightPosition = Light.Position;
				LightDirection = Light.Direction;
			}

			// selection box
			auto AabbModel = translate(Mat4Identity, LightPosition - vec3{0.1575, 0, 0.1575});
			AabbModel = scale(AabbModel, vec3{0.3f, 0.6f, 0.3f});
			RRenderOptions Opts;
			Opts.Wireframe = true;

			Shader->Use();
			Shader->SetMatrix4("model", AabbModel);
			Shader->SetFloat3("color", vec3{0.9, 0.7, 0.9});
			Shader->SetFloat("opacity", 1.0);
			
			auto AabbMesh = GeometryCatalogue.find("aabb")->second;
			RenderMesh(AabbMesh, Opts);

			// direction arrow
			if (SelectedLightType == "spot")
			{
				float Pitch, Yaw;
				RCameraManager::ComputeAnglesFromDirection(Pitch, Yaw, LightDirection);
				vec3 ArrowDirection = ComputeDirectionFromAngles(Pitch, Yaw);

				vec3 ArrowOrigin = LightPosition - vec3{0.0, 0.56, 0.0};
				vec3 ArrowEnd = ArrowOrigin + ArrowDirection * 1.5f;
				RImDraw::AddLine(IMHASH, ArrowOrigin, ArrowEnd, 1.5);
			}

			// @todo: epic fail below (trying to rotate an arrow mesh according to a dir vector)
			// auto arrow_mesh = Geometry_Catalogue.find("axis")->second;
			// vec3 front = ArrowOrigin + Light.Direction;
			// vec3 up = glm::cross(ArrowOrigin, );

			// @todo: this is a workaround, since we are not using quaternions yet, we must
			//       be careful with 0/180 degree angles between up and direction vectors
			//       using glm::lookAt()
			// @todo: Actually now we are using immediate draw and lines.

			// //mat4 arrow_model = translate(Mat4Identity, ArrowOrigin);
			// mat4 arrow_model = 
			//    glm::translate(Mat4Identity, ArrowOrigin) *
			//    glm::rotate(Mat4Identity, glm::radians(90.0f), vec3(1, 0, 0)) *
			//    glm::lookAt(vec3{0.0}, ArrowDirection, vec3{0,1,0})
			// ;
			// //arrow_model = glm::scale(arrow_model, vec3{0.2f, 0.3f, 0.2f});

			//render arrow
			// Shader->use();
			// Shader->setFloat3("color", vec3{0.9, 0.7, 0.9});
			// Shader->setFloat("opacity", 1.0);
			// Shader->setMatrix4("model", arrow_model);
			// Shader->setMatrix4("view", camera->View4x4);
			// Shader->setMatrix4("projection", camera->Projection4x4);
			// render_mesh(arrow_mesh, RRenderOptions{});
		}
	}

	void RenderEntityControlArrows(REntityPanelContext* Panel, RWorld* World, RCamera* Camera)
	{
		auto* ArrowShader = Panel->XArrow->Shader;
		ArrowShader->Use();
		ArrowShader->SetFloat3("color", 0.f, 0.f, 1.f);
		RenderEditorEntity(Panel->XArrow, World, Camera);
		
		ArrowShader->SetFloat3("color", 0.f, 1.f, 0.f);
		RenderEditorEntity(Panel->YArrow, World, Camera);
		
		ArrowShader->SetFloat3("color", 1.f, 0.f, 0.f);
		RenderEditorEntity(Panel->ZArrow, World, Camera);
	}

	void RenderEntityRotationGizmo(REntityPanelContext* Panel, RWorld* World, RCamera* Camera)
	{
		auto RotGizmoShader = Panel->RotationGizmoX->Shader;
		RotGizmoShader->Use();
		RotGizmoShader->SetFloat3("color", 0.f, 0.f, 1.f);
		RenderEditorEntity(Panel->RotationGizmoX, World, Camera);

		RotGizmoShader->SetFloat3("color", 0.f, 1.f, 0.f);
		RenderEditorEntity(Panel->RotationGizmoY, World, Camera);

		RotGizmoShader->SetFloat3("color", 1.f, 0.f, 0.f);
		RenderEditorEntity(Panel->RotationGizmoZ, World, Camera);
	}

	void UpdateEntityControlArrows(REntityPanelContext* Panel)
	{
		// arrow positioning settings
		float Angles[3] = {270, 0, 90};
		EEntity* Arrows[3] = {Panel->XArrow, Panel->YArrow, Panel->ZArrow};
		vec3 RotAxis[3] = {UnitZ, UnitX, UnitX};

		auto Entity = Panel->Entity;

		if (Panel->ReverseScale)
		{
			for (float& Angle : Angles) {
				Angle += 180;
			}
		}
		
		static float ScaleFactor = 20.0f;
		vec3 CameraPos = RCameraManager::Get()->GetEditorCamera()->Position;
		vec3 DirectionVec = Normalize(Entity->BoundingBox.GetCentroid() - CameraPos);
		vec3 GizmoPosition = CameraPos + DirectionVec * ScaleFactor;
		
		auto StartingModel = translate(Mat4Identity, GizmoPosition);
		for (int i = 0; i < 3; i++)
		{
			auto Arrow = Arrows[i];
			auto Model = rotate(StartingModel, glm::radians(Angles[i]), RotAxis[i]);
			Arrow->MatModel = Model;
			Arrow->UpdateCollider();
			Arrow->UpdateBoundingBox();
		}
	}

	void UpdateEntityRotationGizmo(REntityPanelContext* Panel)
	{
		// arrow positioning settings
		float Angles[3] = {270, 0, 90};
		vec3 RotAxis[3] = {UnitZ, UnitX, UnitX};
		int i = 0;
		
		auto Entity = Panel->Entity;
		
		static float ScaleFactor = 20.0f;
		vec3 CameraPos = RCameraManager::Get()->GetEditorCamera()->Position;
		vec3 DirectionVec = Normalize(Entity->BoundingBox.GetCentroid() - CameraPos);
		vec3 GizmoPosition = CameraPos + DirectionVec * ScaleFactor;
		
		auto StartingModel = translate(Mat4Identity, GizmoPosition);
		for (auto* Gizmo : {Panel->RotationGizmoX, Panel->RotationGizmoY, Panel->RotationGizmoZ})
		{
			auto Model = rotate(StartingModel, glm::radians(Angles[i]), RotAxis[i]);
			Gizmo->MatModel = Model;
			Gizmo->UpdateCollider();
			Gizmo->UpdateBoundingBox();
			i++;
		}
	}

	void CheckSelectionToOpenPanel(EPlayer* Player, RWorld* World, RCamera* Camera)
	{
		auto Pickray = CastPickray();
		auto Test = World->Raycast(Pickray, RayCast_TestOnlyVisibleEntities);
		auto TestLight = World->RaycastLights(Pickray);

		if (Test.Hit && (!TestLight.Hit || TestLight.Distance > Test.Distance)) {
			OpenEntityPanel(Test.Entity);
		}
		//else if (TestLight.Hit) {
		//	OpenLightsPanel(TestLight.ObjHitType, TestLight.ObjHitIndex, true);
		//}
	}

	void CheckSelectionToSelectRelatedEntity(RWorld* World, RCamera* Camera)
	{
		auto& EdContext = *GetContext();

		auto Pickray = CastPickray();
		auto Test = World->Raycast(Pickray, RayCast_TestOnlyVisibleEntities);
		if (Test.Hit)
		{
			EdContext.SelectEntityAuxMode = false;
			*EdContext.SelectEntityAuxModeEntitySlot = Test.Entity;

			if (EdContext.SelectEntityAuxModeCallback != EdToolCallback_NoCallback)
			{
				switch (EdContext.SelectEntityAuxModeCallback)
				{
					case EdToolCallback_EntityManagerSetType:
					{
						// EM->SetType(
						// 	*EdContext.select_entity_aux_mode_entity_slot,
						// 	EdContext.select_entity_aux_mode_callback_args.entity_type
						// );
						break;
					}
					default: break;
				}
			}
		}
	}
	
	void CheckSelectionToMoveEntity(RWorld* World, RCamera* Camera)
	{
		auto Pickray = CastPickray();
		auto Test = World->Raycast(Pickray, RayCast_TestOnlyVisibleEntities);
		auto TestLight = World->RaycastLights(Pickray);
		if (Test.Hit && (!TestLight.Hit || TestLight.Distance > Test.Distance)) {
			ActivateMoveMode(Test.Entity);
		}
		//else if (TestLight.Hit) {
		//	ActivateMoveLightMode(TestLight.ObjHitType, TestLight.ObjHitIndex);
		//}
	}
	
	bool CheckSelectionToGrabEntityArrows(RCamera* Camera)
	{
		auto& EdContext = *GetContext();

		auto Pickray = CastPickray();
		EEntity* Arrows[3] = {EdContext.EntityPanel.XArrow, EdContext.EntityPanel.YArrow, EdContext.EntityPanel.ZArrow};

		for (int i = 0; i < 3; i++)
		{
			RRaycastTest Test = TestRayAgainstEntity(Pickray, Arrows[i]);
			if (Test.Hit) {
				ActivateMoveEntityByArrow(i + 1);
				return true;
			}
		}
		return false;
	}

	bool CheckSelectionToGrabEntityRotationGizmo(RCamera* Camera)
	{
		auto& EdContext = *GetContext();

		auto Pickray = CastPickray();
		RRaycastTest Test;

		EEntity* RotGizmos[3] = {
			EdContext.EntityPanel.RotationGizmoX,
			EdContext.EntityPanel.RotationGizmoY,
			EdContext.EntityPanel.RotationGizmoZ
		};

		for (int i = 0; i < 3; i++)
		{
			Test = TestRayAgainstEntity(Pickray, RotGizmos[i]);
			if (Test.Hit) {
				ActivateRotateEntityWithMouse(i + 1);
				return true;
			}
		}

		return false;
	}

	void EditorSave()
	{
		auto* ProgramConfig = ProgramConfig::Get();
		ProgramConfig->InitialScene = RWorld::Get()->SceneName;
		ConfigSerializer::Save(*ProgramConfig);

		Serialization::SaveWorldToDisk();
		CleanupDeletedEntityFiles();
		PrintEditorMsg("World Saved");
	}

	void CleanupDeletedEntityFiles()
	{
		auto* EdContext = GetContext();
		for (RUUID ID : EdContext->DeletionLog)
		{
			string QuotedUUID = Reflection::ToString(ID);
			string UnquotedUUID = QuotedUUID.substr(1, QuotedUUID.length() - 2);
			string Path = Paths::World + UnquotedUUID  + ".ref";
			if (std::remove(Path.c_str()) != 0) {
				Log("Error deletig file '%s' while cleaning up deleted entitie's files.", Path.c_str())
			}
		}
	}
}
