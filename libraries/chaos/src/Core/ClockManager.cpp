#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	// ============================================================
	// ClockEventInfo functions
	// ============================================================

	double ClockEventInfo::GetMaxEventTime() const
	{
		if (IsForeverEvent())
			return std::numeric_limits<double>::max();
		if (IsRepeatedInfinitly())
			return std::numeric_limits<double>::max();
		return start_time + duration + (duration + repetition_delay) * (double)repetition_count;
	}

	ClockEventTickData ClockEventInfo::GetExecutionInfo(double t1, double t2) const
	{
		ClockEventTickData result;
		result.time_slice.first = t1;
		result.time_slice.second = t2;
		result.execution_range.first = std::numeric_limits<double>::max();
		result.execution_range.second = std::numeric_limits<double>::max();
		result.tick_range.first = std::numeric_limits<double>::max();
		result.tick_range.second = std::numeric_limits<double>::max();

		// event to come later
		if (start_time > t2)
			return result;

		// event with unknown duration
		if (IsForeverEvent())
		{
			result.execution_range.first = start_time;
			result.execution_range.second = std::numeric_limits<double>::max();
			result.tick_range.first = std::max(start_time, t1);
			result.tick_range.second = t2;
		}
		// event without REPETITION
		else if (!IsRepeated())
		{
			result.execution_range.first = start_time;
			result.execution_range.second = start_time + duration;
			result.tick_range.first = std::max(start_time, t1);
			result.tick_range.second = std::min(start_time + duration, t2);
		}
		// event with limited duration (0 or finite) + REPETITION
		else
		{
			// we want to test start_time + K.repetition against 't1'
			//
			// start_time + K.(duration + repetition_delay) >= t1

			double tmp = (t1 - start_time) / (duration + repetition_delay);

			double k1 = std::floor(tmp);
			double s1 = start_time + k1 * (duration + repetition_delay);

			if (s1 <= t2 && s1 + duration >= t1)
			{
				result.execution_range.first = s1;
				result.execution_range.second = s1 + duration;
				result.tick_range.first = std::max(s1, t1);
				result.tick_range.second = std::min(s1 + duration, t2);
			}
			else
			{
				double k2 = std::ceil(tmp);
				double s2 = start_time + k2 * (duration + repetition_delay);
				if (s2 <= t2 && s2 + duration >= t1)
				{
					result.execution_range.first = s2;
					result.execution_range.second = s2 + duration;
					result.tick_range.first = std::max(s2, t1);
					result.tick_range.second = std::min(s2 + duration, t2);
				}
			}
		}
		return result;
	}

	bool ClockEventInfo::IsTooLateFor(double current_time) const
	{
		double max_event_time = GetMaxEventTime();
		if (max_event_time < current_time)
			return true;
		return false;
	}

	// ============================================================
	// Clock functions
	// ============================================================

	Clock::Clock(char const * in_name, ClockCreateParams const & params) :
		time_scale(params.time_scale),
		paused(params.paused)
	{
		if (in_name != nullptr)
			name = in_name;
	}

	Clock::~Clock()
	{
		// after destructor, the children vector will be cleared
		// the problem is that children may survive to their parent death due to reference count
		// so orphan all children to ensure they cannot access parent's anymore
		size_t child_count = children_clocks.size();
		for (size_t i = 0; i < child_count; ++i)
			children_clocks[i]->parent_clock = nullptr;
		// same thing with events
		size_t event_count = pending_events.size();
		for (size_t i = 0; i < event_count; ++i)
			pending_events[i]->clock = nullptr;
	}

	bool Clock::IsDescendantClock(Clock const * child_clock) const
	{
		assert(child_clock != nullptr);

		// consider that a clock is it own child !!
		if (child_clock == this)
			return true;
		// recurse
		size_t count = children_clocks.size();
		for (size_t i = 0; i < count; ++i)
			if (children_clocks[i]->IsDescendantClock(child_clock))
				return true;
		return false;
	}

	Clock const * Clock::FindChildClock(char const * in_name, bool recursive) const
	{
		if (in_name == nullptr)
			return nullptr;
		size_t count = children_clocks.size();
		for (size_t i = 0; i < count; ++i)
		{
			Clock const * clock = children_clocks[i].get();
			if (clock->name == in_name)
				return clock;
			if (recursive)
			{
				clock = clock->FindChildClock(in_name, recursive);
				if (clock != nullptr)
					return clock;
			}
		}
		return nullptr;
	}

	Clock * Clock::FindChildClock(char const * in_name, bool recursive)
	{
		if (in_name == nullptr)
			return nullptr;
		size_t count = children_clocks.size();
		for (size_t i = 0; i < count; ++i)
		{
			Clock * clock = children_clocks[i].get();
			if (clock->name == in_name)
				return clock;
			if (recursive)
			{
				clock = clock->FindChildClock(in_name, recursive);
				if (clock != nullptr)
					return clock;
			}
		}
		return nullptr;
	}

	Clock * Clock::GetTopLevelParent()
	{
		Clock * result = this;
		while (result->parent_clock != nullptr)
			result = result->parent_clock;
		return result;
	}

	Clock const * Clock::GetTopLevelParent() const
	{
		Clock const * result = this;
		while (result->parent_clock != nullptr)
			result = result->parent_clock;
		return result;
	}

	void Clock::TriggerClockEvent(ClockEventTickRegistration & registered_event)
	{
		// degenerated use case :
		//   the processing of a previous event remove another event from execution
		shared_ptr<ClockEvent> clock_event = registered_event.clock_event;
		if (clock_event == nullptr)
			return;

		// ensure the event is still in range


		// compute the tick information & tick
		ClockEventTickData tick_data;
		tick_data.time_slice = registered_event.time_slice;
		tick_data.execution_range = registered_event.execution_range;
		tick_data.tick_range = registered_event.tick_range;

		ClockEventTickResult tick_result = clock_event->Tick(tick_data);
		// handle the result
		ClockEventInfo & event_info = clock_event->GetEventInfo();

		// current execution is completed if :
		//   - explicitely required
		//   - time is over
		bool execution_completed = tick_result.IsExecutionCompleted(); // explicit request
		if (!execution_completed)
			if (tick_data.time_slice.second >= tick_data.execution_range.second) // time is over
				execution_completed = true;

		if (execution_completed) // current execution is beeing ended : can start a repetition ?
		{
			if (!tick_result.CanRepeatExecution() || !event_info.IsRepeated()) // want to stop all executions
			{
				clock_event->RemoveFromClock();
			}
			else
			{
				// still repetition possible ?
				if (!event_info.IsRepeatedInfinitly())
				{
					if (event_info.repetition_count == 0) // no more repetition 
					{
						clock_event->RemoveFromClock();
						return;
					}
					else
						event_info.repetition_count--;
				}
				// do the repetition
				event_info.start_time =
					std::min(tick_data.tick_range.second, tick_data.time_slice.second) +
					event_info.repetition_delay;

				clock_event->tick_count = 0;
				clock_event->execution_count++;
			}
		}
		else
		{
			clock_event->tick_count++; // increment internal counter
		}
	}

	bool Clock::TickClock(float delta_time) // should only be called on TopLevel node (public interface)
	{
		assert(parent_clock == nullptr);

		ClockEventTickSet event_tick_set;

		// updates the clocks and collect the events (sorted)
		bool result = TickClockImpl(delta_time, 1.0, event_tick_set);
		// tick the events
		while (event_tick_set.size() > 0)
		{
			auto it = event_tick_set.begin();

			ClockEventTickRegistration registered_event = *it;
			event_tick_set.erase(it);
			TriggerClockEvent(registered_event);
		}
		return result;
	}

	bool Clock::TickClockImpl(float delta_time, double cumulated_factor, ClockEventTickSet & event_tick_set) // protected interface
	{
		// internal tick
		if (paused || time_scale == 0.0)
			return false;
		// register all events in the list
		double time1 = clock_time;
		double time2 = clock_time + time_scale * delta_time;

		clock_time = time2;

		if (tick_events)
		{
			for (size_t i = 0; i < pending_events.size(); ++i)
			{
				ClockEventInfo const & event_info = pending_events[i]->GetEventInfo();

				if (event_info.IsTooLateFor(time1))
				{
					shared_ptr<ClockEvent> clock_event = pending_events[i]; // XXX : important to keep a reference after RemoveFromClock(...)
					clock_event->RemoveFromClock(); // XXX : we know RemoveFromClock = "RemoveReplace" so --i
					--i;
				}
				else
				{
					ClockEventTickData execution_info = event_info.GetExecutionInfo(time1, time2);
					if (execution_info.IsValid())
					{
						ClockEventTickRegistration registration;
						registration.clock_event = pending_events[i];
						registration.time_slice = execution_info.time_slice;
						registration.execution_range = execution_info.execution_range;
						registration.tick_range = execution_info.tick_range;

						if (registration.tick_range.first <= time1)
							registration.abs_time_to_start = 0.0;
						else
							registration.abs_time_to_start = (registration.tick_range.first - time1) * cumulated_factor;

						event_tick_set.insert(registration);
					}
				}
			}
		}
		// recursive tick 
		size_t child_count = children_clocks.size();
		for (size_t i = 0; i < child_count; ++i)
			children_clocks[i]->TickClockImpl(time_scale * delta_time, cumulated_factor * time_scale, event_tick_set);

		return true;
	}

	Clock * Clock::CreateChildClock(char const * in_name, ClockCreateParams const & params)
	{
		// cannot create a child clock with a name already in use (FindChildClock(...) ignores anonymous clocks)
		Clock * child_clock = FindChildClock(in_name);
		if (child_clock != nullptr)
			return nullptr;

		// allocate the clock
		Clock * result = new Clock(in_name, params);
		if (result != nullptr)
		{
			result->parent_clock = this;
			children_clocks.push_back(result);
		}
		return result;
	}

	bool Clock::RemoveFromParent()
	{
		Clock * tmp = parent_clock;
		if (tmp != nullptr)
		{
			size_t count = tmp->children_clocks.size();
			for (size_t i = 0; i < count; ++i)
			{
				if (tmp->children_clocks[i].get() == this) // remove swap
				{
					if (i != count - 1) // nothing to do if already last element
						std::swap(tmp->children_clocks[i], tmp->children_clocks.back());

					parent_clock = nullptr;          // XXX : we cannot invert these 2 lines because, because 'this' could be deleted and then
					tmp->children_clocks.pop_back(); //       and then we would access 'parent_clock' member after destructor
					return true;                     //       that's why we are using 'tmp'					
				}
			}
		}
		return false;
	}


	bool Clock::AddPendingEvent(ClockEvent * clock_event, ClockEventInfo event_info, bool relative_time)
	{
		assert(clock_event != nullptr);

		// event already registered ?
		if (clock_event->clock != nullptr)
			return false;
		// make the event_info, relative to current_time if necessary
		if (relative_time)
			event_info.start_time += clock_time;
		// is it too late for that event ?
		if (event_info.IsTooLateFor(clock_time))
			return false;
		// do the registration
		clock_event->clock = this;
		clock_event->event_info = event_info;
		clock_event->tick_count = 0;
		clock_event->execution_count = 0;
		pending_events.push_back(clock_event);

		return true;
	}

	void Clock::RemoveAllChildClocks()
	{
		while (children_clocks.size() > 0)
		{
			shared_ptr<Clock> child_clock = children_clocks[children_clocks.size() - 1];
			child_clock->RemoveFromParent();
		}
	}

	void Clock::RemoveAllPendingEvents()
	{
		while (pending_events.size() > 0)
		{
			shared_ptr<ClockEvent> clock_event = pending_events[pending_events.size() - 1];
			clock_event->RemoveFromClock();
		}
	}

	bool ClockEvent::RemoveFromClock()
	{
		Clock * tmp = clock; // keep a trace of parent
		if (tmp != nullptr)
		{
			size_t count = tmp->pending_events.size();
			for (size_t i = 0; i < count; ++i)
			{
				if (tmp->pending_events[i].get() == this)
				{
					AddReference(); // because, we want to pop back the event, then call OnEventRemovedFromClock(...)

					if (i != count - 1)
						std::swap(tmp->pending_events[i], tmp->pending_events.back());
					clock = nullptr;
					tmp->pending_events.pop_back();

					OnEventRemovedFromClock();
					SubReference();
					return true;
				}
			}
		}
		return false;
	}

	
	bool Clock::SerializeIntoJSON(nlohmann::json& json) const
	{
		if (!JSONSerializableInterface::SerializeIntoJSON(json))
			return false;
		JSONTools::SetAttribute(json, "CLOCK_TIME", clock_time);
		return true;
	}
	
	bool Clock::SerializeFromJSON(nlohmann::json const& json) 
	{
		if (!JSONSerializableInterface::SerializeFromJSON(json))
			return false;
		JSONTools::GetAttribute(json, "CLOCK_TIME", clock_time);
		return true;
	}

	void Clock::InitializeFromConfiguration(JSONReadConfiguration config)
	{


	}

	void Clock::SetPause(bool in_pause)
	{
		if (in_pause)
			Pause();
		else
			Resume();

	}

	void Clock::Reset(bool remove_events)
	{
		clock_time = 0.0;
		if (remove_events)
			RemoveAllPendingEvents();	
	}

	// XXX : See ParticleAllocationBase::SubReference(...) implementation and comments
	void Clock::SubReference()
	{
		if (parent_clock == nullptr)
		{			
			Object::SubReference(); // the Clock is handled as usual
		}
		else
		{
			if (--shared_count == 1) // the last reference is the one from the parent clock. Destroy it 
				RemoveFromParent();		
		}				
	}



}; // namespace chaos
