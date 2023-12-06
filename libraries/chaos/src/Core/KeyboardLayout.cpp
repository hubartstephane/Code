#include "chaos/ChaosPCH.h"

#include "chaos/ChaosInternals.h"

namespace chaos
{
	// this is the information obtained by running KeyboardLayout::Collect(...) with an azerty keyboard
	// the strings are modified by hand so that we can easily search in the table (Azerty and Qwerty names match as much as possible)
	KeyboardLayout const AzertyKeyboardLayout =
	{ {
	  {0x1, 0x1b, "Esc"},
	  {0x2, 0x31, "&"},
	  {0x3, 0x32, ""},
	  {0x4, 0x33, "\""},
	  {0x5, 0x34, "'"},
	  {0x6, 0x35, "("},
	  {0x7, 0x36, "-"},
	  {0x8, 0x37, ""},
	  {0x9, 0x38, "_"},
	  {0xa, 0x39, ""},
	  {0xb, 0x30, ""},
	  {0xc, 0xdb, ")"},
	  {0xd, 0xbb, "="},
	  {0xe, 0x08, "Backspace"},
	  {0xf, 0x09, "Tab"},
	  {0x10, 0x41, "A"},
	  {0x11, 0x5a, "Z"},
	  {0x12, 0x45, "E"},
	  {0x13, 0x52, "R"},
	  {0x14, 0x54, "T"},
	  {0x15, 0x59, "Y"},
	  {0x16, 0x55, "U"},
	  {0x17, 0x49, "I"},
	  {0x18, 0x4f, "O"},
	  {0x19, 0x50, "P"},
	  {0x1a, 0xdd, "^"}, // ACCENT CIRCONFLEXE
	  {0x1b, 0xba, "$"},
	  {0x1c, 0x0d, "Enter"},
	  {0x1d, 0x11, "Ctrl"},
	  {0x1e, 0x51, "Q"},
	  {0x1f, 0x53, "S"},
	  {0x20, 0x44, "D"},
	  {0x21, 0x46, "F"},
	  {0x22, 0x47, "G"},
	  {0x23, 0x48, "H"},
	  {0x24, 0x4a, "J"},
	  {0x25, 0x4b, "K"},
	  {0x26, 0x4c, "L"},
	  {0x27, 0x4d, "M"},
	  {0x28, 0xc0, ""},
	  {0x29, 0xde, ""},
	  {0x2a, 0x10, "Shift"},
	  {0x2b, 0xdc, "*"},
	  {0x2c, 0x57, "W"},
	  {0x2d, 0x58, "X"},
	  {0x2e, 0x43, "C"},
	  {0x2f, 0x56, "V"},
	  {0x30, 0x42, "B"},
	  {0x31, 0x4e, "N"},
	  {0x32, 0xbc, ","},
	  {0x33, 0xbe, ";"},
	  {0x34, 0xbf, ":"},
	  {0x35, 0xdf, "!"},
	  {0x36, 0x10, "Right Shift"},
	  {0x37, 0x6a, "Num *"},
	  {0x38, 0x12, "Alt"},
	  {0x39, 0x20, "Space"},
	  {0x3a, 0x14, "Caps Lock"},
	  {0x3b, 0x70, "F1"},
	  {0x3c, 0x71, "F2"},
	  {0x3d, 0x72, "F3"},
	  {0x3e, 0x73, "F4"},
	  {0x3f, 0x74, "F5"},
	  {0x40, 0x75, "F6"},
	  {0x41, 0x76, "F7"},
	  {0x42, 0x77, "F8"},
	  {0x43, 0x78, "F9"},
	  {0x44, 0x79, "F10"},
	  {0x45, 0x90, "Pause"},
	  {0x46, 0x91, "Scroll Lock"},
	  {0x47, 0x24, "Num 7"},
	  {0x48, 0x26, "Num 8"},
	  {0x49, 0x21, "Num 9"},
	  {0x4a, 0x6d, "Num -"},
	  {0x4b, 0x25, "Num 4"},
	  {0x4c, 0x0c, "Num 5"},
	  {0x4d, 0x27, "Num 6"},
	  {0x4e, 0x6b, "Num +"},
	  {0x4f, 0x23, "Num 1"},
	  {0x50, 0x28, "Num 2"},
	  {0x51, 0x22, "Num 3"},
	  {0x52, 0x2d, "Num 0"},
	  {0x53, 0x2e, "Num Del"},
	  {0x54, 0x2c, ""},
	  {0x56, 0xe2, "<"},
	  {0x57, 0x7a, "F11"},
	  {0x58, 0x7b, "F12"},
	  {0x59, 0x0c, ""},
	  {0x5a, 0xee, ""},
	  {0x5b, 0xf1, ""},
	  {0x5c, 0xea, ""},
	  {0x5d, 0xf9, ""},
	  {0x5e, 0xf5, ""},
	  {0x5f, 0xf3, ""},
	  {0x62, 0xfb, ""},
	  {0x63, 0x2f, ""},
	  {0x64, 0x7c, ""},
	  {0x65, 0x7d, ""},
	  {0x66, 0x7e, ""},
	  {0x67, 0x7f, ""},
	  {0x68, 0x80, ""},
	  {0x69, 0x81, ""},
	  {0x6a, 0x82, ""},
	  {0x6b, 0x83, ""},
	  {0x6c, 0x84, ""},
	  {0x6d, 0x85, ""},
	  {0x6e, 0x86, ""},
	  {0x6f, 0xed, ""},
	  {0x71, 0xe9, ""},
	  {0x73, 0xc1, ""},
	  {0x76, 0x87, ""},
	  {0x7b, 0xeb, ""},
	  {0x7c, 0x09, ""},
	  {0x7e, 0xc2, ""},
	  {0x102, 0x00, "&"},
	  {0x104, 0x00, "\""},
	  {0x105, 0x00, "'"},
	  {0x106, 0x00, "("},
	  {0x107, 0x00, "-"},
	  {0x109, 0x00, "_"},
	  {0x10c, 0x00, ")"},
	  {0x10d, 0x00, "="},
	  {0x110, 0x00, "A"},
	  {0x111, 0x00, "Z"},
	  {0x112, 0x00, "E"},
	  {0x113, 0x00, "R"},
	  {0x114, 0x00, "T"},
	  {0x115, 0x00, "Y"},
	  {0x116, 0x00, "U"},
	  {0x117, 0x00, "I"},
	  {0x118, 0x00, "O"},
	  {0x119, 0x00, "P"},
	  {0x11a, 0x00, "^"}, // ACCENT CIRCONFLEXE
	  {0x11b, 0x00, "$"},
	  {0x11c, 0x00, "Num Enter"},
	  {0x11d, 0x00, "Right Ctrl"},
	  {0x11e, 0x00, "Q"},
	  {0x11f, 0x00, "S"},
	  {0x120, 0x00, "D"},
	  {0x121, 0x00, "F"},
	  {0x122, 0x00, "G"},
	  {0x123, 0x00, "H"},
	  {0x124, 0x00, "J"},
	  {0x125, 0x00, "K"},
	  {0x126, 0x00, "L"},
	  {0x127, 0x00, "M"},
	  {0x12b, 0x00, "*"},
	  {0x12c, 0x00, "W"},
	  {0x12d, 0x00, "X"},
	  {0x12e, 0x00, "C"},
	  {0x12f, 0x00, "V"},
	  {0x130, 0x00, "B"},
	  {0x131, 0x00, "N"},
	  {0x132, 0x00, ","},
	  {0x133, 0x00, ";"},
	  {0x134, 0x00, ":"},
	  {0x135, 0x00, "Num /"},
	  {0x137, 0x00, "Print Screen"},
	  {0x138, 0x00, "Right Alt"},
	  {0x139, 0x00, " "},
	  {0x145, 0x00, "Num Lock"},
	  {0x146, 0x00, "Break"},
	  {0x147, 0x00, "Home"},
	  {0x148, 0x00, "Up"},
	  {0x149, 0x00, "Page Up"},
	  {0x14a, 0x00, "-"},
	  {0x14b, 0x00, "Left"},
	  {0x14d, 0x00, "Right"},
	  {0x14e, 0x00, "+"},
	  {0x14f, 0x00, "End"},
	  {0x150, 0x00, "Down"},
	  {0x151, 0x00, "Page Down"},
	  {0x152, 0x00, "Insert"},
	  {0x153, 0x00, "Delete"},
	  {0x154, 0x00, "<00>"},
	  {0x156, 0x00, "Help"},
	  {0x15b, 0x00, "Left Windows"},
	  {0x15c, 0x00, "Right Windows"},
	  {0x15d, 0x00, "Application"}
	}};

