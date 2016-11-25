#include <chaos/StandardHeaders.h> 
#include <chaos/FileTools.h> 
#include <chaos/LogTools.h> 
#include <chaos/GLTools.h> 
#include <chaos/StringTools.h> 
#include <chaos/MyGLFWGamepadManager.h> 
#include <chaos/MyGLFWWindow.h> 
#include <chaos/WinTools.h> 
#include <chaos/GLProgramLoader.h>
#include <chaos/Application.h>
#include <chaos/SimpleMeshGenerator.h>
#include <chaos/SkyBoxTools.h>
#include <chaos/GLDebugOnScreenDisplay.h>
#include <chaos/MyGLFWFpsCamera.h>
#include <chaos/SimpleMesh.h>
#include <chaos/MultiMeshGenerator.h>
#include <chaos/GLProgramData.h>
#include <chaos/GLProgram.h>
#include <chaos/VertexDeclaration.h>
#include <chaos/GLTextureTools.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/MathTools.h>

// --------------------------------------------------------------------

//
// accepted markups: 
//
//   [COLORNAME 
//   <COLOR=MyColor> ... </COLOR> 
//   <FONT=MyFont>   ... </FONT>
//   <BITMAP=MyBitmap>
//
//

/** during parsing, some 'commands' are put on the stack for text formating, TextParseStackElement is such a command */
class TextParseStackElement
{
public:

	/** the color to use */
	glm::vec3 color;
	/** the character set selected */
	chaos::BitmapAtlas::CharacterSet const * character_set{nullptr};
};

class TextParseToken
{
public:

	static int const TOKEN_NONE      = 0;
	static int const TOKEN_BITMAP    = 1;
	static int const TOKEN_CHARACTER = 2;
	static int const TOKEN_SEPARATOR = 3;

	/** the type of the token */
	int type{TOKEN_NONE};
	/** the position of the generated image */
	glm::vec2 position{0.0f, 0.0f};
	/** the corresponding bitmap (if valid) */
	chaos::BitmapAtlas::BitmapEntry * bitmap_entry{nullptr};
	/** the corresponding character (if valid) */
	chaos::BitmapAtlas::CharacterEntry * character_entry{nullptr};
};


/** a structure used to contains data during parsing */
class TextParserData
{
public:

	/** the constructor */
	TextParserData(chaos::BitmapAtlas::Atlas const * in_atlas) : atlas(in_atlas){}

	/** start the markup */
	bool StartMarkup(char const * text, int & i, class TextParser & parser, ParseTextParams const & params);
	/** utility method to emit characters */
	void EmitCharacters(char c, int count, ParseTextParams const & params);
	/** utility method to emit character */
	void EmitCharacter(char c, chaos::BitmapAtlas::CharacterEntry const * entry, chaos::BitmapAtlas::CharacterSet const * character_set, ParseTextParams const & params);
	/** emit a bitmap */
	void EmitBitmap(chaos::BitmapAtlas::BitmapEntry const * entry, ParseTextParams const & params);
	/** end the current line */
	void EndCurrentLine(ParseTextParams const & params);

	/** duplicate the last stack element */
	void PushDuplicate();
	/** add an element on parse stack : keep color, but change current character_set */
	void PushCharacterSet(chaos::BitmapAtlas::CharacterSet const * character_set);
	/** add an element on parse stack : keep character_set, but change current color */
	void PushColor(glm::vec3 const & color);

	/** get a character set from its name */
	chaos::BitmapAtlas::CharacterSet const * GetCharacterSetFromName(char const * character_set_name) const;

public:

	/** current line position */
	glm::vec2 position{0.0f, 0.0f};
	/** the lines */
	std::vector<std::vector<TextParseToken>> lines;
	/** the atlas in used */
	chaos::BitmapAtlas::Atlas const * atlas;
	/** the stack used for parsing */
	std::vector<TextParseStackElement> parse_stack;
};


/** some parameters used during text parsing */
class ParseTextParams
{
public:

	static const int ALIGN_LEFT = 0;
	static const int ALIGN_RIGHT = 1;
	static const int ALIGN_CENTER = 2;
	static const int ALIGN_JUSTIFY = 3;

