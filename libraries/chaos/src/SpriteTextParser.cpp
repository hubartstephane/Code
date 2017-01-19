#include <chaos/SpriteTextParser.h>
#include <chaos/MathTools.h>

namespace chaos
{
	// ============================================================
	// TextParserData methods
	// ============================================================

	BitmapAtlas::CharacterSet const * TextParserData::GetCharacterSetFromName(char const * character_set_name) const
	{
		BitmapAtlas::CharacterSet const * result = atlas.GetCharacterSet(character_set_name);
		if (result == nullptr)
		{
			// for convenience, if we cannot find the character set, try to use the one on the top of the stack
			if (parse_stack.size() > 0)
				result = parse_stack.back().character_set;
			// if we still have no character set, take the very first available
			if (result == nullptr)
			{
				auto const & character_sets = atlas.GetCharacterSets();
				if (character_sets.size() > 0)
					result = character_sets[0].get();
			}
		}
		return result;
	}

	void TextParserData::PushDuplicate()
	{
		TextParseStackElement element = parse_stack.back();
		parse_stack.push_back(element); // push a duplicate of previous element
	}

	void TextParserData::PushCharacterSet(BitmapAtlas::CharacterSet const * character_set)
	{
		TextParseStackElement element = parse_stack.back();
		if (character_set != nullptr)
			element.character_set = character_set;
		parse_stack.push_back(element); // push a copy of previous element, except the character set
	}

	void TextParserData::PushColor(glm::vec3 const & color)
	{
		TextParseStackElement element = parse_stack.back();
		element.color = color;
		parse_stack.push_back(element); // push a copy of previous element, except the color
	}

	void TextParserData::EmitCharacters(char c, int count, TextParseParams const & params)
	{
		// get current character set
		BitmapAtlas::CharacterSet const * character_set = parse_stack.back().character_set;
		if (character_set == nullptr)
			return;

		// get entry corresponding to the glyph
		BitmapAtlas::CharacterEntry const * entry = character_set->GetEntry(c);
		if (entry == nullptr)
			return;

		// emit the characters
		for (int i = 0; i < count; ++i)
			EmitCharacter(c, entry, character_set, params);
	}


	void TextParserData::EmitCharacter(char c, BitmapAtlas::CharacterEntry const * entry, BitmapAtlas::CharacterSet const * character_set, TextParseParams const & params)
	{
		TextParseToken token;
		token.type = (c != ' ') ? TextParseToken::TOKEN_CHARACTER : TextParseToken::TOKEN_WHITESPACE;
		token.character = c;
		token.character_entry = entry;
		token.character_set = character_set;
		token.color = parse_stack.back().color;
		InsertTokenInLine(token, params);
	}

	void TextParserData::EmitBitmap(BitmapAtlas::BitmapEntry const * entry, TextParseParams const & params)
	{
		TextParseToken token;
		token.type = TextParseToken::TOKEN_BITMAP;
		token.bitmap_entry = entry;
		InsertTokenInLine(token, params);
	}

	void TextParserData::InsertTokenInLine(TextParseToken & token, TextParseParams const & params)
	{
		// if there was no line, insert the very first one ...
		if (parse_result.size() == 0)
			parse_result.push_back(TextParseLine());

		// insert the token
		if (token.bitmap_entry != nullptr)
		{
			// restrict the bitmap to the size of the line
			float factor = MathTools::CastAndDiv<float>(params.line_height - 2 * params.bitmap_padding.y, token.bitmap_entry->height);

			token.position = bitmap_position + params.bitmap_padding;

			token.size.x = factor * (float)token.bitmap_entry->width;
			token.size.y = params.line_height - 2 * params.bitmap_padding.y;

			bitmap_position.x += token.size.x + params.character_spacing + params.bitmap_padding.x * 2.0f;
			character_position.x += token.size.x + params.character_spacing + params.bitmap_padding.x * 2.0f;
		}
		else if (token.character_entry != nullptr)
		{
			// get the descender 
			TextParseStackElement const & context = parse_stack.back();
			float descender = (context.character_set == nullptr) ? 0.0f : context.character_set->descender;

			// scale the character back to the size of the scanline
			float factor = MathTools::CastAndDiv<float>(params.line_height, token.character_set->ascender - token.character_set->descender);

			token.position = character_position - glm::vec2(0.0f, descender) + // character_position.y is BELOW the scanline (at the descender level)
				factor * glm::vec2(
				(float)(token.character_entry->bitmap_left),
				(float)(token.character_entry->bitmap_top - token.character_entry->height) // XXX : -token.character_entry->height => to have BOTTOM LEFT CORNER
				);

			token.size.x = factor * (float)token.character_entry->width;
			token.size.y = factor * (float)token.character_entry->height;
		
			// XXX : Some fonts are in italic. The 'advance' cause some 'overide' in character bounding box.
			//       That's great for characters that are near one another
			//       But with bitmap that causes real overide.
			//       => that's why we are using two position : 'bitmap_position' & 'character_position'
			bitmap_position.x = token.position.x + params.character_spacing + token.size.x;
			character_position.x = token.position.x + params.character_spacing + factor * (float)(token.character_entry->advance.x); 
		}
		parse_result.back().push_back(token);
	}

