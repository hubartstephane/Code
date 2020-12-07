#ifdef CHAOS_FORWARD_DECLARATION

namespace chaos
{
	class KeyDefinition;

	enum XBoxButton;
	enum XBoxAxis;
	enum class Key;
	enum class MouseButton;

}; // namespace chaos

#else 



namespace chaos
{
#define CHAOS_KEYDEF(KEY) KEY = GLFW_KEY_##KEY

	/**
	 * Key : A keyboard key
	 */
	enum class Key : int
	{
		CHAOS_KEYDEF(UNKNOWN),
		CHAOS_KEYDEF(SPACE),
		CHAOS_KEYDEF(APOSTROPHE),
		CHAOS_KEYDEF(COMMA),
		CHAOS_KEYDEF(MINUS),
		CHAOS_KEYDEF(PERIOD),
		CHAOS_KEYDEF(SLASH),
		NUM_0 = GLFW_KEY_0,
		NUM_1 = GLFW_KEY_1,
		NUM_2 = GLFW_KEY_2,
		NUM_3 = GLFW_KEY_3,
		NUM_4 = GLFW_KEY_4,
		NUM_5 = GLFW_KEY_5,
		NUM_6 = GLFW_KEY_6,
		NUM_7 = GLFW_KEY_7,
		NUM_8 = GLFW_KEY_8,
		NUM_9 = GLFW_KEY_9,
		CHAOS_KEYDEF(SEMICOLON),
		CHAOS_KEYDEF(EQUAL),
		CHAOS_KEYDEF(A),
		CHAOS_KEYDEF(B),
		CHAOS_KEYDEF(C),
		CHAOS_KEYDEF(D),
		CHAOS_KEYDEF(E),
		CHAOS_KEYDEF(F),
		CHAOS_KEYDEF(G),
		CHAOS_KEYDEF(H),
		CHAOS_KEYDEF(I),
		CHAOS_KEYDEF(J),
		CHAOS_KEYDEF(K),
		CHAOS_KEYDEF(L),
		CHAOS_KEYDEF(M),
		CHAOS_KEYDEF(N),
		CHAOS_KEYDEF(O),
		CHAOS_KEYDEF(P),
		CHAOS_KEYDEF(Q),
		CHAOS_KEYDEF(R),
		CHAOS_KEYDEF(S),
		CHAOS_KEYDEF(T),
		CHAOS_KEYDEF(U),
		CHAOS_KEYDEF(V),
		CHAOS_KEYDEF(W),
		CHAOS_KEYDEF(X),
		CHAOS_KEYDEF(Y),
		CHAOS_KEYDEF(Z),
		CHAOS_KEYDEF(LEFT_BRACKET),
		CHAOS_KEYDEF(BACKSLASH),
		CHAOS_KEYDEF(RIGHT_BRACKET),
		CHAOS_KEYDEF(GRAVE_ACCENT),
		CHAOS_KEYDEF(WORLD_1),
		CHAOS_KEYDEF(WORLD_2),
		CHAOS_KEYDEF(ESCAPE),
		CHAOS_KEYDEF(ENTER),
		CHAOS_KEYDEF(TAB),
		CHAOS_KEYDEF(BACKSPACE),
		CHAOS_KEYDEF(INSERT),
		CHAOS_KEYDEF(DELETE),
		CHAOS_KEYDEF(RIGHT),
		CHAOS_KEYDEF(LEFT),
		CHAOS_KEYDEF(DOWN),
		CHAOS_KEYDEF(UP),
		CHAOS_KEYDEF(PAGE_UP),
		CHAOS_KEYDEF(PAGE_DOWN),
		CHAOS_KEYDEF(HOME),
		CHAOS_KEYDEF(END),
		CHAOS_KEYDEF(CAPS_LOCK),
		CHAOS_KEYDEF(SCROLL_LOCK),
		CHAOS_KEYDEF(NUM_LOCK),
		CHAOS_KEYDEF(PRINT_SCREEN),
		CHAOS_KEYDEF(PAUSE),
		CHAOS_KEYDEF(F1),
		CHAOS_KEYDEF(F2),
		CHAOS_KEYDEF(F3),
		CHAOS_KEYDEF(F4),
		CHAOS_KEYDEF(F5),
		CHAOS_KEYDEF(F6),
		CHAOS_KEYDEF(F7),
		CHAOS_KEYDEF(F8),
		CHAOS_KEYDEF(F9),
		CHAOS_KEYDEF(F10),
		CHAOS_KEYDEF(F11),
		CHAOS_KEYDEF(F12),
		CHAOS_KEYDEF(F13),
		CHAOS_KEYDEF(F14),
		CHAOS_KEYDEF(F15),
		CHAOS_KEYDEF(F16),
		CHAOS_KEYDEF(F17),
		CHAOS_KEYDEF(F18),
		CHAOS_KEYDEF(F19),
		CHAOS_KEYDEF(F20),
		CHAOS_KEYDEF(F21),
		CHAOS_KEYDEF(F22),
		CHAOS_KEYDEF(F23),
		CHAOS_KEYDEF(F24),
		CHAOS_KEYDEF(F25),
		CHAOS_KEYDEF(KP_0),
		CHAOS_KEYDEF(KP_1),
		CHAOS_KEYDEF(KP_2),
		CHAOS_KEYDEF(KP_3),
		CHAOS_KEYDEF(KP_4),
		CHAOS_KEYDEF(KP_5),
		CHAOS_KEYDEF(KP_6),
		CHAOS_KEYDEF(KP_7),
		CHAOS_KEYDEF(KP_8),
		CHAOS_KEYDEF(KP_9),
		CHAOS_KEYDEF(KP_DECIMAL),
		CHAOS_KEYDEF(KP_DIVIDE),
		CHAOS_KEYDEF(KP_MULTIPLY),
		CHAOS_KEYDEF(KP_SUBTRACT),
		CHAOS_KEYDEF(KP_ADD),
		CHAOS_KEYDEF(KP_ENTER),
		CHAOS_KEYDEF(KP_EQUAL),
		CHAOS_KEYDEF(LEFT_SHIFT),
		CHAOS_KEYDEF(LEFT_CONTROL),
		CHAOS_KEYDEF(LEFT_ALT),
		CHAOS_KEYDEF(LEFT_SUPER),
		CHAOS_KEYDEF(RIGHT_SHIFT),
		CHAOS_KEYDEF(RIGHT_CONTROL),
		CHAOS_KEYDEF(RIGHT_ALT),
		CHAOS_KEYDEF(RIGHT_SUPER),
		CHAOS_KEYDEF(MENU)
	};
#undef CHAOS_KEYDEF

