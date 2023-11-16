namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	// all classes in this file
#define CHAOS_SOUND_CLASSES \
(PlaySoundDesc) \
(Sound) \
(SoundManager) \
(SoundCallbacks) \
(SoundAutoCallbacks) \
(SoundObject) \
(SoundSource) \
(SoundCategory) \
(SoundSourceLoader) \
(SoundCategoryLoader)

// forward declaration
#define CHAOS_SOUND_FORWARD_DECL(r, data, elem) class elem;

// friendship macro
#define CHAOS_SOUND_FRIEND_DECL(r, data, elem) friend class elem;
#define CHAOS_SOUND_ALL_FRIENDS BOOST_PP_SEQ_FOR_EACH(CHAOS_SOUND_FRIEND_DECL, _, CHAOS_SOUND_CLASSES)

	enum class SoundBlendType;

	BOOST_PP_SEQ_FOR_EACH(CHAOS_SOUND_FORWARD_DECL, _, CHAOS_SOUND_CLASSES);

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	// ==============================================================
	// SoundBlendType
	// ==============================================================

	enum class CHAOS_API SoundBlendType : int
	{
		BLEND_NONE = 0,
		BLEND_IN = 1,
		BLEND_OUT = 2
	};

	// ==============================================================
	// CALLBACKS
	// ==============================================================

	class CHAOS_API SoundCallbacks : public Object
	{
		CHAOS_SOUND_ALL_FRIENDS

	protected:

		/** called whenever an object is finished */
		virtual void OnFinished(SoundObject* object);
		/** called whenever an object is removed from manager */
		virtual void OnRemovedFromManager(SoundObject* object);
	};

	class CHAOS_API SoundAutoCallbacks : public SoundCallbacks
	{
		CHAOS_SOUND_ALL_FRIENDS

	public:

		/** default constructor */
		SoundAutoCallbacks() = default;
		/** assignation constructor */
		template<typename U, typename V>
		SoundAutoCallbacks(U& in_finished, V& in_removed) :
			finished_func(in_finished),
			removed_func(in_removed)
		{
		}

	protected:

		/** called whenever a sound is finished */
		virtual void OnFinished(SoundObject* object) override;
		/** called whenever an object is removed from manager */
		virtual void OnRemovedFromManager(SoundObject* object) override;

	public:

		/** the callbacks function */
		std::function<void(SoundObject*)> finished_func;
		/** the callbacks function */
		std::function<void(SoundObject*)> removed_func;
	};

	// ==============================================================
	// BLEND VOLUME DESC
	// ==============================================================

	class CHAOS_API BlendVolumeDesc
	{
	public:

		/** helper function to create a BLEND_IN effect */
		static BlendVolumeDesc BlendIn(float blend_time);
		/** helper function to create a BLEND_OUTeffect */
		static BlendVolumeDesc BlendOut(float blend_time, bool pause_at_end = false, bool kill_at_end = false, bool kill_when_paused = false);

	public:

		/** the kind of blending */
		SoundBlendType blend_type = SoundBlendType::BLEND_NONE;
		/** the time to blend from [0 to 1] or [1 to 0] => if current blend value is between, the time is renormalized */
		float blend_time = 1.0f;
		/** whether the object should be paused at the end of blend */
		bool  pause_at_end = false;
		/** whether the object should be killed at the end of blend */
		bool  kill_at_end = false;
		/** whether the object should be killed when paused */
		bool  kill_when_paused = false;
		/** some callbacks */
		shared_ptr<SoundCallbacks> callbacks;
	};

	// ==============================================================
	// PLAY SOUND DESC
	// =============================================================

	class CHAOS_API PlaySoundDesc
	{
		CHAOS_SOUND_ALL_FRIENDS

	public:

		/** returns whether the sound is in 3D dimension */
		bool IsSound3D() const;
		/** set the 3D flag */
		void Enable3D(bool enable);

		/** set the position of the sound (this enables the 3D feature) */
		void SetPosition(glm::vec3 const& in_position, bool update_3D_sound = true);
		/** set the velocity of the sound (this enables the 3D feature) */
		void SetVelocity(glm::vec3 const& in_velocity, bool update_3D_sound = true);

	public:

		/** whether the sound is paused at the beginning */
		bool paused = false;
		/** whether the sound is looping */
		bool looping = false;

		/** the initial volume of the object */
		float volume = 1.0f;
		/** the blend in time of the object */
		float blend_in_time = 0.0f;

		/** true whether the sound is in 3D */
		bool is_3D_sound = false;
		/** the position of the sound in 3D */
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		/** the velocity of the sound in 3D */
		glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
		/** the minimal distance for sound in 3D */
		float min_distance = 0.0f;
		/** the maximum distance for sound in 3D */
		float max_distance = 0.0f;
		/** timer for far 3D sound before entering pause */
		float pause_timer_when_too_far = 0.0f;

		/** the name of the sound object to create */
		std::string sound_name;

		/** the names of the categories ... */
		std::vector<std::string> category_names;
		/** ... or a pointer on the category */
		std::vector<SoundCategory*> categories;
	};

	// ==============================================================
	// SOUND OBJECT
	// ==============================================================

	class CHAOS_API SoundObject : public Object, public NamedInterface
	{
		CHAOS_SOUND_ALL_FRIENDS

	public:

		/** getter on the irrklang engine */
		irrklang::ISoundEngine* GetIrrklangEngine();

		/** getter on the manager object */
		SoundManager* GetManager();
		/** getter on the manager object */
		SoundManager const* GetManager() const;

		/** blend the volume */
		bool StartBlend(BlendVolumeDesc const& desc, bool replace_older = false);
		/** start a fade out and pause or kill */
		bool FadeOut(float blend_time, bool kill_at_end = false, bool kill_when_paused = false);
		/** stop the object */
		void Stop();

		/** returns whether the object is attached to a manager */
		bool IsAttachedToManager() const;
		/** change the callbacks associated to this object */
		void SetCallbacks(SoundCallbacks* in_callbacks);
		/** get whether the sound is finished */
		bool IsFinished() const { return is_finished; }

		/** pause the object */
		void Pause(bool in_pause = true);
		/** get the own object pause state */
		bool IsPaused() const;
		/** get the final pause status for the object */
		virtual bool IsEffectivePaused() const;

		/** change the object volume */
		void SetVolume(float in_volume);
		/** get the own object volume */
		float GetVolume() const;
		/** get the final volume for the sound (category and blendings taken into account) */
		virtual float GetEffectiveVolume() const;

		/** returns true whether there is a blendout and waiting stop */
		bool IsPendingKill() const;
		/** returns true whether there is a blending */
		bool HasVolumeBlending() const;

	protected:

		/** internal tick the object */
		virtual void TickObject(float delta_time);
		/** unbind from manager */
		virtual void OnRemovedFromManager();
		/** remove element from manager list and detach it */
		virtual void RemoveFromManager();
		/** get whether the sound is finished */
		virtual bool ComputeFinishedState();

		/** called at terminaison of the object */
		void OnObjectFinished();
		/** update the flag finished and return it */
		bool UpdateFinishedState();
		/** called at blend terminaison */
		void OnBlendFinished();

		/** internal method called when effective volume has been changed */
		virtual void DoUpdateEffectiveVolume(float effective_volume);
		/** internal method called when effective paused state has been changed */
		virtual void DoUpdateEffectivePause(bool effective_pause);

		/** loading from a JSON object */
		virtual bool InitializeFromJSON(nlohmann::json const& json);

	protected:

		/** whether the object is finished */
		bool is_finished = false;
		/** whether the sound is paused */
		bool paused = false;
		/** the volume */
		float volume = 1.0f;
		/** the manager */
		SoundManager* sound_manager = nullptr;
		/** the blending description */
		BlendVolumeDesc blend_desc;
		/** the blend value */
		float           blend_value = 1.0f;
		/** the callbacks */
		shared_ptr<SoundCallbacks> callbacks;
	};

	// ==============================================================
	// SOURCE
	// ==============================================================

	class CHAOS_API SoundSource : public SoundObject, public FileResource
	{
		CHAOS_SOUND_ALL_FRIENDS

	public:

		/** generating and playing a sound */
		Sound* Play(PlaySoundDesc const& play_desc, SoundCallbacks* in_callbacks = nullptr);
		/** set the categories */
		bool SetDefaultCategories(std::vector<SoundCategory*> const& categories);

	protected:

		/** override */
		virtual void DoUpdateEffectiveVolume(float effective_volume);
		/** override */
		virtual void DoUpdateEffectivePause(bool effective_pause);

		/** unbind from manager */
		virtual void OnRemovedFromManager() override;
		/** remove element from manager list and detach it */
		virtual void RemoveFromManager() override;
		/** the sound generation method */
		virtual Sound* GenerateSound();

		/** loading from a JSON object */
		virtual bool InitializeFromJSON(nlohmann::json const& json) override;

	protected:

		/** the irrklang source */
		shared_ptr<irrklang::ISoundSource> irrklang_source;
		/** the default category */
		std::vector<SoundCategory*> default_categories;
	};

	// ==============================================================
	// CATEGORY
	// ==============================================================

	class CHAOS_API SoundCategory : public SoundObject
	{
		CHAOS_SOUND_ALL_FRIENDS

	protected:

		/** override */
		virtual void DoUpdateEffectiveVolume(float effective_volume);
		/** override */
		virtual void DoUpdateEffectivePause(bool effective_pause);

		/** unbind from manager */
		virtual void OnRemovedFromManager() override;
		/** remove element from manager list and detach it */
		virtual void RemoveFromManager() override;
	};

	// ==============================================================
	// SOUND
	// ==============================================================

	class CHAOS_API Sound : public SoundObject
	{
		CHAOS_SOUND_ALL_FRIENDS

	public:

		/** returns true whether this sound belongs to given category */
		bool IsOfCategory(SoundCategory const* category) const;

		/** set the position of the sound */
		void SetPosition(glm::vec3 const& in_position);
		/** set the velocity of the sound */
		void SetVelocity(glm::vec3 const& in_velocity);

		/** returns whether the sound is effectively paused */
		virtual bool IsEffectivePaused() const override;
		/** returns the effective volume of the sound */
		virtual float GetEffectiveVolume() const override;

		/** returns whether the sound is in 3D dimension */
		bool IsSound3D() const;
		/** get the position of the sound */
		glm::vec3 GetPosition() const;
		/** get the velocity of the sound */
		glm::vec3 GetVelocity() const;
		/** get whether the sound is looping */
		bool IsLooping() const;

		// XXX : while we want to manager ourself the sound volume depending on distance, remove this
#if 0
		/** the distance after which the sound is no more heared */
		void SetMaxDistance(float distance);
		/** the distance below which the sound is at its max volume */
		void SetMinDistance(float distance);
#endif

		/** change the position of the sound track */
		void SetSoundTrackPosition(int position);

		/** returns the source */
		SoundSource* GetSource() { return source; }
		/** returns the source */
		SoundSource const* GetSource() const { return source; }

	protected:

		/** override */
		virtual void TickObject(float delta_time);
		/** override */
		virtual void DoUpdateEffectiveVolume(float effective_volume);
		/** override */
		virtual void DoUpdateEffectivePause(bool effective_pause);

		/** get volume modifier due to distance for 3D sounds */
		float Get3DVolumeModifier() const;

		/** update irrklang state */
		void DoUpdateIrrklangPause(bool effective_pause);

		/** the sound method (returns true whether it is immediatly finished) */
		virtual bool DoPlaySound(PlaySoundDesc const& play_desc);
		/** unbind from manager */
		virtual void OnRemovedFromManager() override;
		/** remove element from manager list and detach it */
		virtual void RemoveFromManager() override;

		/** get whether the sound is finished */
		virtual bool ComputeFinishedState() override;

	protected:

		/** returns true whether the sound is in 3D */
		bool is_3D_sound = false;
		/** the min distance below which the sound volume is MAX (3D consideration only) */
		float min_distance = 0.0f;
		/** the max distance after which the sound volume is 0.0f (3D consideration only) (different from irrklang consideration) */
		float max_distance = 0.0f;
		/** timer for far 3D sound before entering pause */
		float pause_timer_when_too_far = 0.0f;
		/** value of the timer  */
		float pause_timer_value = 0.0f;

		/** whether the sound is looping */
		bool looping = false;

		/** the position of the sound in 3D */
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		/** the velocity of the sound in 3D */
		glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };

		/** the categories of the sound */
		std::vector<SoundCategory*> categories;
		/** the source that generated this object */
		SoundSource* source = nullptr;

		/** the irrklang sound */
		shared_ptr<irrklang::ISound> irrklang_sound;
	};

	// ==============================================================
	// SoundSourceLoader
	// ==============================================================

	class CHAOS_API SoundSourceLoader : public ResourceManagerLoader<SoundSource, SoundManager>
	{
	public:

		/** constructor */
		SoundSourceLoader(SoundManager* in_sound_manager) :
			ResourceManagerLoader<SoundSource, SoundManager>(in_sound_manager)
		{
			assert(in_sound_manager != nullptr); // source cannot be loaded outside of a manager
		}

		/** load an object from JSON */
		virtual SoundSource* LoadObject(char const* name, nlohmann::json const& json) const;
		/** program loading from path */
		virtual SoundSource* LoadObject(FilePathParam const& path, char const* name = nullptr) const;

	protected:

		/** internal method to load a source */
		SoundSource* GenSourceObject(FilePathParam const& path) const;

	protected:

		/** search whether the path is already in used in the manager */
		virtual bool IsPathAlreadyUsedInManager(FilePathParam const& path) const override;
		/** search whether the name is already in used in the manager */
		virtual bool IsNameAlreadyUsedInManager(ObjectRequest request) const override;
	};

	// ==============================================================
	// SoundCategoryLoader
	// ==============================================================

	class CHAOS_API SoundCategoryLoader : public ResourceManagerLoader<SoundCategory, SoundManager>
	{
	public:

		/** constructor */
		SoundCategoryLoader(SoundManager* in_sound_manager) :
			ResourceManagerLoader<SoundCategory, SoundManager>(in_sound_manager)
		{
			assert(in_sound_manager != nullptr); // source cannot be loaded outside of a manager
		}

		/** load an object from JSON */
		virtual SoundCategory* LoadObject(char const* name, nlohmann::json const& json) const;

	protected:

		/** search whether the name is already in used in the manager */
		virtual bool IsNameAlreadyUsedInManager(ObjectRequest request) const override;
	};

	// ==============================================================
	// MANAGER
	// ==============================================================

	class CHAOS_API SoundManager : public ResourceManager, public ConfigurableInterface
	{
		CHAOS_SOUND_ALL_FRIENDS

	public:

		/** getter on the irrklang engine */
		irrklang::ISoundEngine* GetIrrklangEngine();

		/** public method to tick the manager */
		void Tick(float delta_time);

		/** find a source by its name */
		SoundSource* FindSource(ObjectRequest request);
		/** find a source by its name */
		SoundSource const* FindSource(ObjectRequest request) const;

		/** find a simple source by its path */
		SoundSource* FindSourceByPath(FilePathParam const& path);
		/** find a simple source by its path */
		SoundSource const* FindSourceByPath(FilePathParam const& path) const;

		/** find a category by its name */
		SoundCategory* FindCategory(ObjectRequest request);
		/** find a category by its name */
		SoundCategory const* FindCategory(ObjectRequest request) const;

		/** find a sound by its name */
		Sound* FindSound(ObjectRequest request);
		/** find a sound by its name */
		Sound const* FindSound(ObjectRequest request) const;

		/** add a category inside the manager */
		SoundCategory* AddCategory(char const* name);
		/** load and add a simple source inside the manager */
		SoundSource* AddSource(FilePathParam const& path, char const* name = nullptr);

		/** update the listener position */
		bool SetListenerPosition(glm::vec3 const& position, glm::vec3 const& velocity = { 0.0f, 0.0f, 0.0f });

		/** initialize the manager from a configuration file */
		virtual bool InitializeFromConfiguration() override;

		/** getters on sound */
		size_t GetSoundCount() const;
		Sound* GetSound(size_t index);
		Sound const* GetSound(size_t index) const;

		/** getters on category */
		size_t GetCategoryCount() const;
		SoundCategory* GetCategory(size_t index);
		SoundCategory const* GetCategory(size_t index) const;

		/** getters on sources */
		size_t GetSourceCount() const;
		SoundSource* GetSource(size_t index);
		SoundSource const* GetSource(size_t index) const;

		/** get the current listener position */
		glm::vec3 GetListenerPosition() const;
		/** get the current listener velocity */
		glm::vec3 GetListenerVelocity() const;

	protected:

		/** override */
		virtual bool OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context) override;

		/** internally start the manager */
		virtual bool DoStartManager() override;
		/** internally stop the manager */
		virtual bool DoStopManager() override;

		/** remove a category from the list */
		void RemoveCategory(SoundCategory* category);
		/** remove a sound from the list */
		void RemoveSound(Sound* sound);
		/** remove a sound source from the list */
		void RemoveSource(SoundSource* source);

		/** remove a category from the list */
		void RemoveCategoryByIndex(size_t index);
		/** remove a sound from the list */
		void RemoveSoundByIndex(size_t index);
		/** remove a sound source from the list */
		void RemoveSourceByIndex(size_t index);

		/** called whenever an object is being removed */
		static void OnObjectRemovedFromManager(SoundObject* object);

		/** destroy all sounds in a category */
		void DestroyAllSoundPerCategory(SoundCategory* category);
		/** destroy all sounds with a given source */
		void DestroyAllSoundPerSource(SoundSource* source);
		/** remove the category of all sources using given argument */
		void UpdateAllSourcesPerCategory(SoundCategory* category);

		/** internal tick list of objects */
		template<typename T, typename REMOVE_FUNC>
		void DoTickObjects(float delta_time, T& vector, REMOVE_FUNC remove_func)
		{
			T objects = vector; // XXX: work on a copy because due to callbacks, the input vector is volatile

			size_t count = objects.size();
			for (size_t i = 0; i < count; ++i)
			{
				auto object = objects[i]; // copy the intrusive_ptr to prevent the destruction
				if (object == nullptr || !object->IsAttachedToManager())
					continue;

				// test whether object was already finished before ticking
				bool finished = object->IsFinished();
				bool paused = object->IsEffectivePaused();

				// call tick if required
				if (!finished && !paused)
				{
					object->TickObject(delta_time);
					finished = object->IsAttachedToManager() && object->UpdateFinishedState(); // XXX : if not attached to manager, do not call OnObjectFinished(...) and there is no need to remove it from manager
				}
				// remove the object if needed
				if (finished)
				{
					object->OnObjectFinished();
					if (object->IsAttachedToManager())
						(this->*remove_func)(object.get());
				}
			}
		}

		/** test whether a category with given name could be inserted in the manager */
		bool CanAddCategory(ObjectRequest request) const;
		/** test whether a source with given name could be inserted in the manager */
		bool CanAddSource(ObjectRequest request) const;
		/** test whether a sound with given name could be inserted in the manager */
		bool CanAddSound(ObjectRequest request) const;

		/** update all sounds pause per category */
		void UpdateAllSoundPausePerCategory(SoundCategory* category);
		/** update all sounds volume per category */
		void UpdateAllSoundVolumePerCategory(SoundCategory* category);
		/** update all sounds pause per source */
		void UpdateAllSoundPausePerSource(SoundSource* source);
		/** update all sounds volume per source */
		void UpdateAllSoundVolumePerSource(SoundSource* source);

		/** add a category from a JSON object */
		SoundCategory* AddJSONCategory(char const* name, nlohmann::json const& json);

		/** load the categories from configuration */
		bool LoadCategoriesFromConfiguration(JSONReadConfiguration config);
		/** load the sources from configuration */
		bool LoadSourcesFromConfiguration(JSONReadConfiguration config);

	protected:

		/** all detected devices */
		shared_ptr<irrklang::ISoundDeviceList> irrklang_devices;
		/** the irrklank engine */
		shared_ptr<irrklang::ISoundEngine> irrklang_engine;

		/** the sources */
		std::vector<shared_ptr<SoundSource>> sources;
		/** the sounds */
		std::vector<shared_ptr<Sound>> sounds;
		/** the categories */
		std::vector<shared_ptr<SoundCategory>> categories;

		/** the listener transform */
		glm::mat4 listener_transform = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
		/** the listener velocity */
		glm::vec3 listener_velocity = { 0.0f, 0.0f, 0.0f };
	};

#endif

}; // namespace chaos