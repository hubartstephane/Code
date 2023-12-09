namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class KeyEventActionProcessor;
	class KeyEventActionEnumerableInterface;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	* KeyEventActionProcessor
	*/

	class CHAOS_API KeyEventActionProcessor
	{
	public:

		/** operate the action. return true whenever iteration is to be stopped */
		virtual bool operator() (KeyboardButton button, int modifier, char const* name, std::function<void()> const& func);
	};

	/**
	* KeyEventActionEnumerableInterface
	*/

	class CHAOS_API KeyEventActionEnumerableInterface
	{
	public:

		/** declare all KeyEventActions */
		virtual bool EnumerateKeyEventActions(KeyEventActionProcessor & processor);
	};

#endif

}; // namespace chaos