#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/MetaProgramming.h>

// XXX : automatic TagType generation
//
//  at first, i test for an automatic tag generation with 
//
//  #define CHAOS_DECLARE_TAG(x) static chaos::TagType const x = (chaos::TagType const)&x
//
//  seemed find until i realized that each time header containing this definition was included, there was a copy of the TagType (with a different address)
//  the '#pragma once' was not preventing this issue due to several file object 'building'
//
//   => in library chaos
//   => in library death (using chaos)
//   => in the executable (using both death and chaos)
//
//  so i used a function that was creating a TagType with its variable name (and using a cache to store result)
//
//  The macro deserve to be used in declaration and so is not thread safe (the function it calls is not thread safe)

#define CHAOS_DECLARE_TAG(x) static chaos::TagType x = chaos::MakeStaticTagType(#x)


namespace chaos
{
	/** a tag is an integer that can reference any address */
	using TagType = uintptr_t;

	/** function to generate a TagType from a name (XXX : not thread safe) */
	TagType MakeStaticTagType(char const * name);

	/** a class that describe an object that can be reference by tag and by name */
	class NamedObject
	{
	public:

		/** get the name of the object */
		char const * GetName() const { return name.c_str(); }
		/** get the Tag of the object */
		TagType GetTag() const { return tag; }

		/** change the name of the object */
		void SetName(char const * in_name);
		/** change the tag of the object */
		void SetTag(TagType in_tag) { tag = in_tag; }

	public:

		/** the name of the object */
		std::string name;
		/** the tag of the object */
		TagType tag = 0;

	public:

		/** search element in a vector */
		template<typename P>
		static auto FindNamedObject(std::vector<P> & elements, TagType tag) -> decltype(meta::get_raw_pointer(elements[0]))
		{
			// maybe tag could math the position in the array
			size_t count = elements.size();
			if (tag >= 0 && (size_t)tag < count)
			{
				if (meta::get_raw_pointer(elements[tag])->GetTag() == tag)
					return meta::get_raw_pointer(elements[tag]);
			}
			// search in the list
			for (size_t i = 0; i < count; ++i)
				if (meta::get_raw_pointer(elements[i])->GetTag() == tag)
					return meta::get_raw_pointer(elements[i]);
			return nullptr;
		}
		/** search element in a vector */
		template<typename P>
		static auto FindNamedObject(std::vector<P> const & elements, TagType tag) -> decltype(meta::get_raw_pointer(elements[0]))
		{
			// maybe tag could math the position in the array
			size_t count = elements.size();
			if (tag >= 0 && (size_t)tag < count)
			{
				if (meta::get_raw_pointer(elements[tag])->GetTag() == tag)
					return meta::get_raw_pointer(elements[tag]);
			}
			// search in the list
			for (size_t i = 0; i < count; ++i)
				if (meta::get_raw_pointer(elements[i])->GetTag() == tag)
					return meta::get_raw_pointer(elements[i]);
			return nullptr;
		}
		/** search element in a vector */
		template<typename P>
		static auto FindNamedObject(std::vector<P> & elements, char const * in_name) -> decltype(meta::get_raw_pointer(elements[0]))
		{
			// early exit
			if (in_name == nullptr)
				return nullptr;
			// search in the list
			size_t count = elements.size();
			for (size_t i = 0; i < count; ++i)
				if (strcmp(meta::get_raw_pointer(elements[i])->GetName(), in_name) == 0)
					return meta::get_raw_pointer(elements[i]);
			return nullptr;
		}
		/** search element in a vector */
		template<typename P>
		static auto FindNamedObject(std::vector<P> const & elements, char const * in_name) -> decltype(meta::get_raw_pointer(elements[0]))
		{
			// early exit
			if (in_name == nullptr)
				return nullptr;
			// search in the list
			size_t count = elements.size();
			for (size_t i = 0; i < count; ++i)
				if (strcmp(meta::get_raw_pointer(elements[i])->GetName(), in_name) == 0)
					return meta::get_raw_pointer(elements[i]);
			return nullptr;
		}
	};

	/** NamedObjectWrapper : this is a wrapper to ba able to use NamedObject's static methods */
	template<typename T>
	class NamedObjectWrapper
	{
	public:

		/** special method to have access to NamedObject static utility functions */
		char const * GetName() const { return object->GetName(); }
		/** special method to have access to NamedObject static utility functions */
		TagType GetTag() const { return object->GetTag(); }

	public:

		/** the renderable to render */
		boost::intrusive_ptr<T> object;
	};

	/** function to serialize into JSON */
	void SaveIntoJSON(NamedObject const & info, nlohmann::json & json_entry);
	/** function to serialize from JSON */
	void LoadFromJSON(NamedObject & info, nlohmann::json const & json_entry);

}; // namespace chaos
