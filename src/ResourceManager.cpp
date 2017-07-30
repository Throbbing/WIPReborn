#include "ResourceManager.h"
#include "FreeImage.h"
#include <iostream>
#include <fstream>

WIPResourceManager::WIPResourceManager()
{

}

WIPResourceManager::~WIPResourceManager()
{

}

WIPResourceManager* WIPResourceManager::get_instance()
{
	static WIPResourceManager* _instance;
	if (!_instance)
		_instance = new WIPResourceManager();
	return _instance;
}

bool WIPResourceManager::startup()
{
	FreeImage_Initialise(TRUE);
	return true;
}

void WIPResourceManager::shutdown()
{
	free_all();
	FreeImage_DeInitialise();
}

//return NULL if fail
ResHandler* WIPResourceManager::load_resource(const char* filename, WIPResourceType type)
{

	file2ptr_map::iterator it = _map.find(filename);
	if (_map.end() != it)
	{
		add_ref(it->second);
		return it->second;
	}
	ResHandler* res = alloc(filename, type);
	if (!res)
	{
		return NULL;
	}
	_map[filename] = res;
	return res;
}

ResHandler* WIPResourceManager::clone(ResHandler* handler)
{
	handler->nref++;
	return handler;
}

void WIPResourceManager::free_all()
{
	ResHandler* handler;
	file2ptr_map::iterator it;
	for (it = _map.begin(); it != _map.end(); ++it)
	{
		handler = it->second;
		free_resource(handler);
		//::free(it->second);
	}
	_map.clear();
	_ref.clear();

}


int WIPResourceManager::free(ResHandler* file, int size)
{
	//remove_ref(file);
	if (file == NULL)
		return SUCC_RELEASE;
	int ret = FAIL;
	unsigned int & ref_num = file->nref;
	if (ref_num > 0)
	{
		ref_num--;
		ret = SUCC_SUB;
	}
	if (ref_num <= 0)
	{

		_map.erase(file->file_name);
		free_resource(file);
		ret = SUCC_RELEASE;
	}
	return ret;
}

void WIPResourceManager::add_ref(ResHandler* file)
{
	file->nref++;
}

void WIPResourceManager::remove_ref(ResHandler* file)
{
	unsigned int& ref_num = file->nref;
	if (ref_num > 0)
		ref_num--;
}

void WIPResourceManager::set_ref(ResHandler* file, int num)
{
	if (num >= 0)
		_ref[file] = num;
}

void WIPResourceManager::delete_handler(ResHandler* handler)
{

	delete handler;
	//useless
	//handler = NULL;
}

void WIPResourceManager::free_resource(ResHandler* handler)
{
	switch (handler->type)
	{
	case TEXT:
	{
		if (TextData::E_STRING == ((TextData *)handler->extra)->conten_type)
		{
			delete (std::string*)handler->ptr;
		}
		//delete[](unsigned char*)handler->ptr;
		delete (TextData *)handler->extra;
		break;
	}
	break;
	case TEXTURE:
		//FreeImage_Unload((FIBITMAP*)handler->ptr_1);
		delete[](unsigned char*)handler->ptr;
		delete (TextureData *)handler->extra;
		break;
	case SOUND:
	{

	}
	case EFont:
	{
		delete[](unsigned char*)handler->ptr;
		delete (TextData*)handler->extra;
		break;
	}
	default:
		printf("open a unkown type resource!\n");
		break;
	}
	delete_handler(handler);
}