	// this is the information obtained by running KeyboardLayout::Collect(...) with an qwerty keyboard
	// the strings are modified by hand so that we can easily search in the table (Azerty and Qwerty names match as much as possible)
	KeyboardLayout const QwertyKeyboardLayout =
	{ {
	  {0x1, 0x1b, "Esc"},
	  {0x2, 0x31, "1"},
	  {0x3, 0x32, "2"},
	  {0x4, 0x33, "3"},
	  {0x5, 0x34, "4"},
	  {0x6, 0x35, "5"},
	  {0x7, 0x36, "6"},
	  {0x8, 0x37, "7"},
	  {0x9, 0x38, "8"},
	  {0xa, 0x39, "9"},
	  {0xb, 0x30, "0"},
	  {0xc, 0xbd, "-"},
	  {0xd, 0xbb, "="},
	  {0xe, 0x08, "Backspace"},
	  {0xf, 0x09, "Tab"},
	  {0x10, 0x51, "Q"},
	  {0x11, 0x57, "W"},
	  {0x12, 0x45, "E"},
	  {0x13, 0x52, "R"},
	  {0x14, 0x54, "T"},
	  {0x15, 0x59, "Y"},
	  {0x16, 0x55, "U"},
	  {0x17, 0x49, "I"},
	  {0x18, 0x4f, "O"},
	  {0x19, 0x50, "P"},
	  {0x1a, 0xdb, "["},
	  {0x1b, 0xdd, "]"},
	  {0x1c, 0x0d, "Enter"},
	  {0x1d, 0x11, "Ctrl"},
	  {0x1e, 0x41, "A"},
	  {0x1f, 0x53, "S"},
	  {0x20, 0x44, "D"},
	  {0x21, 0x46, "F"},
	  {0x22, 0x47, "G"},
	  {0x23, 0x48, "H"},
	  {0x24, 0x4a, "J"},
	  {0x25, 0x4b, "K"},
	  {0x26, 0x4c, "L"},
	  {0x27, 0xba, ";"},
	  {0x28, 0xde, "'"},
	  {0x29, 0xc0, "`"},
	  {0x2a, 0x10, "Shift"},
	  {0x2b, 0xdc, "\\"},
	  {0x2c, 0x5a, "Z"},
	  {0x2d, 0x58, "X"},
	  {0x2e, 0x43, "C"},
	  {0x2f, 0x56, "V"},
	  {0x30, 0x42, "B"},
	  {0x31, 0x4e, "N"},
	  {0x32, 0x4d, "M"},
	  {0x33, 0xbc, ","},
	  {0x34, 0xbe, "."},
	  {0x35, 0xbf, "/"},
	  {0x36, 0x10, "Right Shift"},
	  {0x37, 0x6a, "Num *"},
	  {0x38, 0x12, "Alt"},
	  {0x39, 0x20, "Space"},
	  {0x3a, 0x14, "Caps Lock"},
	  {0x3b, 0x70, "F1"},
	  {0x3c, 0x71, "F2"},
	  {0x3d, 0x72, "F3"},
	  {0x3e, 0x73, "F4"},
	  {0x3f, 0x74, "F5"},
	  {0x40, 0x75, "F6"},
	  {0x41, 0x76, "F7"},
	  {0x42, 0x77, "F8"},
	  {0x43, 0x78, "F9"},
	  {0x44, 0x79, "F10"},
	  {0x45, 0x90, "Pause"},
	  {0x46, 0x91, "Scroll Lock"},
	  {0x47, 0x24, "Num 7"},
	  {0x48, 0x26, "Num 8"},
	  {0x49, 0x21, "Num 9"},
	  {0x4a, 0x6d, "Num -"},
	  {0x4b, 0x25, "Num 4"},
	  {0x4c, 0x0c, "Num 5"},
	  {0x4d, 0x27, "Num 6"},
	  {0x4e, 0x6b, "Num +"},
	  {0x4f, 0x23, "Num 1"},
	  {0x50, 0x28, "Num 2"},
	  {0x51, 0x22, "Num 3"},
	  {0x52, 0x2d, "Num 0"},
	  {0x53, 0x2e, "Num Del"},
	  {0x54, 0x2c, "Sys Req"},
	  {0x56, 0xe2, "\\"},
	  {0x57, 0x7a, "F11"},
	  {0x58, 0x7b, "F12"},
	  {0x59, 0x0c, ""},
	  {0x5a, 0xee, ""},
	  {0x5b, 0xf1, ""},
	  {0x5c, 0xea, ""},
	  {0x5d, 0xf9, ""},
	  {0x5e, 0xf5, ""},
	  {0x5f, 0xf3, ""},
	  {0x62, 0xfb, ""},
	  {0x63, 0x2f, ""},
	  {0x64, 0x7c, ""},
	  {0x65, 0x7d, ""},
	  {0x66, 0x7e, ""},
	  {0x67, 0x7f, ""},
	  {0x68, 0x80, ""},
	  {0x69, 0x81, ""},
	  {0x6a, 0x82, ""},
	  {0x6b, 0x83, ""},
	  {0x6c, 0x84, ""},
	  {0x6d, 0x85, ""},
	  {0x6e, 0x86, ""},
	  {0x6f, 0xed, ""},
	  {0x71, 0xe9, ""},
	  {0x73, 0xc1, ""},
	  {0x76, 0x87, ""},
	  {0x7b, 0xeb, ""},
	  {0x7c, 0x09, "F13"},
	  {0x7d, 0x00, "F14"},
	  {0x7e, 0xc2, "F15"},
	  {0x7f, 0x00, "F16"},
	  {0x80, 0x00, "F17"},
	  {0x81, 0x00, "F18"},
	  {0x82, 0x00, "F19"},
	  {0x83, 0x00, "F20"},
	  {0x84, 0x00, "F21"},
	  {0x85, 0x00, "F22"},
	  {0x86, 0x00, "F23"},
	  {0x87, 0x00, "F24"},
	  {0x102, 0x00, "1"},
	  {0x103, 0x00, "2"},
	  {0x104, 0x00, "3"},
	  {0x105, 0x00, "4"},
	  {0x106, 0x00, "5"},
	  {0x107, 0x00, "6"},
	  {0x108, 0x00, "7"},
	  {0x109, 0x00, "8"},
	  {0x10a, 0x00, "9"},
	  {0x10b, 0x00, "0"},
	  {0x10c, 0x00, "-"},
	  {0x10d, 0x00, "="},
	  {0x110, 0x00, "Q"},
	  {0x111, 0x00, "W"},
	  {0x112, 0x00, "E"},
	  {0x113, 0x00, "R"},
	  {0x114, 0x00, "T"},
	  {0x115, 0x00, "Y"},
	  {0x116, 0x00, "U"},
	  {0x117, 0x00, "I"},
	  {0x118, 0x00, "O"},
	  {0x119, 0x00, "P"},
	  {0x11a, 0x00, "["},
	  {0x11b, 0x00, "]"},
	  {0x11c, 0x00, "Num Enter"},
	  {0x11d, 0x00, "Right Ctrl"},
	  {0x11e, 0x00, "A"},
	  {0x11f, 0x00, "S"},
	  {0x120, 0x00, "D"},
	  {0x121, 0x00, "F"},
	  {0x122, 0x00, "G"},
	  {0x123, 0x00, "H"},
	  {0x124, 0x00, "J"},
	  {0x125, 0x00, "K"},
	  {0x126, 0x00, "L"},
	  {0x127, 0x00, ";"},
	  {0x128, 0x00, "'"},
	  {0x129, 0x00, "`"},
	  {0x12b, 0x00, "\\"},
	  {0x12c, 0x00, "Z"},
	  {0x12d, 0x00, "X"},
	  {0x12e, 0x00, "C"},
	  {0x12f, 0x00, "V"},
	  {0x130, 0x00, "B"},
	  {0x131, 0x00, "N"},
	  {0x132, 0x00, "M"},
	  {0x133, 0x00, ","},
	  {0x134, 0x00, "."},
	  {0x135, 0x00, "Num /"},
	  {0x137, 0x00, "Print Screen"},
	  {0x138, 0x00, "Right Alt"},
	  {0x139, 0x00, " "},
	  {0x145, 0x00, "Num Lock"},
	  {0x146, 0x00, "Break"},
	  {0x147, 0x00, "Home"},
	  {0x148, 0x00, "Up"},
	  {0x149, 0x00, "Page Up"},
	  {0x14a, 0x00, "-"},
	  {0x14b, 0x00, "Left"},
	  {0x14d, 0x00, "Right"},
	  {0x14e, 0x00, "+"},
	  {0x14f, 0x00, "End"},
	  {0x150, 0x00, "Down"},
	  {0x151, 0x00, "Page Down"},
	  {0x152, 0x00, "Insert"},
	  {0x153, 0x00, "Delete"},
	  {0x154, 0x00, "<00>"},
	  {0x156, 0x00, "Help"},
	  {0x15b, 0x00, "Left Windows"},
	  {0x15c, 0x00, "Right Windows"},
	  {0x15d, 0x00, "Application"}
	}};

