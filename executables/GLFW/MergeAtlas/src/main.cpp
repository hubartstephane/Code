#include <chaos/Chaos.h>


void TestMergedAtlas(std::vector<FIBITMAP*> & atlas_bitmaps, chaos::PixelFormat const & pixel_format, boost::filesystem::path const & dst_p, int index)
{
	std::string dir_name = chaos::StringTools::Printf("Test_%d", index);

	boost::filesystem::path atlas_path = dst_p / dir_name;

	// STEP 1 : create atlas from images of distinct formats
	if (boost::filesystem::create_directories(atlas_path))
	{
		chaos::BitmapAtlas::Atlas          atlas;
		chaos::BitmapAtlas::AtlasGenerator generator;
		chaos::BitmapAtlas::AtlasGeneratorParams params;
		chaos::BitmapAtlas::AtlasInput     input;

		chaos::BitmapAtlas::FolderInfoInput * folder_input = input.AddFolder("bitmap_set1", 0);

		for (size_t i = 0; i < atlas_bitmaps.size(); ++i)
		{
			std::string name = chaos::StringTools::Printf("Bitmap_%02d", i);

			folder_input->AddBitmap(atlas_bitmaps[i], false, name.c_str(), (int)i);
		}

		params.merge_params.pixel_format = pixel_format;
		generator.ComputeResult(input, atlas, params);

		atlas.SaveAtlas(atlas_path / "Atlas");
	}
	// STEP 2 : Reload - Resave Atlas
	boost::filesystem::path resave_path = atlas_path / "Resave";

	if (boost::filesystem::create_directories(resave_path))
	{
		chaos::BitmapAtlas::Atlas atlas;

		atlas.LoadAtlas(atlas_path / "Atlas");
		atlas.SaveAtlas(resave_path / "ResavedAtlas");
	}
}

void LoadBitmaps(std::vector<FIBITMAP*> & atlas_bitmaps)
{
	// get the application
	chaos::Application * application = chaos::Application::GetInstance();
	if (application == nullptr)
		return;

	// iterate on the directory
	boost::filesystem::path resources_path = application->GetResourcesPath();

	boost::filesystem::directory_iterator end;
	for (boost::filesystem::directory_iterator it = chaos::FileTools::GetDirectoryIterator(resources_path / "images"); it != end; ++it)
	{
		FIBITMAP * bitmap = chaos::ImageTools::LoadImageFromFile(it->path());
		if (bitmap == nullptr)
			continue;
		atlas_bitmaps.push_back(bitmap);		
	}
}

void ReleaseBitmaps(std::vector<FIBITMAP*> & atlas_bitmaps)
{
	// release the images
	for (FIBITMAP * bitmap : atlas_bitmaps)
		FreeImage_Unload(bitmap);
	atlas_bitmaps.clear();
}

class MyApplication : public chaos::Application
{
protected:

  virtual bool Main() override
  {
    // load the images
    std::vector<FIBITMAP*> atlas_bitmaps;
    LoadBitmaps(atlas_bitmaps);

	std::vector<chaos::PixelFormat> pixel_formats =
	{
		chaos::PixelFormat::Gray,
		chaos::PixelFormat::GrayFloat,
		chaos::PixelFormat::BGR,
		chaos::PixelFormat::BGRA,
		chaos::PixelFormat::RGBFloat,
		chaos::PixelFormat::RGBAFloat
	};

    // the tests
    boost::filesystem::path dst_p;
    if (chaos::FileTools::CreateTemporaryDirectory("TestMergedAtlas", dst_p))
    {
      for (chaos::PixelFormat const & pixel_format : pixel_formats)
      {
        TestMergedAtlas(atlas_bitmaps, pixel_format, dst_p, &pixel_format - &pixel_formats[0]);
      }
      chaos::WinTools::ShowFile(dst_p);
    }

    // release the images
    ReleaseBitmaps(atlas_bitmaps);

    return true;
  }
};

int CHAOS_MAIN(int argc, char ** argv, char ** env)
{
	chaos::RunApplication<MyApplication>(argc, argv, env);
	return 0;
}