//!!!be sure to release memory before return null
ResHandler* WIPResourceManager::alloc(const char* filename, WIPResourceType type)
{
	ResHandler* res = new ResHandler;
	switch (type)
	{
	case TEXT:
	{
		TextData *data = new TextData;
		data->conten_type = TextData::E_STRING;

		std::ifstream file;
		file.open(filename, std::ios::in);
		if (!file.is_open())
		{
			printf("[fatal error]: \"%s\" load error!\n", filename);
			delete data;
			delete_handler(res);
			return NULL;
		}

		std::string* string = new std::string("");
		char sl[1024];
		while (!file.eof())
		{
			file.getline(sl, 1024);
			*string += std::string(sl);
			*string += '\n';
		}
		/*
			file.seekg(0,std::ios::end);
			int len = file.tellg();
			file.seekg(0,std::ios::beg);
			*/
		//const char *mem = string->data();
		/*
		mem[len] = '\0';
		int kl = strlen(mem);
		file.read(mem,len);
		*/
		file.close();

		res->file_name = filename;
		res->type = TEXT;
		res->extra = data;
		res->nref = 1;
		res->ptr = string;
		res->size = string->length();

		_map[filename] = res;

		return res;

	}
	break;
	case TEXTURE:
	{
		TextureData *data = new TextureData;
		

		FIBITMAP* bmpConverted = NULL;
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(filename);
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(filename);
		if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
		{

			FIBITMAP *dib = FreeImage_Load(fif, filename);

			if (!dib)
			{
				printf("[fatal error]: \"%s\" load error!\n", filename);
				delete data;
				delete_handler(res);
				return NULL;
			}

			// we are top left, FIBITMAP is bottom left
			FreeImage_FlipVertical(dib);
			//get infomation
			FREE_IMAGE_TYPE imgType = FreeImage_GetImageType(dib);
			FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(dib);
			unsigned int bpp = FreeImage_GetBPP(dib);

			data->bpp = bpp;
			data->color_type = colorType;
			data->image_type = imgType;
			data->channel = 4;

			int x, y;
			RGBQUAD m_rgb;

			int width = (int)FreeImage_GetWidth(dib);
			int height = (int)FreeImage_GetHeight(dib);

			unsigned char* imgBuf = new unsigned char[width*height * 4];



			bool is_tr = (FreeImage_IsTransparent(dib)!=0);

			bool is_little = (FreeImage_IsLittleEndian()!=0);

			for (y = 0; y < height; y++)
			{
				for (x = 0; x < width; x++)
				{
					FreeImage_GetPixelColor(dib, x, y, &m_rgb);

					if (is_little)
					{
						imgBuf[y*width * 4 + x * 4 + 0] = m_rgb.rgbRed;
						imgBuf[y*width * 4 + x * 4 + 1] = m_rgb.rgbGreen;
						imgBuf[y*width * 4 + x * 4 + 2] = m_rgb.rgbBlue;
						if (is_tr)
							imgBuf[y*width * 4 + x * 4 + 3] = m_rgb.rgbReserved;
						else
							imgBuf[y*width * 4 + x * 4 + 3] = 255;
					}
					else
					{
						//Big Endian Warnning!
						printf("Note:This is a Big endian!\n");
					}
				}
			}


			data->width = width;
			data->height = height;
			data->size = height*width * 4;


			FreeImage_Unload(dib);

			res->file_name = filename;
			res->extra = data;
			res->nref = 1;
			res->ptr = imgBuf;
			res->size = width*height * 4;
			res->type = TEXTURE;

			_map[filename] = res;

			return res;
		}
	}
	break;

	case SOUND:
	{

	}
	break;
	case EFont:
	{
		TextData *data = new TextData;
		data->conten_type = TextData::E_STRING;

		std::ifstream fin(filename, std::ios::binary);
		if (!fin)
		{
			printf("read %s failed!\n", filename);
			return nullptr;
		}
		fin.seekg(0, fin.end);
		int read_size = fin.tellg();
		fin.seekg(0, fin.beg);
		char* mem = new char[read_size];
		fin.read(mem, read_size);
		fin.close();

		res->file_name = filename;
		res->type = EFont;
		res->extra = data;
		res->nref = 1;
		res->ptr = mem;
		res->size = read_size;

		_map[filename] = res;

		return res;
	}
	break;
	default:
		return res;
		break;
	}
	delete_handler(res);
	return NULL;
}

WIPResourceManager* g_res_manager = WIPResourceManager::get_instance();