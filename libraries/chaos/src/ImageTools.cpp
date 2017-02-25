#include <chaos/ImageTools.h>
#include <chaos/Buffer.h>
#include <chaos/FileTools.h>


namespace chaos
{
	FIBITMAP * ImageTools::GenFreeImage(PixelFormat const & pixel_format, int width, int height)
	{
		assert(width >= 0);
		assert(height >= 0);

		FIBITMAP * result = nullptr;

		// test whether pixel format is valid and supported
		if (!pixel_format.IsValid())
			return result;

		// get freeimage format
		int bpp = 0;
		FREE_IMAGE_TYPE image_type = GetFreeImageType(pixel_format, &bpp);
		if (image_type == FIT_UNKNOWN)
			return nullptr;

		// allocate the freeimage
		return FreeImage_AllocateT(image_type, width, height, bpp);
	}

	FREE_IMAGE_TYPE ImageTools::GetFreeImageType(PixelFormat const & pixel_format, int * bpp)
	{
		if (pixel_format.component_type == PixelFormat::TYPE_UNSIGNED_CHAR)
		{
			if (pixel_format.component_count == 1 || pixel_format.component_count == 3 || pixel_format.component_count == 4)
			{
				if (bpp != nullptr)
					*bpp = pixel_format.component_count * 8;
				return FIT_BITMAP;		
			}
		}
		else if (pixel_format.component_type == PixelFormat::TYPE_FLOAT)
		{
			if (pixel_format.component_count == 1)
				return FIT_FLOAT;
			if (pixel_format.component_count == 3)
				return FIT_RGBF;
			if (pixel_format.component_count == 4)
				return FIT_RGBAF;
		}
		return FIT_UNKNOWN;
	}

	ImageDescription ImageTools::GetImageDescription(FIBITMAP * image)
	{
		assert(image != nullptr);

		// test whether we can handle that format
		PixelFormat pixel_format = PixelFormat::FromImage(image);
		if (pixel_format.IsValid())
		{
			ImageDescription result;
			result.width        = FreeImage_GetWidth(image);
			result.height       = FreeImage_GetHeight(image);
			result.pixel_format = pixel_format;
			result.line_size    = FreeImage_GetLine(image);
			result.pitch_size   = FreeImage_GetPitch(image);
			result.padding_size = result.pitch_size - result.line_size;
			result.data         = FreeImage_GetBits(image);

			// test whether the result is valid (line_size & pitch come from FreeImage ... just in case ... helps ensure ::IsValid() implementation is correct)
			if (result.IsValid())
				return result;				
		}			
		return ImageDescription();
	}

	//
	// To copy pixels and make conversions, we have to
	//
	//   - from src_desc => get a Pixel Type
	//   - from dst_desc => get a Pixel Type
	//
	// we use the boost::mpl::for_each(...) function twice for that.
	// and the end we can call a meta function
	//
	//   CopyPixels<DST_TYPE, SRC_TYPE> or CopyPixelsWithCentralSymetry<DST_TYPE, SRC_TYPE> 

	template<typename SRC_TYPE, bool COPY_WITH_SYMETRY> // forward declaration
	class CopyPixelFuncMap2;

	template<bool COPY_WITH_SYMETRY>
	class CopyPixelFuncMap
	{
	public:

		/// whether the copy must use a central symetry or not
		static bool const copy_with_symetry = COPY_WITH_SYMETRY;

		/// constructor
		CopyPixelFuncMap(ImageDescription const & in_src_desc, ImageDescription & in_dst_desc, int in_src_x, int in_src_y, int in_dst_x, int in_dst_y, int in_width, int in_height) :
			src_desc(in_src_desc), dst_desc(in_dst_desc),
			src_x(in_src_x), src_y(in_src_y),
			dst_x(in_dst_x), dst_y(in_dst_y),
			width(in_width), height(in_height)
		{
			assert(src_desc.IsValid());
			assert(dst_desc.IsValid());

			assert(width >= 0);
			assert(height >= 0);
			assert(src_x >= 0 && src_x + width <= src_desc.width);
			assert(src_y >= 0 && src_y + height <= src_desc.height);
			assert(dst_x >= 0 && dst_x + width <= dst_desc.width);
			assert(dst_y >= 0 && dst_y + height <= dst_desc.height);

			src_format = src_desc.pixel_format.GetFormat();
			dst_format = dst_desc.pixel_format.GetFormat();
		}