	/** the size to use for the characters */
	float character_height{ 32.0f };
	/** the text limits */
	float max_text_width{ 0.0f };
	/** word wrap enabled */
	bool word_wrap{ true };
	/** the line alignment */
	int alignment{ ALIGN_LEFT };
	/** the color to use by default */
	glm::vec3 default_color{ 1.0f, 1.0f, 1.0f };
	/** the font to use by default */
	std::string character_set_name;
	/** tab size */
	int tab_size{ 2 };
};


class TextParser
{
public:

	/** default constructor */
	TextParser() = default;
	/** constructor with atlas initialization */
	TextParser(chaos::BitmapAtlas::Atlas const & in_atlas) : atlas(&in_atlas) {}

	/** add a named color in the parser */
	bool AddColor(char const * name, glm::vec3 const & color);
	/** add a named bitmap in the parser */
	bool AddBitmap(char const * name, char const * bitmap_set_name, char const * bitmap_name);
	/** add a named bitmap in the parser */
	bool AddBitmap(char const * name, chaos::BitmapAtlas::BitmapEntry const * entry);

	/** add a named character set in the parser */
	bool AddCharacterSet(char const * name, char const * font_name);
	/** add a named character set in the parser */
	bool AddCharacterSet(char const * name, chaos::BitmapAtlas::CharacterSet const * character_set);

	/** the main method to parse a text */
	bool ParseText(char const * text, ParseTextParams const & params = ParseTextParams());

	/** get a color by its name */
	glm::vec3 const * GetColor(char const * name) const;
	/** get a bitmap by its name */
	chaos::BitmapAtlas::BitmapEntry const * GetBitmap(char const * name) const;
	/** get a character set by its name */
	chaos::BitmapAtlas::CharacterSet const * GetCharacterSet(char const * name) const;
	/** test whether a name is a key in one of the following maps : colors, bitmaps, character_sets */
	bool IsNameValid(char const * name) const;

protected:

	bool GenerateBitmaps(char const * text, ParseTextParams const & params, TextParserData & parse_data);

public:

	/** the colors to use, indexed by a joker name */
	std::map<std::string, glm::vec3> colors;
	/** the bitmaps to use, indexed by a joker name */
	std::map<std::string, chaos::BitmapAtlas::BitmapEntry const *> bitmaps;
	/** the character_set to use, indexed by a joker name */
	std::map<std::string, chaos::BitmapAtlas::CharacterSet const *> character_sets;
	/** the atlas where to find entries */
	chaos::BitmapAtlas::Atlas const * atlas{ nullptr };
};


// ============================================================
// TextParserData methods
// ============================================================

chaos::BitmapAtlas::CharacterSet const * TextParserData::GetCharacterSetFromName(char const * character_set_name) const
{
	chaos::BitmapAtlas::CharacterSet const * result = nullptr;
	if (atlas != nullptr)
	{
		result = atlas->GetCharacterSet(character_set_name);
		if (result == nullptr)
		{
			// for convinience, if we cannot find the character set, try to use the one on the top of the stack
			if (parse_stack.size() > 0)
				result = parse_stack[parse_stack.size() - 1].character_set;
			// if we still have no character set, take the very first available
			if (result == nullptr)
			{
				auto const & character_sets = atlas->GetCharacterSets();
				if (character_sets.size() > 0)
					result = character_sets[0].get();
			}
		}
	}
	return result;
}

void TextParserData::PushDuplicate()
{
	TextParseStackElement element = parse_stack[parse_stack.size() - 1];
	parse_stack.push_back(element);
}

void TextParserData::PushCharacterSet(chaos::BitmapAtlas::CharacterSet const * character_set)
{
	TextParseStackElement element = parse_stack[parse_stack.size() - 1]; 
	if (character_set != nullptr)
		element.character_set = character_set;
	parse_stack.push_back(element); // push a copy of previous element, except the character set
}

void TextParserData::PushColor(glm::vec3 const & color)
{
	TextParseStackElement element = parse_stack[parse_stack.size() - 1];
	element.color = color;
	parse_stack.push_back(element); // push a copy of previous element, except the color
}

void TextParserData::EmitCharacters(char c, int count, ParseTextParams const & params)
{
	// get current character set
	chaos::BitmapAtlas::CharacterSet const * character_set = parse_stack[parse_stack.size() - 1].character_set;
	if (character_set == nullptr)
		return;

	// get entry corresponding to the glyph
	chaos::BitmapAtlas::CharacterEntry const * entry = character_set->GetEntry(c);
	if (entry == nullptr)
		return;

	// if there was no line, insert the very first one ...
	if (lines.size() == 0)
		lines.push_back(std::vector<TextParseToken>()); 

	// emit the characters
	for (int i = 0 ; i < count ; ++i)
		EmitCharacter(c, entry, character_set, params);
}