	/**
	 * MouseButton
	 */
	enum class MouseButton : int
	{
		UNKNOWN  = -1,
		BUTTON_1 = GLFW_MOUSE_BUTTON_1,
		BUTTON_2 = GLFW_MOUSE_BUTTON_2,
		BUTTON_3 = GLFW_MOUSE_BUTTON_3,
		BUTTON_4 = GLFW_MOUSE_BUTTON_4,
		BUTTON_5 = GLFW_MOUSE_BUTTON_5,
		BUTTON_6 = GLFW_MOUSE_BUTTON_6,
		BUTTON_7 = GLFW_MOUSE_BUTTON_7,
		BUTTON_8 = GLFW_MOUSE_BUTTON_8
	};


	// XXX: there are important changes in GLFW 3.3 relative to 3.1
	//
	//  -XBOX_RIGHT_AXIS_X and XBOX_RIGHT_AXIS_Y : are bound to other indices.
	//
	//  -in GLFW 3.1, LEFT and RIGHT triggers were bound to the same index
	//                LEFT  giving a value in [-1 .. 0] 
	//                RIGHT giving a value in [0 .. +1] 
	//
	//  -in GLFW 3.3, LEFT are RIGHT TRIGGERS are now bound to different indices 
	//                LEFT  giving a value in [-1 .. +1] 
	//                RIGHT giving a value in [-1 .. +1] 
	//
	//                while they are not bound to the same index, you can check both values

