#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/MetaProgramming.h>
#include <chaos/Object.h>
#include <chaos/StringTools.h>

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










	/** a utility class for parameter passing */
	enum class NamedObjectRequestType
	{
		/** accept anything */
		EMPTY = 0,
		/** search by string */
		STRING = 1,
		/** search by tag */
		TAG = 2
	};


    /** a utility class for parameter passing */
    class NamedObjectRequest
    {
    public:

		/** constructor */
		NamedObjectRequest() = default;
		/** constructor */
		NamedObjectRequest(NamedObjectRequest const & src) = default;
        /** constructor */
        NamedObjectRequest(char const* in_name) :
            name(in_name), request_type(NamedObjectRequestType::STRING)
        {            
        }
		/** constructor */
		NamedObjectRequest(std::string const & in_name) :
			name(in_name.c_str()), request_type(NamedObjectRequestType::STRING)
		{			
		}
        /** constructor */
        NamedObjectRequest(TagType in_tag) :
            tag(in_tag), request_type(NamedObjectRequestType::TAG)
        {
        }

		/** test whether the object name/tag match */
		template<typename T>
		bool Match(T const& object) const // use template to use NamedObjectWrapper as well as NamedObject
		{
			if (request_type == NamedObjectRequestType::EMPTY)
				return true;
			else if (request_type == NamedObjectRequestType::STRING)
				return (StringTools::Stricmp(object.GetName(), name) == 0);
			else if (request_type == NamedObjectRequestType::TAG)
				return (object.GetTag() == tag);
			return false; // should never happen
		}

		/** search element in a vector */
		template<typename P>
		auto FindNamedObject(std::vector<P>& elements) const -> decltype(meta::get_raw_pointer(elements[0]))
		{
			// search in the list
			size_t count = elements.size();
			for (size_t i = 0; i < count; ++i)
			{
				auto e = meta::get_raw_pointer(elements[i]);
				if (Match(*e))
					return e;
			}
			return nullptr;
		}
		/** search element in a vector */
		template<typename P>
		auto FindNamedObject(std::vector<P> const& elements) const -> decltype(meta::get_raw_pointer(elements[0]))
		{
			// search in the list
			size_t count = elements.size();
			for (size_t i = 0; i < count; ++i)
			{
				auto e = meta::get_raw_pointer(elements[i]);
				if (Match(*e))
					return e;
			}
			return nullptr;
		}

	public:

        /** the name for the request */
        char const* name = nullptr;
        /** the tag for the request */
        TagType tag = 0;
        /** the kind of request of interrest */
		NamedObjectRequestType request_type = NamedObjectRequestType::EMPTY;
    };

















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

	protected:

		/** the name of the object */
		std::string name;
		/** the tag of the object */
		TagType tag = 0;
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
		shared_ptr<T> object;
	};









	/** NamedObjectFilter : a class to filter objects by their name */
	class NamedObjectFilter : public Object
	{
	public:

		virtual bool CheckName(NamedObject const & object) const { return true; }
	};

	/** NamedObjectFilterList : filter objects with a list */
	class NamedObjectFilterList : public NamedObjectFilter
	{
	public:

		/** override */
		virtual bool CheckName(NamedObject const & object) const override;

	public:

		/** if not empty, refuse objects that are not in this list (or in enable_tags) */
		std::vector<std::string> enable_names;
		/** if not empty, refuse objects that are not in this list (or in enable_names) */
		std::vector<TagType> enable_tags;
		/** refuse any object in this list */
		std::vector<std::string> forbidden_names;
		/** refuse any object in this list */
		std::vector<TagType> forbidden_tags;
	};


	/** function to serialize into JSON */
	bool SaveIntoJSON(nlohmann::json & json_entry, NamedObject const & src);
	/** function to serialize from JSON */
	bool LoadFromJSON(nlohmann::json const & json_entry, NamedObject & dst);

}; // namespace chaos