void TextParserData::EmitCharacter(char c, chaos::BitmapAtlas::CharacterEntry const * entry, chaos::BitmapAtlas::CharacterSet const * character_set, ParseTextParams const & params)
{
	// XXX : we do not draw space because, for justification reason we want to skip space and tabs at end of line
	//       we cannot afford to have pixels drawn in that case. We still have to insert an entry in the line because
	//       we may have another character coming

	TextParseToken token;
	token.type = (c != ' ')? TextParseToken::TOKEN_CHARACTER : TextParseToken::TOKEN_SEPARATOR;

	// get references on last line and last token
	std::vector<TextParseToken> & current_line = lines[lines.size() - 1];

	TextParseToken const * previous_token = (current_line.size() > 0)? &current_line[current_line.size() - 1] : nullptr;


	if (params.max_text_width > 0.0f)
	{

	
	}


	// insert the token in the list
	token.position = position;
	lines[lines.size() - 1].push_back(token);
	// decal current cursor
	position += entry->width * (entry->height / params.character_height);
}

void TextParserData::EmitBitmap(chaos::BitmapAtlas::BitmapEntry const * entry, ParseTextParams const & params)
{
	// if there was no line, insert the very first one ...
	if (lines.size() == 0)
		lines.push_back(std::vector<TextParseToken>()); 


}

void TextParserData::EndCurrentLine(ParseTextParams const & params)
{
	// update position
	position.x  = 0.0f;
	position.y -= params.character_height;
	// if there was no line, insert the very first one ...
	if (lines.size() == 0)
		lines.push_back(std::vector<TextParseToken>()); 
	// ... then you can add a new line
	lines.push_back(std::vector<TextParseToken>()); 
}


bool TextParserData::StartMarkup(char const * text, int & i, class TextParser & parser, ParseTextParams const & params)
{
	int j = i;
	while (text[i] != 0)
	{
		if (!chaos::StringTools::IsVariableCharacter(text[i])) // searched string is contained in  [j .. i-1]
		{
			// no character : skip
			if (i - j <= 1) 
				return false; // ill-formed string
			// the markup
			std::string markup = std::string(&text[j], &text[i]);
			// markup correspond to a bitmap, the current character MUST be ']'
			auto bitmap = parser.GetBitmap(markup.c_str());
			if (bitmap != nullptr)
			{
				if (text[i] == ']')
				{
					EmitBitmap(bitmap, params);
					return true;
				}
				return false; // ill-formed string
			}
			// if ']' is found, do nothing because, we are about to push a color/character set on the stack that is to be immediatly popped
			if (text[i] == ']') 
				return true;
			// color markup found
			auto color = parser.GetColor(markup.c_str());
			if (color != nullptr)
			{
				PushColor(*color);
				return true;
			}
			// character set markup found
			auto character_set = parser.GetCharacterSet(markup.c_str());
			if (character_set != nullptr)
			{
				PushCharacterSet(character_set);
				return true;
			}
			// a markup has been detected but we don'k know to what it corresponds, so push a duplicate on the stack 
			// because we expect a markup closure later
			PushDuplicate(); 
			return true;     
		}
		++i;
	}
	return false; // markup started, but not finished : ill-formed
}

// ============================================================
// TextParser methods
// ============================================================

glm::vec3 const * TextParser::GetColor(char const * name) const
{
	auto it = colors.find(name);
	if (it == colors.end())
		return nullptr;
	return &it->second;
}

chaos::BitmapAtlas::BitmapEntry const * TextParser::GetBitmap(char const * name) const
{
	auto it = bitmaps.find(name);
	if (it == bitmaps.end())
		return nullptr;
	return it->second;
}
chaos::BitmapAtlas::CharacterSet const * TextParser::GetCharacterSet(char const * name) const
{
	auto it = character_sets.find(name);
	if (it == character_sets.end())
		return nullptr;
	return it->second;
}

bool TextParser::IsNameValid(char const * name) const
{
	// ignore empty name
	if (name == nullptr)
		return false;
	// ensure name is a valid variable name
	if (!chaos::StringTools::IsVariableName(name))
		return false;
	// ensure name is not already used by a color
	if (colors.find(name) != colors.end())
		return true;
	// ensure name is not already used by a bitmap
	if (bitmaps.find(name) != bitmaps.end())
		return true;
	// ensure name is not already used by a character set
	if (character_sets.find(name) != character_sets.end())
		return true;
	return false;
}

