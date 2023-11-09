namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	//
	//                                      TICK RANGE
	//                                   +--------------+
	//                                   v              v
	//                           EXECUTION RANGE
	//                +---------------------------------+
	//                v                                 v
	// ----+-----------------------------+-----------------------------+ Tick system
	//                                   |          TIME SLICE         |
	//
	//

	using ClockTimeRange = std::pair<double, double>;

	class ClockEventTickData;
	class ClockEventTickRegistration;
	class ClockEventRepetitionInfo;
	class ClockEventInfo;
	class ClockEventTickResult;
	class ClockEvent;
	class ClockEventTickSort;
	class ClockCreateParams;
	class Clock;

	/** events ordered by start time */
	using ClockEventTickSet = std::set<ClockEventTickRegistration, ClockEventTickSort>;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	* ClockEventTickData : describe all information for ticking an event
	*/

	class CHAOS_API ClockEventTickData
	{

	public:

		/** returns the event is really to be ticked */
		bool IsValid() const
		{
			return
				(tick_range.first != std::numeric_limits<double>::max()) &&
				(tick_range.second != std::numeric_limits<double>::max());
		}

	public:

		/** the event without any consideration of time slicing */
		ClockTimeRange execution_range{ 0.0, 0.0 };
		/** the whole tick range */
		ClockTimeRange time_slice{ 0.0, 0.0 };
		/** the tick range where event is activate */
		ClockTimeRange tick_range{ 0.0, 0.0 };
	};

	/**
	* ClockEventTickRegistration : events to tick are registered inside that structure
	*/

	class CHAOS_API ClockEventTickRegistration : public ClockEventTickData
	{
	public:

		/** the event */
		shared_ptr<ClockEvent> clock_event;
		/** the absolute (top level clock reference) time lapse before starting the event */
		double abs_time_to_start = 0.0;
	};

	/**
	* ClockEventRepetitionInfo
	*/

	class CHAOS_API ClockEventRepetitionInfo
	{
	public:

		/** default constructor */
		ClockEventRepetitionInfo() = default;
		/** intialization constructor */
		ClockEventRepetitionInfo(double in_repetition_delay, int in_repetition_count) :
			repetition_delay(in_repetition_delay),
			repetition_count(in_repetition_count) {}

		/** return an instance with no repetition */
		static ClockEventRepetitionInfo NoRepetition()
		{
			return ClockEventRepetitionInfo();
		}

		static ClockEventRepetitionInfo InfiniteRepetition(double in_repetition_delay)
		{
			assert(in_repetition_delay >= 0.0);
			return ClockEventRepetitionInfo(in_repetition_delay, -1);
		}

		static ClockEventRepetitionInfo Repetition(double in_repetition_delay, int in_repetition_count)
		{
			assert(in_repetition_delay >= 0.0);
			assert(in_repetition_count >= 0);
			return ClockEventRepetitionInfo(in_repetition_delay, in_repetition_count);
		}

		/** returns true whether the event is repeated */
		bool IsRepeated() const { return (repetition_count != 0); }
		/** returns true whether the event is repeated a limited number of time */
		bool IsRepeatedLimited() const { return (repetition_count > 0.0); }
		/** returns true whether the event is repeated forever  */
		bool IsRepeatedInfinitly() const { return (repetition_count < 0.0); }

	public:

		/** the delay between repetitions */
		double repetition_delay = 0.0;
		/** the number of repetitions (0 for no repetition, <0 for unlimited repetition, >0 for well known repetition count) */
		int repetition_count = 0;
	};

	/**
	* ClockEventInfo : some information that describes the event
	*/

	class CHAOS_API ClockEventInfo : public ClockEventRepetitionInfo
	{
	public:

		/** default constructor */
		ClockEventInfo() = default;
		/** constructor with initialization */
		ClockEventInfo(double in_start_time, double in_duration, ClockEventRepetitionInfo const& in_repetition_info = ClockEventRepetitionInfo::NoRepetition()) :
			ClockEventRepetitionInfo(in_repetition_info),
			start_time(in_start_time),
			duration(in_duration)
		{
		}

		/** creates an instance that describes an event to be ticked once */
		static ClockEventInfo SingleTickEvent(double in_start_time, ClockEventRepetitionInfo const& in_repetition_info = ClockEventRepetitionInfo::NoRepetition())
		{
			return ClockEventInfo(in_start_time, 0.0, in_repetition_info);
		}
		/** creates an instance that describes an event to be ticked for a given time */
		static ClockEventInfo RangeEvent(double in_start_time, double in_duration, ClockEventRepetitionInfo const& in_repetition_info = ClockEventRepetitionInfo::NoRepetition())
		{
			return ClockEventInfo(in_start_time, in_duration, in_repetition_info);
		}
		/** creates an instance that describes an event to be ticked for a given time */
		static ClockEventInfo ForeverEvent(double in_start_time, ClockEventRepetitionInfo const& in_repetition_info = ClockEventRepetitionInfo::NoRepetition())
		{
			return ClockEventInfo(in_start_time, -1.0, in_repetition_info);
		}

		/** returns true whether the event is to be ticked only once */
		bool IsSingleTickEvent() const { return (duration == 0.0); }
		/** returns true whether the event is to be ticked for a given range */
		bool IsRangeEvent() const { return (duration > 0.0); }
		/** returns true whether the event is to be ticked for an undermined time */
		bool IsForeverEvent() const { return (duration < 0.0); }

		/** the maximum time the event can be triggered */
		double GetMaxEventTime() const;
		/** get the time range for the first execution in this time slice */
		ClockEventTickData GetExecutionInfo(double t1, double t2) const;
		/** returns true whether this event will never be triggered */
		bool IsTooLateFor(double current_time) const;

	public:

		/** the initial time */
		double start_time = 0.0;
		/** duration (0 for single tick event, <0 for unknown duration, >0 for a well known duration) */
		double duration = 0.0;
	};

	/**
	* ClockEventTickResult
	*/

	class CHAOS_API ClockEventTickResult
	{
	protected:

		/** default constructor is protected */
		ClockEventTickResult() = default;

	public:

		/** returns true whether the event is completed */
		bool IsExecutionCompleted() const { return completed; }
		/** returns true whether the event is completed and is to be restarted */
		bool CanRepeatExecution() const { return can_repeat; }

		/** some default 'constructor' */
		static ClockEventTickResult CompleteExecution() // this executions ends. A new one may be started if repetition are enabled
		{
			ClockEventTickResult result;
			result.completed = true;
			result.can_repeat = true;
			return result;
		}
		/** some default 'constructor' */
		static ClockEventTickResult CompleteAll() // current execution is finished, next repetition are aborded
		{
			ClockEventTickResult result;
			result.completed = true;
			result.can_repeat = false;
			return result;
		}
		/** some default 'constructor' */
		static ClockEventTickResult ContinueExecution() // this execution is not finished. continued on next tick
		{
			ClockEventTickResult result;
			result.completed = false;
			result.can_repeat = true;
			return result;
		}

	public:

		/** whether event is completed */
		bool completed = true;
		/** whether event can be repeated */
		bool can_repeat = true;
	};

	/**
	* Event that can be triggered by clock
	*/

	class CHAOS_API ClockEvent : public Object
	{
		friend class Clock;

	public:

		/** destructor */
		virtual ~ClockEvent() = default;
		/** remove event from its clock */
		bool RemoveFromClock();

		/** get the info */
		ClockEventInfo& GetEventInfo() { return event_info; }
		ClockEventInfo const& GetEventInfo() const { return event_info; }

		/** returns true whether the event is ticked for the very first time */
		bool IsFirstTick() const { return tick_count == 0; }
		/** returns true whether the event is executed for the very first time */
		bool IsFirstExecution() const { return execution_count == 0; }

		/** returns the tick count */
		int GetTickCount() const { return tick_count; }
		/** returns the execution count */
		int GetExecutionCount() const { return execution_count; }

		/** the public method to process */
		virtual ClockEventTickResult Tick(ClockEventTickData const& tick_data) { return CompleteExecution(); }
		/** called whenever the event is removed from the clock */
		virtual void OnEventRemovedFromClock() {}

	protected:

		/** utility functions to for process results */
		ClockEventTickResult CompleteExecution() const { return ClockEventTickResult::CompleteExecution(); }
		/** utility functions to for process results */
		ClockEventTickResult ContinueExecution() const { return ClockEventTickResult::ContinueExecution(); }
		/** utility functions to for process results */
		ClockEventTickResult CompleteAll() const { return ClockEventTickResult::CompleteAll(); }

	protected:

		/** the information for the event */
		ClockEventInfo event_info;
		/** the tick count for current execution */
		int tick_count = 0;
		/** the execution count (used for restarted event) */
		int execution_count = 0;
		/** the clock it belongs to */
		class Clock* clock = nullptr;
	};

	/**
	* ClockEventTickSort : used to sort events by start time
	*/

	class CHAOS_API ClockEventTickSort
	{
	public:

		bool operator ()(ClockEventTickRegistration const& src1, ClockEventTickRegistration const& src2) const
		{
			return src1.abs_time_to_start < src2.abs_time_to_start;
		};
	};

	/**
	* ClockCreateParams : data for clock creation
	*/

	class CHAOS_API ClockCreateParams
	{
	public:

		/** a time scale for the clock */
		float time_scale = 1.0;
		/** whether the clock is paused or not */
		bool   paused = false;
	};

	/**
	* This represents a clock that may paused or have a different time scale than absolute time. It can have inner clocks
	*/

	class CHAOS_API Clock : public Object, public JSONSerializableInterface
	{
		friend class ClockEvent;

		CHAOS_DECLARE_OBJECT_CLASS(Clock, Object);

	public:

		/** the constructor */
		Clock(char const* in_name = nullptr, ClockCreateParams const& params = ClockCreateParams());
		/** the destructor */
		virtual ~Clock();

		/** Reference count updated : 2 use cases (external reference or not) */
		virtual void SubReference() override;

		/** returns the internal time */
		double GetClockTime() const { return clock_time; }
		/** returns the clock name */
		char const* GetClockName() const { return name.c_str(); }
		/** returns the parent clock */
		Clock* GetParentClock() { return parent_clock; }
		/** returns the parent clock */
		Clock const* GetParentClock() const { return parent_clock; }

		/** change the pause state */
		void SetPause(bool in_pause);
		/** pause the clock */
		void Pause() { paused = true; }
		/** resume the clock */
		void Resume() { paused = false; }
		/** toggle the clock */
		void Toggle() { paused = !paused; }
		/** returns the status of the clock */
		bool IsPaused() const { return paused; }
		/** Reset the time to 0 and erase all pending events */
		void Reset(bool remove_events = true);

		/** change the time scale */
		void SetTimeScale(float new_scale) { time_scale = new_scale; }
		/** returns the time scale */
		float GetTimeScale() const { return time_scale; }

		/** gets a clock by name */
		Clock* FindChildClock(char const* in_name, bool recursive = true);
		/** gets a clock by name */
		Clock const* FindChildClock(char const* in_name, bool recursive = true) const;

		/** get the top level clock */
		Clock* GetTopLevelParent();
		/** get the top level clock */
		Clock const* GetTopLevelParent() const;

		/** advance the clock (public interface) */
		bool TickClock(float delta_time);

		/** add a clock */
		Clock* CreateChildClock(char const* in_name = nullptr, ClockCreateParams const& params = ClockCreateParams());
		/** remove a clock */
		bool RemoveFromParent();

		/** add an event to be ticked */
		bool AddPendingEvent(ClockEvent* clock_event, ClockEventInfo event_info, bool relative_time);
		/** remove all child clocks */
		void RemoveAllChildClocks();
		/** remove all pending events */
		void RemoveAllPendingEvents();

		/** change the behavior for tick events */
		void EnableTickEvents(bool value) { tick_events = value; }
		/** get the value of ticking event */
		bool GetTickEventsEnabled() const { return tick_events; }

		/** initialization of the manager from configuration file */
		virtual void InitializeFromConfiguration(JSONReadConfiguration config);

		/** the processor may save its configuration into a JSON file */
		virtual bool SerializeIntoJSON(nlohmann::json& json) const override;
		/** the processor may save its configuration from a JSON file */
		virtual bool SerializeFromJSON(nlohmann::json const& json) override;

	protected:

		/** advance the clock */
		bool TickClockImpl(float delta_time, double cumulated_factor, ClockEventTickSet& event_tick_set);
		/** internal methods to trigger all the event */
		void TriggerClockEvent(ClockEventTickRegistration& registered_event);
		/** ensure given clock is a child of the hierarchy tree */
		bool IsDescendantClock(Clock const* child_clock) const;

	protected:

		/** the parent clock */
		Clock* parent_clock = nullptr;
		/** the time of the clock */
		double clock_time = 0.0;
		/** a time scale for the clock */
		float time_scale = 1.0;
		/** whether the clock is paused or not */
		bool   paused = false;
		/** whether events should be ticked */
		bool   tick_events = true;
		/** the name of the clock */
		std::string name;

		/** the events */
		std::vector<shared_ptr<ClockEvent>> pending_events;
		/** the child clocks */
		std::vector<shared_ptr<Clock>> children_clocks;
	};

#endif

}; // namespace chaos