	static std::vector<std::pair<KeyboardLayoutType, char const*>> const keyboard_layout_map = {
		{ KeyboardLayoutType::AZERTY, "azerty" }, //default
		{ KeyboardLayoutType::QWERTY, "qwerty" },
		{ KeyboardLayoutType::CURRENT, "current" }
	};

	CHAOS_IMPLEMENT_ENUM_METHOD(KeyboardLayoutType, keyboard_layout_map);

#if _DEBUG

	bool DumpKeyboardLayoutToFile(char const* filename, char const* table_name, KeyboardLayout const& layout)
	{
		if (std::ofstream file = std::ofstream(filename, std::ios_base::trunc))
		{
			file << "KeyboardLayout const " << table_name << " = \n{{\n"; // << ;

			bool first_line = true;
			for (ScancodeInformation const& scancode_info : layout.GetScancodeTable())
			{
				if (!first_line)
					file << ",\n";

				file << "  { 0x" << std::hex << std::uppercase << std::setw(3) << std::setfill('0') << scancode_info.scancode
					 << ", 0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << scancode_info.vk
					 << ", \"" << scancode_info.name << "\"}";
				first_line = false;
			}
			file << "\n}};\n\n";

			return true;
		}
		return false;
	}

#endif // #if _DEBUG

