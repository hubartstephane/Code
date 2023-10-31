#include "chaos/Chaos.h"

class WindowOpenGLTest : public chaos::Window
{
	CHAOS_DECLARE_OBJECT_CLASS(WindowOpenGLTest, chaos::Window);

public:

	virtual void OnDrawWindowImGuiMenu() override
	{
		 chaos::Window::OnDrawWindowImGuiMenu();
		// do not call super so that the application items are not inserted here
	}

	virtual bool OnKeyEventImpl(chaos::KeyEvent const& event) override
	{
		if (event.action == GLFW_PRESS)
		{
			last_scancode = event.scancode;
			last_keycode  = event.keycode;
			last_key_pressed = chaos::KeyboardLayout::GetKnownLayout(chaos::KeyboardLayoutType::AZERTY).GetInformationFromScancode(event.scancode);
		}

		return chaos::Window::OnKeyEventImpl(event);
	}

	virtual void OnDrawWindowImGuiContent()
	{
		ImGuiDrawableInterface::FullscreenWindow("conversion", false, [this]()
		{
			auto ImGui_DisplayConversion = [](int src_vk, chaos::KeyboardLayoutType src_layout)
			{
				chaos::ScancodeInformation const* src_entry = nullptr;

				int new_vk = chaos::KeyboardLayoutConversion::ConvertToCurrentLayoutConvertVirtualKeyToCurrentLayout(src_vk, src_layout, &src_entry);

				int new_scancode = ::MapVirtualKey(new_vk, MAPVK_VK_TO_VSC);
				std::string name = chaos::KeyboardLayout::ScancodeToName(new_scancode);

				char const* layout_name = (src_layout == chaos::KeyboardLayoutType::AZERTY) ? "azerty" : "qwerty";

				char const * src_entry_name = (src_entry != nullptr) ? src_entry->name.c_str() : "unknown";

				ImGui::Text("%s (%s) -> %s (current)", src_entry_name, layout_name, name.c_str());
			};

			auto xxx = glfwGetKeyScancode(GLFW_KEY_A);

			auto ppp = ::MapVirtualKey('A', MAPVK_VK_TO_VSC);

			ImGui_DisplayConversion('A', chaos::KeyboardLayoutType::AZERTY);
			ImGui_DisplayConversion('A', chaos::KeyboardLayoutType::QWERTY);
			ImGui::Separator();

			ImGui_DisplayConversion('Q', chaos::KeyboardLayoutType::AZERTY);
			ImGui_DisplayConversion('Q', chaos::KeyboardLayoutType::QWERTY);
			ImGui::Separator();

			ImGui_DisplayConversion('M', chaos::KeyboardLayoutType::AZERTY);
			ImGui_DisplayConversion('M', chaos::KeyboardLayoutType::QWERTY);
			ImGui::Separator();

			ImGui_DisplayConversion(VK_SPACE, chaos::KeyboardLayoutType::AZERTY);
			ImGui_DisplayConversion(VK_SPACE, chaos::KeyboardLayoutType::QWERTY);
			ImGui::Separator();

			ImGui_DisplayConversion(VK_SHIFT, chaos::KeyboardLayoutType::AZERTY);
			ImGui_DisplayConversion(VK_SHIFT, chaos::KeyboardLayoutType::QWERTY);
			ImGui::Separator();

			ImGui_DisplayConversion('0', chaos::KeyboardLayoutType::AZERTY);
			ImGui_DisplayConversion('0', chaos::KeyboardLayoutType::QWERTY);
			ImGui::Separator();

			int vk1 = ::VkKeyScan('*') & 0xFF;
			ImGui_DisplayConversion(vk1, chaos::KeyboardLayoutType::AZERTY);
			ImGui_DisplayConversion(vk1, chaos::KeyboardLayoutType::QWERTY);
			ImGui::Separator();

			int vk2 = ::VkKeyScan('$') & 0xFF;
			ImGui_DisplayConversion(vk2, chaos::KeyboardLayoutType::AZERTY);
			ImGui_DisplayConversion(vk2, chaos::KeyboardLayoutType::QWERTY);
			ImGui::Separator();

			if (last_keycode != -1)
			{
				chaos::Key k = chaos::Key(chaos::KeyboardButton(last_keycode));
				ImGui::Text("KEYCODE: [0x%x]  [%s]", last_keycode, k.GetName());
			}


			if (last_key_pressed)
			{
				ImGui::Text("SCANCODE: [0x%x]", last_key_pressed->scancode);
				ImGui::Text("VK:       [0x%x]", last_key_pressed->vk);
				ImGui::Text("NAME:     [%s]", last_key_pressed->name.c_str());
		
			}
			else if (last_scancode > 0)
			{
				ImVec4 new_color = { 1.0f, 0.0f, 0.0f, 1.0f };
				ImGui::PushStyleColor(ImGuiCol_Text, new_color);
				ImGui::Text("SCANCODE: [0x%x]", last_scancode); 
				ImGui::Text("VK:       [0x%x]", ::MapVirtualKeyA(last_scancode & 0xFF, MAPVK_VSC_TO_VK));
				ImGui::Text("NAME:     [UNKNOWN] !!!");

				ImGui::PopStyleColor(1);
			}
		});
	}

protected:

	chaos::ScancodeInformation const* last_key_pressed = nullptr;

	int last_scancode = -1;

	int last_keycode = -1;
};




#if 0
void GenerateKeyboardLayoutFiles()
{
	int layout_count = ::GetKeyboardLayoutList(0, nullptr);
	if (HKL* hkl = new HKL[layout_count])
	{
		if (::GetKeyboardLayoutList(layout_count, hkl) == layout_count)
		{
			for (int i = 0; i < layout_count; ++i)
			{

				::ActivateKeyboardLayout(hkl[i], KLF_SETFORPROCESS);

				char buffer[KL_NAMELENGTH];
				::GetKeyboardLayoutName(buffer);

				chaos::KeyboardLayout information = chaos::KeyboardLayout::Collect();

				if (information.GetInformationFromScancode(0x10)->vk == 'A')
					chaos::DumpKeyboardLayoutToFile("azerty2.txt", "AzertyKeyboardLayout", information);
				if (information.GetInformationFromScancode(0x10)->vk == 'Q')
					chaos::DumpKeyboardLayoutToFile("qwerty2.txt", "QwertyKeyboardLayout", information);
			}

		}
		delete[](hkl);
	}
}
#endif


int main(int argc, char ** argv, char ** env)
{
	//GenerateKeyboardLayoutFiles();

	chaos::WindowCreateParams create_params;
	create_params.monitor = nullptr;
	create_params.width = 800;
	create_params.height = 800;
	create_params.monitor_index = 0;
	 
	return chaos::RunWindowApplication<WindowOpenGLTest>(argc, argv, env, create_params);
}