bool TextParser::AddColor(char const * name, glm::vec3 const & color)
{
	if (IsNameValid(name))
		return false;
	colors.insert(std::make_pair(name, color));
	return true;
}

bool TextParser::AddCharacterSet(char const * name, char const * font_name)
{
	assert(name      != nullptr);
	assert(font_name != nullptr);

	if (atlas == nullptr)
		return false;

	chaos::BitmapAtlas::CharacterSet const * character_set = atlas->GetCharacterSet(font_name);
	if (character_set == nullptr)
		return false;
	return AddCharacterSet(name, character_set);
}

bool TextParser::AddCharacterSet(char const * name, chaos::BitmapAtlas::CharacterSet const * character_set)
{
	assert(character_set != nullptr);

	if (IsNameValid(name))
		return false;
	character_sets.insert(std::make_pair(name, character_set));
	return true;
}

bool TextParser::AddBitmap(char const * name, char const * bitmap_set_name, char const * bitmap_name)
{
	assert(name            != nullptr);
	assert(bitmap_set_name != nullptr);
	assert(bitmap_name     != nullptr);

	if (atlas == nullptr)
		return false;

	chaos::BitmapAtlas::BitmapSet const * bitmap_set = atlas->GetBitmapSet(bitmap_set_name);
	if (bitmap_set == nullptr)
		return false;
	chaos::BitmapAtlas::BitmapEntry const * entry = bitmap_set->GetEntry(bitmap_name);
	if (entry == nullptr)
		return false;
	return AddBitmap(name, entry);
}

bool TextParser::AddBitmap(char const * name, chaos::BitmapAtlas::BitmapEntry const * entry)
{
	assert(entry != nullptr);

	if (IsNameValid(name))
		return false;
	bitmaps.insert(std::make_pair(name, entry));
	return true;
}

bool TextParser::ParseText(char const * text, ParseTextParams const & params)
{
	assert(text != nullptr);

	// initialize parse params
	TextParserData parse_data(atlas);

	TextParseStackElement element;
	element.color = params.default_color;
	element.character_set = parse_data.GetCharacterSetFromName(params.character_set_name.c_str());
	parse_data.parse_stack.push_back(element);

	// first iteration to generate the bitmaps/glyphs
	if (GenerateBitmaps(text, params, parse_data))
	{

		return true;
	}
	return false;
}


bool TextParser::GenerateBitmaps(char const * text, ParseTextParams const & params, TextParserData & parse_data)
{
	// iterate over all characters
	bool escape_character = false;
	for (int i = 0 ; text[i] != 0 ; ++i)
	{
		char c = text[i];

		bool new_escape_character = (c == '\\');

		// ignore chariot return (UNIX/WINDOWS differences) : no different handling if previous character was an escape character
		if (c == '\r') 
		{

		}
		// next line  : no different handling if previous character was an escape character
		else if (c == '\n') 
		{			
			parse_data.EndCurrentLine(params);
		}
		// tabulation : no different handling if previous character was an escape character
		else if (c == '\t') 
		{			
			parse_data.EmitCharacters(' ', (params.tab_size < 1)? 1 : params.tab_size, params);
		}
		// if escape is set, simply display the incomming character no matter what it is (except \n \r \t)
		else if (escape_character) 
		{
			parse_data.EmitCharacters(c, 1, params);	
		}
		// start an escape
		else if (new_escape_character) 
		{

		}
		// close previously started markup 
		else if (c == ']') 
		{
			if (parse_data.parse_stack.size() > 1) // decrease parse stack (keep the very first element, in case of error)
				parse_data.parse_stack.pop_back();
		}
		// start a new markup
		else if (c == '[') 
		{
			if (!parse_data.StartMarkup(text, ++i, *this, params)) // ill-formed markup
				return false;
		}
		// finally, this is not a special character  		
		else
		{
			parse_data.EmitCharacters(c, 1, params); 
		}

		escape_character = !escape_character && new_escape_character;
	}
	return true;
}
















// -----------------------------------------------

class SpriteVertex
{
public:

  glm::vec2 position;
  glm::vec3 texcoord;


};

class SpriteManager
{
public:

  /** theses are the offset are the ones to apply to position, if position correspond to a given border */

  static int const HOTPOINT_LEFT    = 1;
  static int const HOTPOINT_RIGHT   = 2;
  static int const HOTPOINT_HMIDDLE = 0;

  static int const HOTPOINT_BOTTOM  = 4;
  static int const HOTPOINT_TOP     = 8;
  static int const HOTPOINT_VMIDDLE = 0;

  static int const HOTPOINT_BOTTOM_LEFT  = HOTPOINT_BOTTOM  | HOTPOINT_LEFT;
  static int const HOTPOINT_TOP_LEFT     = HOTPOINT_TOP     | HOTPOINT_LEFT;
  static int const HOTPOINT_BOTTOM_RIGHT = HOTPOINT_BOTTOM  | HOTPOINT_RIGHT;
  static int const HOTPOINT_TOP_RIGHT    = HOTPOINT_TOP     | HOTPOINT_RIGHT;
  static int const HOTPOINT_CENTER       = HOTPOINT_VMIDDLE | HOTPOINT_HMIDDLE;

  /** given a hotpoint, a sprite size, and the hotpoint_type, returns the BottomLeft hotpoint position */
  static glm::vec2 GetBottomLeftHotpointPosition(glm::vec2 const & hotpoint, glm::vec2 const & size, int hotpoint_type);
  /** given a hotpoint, a sprite size, and the hotpoint_type, returns the any other hotpoint_type position */
  static glm::vec2 GetHotpointPosition(glm::vec2 const & position, glm::vec2 const & size, int initial_hotpoint_type, int final_hotpoint_type);


  void Initialize();

  void AddSprite(chaos::BitmapAtlas::BitmapEntry * entry, glm::vec2 const & position, glm::vec2 const & size, glm::vec2 const & handler);

protected:

  /** the vertex array */
  boost::intrusive_ptr<chaos::VertexArray> vertex_array;
  /** the vertex buffer */
  boost::intrusive_ptr<chaos::VertexBuffer> vertex_buffer;
  /** the declaration of the vertex buffer */
  chaos::VertexDeclaration declaration;
  /** the texture atlas */
  chaos::BitmapAtlas::TextureArrayAtlas atlas;
  /** the sprites */
  std::vector<SpriteVertex> sprites;
};

void SpriteManager::Initialize()
{

}

glm::vec2 SpriteManager::GetHotpointPosition(glm::vec2 const & hotpoint, glm::vec2 const & size, int initial_hotpoint_type, int final_hotpoint_type)
{
  if (initial_hotpoint_type == final_hotpoint_type)
    return hotpoint;

  static float const offset_factor[] = { -0.5f, 0.0f, -1.0f, 0.0f };

  int initial_h_part = (initial_hotpoint_type >> 0) & 3;
  int initial_v_part = (initial_hotpoint_type >> 2) & 3;

  int final_h_part = (final_hotpoint_type >> 0) & 3;
  int final_v_part = (final_hotpoint_type >> 2) & 3;

  assert(initial_h_part != 3); // not both LEFT and RIGHT, nor TOP and BOTTOM at the same time
  assert(initial_v_part != 3);
  assert(final_h_part   != 3); // not both LEFT and RIGHT, nor TOP and BOTTOM at the same time
  assert(final_v_part   != 3);

  glm::vec2 result = hotpoint;
  result.x += size.x * (offset_factor[initial_h_part] - offset_factor[final_h_part]);
  result.y += size.y * (offset_factor[initial_v_part] - offset_factor[final_v_part]);
  return result;

}

glm::vec2 SpriteManager::GetBottomLeftHotpointPosition(glm::vec2 const & hotpoint, glm::vec2 const & size, int hotpoint_type)
{
  static float const offset_factor[] = { -0.5f, 0.0f, -1.0f, 0.0f };

  int h_part = (hotpoint_type >> 0) & 3;
  int v_part = (hotpoint_type >> 2) & 3;

  assert(h_part != 3); // not both LEFT and RIGHT, nor TOP and BOTTOM at the same time
  assert(v_part != 3);

  glm::vec2 result = hotpoint;
  result.x += size.x * offset_factor[h_part];
  result.y += size.y * offset_factor[v_part];
  return result;
}



