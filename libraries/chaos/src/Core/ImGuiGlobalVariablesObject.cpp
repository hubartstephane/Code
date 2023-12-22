#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	void ImGuiGlobalVariablesObject::OnDrawImGuiContent(ImGuiDrawFlags flags)
	{
		if (ImGui::BeginTable("Global Variables", 2, ImGuiTableFlags_Resizable))
		{
			for (GlobalVariableBase* variable : chaos::GlobalVariableManager::GetInstance()->GetVariables())
			{
				if (variable != nullptr)
				{
					if (GlobalVariableInfo const* variable_info = variable->GetVariableInfo())
					{
						if (GlobalVariableImGuiRendererBase const* imgui_renderer = variable_info->GetImGuiRenderer())
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", variable->GetName());

							ImGui::TableNextColumn();
							imgui_renderer->DrawImGuiVariable(variable);
						}
					}
				}
			}
			ImGui::EndTable();
		}
	}

}; // namespace chaos