#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	class DragAndDropImGuiVectorSolverBase
	{
	public:

		virtual void Process(void* vector, size_t source_index, size_t destination_index) = 0;
	};

	template<typename T>
	class DragAndDropImGuiVectorSolver : public DragAndDropImGuiVectorSolverBase
	{
	public:

		static DragAndDropImGuiVectorSolverBase* GetInstance()
		{
			static DragAndDropImGuiVectorSolver<T> result;
			return &result;
		}

		virtual void Process(void* vector, size_t source_index, size_t destination_index) override
		{
			std::vector<T>* typed_vector = (std::vector<T> *)vector;




		}
	};


	class DragAndDropImGuiVectorHelper
	{

	protected:

		/** constructor */
		template<typename T>
		DragAndDropImGuiVectorHelper(std::vector<T> * in_source_vector, size_t in_source_index):
			source_vector(in_source_vector),
			source_index(in_source_index),
			solver(DragAndDropImGuiVectorSolver<T>::GetInstance())
		{
		}

		/** try to get the payload. only success if target_vector is the same than source_vector */
		template<typename T>
		static class DragAndDropImGuiVectorHelper const* GetCurrentPlayload(std::vector<T>& in_target_vector)
		{
			if (ImGuiPayload const* payload = ImGui::GetDragDropPayload())
			{
				DragAndDropImGuiVectorHelper const* result = (DragAndDropImGuiVectorHelper const*)payload;
				if (result->source_vector == &in_target_vector)
					return result;
			}
			return nullptr;
		}

	public:

		/** start a drag-and-drop */
		template<typename T>
		static bool DragSource(std::vector<T> & in_source_vector, size_t in_source_index, ImGuiDragDropFlags source_flags, LightweightFunction<void()> draw_tooltip_func = {})
		{
			if (ImGui::BeginDragDropSource(source_flags))
			{
				DragAndDropImGuiVectorHelper data(&in_source_vector, in_source_index);
				ImGui::SetDragDropPayload("CHAOS_VECTOR_DRAG_AND_DROP", &data, sizeof(DragAndDropImGuiVectorHelper));

				if (!(source_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
					draw_tooltip_func();

				ImGui::EndDragDropSource();
				return true;
			}
			return false;
		}

		/** end a drag-and-drop */
		template<typename T>
		static bool DropOnTarget(std::vector<T> & in_target_vector, size_t in_destination_index, ImGuiDragDropFlags target_flags)
		{
			if (ImGui::BeginDragDropTarget())
			{
				if (DragAndDropImGuiVectorHelper* payload = (DragAndDropImGuiVectorHelper*)ImGui::AcceptDragDropPayload("CHAOS_VECTOR_DRAG_AND_DROP", target_flags))
				{
					if (payload->source_vector == &in_target_vector)
					{
						payload->destination_index = in_destination_index;
						return true;
					}
				}
				ImGui::EndDragDropTarget();
			}
			return false;
		}

		/** finalize the drag-and-drop */
		template<typename T>
		static bool SolveDragAndDrop(std::vector<T> & in_vector)
		{
			if (DragAndDropImGuiVectorHelper const * payload = GetCurrentPlayload(in_vector))
			{
				if (payload->source_index.has_value() && payload->destination_index.has_value())
				{
					payload->solver->Process(payload->source_vector, payload->source_index.value(), payload->destination_index.value());
					return true;
				}

			}
			return false;
		}

	public:

		/** the vector which is concerned by the drag-and-drop */
		void * source_vector = nullptr;
		/** the index in the vector of the element as source for drag-and-drop */
		std::optional<size_t> destination_index;
		/** the index in the vector of the element as destination for drag-and-drop */
		std::optional<size_t> source_index;
		/** the operator for solving drag-and-drop */
		DragAndDropImGuiVectorSolverBase * solver = nullptr;
	};


	void DrawArray(std::vector<std::string> & names)
	{
		for (size_t i = 0; i < names.size(); ++i)
		{
			//ImGui::PushID(&names[i]);

			// render the drag and drop handler
			ImGui::Button(names[i].c_str());

			// source for drag and drop
			ImGuiDragDropFlags source_flags = 0;
			source_flags |= ImGuiDragDropFlags_SourceNoDisableHover;     // Keep the source displayed as hovered
			source_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Because our dragging is local, we disable the feature of opening foreign treenodes/tabs while dragging
			
			DragAndDropImGuiVectorHelper::DragSource(names, i, source_flags, [&names, i]()
			{
				ImGui::Text(names[i].c_str()); // ImGui::SameLine(); //ImGui::Button(names[i].c_str());
			});
			
			// target for drag and drop
			ImGuiDragDropFlags target_flags = 0;
			target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a target) to do something
			DragAndDropImGuiVectorHelper::DropOnTarget(names, i, target_flags);
			

			// render the object itself
			ImGui::SameLine(); ImGui::Text(names[i].c_str()); // ImGui::SameLine(); //ImGui::Button(names[i].c_str());


			//ImGui::PopID();
		}

		DragAndDropImGuiVectorHelper::SolveDragAndDrop(names);
	}











	void ImGuiGlobalVariablesObject::OnDrawImGuiContent()
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
							if (imgui_renderer->CanDrawVariable())
							{
								ImGui::TableNextColumn();
								ImGui::Text("%s", variable->GetName());

								ImGui::TableNextColumn();
								imgui_renderer->DrawVariable(variable);

							}
						}
					}
				}
			}
			ImGui::EndTable();


			ImGui::BeginDisabled();

			static glm::vec4 color = { 0.0f, 0.0f, 0.0f , 1.0f };
			DrawImGuiVariableImpl(color, DrawImGuiVariableFlags::COLOR);


			ImGui::EndDisabled();

			//static float col[4] = { 0.0f, 0.0f, 0.0f , 0.0f};
			//ImGui::ColorEdit4("colour", col, 0);

			//static ImVec4 col2 = { 0.0f, 0.0f, 0.0f, 0.0f };
			//ImGui::ColorButton("desc_id", col2);


			if (WindowApplication* WA = Application::GetInstance())
			{
				if (GPUResourceManager* RM = WA->GetGPUResourceManagerInstance())
				{
					if (GPUTexture const* T = RM->FindTexture("brightsideoflife"))
					{
						ImVec2 size = { 100, 100 };

						GPUTexture const* T2 = RM->FindTexture("background1");

						if (ImGui::ImageButton("", (ImTextureID)(intptr_t)T->GetResourceID(), size, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
						{
							T = T;
						}

						if (ImGui::IsItemHovered())
						{
							ImGui::ImageButton("", (ImTextureID)(intptr_t)T2->GetResourceID(), size, { 0.0f, 1.0f }, { 1.0f, 0.0f });
						}
					}


					RM = RM;
				}
			}	
		}

		ImGui::Separator();

		static std::vector<std::string> names1 = { "name1" , "name2", "name3", "name4", "name5", "name6", "name7", "name8", "name9", "name10" };
		ImGui::PushID(1);
		DrawArray(names1);
		ImGui::PopID();

		ImGui::Separator();

		static std::vector<std::string> names2 = { "xxxxxxx1" , "xxxxxxx2", "xxxxxxx3", "xxxxxxx4", "xxxxxxx5", "xxxxxxx6" };
		ImGui::PushID(1);
		DrawArray(names2);
		ImGui::PopID();


	}

}; // namespace chaos