#include "EditorLightsPanel.h"
#include <imgui.h>
#include "EditorMain.h"
#include "tools/EditorTools.h"
#include "engine/camera/camera.h"
#include "engine/entities/Lights.h"
#include "engine/world/World.h"

namespace Editor
{
	void OpenLightsPanel(string Type, int Index, bool FocusTab)
	{
		auto& EdContext = *GetContext();

		EdContext.LightsPanel.Active = true;
		if (Type != "" && Index > -1)
		{
			EdContext.LightsPanel.SelectedLight = Index;
			EdContext.LightsPanel.SelectedLightType = Type;
		}
		if (FocusTab)
			EdContext.LightsPanel.FocusTab = true;
	}


	vec3 ComputeDirectionFromAngles(float Pitch, float Yaw)
	{
		vec3 ArrowDirection;
		ArrowDirection.x = cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
		ArrowDirection.y = sin(glm::radians(Pitch));
		ArrowDirection.z = cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));
		ArrowDirection = normalize(ArrowDirection);
		return ArrowDirection;
	}


	void RenderLightsPanel(RLightsPanelContext* Panel, RWorld* World)
	{
		ImGui::SetNextWindowPos(ImVec2(180, 80), ImGuiCond_Appearing);
		ImGui::Begin("Lights Panel", &Panel->Active, ImGuiWindowFlags_None);
		ImGui::SetWindowSize("Lights Panel", ImVec2(330, 900), ImGuiCond_Always);
		Panel->Focused = ImGui::IsWindowFocused();

		ImGui::BeginTabBar("Types");

		// -------------
		// POINT LIGHTS
		// -------------
		auto PointFlags = (Panel->FocusTab && Panel->SelectedLightType == "point") ?
		ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
		if (ImGui::BeginTabItem("Point Lights", nullptr, PointFlags))
		{
			int DeletedLightIndex = -1;
			auto& PointLights = World->PointLights;

			// UNFOCUS TAB
			if (PointFlags == ImGuiTabItemFlags_SetSelected)
				Panel->FocusTab = false;

			// ADD BUTTON
			ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.6f, 0.6f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.7f, 0.7f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.8f, 0.8f)));
			if (ImGui::Button("Add new##point"))
			{
				// create new spotLight
				auto NewPointLight = new EPointLight();
				PointLights.push_back(NewPointLight);
				ActivateMoveLightMode("point", PointLights.size() - 1);
			}
			ImGui::PopStyleColor(3);

			for (int i = 0; i < PointLights.size(); i++)
			{
				string Header = "point Light source (" + std::to_string(i) + ")";
				bool IsActive = Panel->SelectedLight == i && Panel->SelectedLightType == "point";
				if (ImGui::CollapsingHeader(Header.c_str(), ImGuiTreeNodeFlags_NoAutoOpenOnLog) || IsActive)
				{
					// SHOW BUTTON
					EPointLight& Light = *PointLights[i];
					auto ShowName = "show##point" + std::to_string(i);
					if (ImGui::Checkbox(ShowName.c_str(), &IsActive))
					{
						Panel->SelectedLight = IsActive ? i : -1;
						Panel->SelectedLightType = "point";
					}

					// DELETE BUTTON
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.6f, 0.6f)));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.7f, 0.7f)));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.8f, 0.8f)));
					auto DeleteBtnLabel = "Delete##point" + std::to_string(i);
					if (ImGui::Button(DeleteBtnLabel.c_str()))
					{
						DeletedLightIndex = i;
					}
					ImGui::PopStyleColor(3);

					// position 
					ImGui::NewLine();
					float Positions[]{Light.Position[0], Light.Position[1], Light.Position[2]};
					auto LabelPos = "position##point" + std::to_string(i);
					if (ImGui::DragFloat3(LabelPos.c_str(), Positions, 0.3, -10.0, 10.0))
					{
						Light.Position = vec3{Positions[0], Positions[1], Positions[2]};
					}

					// diffuse color 
					float Diffuse[]{Light.Diffuse[0], Light.Diffuse[1], Light.Diffuse[2]};
					auto LabelDiffuse = "diffuse##point" + std::to_string(i);
					if (ImGui::ColorPicker3(LabelDiffuse.c_str(), Diffuse, ImGuiColorEditFlags_NoAlpha))
					{
						Light.Diffuse = vec3{Diffuse[0], Diffuse[1], Diffuse[2]};
					}

					// specular color 
					float Specular[]{Light.Specular[0], Light.Specular[1], Light.Specular[2]};
					auto LabelSpecular = "specular##point" + std::to_string(i);
					if (ImGui::ColorPicker3(LabelSpecular.c_str(), Specular, ImGuiColorEditFlags_NoAlpha))
					{
						Light.Specular = vec3{Specular[0], Specular[1], Specular[2]};
					}

					// intensity (decay)
					ImGui::Text("Intensity decay");
					auto LabelIntensityConst = "const##point" + std::to_string(i);
					ImGui::DragFloat(LabelIntensityConst.c_str(), &Light.IntensityConstant, 0.05, 0.0, 5.0);

					auto LabelIntensityLinear = "linear##point" + std::to_string(i);
					ImGui::DragFloat(LabelIntensityLinear.c_str(), &Light.IntensityLinear, 0.005, 0.0, 5.0);

					auto LabelIntensityQuad = "quadratic##point" + std::to_string(i);
					ImGui::DragFloat(LabelIntensityQuad.c_str(), &Light.IntensityQuadratic, 0.0005, 0.0, 5.0);

					ImGui::NewLine();
				}
			}

			if (DeletedLightIndex > -1)
				EditorDeleteLight(DeletedLightIndex, "point", World);


			ImGui::EndTabItem();
		}

		// ------------
		// SPOT LIGHTS
		// ------------
		auto SpotFlags = (Panel->FocusTab && Panel->SelectedLightType == "spot") ?
		ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
		if (ImGui::BeginTabItem("Spot Lights", nullptr, SpotFlags))
		{
			int DeletedLightIndex = -1;
			auto& SpotLights = World->SpotLights;

			// UNFOCUS TAB
			if (SpotFlags == ImGuiTabItemFlags_SetSelected)
				Panel->FocusTab = false;

			// ADD BUTTON
			ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.6f, 0.6f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.7f, 0.7f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.8f, 0.8f)));
			if (ImGui::Button("Add new##spot"))
			{
				// create new spotLight
				auto NewSpotLight = new ESpotLight();
				SpotLights.push_back(NewSpotLight);
				ActivateMoveLightMode("spot", SpotLights.size() - 1);
			}
			ImGui::PopStyleColor(3);

			for (int i = 0; i < SpotLights.size(); i++)
			{
				string Header = "spot Light source (" + std::to_string(i) + ")";
				bool IsActive = Panel->SelectedLight == i && Panel->SelectedLightType == "spot";
				if (ImGui::CollapsingHeader(Header.c_str()) || IsActive)
				{
					ESpotLight& Light = *SpotLights[i];

					// SHOW BUTTON
					auto ShowName = "show##spot" + std::to_string(i);
					if (ImGui::Checkbox(ShowName.c_str(), &IsActive))
					{
						Panel->SelectedLight = IsActive ? i : -1;
						Panel->SelectedLightType = "spot";
					}

					// DELETE BUTTON
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.6f, 0.6f)));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.7f, 0.7f)));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.8f, 0.8f)));
					auto DeleteBtnSpotLabel = "Delete##spot" + std::to_string(i);
					if (ImGui::Button(DeleteBtnSpotLabel.c_str()))
					{
						DeletedLightIndex = i;
					}
					ImGui::PopStyleColor(3);

					ImGui::NewLine();

					// position 
					float Positions[]{Light.Position[0], Light.Position[1], Light.Position[2]};
					auto LabelPos = "position##spot" + std::to_string(i);
					if (ImGui::DragFloat3(LabelPos.c_str(), Positions, 0.3, -10.0, 10.0))
					{
						Light.Position = vec3{Positions[0], Positions[1], Positions[2]};
					}

					// cones 
					auto LabelInnercone = "innercone##spot" + std::to_string(i);
					ImGui::DragFloat(LabelInnercone.c_str(), &Light.Innercone, 0.001, 1, MaxFloat);

					auto LabelOutercone = "outercone##spot" + std::to_string(i);
					ImGui::DragFloat(LabelOutercone.c_str(), &Light.Outercone, 0.001, 0, 1);

					// direction
					{
						float Yaw, Pitch;
						RCameraManager::ComputeAnglesFromDirection(Pitch, Yaw, Light.Direction);

						// pitch
						auto LabelPitch = "pitch##spot" + std::to_string(i);
						if (ImGui::SliderFloat(LabelPitch.c_str(), &Pitch, -89.0, 89.0))
						{
							Light.Direction = ComputeDirectionFromAngles(Pitch, Yaw);
						}

						//yaw
						auto LabelYaw = "yaw##spot" + std::to_string(i);
						if (ImGui::SliderFloat(LabelYaw.c_str(), &Yaw, -360.0, 360.0))
						{
							Light.Direction = ComputeDirectionFromAngles(Pitch, Yaw);
						}
					}

					// diffuse color 
					float Diffuse[]{Light.Diffuse[0], Light.Diffuse[1], Light.Diffuse[2]};
					auto LabelDiffuse = "diffuse##spot" + std::to_string(i);
					if (ImGui::ColorPicker3(LabelDiffuse.c_str(), Diffuse, ImGuiColorEditFlags_NoAlpha))
					{
						Light.Diffuse = vec3{Diffuse[0], Diffuse[1], Diffuse[2]};
					}

					// specular color 
					float Specular[]{Light.Specular[0], Light.Specular[1], Light.Specular[2]};
					auto LabelSpecular = "specular##spot" + std::to_string(i);
					if (ImGui::ColorPicker3(LabelSpecular.c_str(), Specular, ImGuiColorEditFlags_NoAlpha))
					{
						Light.Specular = vec3{Specular[0], Specular[1], Specular[2]};
					}

					// intensity
					// intensity (decay)
					ImGui::Text("Intensity decay");
					auto LabelIntensityConst = "const##spot" + std::to_string(i);
					ImGui::DragFloat(LabelIntensityConst.c_str(), &Light.IntensityConstant, 0.05, 0.0, 5.0);

					auto LabelIntensityLinear = "linear##spot" + std::to_string(i);
					ImGui::DragFloat(LabelIntensityConst.c_str(), &Light.IntensityLinear, 0.005, 0.0, 5.0);

					auto LabelIntensityQuad = "quadratic##spot" + std::to_string(i);
					ImGui::DragFloat(LabelIntensityConst.c_str(), &Light.IntensityQuadratic, 0.0005, 0.0, 5.0);


					ImGui::NewLine();
				}
			}

			if (DeletedLightIndex > -1)
				EditorDeleteLight(DeletedLightIndex, "spot", World);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();

		ImGui::End();
	}
}