void SpriteManager::AddSprite(chaos::BitmapAtlas::BitmapEntry * entry, glm::vec2 const & position, glm::vec2 const & size, glm::vec2 const & handler)
{
  glm::vec2 atlas_size = atlas.GetAtlasDimension();
  glm::vec2 pos_bottom_left = position + size * handler;
  glm::vec2 pos_top_right = pos_bottom_left + size;

  float tex_x1 = chaos::MathTools::CastAndDiv<float>(entry->x, atlas_size.x);
  float tex_y1 = chaos::MathTools::CastAndDiv<float>(entry->y, atlas_size.y);
  float tex_x2 = chaos::MathTools::CastAndDiv<float>(entry->x + entry->width, atlas_size.x);
  float tex_y2 = chaos::MathTools::CastAndDiv<float>(entry->y + entry->height, atlas_size.y);

  float bitmap_index = (float)entry->bitmap_index;

  SpriteVertex bl;
  bl.position = pos_bottom_left;
  bl.texcoord.x = tex_x1;
  bl.texcoord.y = tex_y1;
  bl.texcoord.z = bitmap_index;

  SpriteVertex tr;
  tr.position = pos_top_right;
  tr.texcoord.x = tex_x2;
  tr.texcoord.y = tex_y2;
  tr.texcoord.z = bitmap_index;

  SpriteVertex tl;
  tl.position.x = pos_bottom_left.x;
  tl.position.y = pos_top_right.y;
  tl.texcoord.x = tex_x1;
  tl.texcoord.y = tex_y2;
  tl.texcoord.z = bitmap_index;

  SpriteVertex br;
  br.position.x = pos_top_right.x;
  br.position.y = pos_bottom_left.y;
  br.texcoord.x = tex_x2;
  br.texcoord.y = tex_y1;
  br.texcoord.z = bitmap_index;

  sprites.push_back(bl);
  sprites.push_back(br);
  sprites.push_back(tr);

  sprites.push_back(bl);
  sprites.push_back(tr);
  sprites.push_back(tl);
}



























// --------------------------------------------------------------------

class MyGLFWWindowOpenGLTest1 : public chaos::MyGLFWWindow
{
public:

  MyGLFWWindowOpenGLTest1()
  {








    auto u = SpriteManager::HOTPOINT_BOTTOM_LEFT;


    glm::vec2 p(0.0f, 0.0f);
    glm::vec2 s(10.0f, 10.0f);

    auto p2 = SpriteManager::GetBottomLeftHotpointPosition(p, s, SpriteManager::HOTPOINT_TOP);

    p2 = p2;

    auto p3 = SpriteManager::GetHotpointPosition(p, s, SpriteManager::HOTPOINT_TOP, SpriteManager::HOTPOINT_BOTTOM_LEFT);
    p3 = p3;

    auto p4 = SpriteManager::GetHotpointPosition(p2, s, SpriteManager::HOTPOINT_BOTTOM_LEFT, SpriteManager::HOTPOINT_TOP);
    p4 = p4;
  }

protected:

  virtual bool OnDraw(int width, int height) override
  {
    glm::vec4 clear_color(0.0f, 0.0f, 0.7f, 0.0f);
    glClearBufferfv(GL_COLOR, 0, (GLfloat*)&clear_color);

    float far_plane = 1000.0f;
    glClearBufferfi(GL_DEPTH_STENCIL, 0, far_plane, 0);

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);   // when viewer is inside the cube

                              // XXX : the scaling is used to avoid the near plane clipping
    chaos::box3 b(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    static float FOV = 60.0f;
    glm::mat4 projection = glm::perspectiveFov(FOV * (float)M_PI / 180.0f, (float)width, (float)height, 1.0f, far_plane);
    glm::mat4 world_to_camera = fps_camera.GlobalToLocal();
    glm::mat4 local_to_world = glm::translate(b.position) * glm::scale(b.half_size);

    chaos::GLProgramData const & program_data = program_box->GetProgramData();

    glUseProgram(program_box->GetResourceID());
    program_data.SetUniform("projection", projection);
    program_data.SetUniform("world_to_camera", world_to_camera);
    program_data.SetUniform("local_to_world", local_to_world);



    mesh_box->Render(program_box->GetProgramData(), nullptr, 0, 0);

    debug_display.Display(width, height);

    return true;
  }

  virtual void Finalize() override
  {
    mesh_box = nullptr;
    program_box = nullptr;
    atlas.Clear();

    debug_display.Finalize();
  }