	//
	// KeyboardLayout implementation
	//

	std::string KeyboardLayout::ScancodeToName(unsigned int scancode)
	{
#if _WIN32
		char name[256];
		if (::GetKeyNameTextA(scancode << 16, name, sizeof(name)))
			return name;
#endif // #if _WIN32
		return {};
	}

	KeyboardLayout KeyboardLayout::Collect(bool capture_names_from_known_layout)
	{
		KeyboardLayout result;

#if _WIN32

		for (unsigned int scancode = 0; scancode <= 0x1FF; ++scancode)
		{
			unsigned int vk = ::MapVirtualKey(scancode, MAPVK_VSC_TO_VK);
			if (vk == 0)
				vk = ::MapVirtualKey(scancode & 0xFF, MAPVK_VSC_TO_VK);

			// prepare the new entry
			ScancodeInformation new_scancode_info;
			new_scancode_info.scancode = scancode;
			new_scancode_info.vk = vk;

			// search the name from the known layout (if necessary)
			if (capture_names_from_known_layout)
			{
				if (vk != 0) // there is a valid VK to look at
				{
					for (KeyboardLayoutType type : {KeyboardLayoutType::AZERTY, KeyboardLayoutType::QWERTY})
					{
						if (ScancodeInformation const* scancode_info = KeyboardLayout::GetKnownLayout(type).GetInformationFromVK(vk))
						{
							if (scancode_info->name.length() > 0)
							{
								new_scancode_info.name = scancode_info->name; // we have the name
								break;
							}
						}
					}
				}

				if (new_scancode_info.name.length() == 0) // no name yet: search with scancode
				{
					for (KeyboardLayoutType type : {KeyboardLayoutType::AZERTY, KeyboardLayoutType::QWERTY})
					{
						if (ScancodeInformation const* scancode_info = KeyboardLayout::GetKnownLayout(type).GetInformationFromScancode(scancode))
						{
							if (scancode_info->name.length() > 0)
							{
								new_scancode_info.name = scancode_info->name; // we have the name
								break;
							}
						}
					}
				}
			}

			// no name: need to ask from OS API
			if (new_scancode_info.name.length() == 0)
			{
				new_scancode_info.name = ScancodeToName(scancode);

				// ignore names with special characters
				// this can happen for extended scancode > 0x100
				// this happens for
				//   ESCAPE + extended flag
				//   BACKSPACE + extended flag
				//   TAB
				for (size_t i = 0; i < new_scancode_info.name.length(); ++i)
					if (new_scancode_info.name[i] < 32) // 32 is SPACE. characters before are specials
						new_scancode_info.name = {}; // this stop the iteration and reset the name below
			}

			// insert the new element (if at least a vk or a name)
			if (new_scancode_info.vk != 0 || new_scancode_info.name.length() > 0)
				result.key_info.push_back(std::move(new_scancode_info));
		}
#endif // #if _WIN32
		return result;
	}

