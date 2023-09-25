#include "EditorLightsPanel.h"
#include <imgui.h>
#include "editor.h"
#include "tools/EditorTools.h"
#include "engine/camera/camera.h"
#include "engine/entities/lights.h"
#include "engine/world/World.h"

namespace Editor
{
	void OpenLightsPanel(string Type, int Index, bool FocusTab)
	{
		auto& EdContext = *GetContext();

		EdContext.LightsPanel.Active = true;
		if (type != "" && Index > -1)
		{
			EdContext.LightsPanel.SelectedLight = Index;
			EdContext.LightsPanel.SelectedLightType = type;
		}
		if (FocusTab)
			EdContext.LightsPanel.FocusTab = true;
	}


	vec3 ComputeDirectionFromAngles(float Pitch, float Yaw)
	{
		vec3 ArrowDirection;
		arrow_direction.x = cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
		arrow_direction.y = sin(glm::radians(Pitch));
		arrow_direction.z = cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));
		arrow_direction = normalize(arrow_direction);
		return arrow_direction;
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
		if (ImGui::BeginTabItem("Point Lights", nullptr, point_flags))
		{
			int DeletedLightIndex = -1;
			auto& Pointlights = World->PointLights;

			// UNFOCUS TAB
			if (point_flags == ImGuiTabItemFlags_SetSelected)
				Panel->FocusTab = false;

			// ADD BUTTON
			ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.6f, 0.6f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.7f, 0.7f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.8f, 0.8f)));
			if (ImGui::Button("Add new##point"))
			{
				// create new spotlight
				auto NewPointlight = new PointLight();
				pointlights.push_back(NewPointlight);
				ActivateMoveLightMode("point", pointlights.size() - 1);
			}
			ImGui::PopStyleColor(3);

			for (int I = 0; I < pointlights.size(); I++)
			{
				string Header = "point light source (" + std::to_string(i) + ")";
				bool IsActive = Panel->SelectedLight == I && Panel->SelectedLightType == "point";
				if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_NoAutoOpenOnLog) || IsActive)
				{
					// SHOW BUTTON
					PointLight& Light = *pointlights[I];
					auto ShowName = "show##point" + std::to_string(i);
					if (ImGui::Checkbox(show_name.c_str(), &IsActive))
					{
						Panel->SelectedLight = IsActive ? I : -1;
						Panel->SelectedLightType = "point";
					}

					// DELETE BUTTON
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.6f, 0.6f)));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.7f, 0.7f)));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.8f, 0.8f)));
					auto DeleteBtnLabel = "Delete##point" + std::to_string(i);
					if (ImGui::Button(delete_btn_label.c_str()))
					{
						DeletedLightIndex = I;
					}
					ImGui::PopStyleColor(3);

					// position 
					ImGui::NewLine();
					float Positions[]{Light.Position[0], Light.Position[1], Light.Position[2]};
					auto LabelPos = "position##point" + std::to_string(i);
					if (ImGui::DragFloat3(label_pos.c_str(), Positions, 0.3, -10.0, 10.0))
					{
						light.position = vec3{positions[0], positions[1], positions[2]};
					}

					// diffuse color 
					float Diffuse[]{Light.Diffuse[0], Light.Diffuse[1], Light.Diffuse[2]};
					auto LabelDiffuse = "diffuse##point" + std::to_string(i);
					if (ImGui::ColorPicker3(label_diffuse.c_str(), Diffuse, ImGuiColorEditFlags_NoAlpha))
					{
						light.diffuse = vec3{diffuse[0], diffuse[1], diffuse[2]};
					}

					// specular color 
					float Specular[]{Light.Specular[0], Light.Specular[1], Light.Specular[2]};
					auto LabelSpecular = "specular##point" + std::to_string(i);
					if (ImGui::ColorPicker3(label_specular.c_str(), Specular, ImGuiColorEditFlags_NoAlpha))
					{
						light.specular = vec3{specular[0], specular[1], specular[2]};
					}

					// intensity (decay)
					ImGui::Text("Intensity decay");
					auto LabelIntensityConst = "const##point" + std::to_string(i);
					ImGui::DragFloat(label_intensity_const.c_str(), &Light.IntensityConstant, 0.05, 0.0, 5.0);

					auto LabelIntensityLinear = "linear##point" + std::to_string(i);
					ImGui::DragFloat(label_intensity_linear.c_str(), &Light.IntensityLinear, 0.005, 0.0, 5.0);

					auto LabelIntensityQuad = "quadratic##point" + std::to_string(i);
					ImGui::DragFloat(label_intensity_quad.c_str(), &Light.IntensityQuadratic, 0.0005, 0.0, 5.0);

					ImGui::NewLine();
				}
			}

			if (DeletedLightIndex > -1)
				EditorEraseLight(DeletedLightIndex, "point", World);


			ImGui::EndTabItem();
		}

		// ------------
		// SPOT LIGHTS
		// ------------
		auto SpotFlags = (Panel->FocusTab && Panel->SelectedLightType == "spot") ?
		ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
		if (ImGui::BeginTabItem("Spot Lights", nullptr, spot_flags))
		{
			int DeletedLightIndex = -1;
			auto& Spotlights = World->SpotLights;

			// UNFOCUS TAB
			if (spot_flags == ImGuiTabItemFlags_SetSelected)
				Panel->FocusTab = false;

			// ADD BUTTON
			ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.6f, 0.6f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.7f, 0.7f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.8f, 0.8f)));
			if (ImGui::Button("Add new##spot"))
			{
				// create new spotlight
				auto NewSpotlight = new SpotLight();
				spotlights.push_back(NewSpotlight);
				ActivateMoveLightMode("spot", spotlights.size() - 1);
			}
			ImGui::PopStyleColor(3);

			for (int I = 0; I < spotlights.size(); I++)
			{
				string Header = "spot light source (" + std::to_string(i) + ")";
				bool IsActive = Panel->SelectedLight == I && Panel->SelectedLightType == "spot";
				if (ImGui::CollapsingHeader(header.c_str()) || IsActive)
				{
					SpotLight& Light = *spotlights[I];

					// SHOW BUTTON
					auto ShowName = "show##spot" + std::to_string(i);
					if (ImGui::Checkbox(show_name.c_str(), &IsActive))
					{
						Panel->SelectedLight = IsActive ? I : -1;
						Panel->SelectedLightType = "spot";
					}

					// DELETE BUTTON
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.6f, 0.6f)));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.7f, 0.7f)));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.8f, 0.8f)));
					auto DeleteBtnSpotLabel = "Delete##spot" + std::to_string(i);
					if (ImGui::Button(delete_btn_spot_label.c_str()))
					{
						DeletedLightIndex = I;
					}
					ImGui::PopStyleColor(3);

					ImGui::NewLine();

					// position 
					float Positions[]{Light.Position[0], Light.Position[1], Light.Position[2]};
					auto LabelPos = "position##spot" + std::to_string(i);
					if (ImGui::DragFloat3(label_pos.c_str(), Positions, 0.3, -10.0, 10.0))
					{
						light.position = vec3{positions[0], positions[1], positions[2]};
					}

					// cones 
					auto LabelInnercone = "innercone##spot" + std::to_string(i);
					ImGui::DragFloat(label_innercone.c_str(), &Light.Innercone, 0.001, 1, MaxFloat);

					auto LabelOutercone = "outercone##spot" + std::to_string(i);
					ImGui::DragFloat(label_outercone.c_str(), &Light.Outercone, 0.001, 0, 1);

					// direction
					{
						float Yaw, Pitch;
						RCameraManager::ComputeAnglesFromDirection(Pitch, Yaw, Light.Direction);

						// pitch
						auto LabelPitch = "pitch##spot" + std::to_string(i);
						if (ImGui::SliderFloat(label_pitch.c_str(), &Pitch, -89.0, 89.0))
						{
							light.direction = ComputeDirectionFromAngles(pitch, yaw);
						}

						//yaw
						auto LabelYaw = "yaw##spot" + std::to_string(i);
						if (ImGui::SliderFloat(label_yaw.c_str(), &Yaw, -360.0, 360.0))
						{
							light.direction = ComputeDirectionFromAngles(pitch, yaw);
						}
					}

					// diffuse color 
					float Diffuse[]{Light.Diffuse[0], Light.Diffuse[1], Light.Diffuse[2]};
					auto LabelDiffuse = "diffuse##spot" + std::to_string(i);
					if (ImGui::ColorPicker3(label_diffuse.c_str(), Diffuse, ImGuiColorEditFlags_NoAlpha))
					{
						light.diffuse = vec3{diffuse[0], diffuse[1], diffuse[2]};
					}

					// specular color 
					float Specular[]{Light.Specular[0], Light.Specular[1], Light.Specular[2]};
					auto LabelSpecular = "specular##spot" + std::to_string(i);
					if (ImGui::ColorPicker3(label_specular.c_str(), Specular, ImGuiColorEditFlags_NoAlpha))
					{
						light.specular = vec3{specular[0], specular[1], specular[2]};
					}

					// intensity
					// intensity (decay)
					ImGui::Text("Intensity decay");
					auto LabelIntensityConst = "const##spot" + std::to_string(i);
					ImGui::DragFloat(label_intensity_const.c_str(), &Light.IntensityConstant, 0.05, 0.0, 5.0);

					auto LabelIntensityLinear = "linear##spot" + std::to_string(i);
					ImGui::DragFloat(label_intensity_linear.c_str(), &Light.IntensityLinear, 0.005, 0.0, 5.0);

					auto LabelIntensityQuad = "quadratic##spot" + std::to_string(i);
					ImGui::DragFloat(label_intensity_quad.c_str(), &Light.IntensityQuadratic, 0.0005, 0.0, 5.0);


					ImGui::NewLine();
				}
			}

			if (DeletedLightIndex > -1)
				EditorEraseLight(DeletedLightIndex, "spot", World);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();

		ImGui::End();
	}
}
