#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ReferencedObject.h>
#include <chaos/Buffer.h>

namespace chaos
{
  namespace TiledMap
  {
    //
    // Property : base class for some properties
    //

    class Property : public ReferencedObject
    {
    public:

      /** returns whether the property is of type int */
      bool IsIntProperty() const { return GetIntProperty() != nullptr; }
      /** returns whether the property is of type float */
      bool IsFloatProperty() const { return GetFloatProperty() != nullptr; }
      /** returns whether the property is of type bool */
      bool IsBoolProperty() const { return GetBoolProperty() != nullptr; }
      /** returns whether the property is of type string */
      bool IsStringProperty() const { return GetStringProperty() != nullptr; }

      /** returns a pointer on the int property */
      virtual int * GetIntProperty() { return nullptr; }
      virtual int const * GetIntProperty() const { return nullptr; }
      /** returns a pointer on the float property */
      virtual float * GetFloatProperty() { return nullptr; }
      virtual float const * GetFloatProperty() const { return nullptr; }
      /** returns a pointer on the bool property */
      virtual bool * GetBoolProperty() { return nullptr; }
      virtual bool const * GetBoolProperty() const { return nullptr; }
      /** returns a pointer on the string property */
      virtual std::string * GetStringProperty() { return nullptr; }
      virtual std::string const * GetStringProperty() const { return nullptr; }

      /** returns the name of the property */
      char const * GetPropertyName() const { return property_name.c_str(); }

    protected:

      /** the name of the property */
      std::string property_name;
    };

    //
    // PropertyTemplate : templated specialization for properties
    //

    template<typename T>
    class PropertyTemplate : public Property
    {
    public:

      /** the type of the property */
      typedef T property_type;

      /** returns a pointer on the int property */
      virtual int * GetIntProperty() { return CastPropertyTo(&property_value, boost::mpl::identity<int>()); }
      virtual int const * GetIntProperty() const { return CastPropertyTo(&property_value, boost::mpl::identity<int>()); }
      /** returns a pointer on the float property */
      virtual float * GetFloatProperty() { return CastPropertyTo(&property_value, boost::mpl::identity<float>()); }
      virtual float const * GetFloatProperty() const { return CastPropertyTo(&property_value, boost::mpl::identity<float>()); }
      /** returns a pointer on the int property */
      virtual bool * GetBoolProperty() { return CastPropertyTo(&property_value, boost::mpl::identity<bool>()); }
      virtual bool const * GetBoolProperty() const { return CastPropertyTo(&property_value, boost::mpl::identity<bool>()); }
      /** returns a pointer on the int property */
      virtual std::string * GetStringProperty() { return CastPropertyTo(&property_value, boost::mpl::identity<std::string>()); }
      virtual std::string const * GetStringProperty() const { return CastPropertyTo(&property_value, boost::mpl::identity<std::string>()); }

    protected:

      /** default template to try casting into given class */
      template<typename U, typename V>
      static typename V::type * CastPropertyTo(U * ptr, V) { return nullptr; }
      /** specialisation if input and wanted class match */
      template<typename U>
      static typename U * CastPropertyTo(U * ptr, boost::mpl::identity<U>) { return ptr; }
      /** specialisation if input and wanted class match (const version) */
      template<typename U>
      static typename U const * CastPropertyTo(U const * ptr, boost::mpl::identity<U>) { return ptr; }

    public:

      /** the value of the property */
      T property_value;
    };

    //
    // Specialization of properties
    //

    using PropertyInt = PropertyTemplate<int>;
    using PropertyFloat = PropertyTemplate<float>;
    using PropertyBool = PropertyTemplate<bool>;
    using PropertyString = PropertyTemplate<std::string>;














    //
    // TiledMapObject : some objects that have dynamic properties
    //

    class TiledMapObject : public ReferencedObject
    {
      friend class Manager;

    public:

      /** find property by name */
      Property * FindProperty(char const * property_name);
      /** find property by name */
      Property * FindProperty(char const * property_name) const;

    protected:

      /** the properties of the object */
      std::vector<boost::intrusive_ptr<Property>> properties;




    };





    //
    // TiledMapObjectBase : base object for both Map and both MapSet
    //

    class TiledMapObjectBase : public ReferencedObject
    {
      friend class Manager;

    public:

      /** returns true whether the name match the resource name */
      bool IsMatchingName(char const * in_filename) const;
      /** get the path */
      boost::filesystem::path const & GetPath() const { return filename; }

      /** find property by name */
      Property * FindProperty(char const * property_name);
      /** find property by name */
      Property * FindProperty(char const * property_name) const;

    protected:

      /** the constructor */
      TiledMapObjectBase(class Manager * in_manager, char const * in_filename);
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLDocument const * doc);

    protected:

      /** the manager */
      Manager * manager = nullptr;
      /** the filename */
      boost::filesystem::path filename;


    };





    //
    // Map : some map class
    //

    class Map : public TiledMapObjectBase
    {
      friend class Manager;

    protected:

      /** the constructor */
      Map(class Manager * in_manager, char const * in_filename);
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLDocument const * doc) override;

    protected:

      int  width = 0;
      int  height = 0;
      int  tilewidth = 0;
      int  tileheight = 0;
      bool infinite = false;
    };

    //
    // TileSet
    //

    class TileSet : public TiledMapObjectBase
    {
      friend class Manager;

    protected:

      /** the constructor */
      TileSet(class Manager * in_manager, char const * in_filename);
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLDocument const * doc) override;
    };

    //
    // Manager : container for maps and tileset
    //

    class Manager
    {
    public:

      /** load a tiled map set */
      Map * LoadMap(char const * in_filename);
      /** load a tiled map set */
      Map * LoadMap(char const * name, Buffer<char> buffer);
      /** load a tiled map set */
      Map * LoadMap(char const * name, tinyxml2::XMLDocument const * doc);

      /** load a tiled map */
      TileSet * LoadTileSet(char const * in_filename);
      /** load a tiled map */
      TileSet * LoadTileSet(char const * in_filename, Buffer<char> buffer);
      /** load a tiled map */
      TileSet * LoadTileSet(char const * in_filename, tinyxml2::XMLDocument const * doc);

      /** find tiled map */
      Map * FindMap(char const * in_filename);
      Map const * FindMap(char const * in_filename) const;
      /** find tiled map set */
      TileSet * FindTileSet(char const * in_filename);
      TileSet const * FindTileSet(char const * in_filename) const;

    protected:

      /** internal method to load a tiled map set (with no search for exisiting items) */
      Map * DoLoadMap(char const * in_filename);
      /** internal method to load a tiled map set (with no search for exisiting items) */
      Map * DoLoadMap(char const * name, Buffer<char> buffer);
      /** internal method to load a tiled map set (with no search for exisiting items) */
      Map * DoLoadMap(char const * name, tinyxml2::XMLDocument const * doc);

      /** internal method to load a tiled map (with no search for exisiting items) */
      TileSet * DoLoadTileSet(char const * in_filename);
      /** internal method to load a tiled map (with no search for exisiting items) */
      TileSet * DoLoadTileSet(char const * in_filename, Buffer<char> buffer);
      /** internal method to load a tiled map (with no search for exisiting items) */
      TileSet * DoLoadTileSet(char const * in_filename, tinyxml2::XMLDocument const * doc);

    protected:

      /** the maps */
      std::vector<boost::intrusive_ptr<Map>> maps;
      /** the assets */
      std::vector<boost::intrusive_ptr<TileSet>> tile_sets;
    };


  }; // namespace TiledMap

}; // namespace chaos