		/// the dispatch function
		template<typename DST_TYPE>
		void operator()(DST_TYPE x)
		{
			PixelFormat pf = PixelFormat::GetPixelFormat<DST_TYPE>();
			if (pf.GetFormat() == dst_format)
				boost::mpl::for_each < PixelTypes, boost::mpl::identity < boost::mpl::_1 > >(
					CopyPixelFuncMap2<DST_TYPE, COPY_WITH_SYMETRY>(this));
		}

	public:

		/// copy function
		template<typename DST_TYPE, typename SRC_TYPE>
		void CopyPixels()
		{
			if (boost::is_same<DST_TYPE, SRC_TYPE>::value)
			{
				for (int l = 0; l < height; ++l) // optimized version using memcopy, if there is no conversion to do
				{
					using TYPE = DST_TYPE; // same types fro both src and dst

					TYPE const * s = ImageTools::GetPixelAddress<TYPE>(src_desc, src_x, src_y + l);
					TYPE       * d = ImageTools::GetPixelAddress<TYPE>(dst_desc, dst_x, dst_y + l);
					memcpy(d, s, width * sizeof(TYPE));
				}
			}
			else
			{
				for (int l = 0; l < height; ++l)
				{
					SRC_TYPE const * s = ImageTools::GetPixelAddress<SRC_TYPE>(src_desc, src_x, src_y + l);
					DST_TYPE       * d = ImageTools::GetPixelAddress<DST_TYPE>(dst_desc, dst_x, dst_y + l);
					for (int c = 0; c < width; ++c)
						PixelConverter::Convert(d[c], s[c]);
				}
			}
		}
		/// copy function with symetry		
		template<typename DST_TYPE, typename SRC_TYPE>
		void CopyPixelsWithCentralSymetry()
		{
			for (int l = 0; l < height; ++l)
			{
				SRC_TYPE const * s = ImageTools::GetPixelAddress<SRC_TYPE>(src_desc, src_x, src_y + l);
				DST_TYPE       * d = ImageTools::GetPixelAddress<DST_TYPE>(dst_desc, dst_x, dst_y + height - 1 - l);
				for (int c = 0; c < width; ++c)
					PixelConverter::Convert(d[width - 1 - c], s[c]);
			}
		}

	public:

		/// the well known format for source pixels
		int src_format;
		/// the well known format for destination pixels
		int dst_format;
		/// the parameters for copy
		ImageDescription src_desc;
		ImageDescription dst_desc;
		int src_x;
		int src_y;
		int dst_x;
		int dst_y;
		int width; 
		int height;
	};

	//
	// CopyPixelFuncMap2 : used to find SRC_TYPE and start the copy
	// (DST_TYPE is already well known)
	//

	template<typename DST_TYPE, bool COPY_WITH_SYMETRY> 
	class CopyPixelFuncMap2
	{
	public:

		/// constructor
		CopyPixelFuncMap2(CopyPixelFuncMap<COPY_WITH_SYMETRY> * in_params) : params(in_params) {}

		/// dispatch function
		template<typename SRC_TYPE>
		void operator()(SRC_TYPE x)
		{
			PixelFormat pf = PixelFormat::GetPixelFormat<SRC_TYPE>();
			if (pf.GetFormat() == params->src_format)
			{
				if (params->copy_with_symetry)
					params->CopyPixelsWithCentralSymetry<DST_TYPE, SRC_TYPE>(); 
				else
					params->CopyPixels<DST_TYPE, SRC_TYPE>();
			}
		}

	public:

		CopyPixelFuncMap<COPY_WITH_SYMETRY> * params;
	};

	void ImageTools::CopyPixels(ImageDescription const & src_desc, ImageDescription & dst_desc, int src_x, int src_y, int dst_x, int dst_y, int width, int height)
	{
		CopyPixelFuncMap<false> copy_func_map(src_desc, dst_desc, src_x, src_y, dst_x, dst_y, width, height);

		boost::mpl::for_each<
			PixelTypes, boost::mpl::identity < boost::mpl::_1 > // start by detecting DST_TYPE
		> (copy_func_map);			
	}

	void ImageTools::CopyPixelsWithCentralSymetry(ImageDescription const & src_desc, ImageDescription & dst_desc, int src_x, int src_y, int dst_x, int dst_y, int width, int height)
	{
		CopyPixelFuncMap<true> copy_func_map(src_desc, dst_desc, src_x, src_y, dst_x, dst_y, width, height);

		boost::mpl::for_each<
			PixelTypes, boost::mpl::identity < boost::mpl::_1 > // start by detecting DST_TYPE
		>(copy_func_map);
	}

