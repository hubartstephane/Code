#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ReferencedObject.h>
#include <chaos/Buffer.h>
#include <chaos/FilePath.h>

namespace chaos
{
	namespace TiledMap
	{
    class BaseObject : public ReferencedObject
    {
    protected:

      /** utility function to load a layer */
      template<typename T, typename ...PARAMS>
      static bool DoLoadObjectListHelper(tinyxml2::XMLElement const * element, std::vector<boost::intrusive_ptr<T>> & result, char const * element_name, char const * container_name, PARAMS...params)
      {
        if (container_name != nullptr) // is there an intermediate node to contain all objects
        {
          element = element->FirstChildElement(container_name);
          if (element == nullptr)
            return true;
        }

        tinyxml2::XMLElement const * e = element->FirstChildElement(element_name);
        for (; e != nullptr; e = e->NextSiblingElement(element_name))
        {
          T * object = new T(params...);
          if (object == nullptr)
            break;
          if (!object->DoLoad(e))
            delete(object);
          else
            result.push_back(object);
        }
        return true;
      }
    };

		//
		// Property : base class for some properties
		//

		class Property : public BaseObject
		{
			friend class PropertyOwner;

		public:

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

      /** returns whether the property is of type int */
      bool IsIntProperty() const { return GetIntProperty() != nullptr; }
      /** returns whether the property is of type float */
      bool IsFloatProperty() const { return GetFloatProperty() != nullptr; }
      /** returns whether the property is of type bool */
      bool IsBoolProperty() const { return GetBoolProperty() != nullptr; }
      /** returns whether the property is of type string */
      bool IsStringProperty() const { return GetStringProperty() != nullptr; }

			/** returns the name of the property */
			char const * GetName() const { return name.c_str(); }

		protected:

			/** the name of the property */
			std::string name;
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
			virtual int * GetIntProperty() { return CastPropertyTo(&value, boost::mpl::identity<int>()); }
			virtual int const * GetIntProperty() const { return CastPropertyTo(&value, boost::mpl::identity<int>()); }
			/** returns a pointer on the float property */
			virtual float * GetFloatProperty() { return CastPropertyTo(&value, boost::mpl::identity<float>()); }
			virtual float const * GetFloatProperty() const { return CastPropertyTo(&value, boost::mpl::identity<float>()); }
			/** returns a pointer on the int property */
			virtual bool * GetBoolProperty() { return CastPropertyTo(&value, boost::mpl::identity<bool>()); }
			virtual bool const * GetBoolProperty() const { return CastPropertyTo(&value, boost::mpl::identity<bool>()); }
			/** returns a pointer on the int property */
			virtual std::string * GetStringProperty() { return CastPropertyTo(&value, boost::mpl::identity<std::string>()); }
			virtual std::string const * GetStringProperty() const { return CastPropertyTo(&value, boost::mpl::identity<std::string>()); }

      /** returns the value of the property */
      T GetValue() { return value; }

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
			T value;
		};

		//
		// Specialization of properties
		//

		using PropertyInt = PropertyTemplate<int>;
		using PropertyFloat = PropertyTemplate<float>;
		using PropertyBool = PropertyTemplate<bool>;
		using PropertyString = PropertyTemplate<std::string>;

		//
		// PropertyOwner : some objects that have dynamic properties
		//

		class PropertyOwner : public BaseObject
		{
			friend class Manager;

		public:

			/** find property by name */
			Property * FindProperty(char const * name);
			/** find property by name */
			Property * FindProperty(char const * name) const;

		protected:

			/** the method to override */
			virtual bool DoLoad(tinyxml2::XMLElement const * element);

			/** create property */
			PropertyInt * DoInsertProperty(char const * name, int value);
			/** create property */
			PropertyFloat * DoInsertProperty(char const * name, float value);
			/** create property */
			PropertyBool * DoInsertProperty(char const * name, bool value);
			/** create property */
			PropertyString * DoInsertProperty(char const * name, char const * value);

		protected:

			/** the properties of the object */
			std::vector<boost::intrusive_ptr<Property>> properties;
		};

    //
    // GeometricObject
    //

    class GeometricObject : public PropertyOwner
    {
      friend class Manager;
      friend class BaseObject;
      friend class TileSet;
      friend class ObjectLayer;

    public:

      /** cast method into iyts subtype */
      virtual class GeometricObjectPoint * GetObjectPoint() { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectPoint const * GetObjectPoint() const { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectRectangle * GetObjectRectangle() { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectRectangle const * GetObjectRectangle() const { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectEllipse * GetObjectEllipse() { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectEllipse const * GetObjectEllipse() const { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectPolygon * GetObjectPolygon() { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectPolygon const * GetObjectPolygon() const { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectPolyline * GetObjectPolyline() { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectPolyline const * GetObjectPolyline() const { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectText * GetObjectText() { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectText const * GetObjectText() const { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectTile * GetObjectTile() { return nullptr; }
      /** cast method into iyts subtype */
      virtual class GeometricObjectTile const * GetObjectTile() const { return nullptr; }

    protected:

      /** protected constructor */
      GeometricObject() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;
      /** loading method from XML */
      std::vector<glm::vec2> GetPointArray(tinyxml2::XMLElement const * element, char const * attribute_name);

    public:

      /** object information */
      int id = 0;
      /** object information */
      std::string name;
      /** object information */
      std::string type;
      /** object information */
      bool visible = true;
      /** object information */
      glm::vec2 position = glm::vec2(0.0f, 0.0f);
      /** object information */
      float rotation = 0.0f; // clockwise rotation in degree
    };

    //
    // GeometricObjectPoint
    //

    class GeometricObjectPoint : public GeometricObject
    {
      friend class ObjectLayer;

    public:

      virtual GeometricObjectPoint * GetObjectPoint() override { return this; }
      virtual GeometricObjectPoint const * GetObjectPoint() const override { return this; }

    protected:

      /** protected constructor */
      GeometricObjectPoint() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;
    };

    // 
    // GeometricObjectSurface
    //

    class GeometricObjectSurface : public GeometricObject
    {
      friend class ObjectLayer;

    protected:

      /** protected constructor */
      GeometricObjectSurface() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;

    public:

      /** object information */
      glm::vec2 size = glm::vec2(0.0f, 0.0f);
    };

    // 
    // GeometricObjectRectangle
    //

    class GeometricObjectRectangle : public GeometricObjectSurface
    {
      friend class ObjectLayer;

    public:

      virtual GeometricObjectRectangle * GetObjectRectangle() override { return this; }
      virtual GeometricObjectRectangle const * GetObjectRectangle() const override { return this; }

    protected:

      /** protected constructor */
      GeometricObjectRectangle() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;
    };

    // 
    // GeometricObjectEllipse
    //

    class GeometricObjectEllipse : public GeometricObjectSurface
    {
      friend class ObjectLayer;

    public:

      virtual GeometricObjectEllipse * GetObjectEllipse() override { return this; }
      virtual GeometricObjectEllipse const * GetObjectEllipse() const override { return this; }

    protected:

      /** protected constructor */
      GeometricObjectEllipse() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;
    };

    // 
    // GeometricObjectPolygon
    //

    class GeometricObjectPolygon : public GeometricObject
    {
      friend class ObjectLayer;

    public:

      virtual GeometricObjectPolygon * GetObjectPolygon() override { return this; }
      virtual GeometricObjectPolygon const * GetObjectPolygon() const override { return this; }

    protected:

      /** protected constructor */
      GeometricObjectPolygon() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;

    public:

      /** object information */
      std::vector<glm::vec2> points;
    };

    // 
    // GeometricObjectPolyline
    //

    class GeometricObjectPolyline : public GeometricObject
    {
      friend class ObjectLayer;

    public:

      virtual GeometricObjectPolyline * GetObjectPolyline() override { return this; }
      virtual GeometricObjectPolyline const * GetObjectPolyline() const override { return this; }

    protected:

      /** protected constructor */
      GeometricObjectPolyline() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;

    public:

      /** object information */
      std::vector<glm::vec2> points;
    };

    // 
    // GeometricObjectText
    //

    class GeometricObjectText : public GeometricObjectSurface
    {
      friend class ObjectLayer;

    public:

      static int const HALIGN_LEFT = 0;
      static int const HALIGN_CENTER = 1;
      static int const HALIGN_RIGHT = 2;
      static int const HALIGN_JUSTIFY = 3;

      static int const VALIGN_TOP = 0;
      static int const VALIGN_CENTER = 1;
      static int const VALIGN_BOTTOM = 2;

    public:

      virtual GeometricObjectText * GetObjectText() override { return this; }
      virtual GeometricObjectText const * GetObjectText() const override { return this; }

    protected:

      /** protected constructor */
      GeometricObjectText() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;

    public:

      /** object information */
      int halign = HALIGN_LEFT;
      /** object information */
      int valign = VALIGN_TOP;
      /** object information */
      std::string fontfamily;
      /** object information */
      std::string text;
      /** object information */
      int pixelsize = 0;
      /** object information */
      int wrap = 0;
      /** object information */
      glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    };

    // 
    // GeometricObjectTile
    //

    class GeometricObjectTile : public GeometricObjectSurface
    {
    public:

      virtual GeometricObjectTile * GetObjectTile() override { return this; }
      virtual GeometricObjectTile const * GetObjectTile() const override { return this; }

      /** protected constructor */
      GeometricObjectTile() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;

    public:

      /** object information */
      int gid = 0;
      /** object information */
      bool horizontal_flip = false;
      /** object information */
      bool vertical_flip = false;
    };

    //
    // GroundData
    //

    class GroundData : public PropertyOwner
    {
      friend class Manager;
      friend class BaseObject;
      friend class TileSet;

    protected:

      /** protected constructor */
      GroundData() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;

    public:

      /** object information */
      int tile_index = 0;
      /** object information */
      std::string name;
    };

    //
    // TileData
    //

    class TileData : public PropertyOwner
    {
      friend class Manager;
      friend class BaseObject;
      friend class TileSet;

    protected:

      /** protected constructor */
      TileData() = default;
      /** loading method from XML */
      virtual bool DoLoad(tinyxml2::XMLElement const * element) override;

    public:

      /** object information */
      int id = 0;
      /** object information */
      std::string type;
      /** object information */
      float probability = 1.0f;
    };


		//
		// LayerBase
		//

		class LayerBase : public PropertyOwner
		{
			friend class Map;

		protected:

			/** constructor */
			LayerBase(class Map * in_map) : 
				map(in_map) { assert(map != nullptr); }

			/** the loading method */
			virtual bool DoLoad(tinyxml2::XMLElement const * element);

		protected:

			/** the owner of the object */
			class Map * map = nullptr;

    public:

			/** the name of the layer */
			std::string name;
			/** whether the layer is visible */
			bool visible = true;
			/** whether the layer is locked */
			bool locked = false;
			/** the opacity */
			float opacity = 1.0f;
			/** the offset of the layer */
			glm::ivec2 offset = glm::ivec2(0, 0);
		};

		//
		// ImageLayer
		//

		class ImageLayer : public LayerBase
		{
			friend class Map;
      friend class BaseObject;

		protected:

			/** constructor */
			ImageLayer(class Map * in_map) :
				LayerBase(in_map) {  }

			/** the loading method */
			virtual bool DoLoad(tinyxml2::XMLElement const * element) override;

		public:

			/** object information */
			boost::filesystem::path image_path;
			/** object information */
			glm::ivec2 size = glm::ivec2(0, 0);
			/** object information */
			glm::vec4 transparent_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		};

		//
		// ObjectLayer
		//

		class ObjectLayer : public LayerBase
		{
			friend class Map;
      friend class BaseObject;

			static int const DRAW_ORDER_MANUAL  = 0;
			static int const DRAW_ORDER_TOPDOWN = 1;

		protected:

			/** constructor */
			ObjectLayer(class Map * in_map) :
				LayerBase(in_map) {  }

			/** the loading method */
			virtual bool DoLoad(tinyxml2::XMLElement const * element) override;
			/** the loading method */
			bool DoLoadObjects(tinyxml2::XMLElement const * element);
      /** the loading method */
      GeometricObject * DoLoadOneObject(tinyxml2::XMLElement const * element);

		public:

			/** object information */
			glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			/** object information */
      int       draw_order = DRAW_ORDER_MANUAL;

      /** the properties of the object */
      std::vector<boost::intrusive_ptr<GeometricObject>> geometric_objects;
		};

    //
    // TileLayer
    //

		class TileLayer : public LayerBase
		{

			friend class Map;
      friend class BaseObject;
      
		protected:

			/** constructor */
			TileLayer(class Map * in_map) :
				LayerBase(in_map) {  }

			/** the loading method */
			virtual bool DoLoad(tinyxml2::XMLElement const * element) override;
			/** the loading method */
			bool DoLoadTileBuffer(tinyxml2::XMLElement const * element);
			/** loading buffer method */
			void DoLoadTileBufferFromBase64(Buffer<char> const & buffer);

		public:

			/** object information */
			glm::ivec2 size = glm::ivec2(0, 0);
			/** the tiles */
			std::vector<int> tile_indices;
		};

		//
		// ManagerObject : objects control by the manager (Map & TileSet)
		//

		class ManagerObject : public PropertyOwner
		{
			friend class Manager;

		public:

			/** returns true whether the name match the resource name */
			bool IsMatchingName(boost::filesystem::path const & in_path) const;
			/** get the path */
			boost::filesystem::path const & GetPath() const { return path; }

		protected:

			/** the constructor */
			ManagerObject(class Manager * in_manager, boost::filesystem::path in_path);
			/** loading method from XML */
			virtual bool DoLoadDocument(tinyxml2::XMLDocument const * doc);
			/** the method to override */
			virtual bool DoLoad(tinyxml2::XMLElement const * element) override;
			/** the method to override */
			virtual bool DoLoadMembers(tinyxml2::XMLElement const * element) { return true; }
			/** get the name of the expected markup */
			virtual char const * GetXMLMarkupName() const { return nullptr; }

		protected:

			/** the manager */
			Manager * manager = nullptr;

    public:

			/** the filename */
			boost::filesystem::path path;
		};

		//
		// TileSet
		//

		class TileSet : public ManagerObject
		{
			static int const ORIENTATION_ORTHOGONAL = 0;
			static int const ORIENTATION_ISOMETRIC  = 1;

			friend class Manager;
			friend class Map;

		protected:

			/** the constructor */
			TileSet(class Manager * in_manager, boost::filesystem::path in_path);
			/** loading method from XML */
			virtual bool DoLoad(tinyxml2::XMLElement const * element) override;
			/** loading method from XML */
			virtual bool DoLoadMembers(tinyxml2::XMLElement const * element) override;
			/** loading method from XML */
			bool DoLoadTiles(tinyxml2::XMLElement const * element);
      /** loading method from XML */
      bool DoLoadGrounds(tinyxml2::XMLElement const * element);

			/** get the name of the expected markup */
			virtual char const * GetXMLMarkupName() const override { return "tileset"; }

		public:

			/** object information */
			std::string name;
			/** object information */
			int         orientation = ORIENTATION_ORTHOGONAL;
			/** object information */
			glm::ivec2  size = glm::ivec2(32, 32);
			/** object information */
			glm::ivec2  tile_size = glm::ivec2(0, 0);
			/** object information */
			int         columns = 0;
			/** object information */
			int         tile_count = 0;
			/** object information */
			glm::vec4   background_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			/** object information */
			boost::filesystem::path image_path;
			/** object information */
			glm::ivec2        image_size = glm::vec2(0, 0);
			/** object information */
			int               image_margin = 0;
			/** object information */
			int               image_spacing = 0;
			/** object information */
			glm::vec4         transparent_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			/** the data for the tiles */
			std::vector<boost::intrusive_ptr<TileData>> tiles;
      /** the data for the tiles */
      std::vector<boost::intrusive_ptr<GroundData>> grounds;
		};

		//
		// Map : some map class
		//

		class TileSetData
		{
		public:

			/** the first gid for the tileset */
			int first_gid = 1;
			/** the tileset */
			boost::intrusive_ptr<TileSet> tileset;
		};

    class TileInfo
    {
    public:

      /** the final gid of the search tile */
      int gid = 0;
      /** the tileset considered */
      TileSet * tileset = nullptr;
    };

		class Map : public ManagerObject
		{
			friend class Manager;
      friend class TileSet;

			static int const ORIENTATION_ORTHOGONAL = 0;
			static int const ORIENTATION_ISOMETRIC  = 1;
			static int const ORIENTATION_STAGGERED  = 2;
			static int const ORIENTATION_HEXAGONAL  = 3;

			static int const STAGGERED_AXIS_X = 0;
			static int const STAGGERED_AXIS_Y = 1;

			static int const STAGGERED_INDEX_ODD  = 0;
			static int const STAGGERED_INDEX_EVEN = 1;

			static int const RENDER_ORDER_RIGHT_UP   = 0;
			static int const RENDER_ORDER_RIGHT_DOWN = 1;
			static int const RENDER_ORDER_LEFT_UP    = 2;
			static int const RENDER_ORDER_LEFT_DOWN  = 3;

		protected:

			/** the constructor */
			Map(class Manager * in_manager, boost::filesystem::path in_path);
			/** loading method from XML */
			virtual bool DoLoad(tinyxml2::XMLElement const * element) override;
			/** get the name of the expected markup */
			virtual char const * GetXMLMarkupName() const override { return "map"; }

			/** load internal data */
			virtual bool DoLoadMembers(tinyxml2::XMLElement const * element) override;
			/** load the tile sets */
			bool DoLoadTileSet(tinyxml2::XMLElement const * element);
			/** load the tile layers */
			bool DoLoadTileLayers(tinyxml2::XMLElement const * element);
			/** load the image layers */
			bool DoLoadImageLayers(tinyxml2::XMLElement const * element);
			/** load the object groups */
			bool DoLoadObjectGroups(tinyxml2::XMLElement const * element);

      /** find tileset data for a given gid */
      TileInfo FindTileInfo(int gid);
      /** find tileset data for a given gid */
      TileInfo const FindTileInfo(int gid) const;

		public:

			/** object information */
			int orientation = ORIENTATION_ORTHOGONAL;
			/** object information */
			glm::ivec2 size = glm::ivec2(100, 100);
			/** object information */
			glm::ivec2 tile_size = glm::ivec2(32, 32);
			/** object information */
			bool infinite = false;
			/** object information */
			int hex_side_length = 0;
			/** object information */
			int stagger_axis = STAGGERED_AXIS_Y;
			/** object information */
			int stagger_index = STAGGERED_INDEX_ODD;
			/** object information */
			int render_order = RENDER_ORDER_RIGHT_DOWN;
			/** object information */
			glm::vec4 background_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			/** object information */
			std::string version;

			/** the tileset used */
			std::vector<TileSetData> tilesets;
			/** the layers composing the map */
			std::vector<boost::intrusive_ptr<ImageLayer>> image_layers;
			/** the layers composing the map */
			std::vector<boost::intrusive_ptr<TileLayer>> tile_layers;
			/** the layers composing the map */
			std::vector<boost::intrusive_ptr<ObjectLayer>> object_layers;
		};

		//
		// Manager : container for maps and tileset
		//

		class Manager
		{
			friend class Map;
			friend class TileSet;

		public:

			/** load a tiled map set */
			Map * LoadMap(boost::filesystem::path path);
			/** load a tiled map set */
			Map * LoadMap(boost::filesystem::path path, Buffer<char> buffer);
			/** load a tiled map set */
			Map * LoadMap(boost::filesystem::path path, tinyxml2::XMLDocument const * doc);

			/** load a tiled map */
			TileSet * LoadTileSet(boost::filesystem::path path);
			/** load a tiled map */
			TileSet * LoadTileSet(boost::filesystem::path path, Buffer<char> buffer);
			/** load a tiled map */
			TileSet * LoadTileSet(boost::filesystem::path path, tinyxml2::XMLDocument const * doc);

			/** find tiled map */
			Map * FindMap(boost::filesystem::path const & path);
			Map const * FindMap(boost::filesystem::path const & path) const;
			/** find tiled map set */
			TileSet * FindTileSet(boost::filesystem::path const & path);
			TileSet const * FindTileSet(boost::filesystem::path const & path) const;

		protected:

			/** internal method to load a tiled map set (with no search for exisiting items) */
			Map * DoLoadMap(boost::filesystem::path path);
			/** internal method to load a tiled map set (with no search for exisiting items) */
			Map * DoLoadMap(boost::filesystem::path path, Buffer<char> buffer);
			/** internal method to load a tiled map set (with no search for exisiting items) */
			Map * DoLoadMap(boost::filesystem::path path, tinyxml2::XMLDocument const * doc);

			/** internal method to load a tiled map (with no search for exisiting items) */
			TileSet * DoLoadTileSet(boost::filesystem::path path);
			/** internal method to load a tiled map (with no search for exisiting items) */
			TileSet * DoLoadTileSet(boost::filesystem::path path, Buffer<char> buffer);
			/** internal method to load a tiled map (with no search for exisiting items) */
			TileSet * DoLoadTileSet(boost::filesystem::path path, tinyxml2::XMLDocument const * doc);

		protected:

			/** the maps */
			std::vector<boost::intrusive_ptr<Map>> maps;
			/** the assets */
			std::vector<boost::intrusive_ptr<TileSet>> tile_sets;
		};


	}; // namespace TiledMap

}; // namespace chaos