	void TextParserData::EndCurrentLine(TextParseParams const & params)
	{
		// update position
		float delta_y = params.line_height + params.line_spacing;

		bitmap_position.x = 0.0f;
		bitmap_position.y -= delta_y;
		character_position.x = 0.0f;
		character_position.y -= delta_y;
		// if there was no line, insert the very first one ...
		if (parse_result.size() == 0)
			parse_result.push_back(TextParseLine());
		// ... then you can add a new line
		parse_result.push_back(TextParseLine());
	}

	bool TextParserData::StartMarkup(char const * text, int & i, class TextParser & parser, TextParseParams const & params)
	{
		int j = i;
		while (text[i] != 0)
		{
			char c = text[i];

			if (!StringTools::IsVariableCharacter(c)) // searched string is contained in  [j .. i-1]
			{
				// no character : skip
				if (i - j < 1)
					return false; // ill-formed string								  
												// the markup
				std::string markup = std::string(&text[j], &text[i]);
				// markup correspond to a bitmap, the current character MUST be ']'
				auto bitmap = parser.GetBitmap(markup.c_str());
				if (bitmap != nullptr)
				{
					if (c == ']')
					{
						EmitBitmap(bitmap, params);
						return true;
					}
					return false; // ill-formed string
				}
				// if ']' is found, do nothing because, we are about to push a color/character set on the stack that is to be immediatly popped
				if (c == ']')
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

	BitmapAtlas::BitmapEntry const * TextParser::GetBitmap(char const * name) const
	{
		auto it = bitmaps.find(name);
		if (it == bitmaps.end())
			return nullptr;
		return it->second;
	}
	BitmapAtlas::CharacterSet const * TextParser::GetCharacterSet(char const * name) const
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
		if (!StringTools::IsVariableName(name))
			return false;
		// ensure name is not already used by a color
		if (colors.find(name) != colors.end())
			return false;
		// ensure name is not already used by a bitmap
		if (bitmaps.find(name) != bitmaps.end())
			return false;
		// ensure name is not already used by a character set
		if (character_sets.find(name) != character_sets.end())
			return false;
		return true;
	}

	bool TextParser::AddColor(char const * name, glm::vec3 const & color)
	{
		if (!IsNameValid(name))
			return false;
		colors.insert(std::make_pair(name, color));
		return true;
	}

	bool TextParser::AddCharacterSet(char const * name, char const * font_name)
	{
		assert(name != nullptr);
		assert(font_name != nullptr);

		BitmapAtlas::CharacterSet const * character_set = atlas.GetCharacterSet(font_name);
		if (character_set == nullptr)
			return false;
		return AddCharacterSet(name, character_set);
	}

	bool TextParser::AddCharacterSet(char const * name, BitmapAtlas::CharacterSet const * character_set)
	{
		assert(character_set != nullptr);

		if (!IsNameValid(name))
			return false;
		character_sets.insert(std::make_pair(name, character_set));
		return true;
	}

	bool TextParser::AddBitmap(char const * name, char const * bitmap_set_name, char const * bitmap_name)
	{
		assert(name != nullptr);
		assert(bitmap_set_name != nullptr);
		assert(bitmap_name != nullptr);

		BitmapAtlas::BitmapSet const * bitmap_set = atlas.GetBitmapSet(bitmap_set_name);
		if (bitmap_set == nullptr)
			return false;
		BitmapAtlas::BitmapEntry const * entry = bitmap_set->GetEntry(bitmap_name);
		if (entry == nullptr)
			return false;
		return AddBitmap(name, entry);
	}

	bool TextParser::AddBitmap(char const * name, BitmapAtlas::BitmapEntry const * entry)
	{
		assert(entry != nullptr);

		if (!IsNameValid(name))
			return false;
		bitmaps.insert(std::make_pair(name, entry));
		return true;
	}

	bool TextParser::ParseText(char const * text, SpriteManager * sprite_manager, TextParseResult * parse_result, TextParseParams const & params)
	{
		assert(text != nullptr);

		// initialize parse params stack with a default element that defines current color and fonts
		TextParserData parse_data(atlas);

		TextParseStackElement element;
		element.color = params.default_color;
		element.character_set = parse_data.GetCharacterSetFromName(params.character_set_name.c_str());
		parse_data.parse_stack.push_back(element);

		// all steps to properly generate the result
		if (!GenerateLines(text, params, parse_data))
			return false;
		if (!RemoveUselessWhitespaces(parse_data))
			return false;
		if (!CutLines(params, parse_data))
			return false;
		if (!JustifyLines(params, parse_data))
			return false;
		if (!RemoveWhitespaces(parse_data))
			return false;
		if (!MoveSpritesToHotpoint(params, parse_data))
			return false;

		// output the sprites if wanted
		if (sprite_manager != nullptr)
			if (!GenerateSprites(sprite_manager, params, parse_data))
				return false;

		// output the result if wanted
		if (parse_result != nullptr)
			*parse_result = std::move(parse_data.parse_result);

		return true;
	}

	bool TextParser::GenerateLines(char const * text, TextParseParams const & params, TextParserData & parse_data)
	{
		// iterate over all characters
		bool escape_character = false;
		for (int i = 0; text[i] != 0; ++i)
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
				parse_data.EmitCharacters(' ', (params.tab_size < 1) ? 1 : params.tab_size, params);
			}
			// if escape is set, simply display the incoming character no matter what it is (except \n \r \t)
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
				if (parse_data.parse_stack.size() <= 1) // the very first element is manually inserted. It should never be popped
					return false;
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

		if (parse_data.parse_stack.size() != 1) // all markups should be correctly closed (except the very first we have manually inserted)
			return false;

		return true;
	}

	bool TextParser::GetBoundingBox(TextParseLine const & parse_line, glm::vec2 & min_line_position, glm::vec2 & max_line_position) const
	{
		min_line_position.x = min_line_position.y = std::numeric_limits<float>::max();
		max_line_position.x = max_line_position.y = -std::numeric_limits<float>::max();

		if (parse_line.size() == 0)
			return false;

		for (TextParseToken const & token : parse_line)
		{
			min_line_position = glm::min(min_line_position, token.position);
			max_line_position = glm::max(max_line_position, token.position + token.size);
		}
		return true;
	}

	bool TextParser::GetBoundingBox(TextParseResult const & parse_result, glm::vec2 & min_position, glm::vec2 & max_position) const
	{
		bool result = false;
		if (parse_result.size() > 0)
		{
			min_position.x = min_position.y = std::numeric_limits<float>::max();
			max_position.x = max_position.y = -std::numeric_limits<float>::max();

			for (auto const & line : parse_result)
			{
				glm::vec2 min_line_position;
				glm::vec2 max_line_position;
				if (GetBoundingBox(line, min_line_position, max_line_position))
				{
					min_position = glm::min(min_position, min_line_position);
					max_position = glm::max(max_position, max_line_position);
					result = true;
				}
			}
		}
		return result;
	}

	void TextParser::MoveSprites(TextParseLine & parse_line, glm::vec2 const & offset)
	{
		if (offset.x != 0.0f || offset.y != 0.0f)
			for (TextParseToken & token : parse_line)
				token.position += offset;
	}

	void TextParser::MoveSprites(TextParseResult & parse_result, glm::vec2 const & offset)
	{
		if (offset.x != 0.0f || offset.y != 0.0f)
			for (auto & line : parse_result)
				for (TextParseToken & token : line)
					token.position += offset;
	}

	bool TextParser::MoveSpritesToHotpoint(TextParseParams const & params, TextParserData & parse_data)
	{
		// compute the min/max bounding box
		glm::vec2 min_position;
		glm::vec2 max_position;
		if (!GetBoundingBox(parse_data.parse_result, min_position, max_position)) // no sprite, nothing to do
			return true;

		// displace all the sprites to match the position
		glm::vec2 offset =
			params.position -
			Hotpoint::Convert(min_position, max_position - min_position, Hotpoint::BOTTOM_LEFT, params.hotpoint_type);

		MoveSprites(parse_data.parse_result, offset);

		return true;
	}

	bool TextParser::RemoveWhitespaces(TextParserData & parse_data)
	{
		for (auto & line : parse_data.parse_result)
		{
			auto it = std::remove_if(line.begin(), line.end(), [](TextParseToken const & token) {
				return (token.type == TextParseToken::TOKEN_WHITESPACE);
			});

			line.erase(it, line.end());
		}
		return true;
	}

	bool TextParser::RemoveUselessWhitespaces(TextParserData & parse_data)
	{
		// remove whitespace at the end of lines
		for (auto & line : parse_data.parse_result)
			while (line.size() > 0 && line.back().type == TextParseToken::TOKEN_WHITESPACE)
				line.pop_back();
		// remove all empty lines at the end
		while (parse_data.parse_result.size() > 0 && parse_data.parse_result.back().size() == 0)
			parse_data.parse_result.pop_back();
		return true;
	}

	bool TextParser::CutLines(TextParseParams const & params, TextParserData & parse_data)
	{
		if (params.max_text_width > 0)
		{
			TextParseResult parse_result;

			float y = 0.0f;
			for (auto const & line : parse_data.parse_result)
			{
				// line may have been left empty after useless whitespace removal. 
				// Can simply ignore it, no sprites will be generated for that
				if (line.size() != 0)
					CutOneLine(y, line, parse_result, params, parse_data);
				// update the y position of characters
				y -= params.line_height + params.line_spacing;
			}
			parse_data.parse_result = std::move(parse_result); // replace the line after filtering
		}
		return true;
	}

	void TextParser::FlushLine(float & x, float & y, TextParseLine & current_line, TextParseResult & parse_result, TextParseParams const & params)
	{
		x = 0.0f;
		y += params.line_height;

		parse_result.push_back(std::move(current_line));
		current_line = TextParseLine();
	}

	void TextParser::CutOneLine(float & y, TextParseLine const & line, TextParseResult & parse_result, TextParseParams const & params, TextParserData & parse_data)
	{

	}




















	bool TextParser::JustifyLines(TextParseParams const & params, TextParserData & parse_data)
	{
		if (params.alignment == TextParseParams::ALIGN_LEFT)
			return true;

		glm::vec2 min_position;
		glm::vec2 max_position;
		if (!GetBoundingBox(parse_data.parse_result, min_position, max_position)) // nothing to do for empty sprite
			return true;

		float W1 = max_position.x - min_position.x;
		for (TextParseLine & line : parse_data.parse_result)
		{
			glm::vec2 min_line_position;
			glm::vec2 max_line_position;
			if (GetBoundingBox(line, min_line_position, max_line_position))
			{
				float W2 = max_line_position.x - min_line_position.x;
				if (params.alignment == TextParseParams::ALIGN_RIGHT)
				{
					MoveSprites(line, glm::vec2(W1 - W2, 0.0f));
				}
				else if (params.alignment == TextParseParams::ALIGN_CENTER)
				{
					MoveSprites(line, glm::vec2((W1 - W2) * 0.5f, 0.0f));
				}
				else if (params.alignment == TextParseParams::ALIGN_JUSTIFY)
				{
					if (W1 != W2)
					{
						// count the number of whitespace token
						int   whitespace_count = 0;
						float whitespace_width = 0.0f;
						for (TextParseToken const & token : line)
						{
							// XXX : there may be multiple fonts on the same line, so several whitespace size.
							//       there is no universal reference for a whitespace size
							if (token.type == TextParseToken::TOKEN_CHARACTER)
								whitespace_width = max(whitespace_width, token.size.x); // considere that the widdest character is a reference for whitespace size
							else if (token.type == TextParseToken::TOKEN_WHITESPACE)
								++whitespace_count;
						}
						// no whitespace, we cannot redistribute extra size => next line
						if (whitespace_count == 0 || whitespace_width == 0.0f)
							continue;

						// count how much space must be redistributed for each whitespace
						float extra_whitespace_width = MathTools::CastAndDiv<float>(W1 - W2, whitespace_count);

						// new whitespace cannot be X time greater than initial one
						static float const FACTOR_LIMIT = 2.0f;
						if ((whitespace_width + extra_whitespace_width) > FACTOR_LIMIT * whitespace_width) // => else ignore
							continue;

						// redistribute extra space
						float offset = 0.0f;
						for (TextParseToken & token : line)
						{
							token.position.x += offset;
							if (token.type == TextParseToken::TOKEN_WHITESPACE)
								offset += extra_whitespace_width;
						}
					}
				}
			}
		}
		return true;
	}

	bool TextParser::GenerateSprites(SpriteManager * sprite_manager, TextParseParams const & params, TextParserData & parse_data)
	{
		for (TextParseLine const & line : parse_data.parse_result)
		{
			for (TextParseToken const & token : line)
			{
				if (token.type == TextParseToken::TOKEN_NONE)
					continue;
				if (token.type == TextParseToken::TOKEN_WHITESPACE)
					continue;

				if (token.bitmap_entry != nullptr)
					sprite_manager->AddSpriteBitmap(token.bitmap_entry, token.position, token.size, Hotpoint::BOTTOM_LEFT);
				if (token.character_entry != nullptr)
					sprite_manager->AddSpriteCharacter(token.character_entry, token.position, token.size, Hotpoint::BOTTOM_LEFT, token.color);
			}
		}
		return true;
	}

};