	bool ImageTools::IsGrayscaleImage(FIBITMAP * image, bool * alpha_needed)
	{
		assert(image != nullptr);

		// pointer on alpha_needed
		bool tmp_alpha_needed = false;
		if (alpha_needed == nullptr)
			alpha_needed = &tmp_alpha_needed;

		*alpha_needed = false;

		// ignore unhandled pixel format
		PixelFormat pixel_format = PixelFormat::FromImage(image);
		if (!pixel_format.IsValid())
			return false;

		// multiple components => not grayscale
		if (pixel_format.component_count != 1)
			return false;

		// a 'luminance' image is a grayscale
		if (pixel_format.component_type == PixelFormat::TYPE_FLOAT)
			return true;

		// 1 component of type UNSIGNED CHAR :
		//   - can be an index on a palette
		//   - can be a 'luminance' value

		RGBQUAD * palette = FreeImage_GetPalette(image); // luminance ?
		if (palette == nullptr)
			return true;

		// test whether the palette is a grayscale palette
		unsigned int color_count = FreeImage_GetColorsUsed(image);
		if (color_count != 256)
			return false;

		for (unsigned int i = 0 ; i < color_count ; ++i)
		{
			if (palette[i].rgbRed != (BYTE)i)
				return false;
			if (palette[i].rgbGreen != (BYTE)i)
				return false;
			if (palette[i].rgbBlue != (BYTE)i)
				return false;

			*alpha_needed |= (palette[i].rgbReserved != 255); // alpha should always be 255 or the palette is interesting for encoding alpha
		}
		return true;
	}

	FIBITMAP * ImageTools::ConvertToSupportedType(FIBITMAP * image, bool can_delete_src)
	{
		if (image == nullptr)
			return nullptr;
		
		// try convert some format
		if (FreeImage_GetImageType(image) == FIT_BITMAP)
		{
			int bpp = FreeImage_GetBPP(image);

			if (bpp == 8)
			{
				bool alpha_needed = false;
				if (!IsGrayscaleImage(image, &alpha_needed)) // don't want a palette any more (this code is good even if the conversion fails)
				{
					FIBITMAP * other = nullptr;					
					if (alpha_needed)
						other = FreeImage_ConvertTo32Bits(image); // keep alpha
					else
						other = FreeImage_ConvertTo24Bits(image);
					if (can_delete_src)
						FreeImage_Unload(image);
					return other;
				}
				return image; // ok					
			}
			else if (bpp == 16) // don't want 16 bpp any more
			{
				FIBITMAP * other = FreeImage_ConvertTo24Bits(image);
				if (can_delete_src)
					FreeImage_Unload(image);
				return other;
			}
		}	

		// test whether pixel format is valid
		PixelFormat pixel_format = PixelFormat::FromImage(image);
		if (!pixel_format.IsValid())
		{
			if (can_delete_src)
				FreeImage_Unload(image);
			return nullptr;		
		}
		return image;
	}

	FIBITMAP * ImageTools::LoadImageFromBuffer(Buffer<char> buffer)
	{
		FIBITMAP * result = nullptr;

		if (buffer != nullptr)
		{
			FIMEMORY * memory = FreeImage_OpenMemory((BYTE*)buffer.data, (DWORD)buffer.bufsize);
			if (memory != nullptr)
			{
				FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(memory, 0);

				result = ConvertToSupportedType(FreeImage_LoadFromMemory(format, memory, 0), true);

				FreeImage_CloseMemory(memory);
			}
		}	
		return result;	
	}

	FIBITMAP * ImageTools::LoadImageFromFile(char const * filename)
	{
		assert(filename != nullptr);
		return LoadImageFromBuffer(FileTools::LoadFile(filename, false));
	}

	FIMULTIBITMAP * ImageTools::LoadMultiImageFromFile(char const * filename)
	{
		assert(filename != nullptr);

		FIMULTIBITMAP * result = nullptr;

		Buffer<char> buffer = FileTools::LoadFile(filename, false);
		if (buffer != nullptr)
		{
			FIMEMORY * memory = FreeImage_OpenMemory((BYTE*)buffer.data, (DWORD)buffer.bufsize);
			if (memory != nullptr)
			{
				FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(memory, 0);

				result = FreeImage_LoadMultiBitmapFromMemory(format, memory, 0);

				FreeImage_CloseMemory(memory);
			}
		}
		return result;
	}

}; // namespace chaos