	KeyboardLayout const & KeyboardLayout::GetKnownLayout(KeyboardLayoutType type)
	{
		if (type == KeyboardLayoutType::AZERTY)
			return AzertyKeyboardLayout;
		if (type == KeyboardLayoutType::QWERTY)
			return QwertyKeyboardLayout;
		assert(type == KeyboardLayoutType::CURRENT);
		return GetCurrentLayout();
	}

	ScancodeInformation const* KeyboardLayout::GetInformationFromName(char const* name) const
	{
		for (ScancodeInformation const& scancode_info : key_info)
			if (StringTools::Stricmp(scancode_info.name, name) == 0)
				return &scancode_info;
		return nullptr;
	}

	ScancodeInformation const* KeyboardLayout::GetInformationFromScancode(unsigned int scancode) const
	{
		for (ScancodeInformation const& scancode_info : key_info)
			if (scancode_info.scancode == scancode)
				return &scancode_info;
		return nullptr;
	}

	ScancodeInformation const* KeyboardLayout::GetInformationFromVK(unsigned int vk) const
	{
		for (ScancodeInformation const& scancode_info : key_info)
			if (scancode_info.vk == vk)
				return &scancode_info;
		return nullptr;
	}

	std::optional<KeyboardLayout>& KeyboardLayout::GetCachedLayout()
	{
		static std::optional<KeyboardLayout> cached_layout;
		return cached_layout;
	}

	KeyboardLayout const& KeyboardLayout::GetCurrentLayout()
	{
		std::optional<KeyboardLayout> & cached_layout = GetCachedLayout();
		if (!cached_layout.has_value())
			cached_layout = Collect(true);
		return cached_layout.value();
	}

	void KeyboardLayout::InvalidateCachedLayout()
	{
		GetCachedLayout().reset();
	}

}; // namespace chaos