	// shuxxx see glfw3.h => some values seem to differ    GLFW_GAMEPAD_BUTTON_LEFT_THUMB ? etc

	enum XBoxButton : int // XXXX: no class, so this can be implicitly converted to int
	{
		/** index in buttons of A for XBOX like pad */
		BUTTON_A = 0,
		/** index in buttons of B for XBOX like pad */
		BUTTON_B = 1,
		/** index in buttons of X for XBOX like pad */
		BUTTON_X = 2,
		/** index in buttons of Y for XBOX like pad */
		BUTTON_Y = 3,
		/** index in buttons of LEFT for XBOX like pad (the one behind the LEFT TRIGGER) */
		BUTTON_LEFTBUT = 4,
		/** index in buttons of RIGHT for XBOX like pad (the one behind the RIGHT TRIGGER) */
		BUTTON_RIGHTBUT = 5,
		/** index in buttons of SELECT for XBOX like pad */
		BUTTON_SELECT = 6,
		/** index in buttons of START for XBOX like pad */
		BUTTON_START = 7,
		/** index in buttons of LEFT-STICK-CLICKED for XBOX like pad */
		BUTTON_LEFTSTICK = 8,
		/** index in buttons of RIGHT-STICK-CLICKED for XBOX like pad */
		BUTTON_RIGHTSTICK = 9,
		/** index in buttons of UP for XBOX like pad */
		BUTTON_UP = 10,
		/** index in buttons of DOWN for XBOX like pad */
		BUTTON_DOWN = 12,
		/** index in buttons of LEFT for XBOX like pad */
		BUTTON_LEFT = 13,
		/** index in buttons of RIGHT for XBOX like pad */
		BUTTON_RIGHT = 11,

		/** index in buttons of LEFT TRIGGER for XBOX like pad (this is a simulate button, while the physical left trigger is an axis) */
		BUTTON_LEFTTRIGGER = 101,
		/** index in buttons of RIGHT TRIGGER for XBOX like pad (this is a simulate button, while the physical right trigger is an axis) */
		BUTTON_RIGHTTRIGGER = 102
	};

	enum XBoxAxis : int // XXXX: no class, so this can be implicitly converted to int
	{
		/** index in axis of LEFT X for XBOX like pad */
		LEFT_AXIS_X = 0,
		/** index in axis of LEFT Y for XBOX like pad */
		LEFT_AXIS_Y = 1, // STICK DOWN = positive values

		/** index in axis of RIGHT X for XBOX like pad */
		RIGHT_AXIS_X = 2,
		/** index in axis of RIGHT Y for XBOX like pad */
		RIGHT_AXIS_Y = 3,  // STICK DOWN = positive values

		/** index in axis for the LEFT trigger for XBOX like pad (beware its value is between [-1 .. +1]) */
		LEFT_TRIGGER = 4,
		/** index in axis for the RIGHT trigger for XBOX like pad (beware its value is between [-1 .. +1]) */
		RIGHT_TRIGGER = 5,

		/** returns the direction of left stick (beware the low level interface only knows for axis) */
		LEFT_AXIS = 0,
		/** returns the direction of right stick (beware the low level interface only knows for axis) */
		RIGHT_AXIS = 1
	};
















	class KeyDefinition
	{
	public:

		/** get a key from its name */
		static Key GetKey(char const* name);
		/** get a key name  */
		static char const * GetKeyName(Key value);

		/** get a mouse button from its name */
		static MouseButton GetMouseButton(char const* name);
		/** get a mouse button name  */
		static char const* GetMouseButtonName(MouseButton value);

	protected:

		/** the map between key and their name */
		static std::vector<std::pair<Key, char const *>> const key_map;
		/** the map between mouse button and their name */
		static std::vector<std::pair<MouseButton, char const*>> const mousebutton_map;
	};

}; // namespace chaos

#endif // CHAOS_FORWARD_DECLARATION

