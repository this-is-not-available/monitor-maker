#ifndef MAIN_H
#define MAIN_H

#include <vector>
#include <span>
#include <filesystem>
#include <FL/Fl_Image.H>
#include <vtfpp/vtfpp.h>
#include <fspp/fspp.h>

#include <format>

// RGBA 32bpp format
#define RGBATexture std::vector<std::byte>

// according to VDC page on skins, StudioMDL can only compile models with 31 skins, even though it claims it can do 32
// we aren't really constrained by the limitations of StudioMDL, but 31 seems like a sane maximum for now
#define MAX_SKINS 31
#define MAX_FRAMES 99 // just so the spinner text doesn't overflow the frame

#define PORTAL2_STEAM_APPID 620

#ifdef _WIN32
    #define DEFAULT_PORTAL2_DIRECTORY_EXAMPLE "C:/Program Files (x86)/Steam/steamapps/common/Portal 2"
#else
    // TODO: Use WSL mount as example?
    #define DEFAULT_PORTAL2_DIRECTORY_EXAMPLE "/mnt/c/Program Files (x86)/Steam/steamapps/common/Portal 2"
#endif

class Skin {
public:
    bool animated = false;
    int fps = 24;
    
    bool scanline = true;
    
    bool scrolling = false;
    bool horizontalScrolling = false;
    float scrollingRate = .2f;

    int width = 0;
    int height = 0;
    std::vector<RGBATexture> frames = {};
    std::string materialName;

    bool imageLoaded[MAX_FRAMES] = {};
    std::vector<Fl_Image*> previewImages = {};
};

extern std::string modelName; // Model name starting with models/ and ending with .mdl
extern std::vector<Skin> skins; // List of skins
extern unsigned char selectedSkin; // This is 0-based
extern unsigned int selectedFrame; // This is 1-based for easier displaying  TODO: store as 0-based and display differently? (would require changing logic everywhere)
extern bool angled; // Is angled option selected for monitor shape?

extern std::optional<fspp::FileSystem> gamefs;
extern std::filesystem::path p2Path;
extern bool p2Found;

void AddNewSkin();
void RemoveSkin(unsigned char skin);

void RemoveFrameFromSkin(Skin* skin, unsigned int frame);
void AddEmptyFrameToSkin(Skin* skin);

void LoadImageForCurrentFrame(const char* filename);
void ExportModel(std::filesystem::path outputPath);

#endif