  boost::intrusive_ptr<chaos::GLProgram> LoadProgram(boost::filesystem::path const & resources_path, char const * ps_filename, char const * vs_filename)
  {
    chaos::GLProgramLoader loader;
    loader.AddShaderSourceFile(GL_FRAGMENT_SHADER, resources_path / ps_filename);
    loader.AddShaderSourceFile(GL_VERTEX_SHADER, resources_path / vs_filename);

    return loader.GenerateProgramObject();
  }


  bool LoadTextureArray(boost::filesystem::path const & resources_path)
  {
    return atlas.LoadAtlas(resources_path / "MyAtlas.json");
  }

  virtual bool Initialize() override
  {
    chaos::Application * application = chaos::Application::GetInstance();
    if (application == nullptr)
      return false;

    // compute resource path
    boost::filesystem::path resources_path = application->GetResourcesPath();
    boost::filesystem::path font_path = resources_path / "font.png";

    // initialize debug font display 
    chaos::GLDebugOnScreenDisplay::Params debug_params;
    debug_params.texture_path = font_path;
    debug_params.font_characters = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    debug_params.font_characters_per_line = 10;
    debug_params.font_characters_line_count = 10;
    debug_params.character_width = 20;
    debug_params.spacing = glm::ivec2(0, 0);
    debug_params.crop_texture = glm::ivec2(15, 7);

    if (!debug_display.Initialize(debug_params))
      return false;

    // load texture
    if (!LoadTextureArray(resources_path))
      return false;

    // load programs      
    program_box = LoadProgram(resources_path, "pixel_shader_box.txt", "vertex_shader_box.txt");
    if (program_box == nullptr)
      return false;

    // create meshes
    chaos::box3 b = chaos::box3(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    chaos::MultiMeshGenerator generators;
    generators.AddGenerator(chaos::CubeMeshGenerator(b), mesh_box);

    if (!generators.GenerateMeshes())
      return false;

    // place camera
    fps_camera.fps_controller.position.y = 0.0f;
    fps_camera.fps_controller.position.z = 10.0f;

    // initial display
    debug_display.AddLine("Draw a box with a texture array : \n  Use +/- to change slice.\n  Array composed of images with different size and BPP");

    return true;
  }

  virtual void TweakSingleWindowApplicationHints(chaos::MyGLFWWindowHints & hints, GLFWmonitor * monitor, bool pseudo_fullscreen) const override
  {
    chaos::MyGLFWWindow::TweakSingleWindowApplicationHints(hints, monitor, pseudo_fullscreen);

    hints.toplevel = 0;
    hints.decorated = 1;
  }

  virtual bool Tick(double delta_time) override
  {
    if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      RequireWindowClosure();

    fps_camera.Tick(glfw_window, delta_time);

    debug_display.Tick(delta_time);

    return true; // refresh
  }

  virtual void OnKeyEvent(int key, int scan_code, int action, int modifier) override
  {
    if (key == GLFW_KEY_KP_ADD && action == GLFW_RELEASE)
      ++bitmap_index;
    else if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_RELEASE)
      --bitmap_index;
  }


  virtual void OnMouseButton(int button, int action, int modifier) override
  {
    fps_camera.OnMouseButton(glfw_window, button, action, modifier);
  }

  virtual void OnMouseMove(double x, double y) override
  {
    fps_camera.OnMouseMove(glfw_window, x, y);
  }

protected:

  // rendering for the box  
  boost::intrusive_ptr<chaos::SimpleMesh> mesh_box;
  boost::intrusive_ptr<chaos::GLProgram>  program_box;

  chaos::BitmapAtlas::TextureArrayAtlas atlas;

  int bitmap_index{ 0 };

  chaos::MyGLFWFpsCamera fps_camera;

  chaos::GLDebugOnScreenDisplay debug_display;
};

int _tmain(int argc, char ** argv, char ** env)
{





  TextParser parser;

  char const * t = "abc[TOTO]uv";
  parser.ParseText(t);


  return 0;


  chaos::Application::Initialize<chaos::Application>(argc, argv, env);

  chaos::WinTools::AllocConsoleAndRedirectStdOutput();

  chaos::MyGLFWSingleWindowApplicationParams params;
  params.monitor = nullptr;
  params.width = 1200;
  params.height = 600;
  params.monitor_index = 0;
  chaos::MyGLFWWindow::RunSingleWindowApplication<MyGLFWWindowOpenGLTest1>(params);

  chaos::Application::Finalize();

  return 0;
}


