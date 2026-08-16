#include "main.h"

#include "gui.h"
#include "monitor_generate.h"

#include <fspp/fspp.h>
#include <vtfpp/vtfpp.h>
#include <steampp/steampp.h>
#include <FL/fl_ask.H>
#include <FL/Fl_Preferences.H>

#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

// TODO: refactor this file to be smaller
// Most of this is main functionality but main.cpp should be cleaner

// default model name
std::string modelName = "models/props/lab_monitor/model_name.mdl";
std::vector<Skin> skins;
unsigned char selectedSkin = 0;
unsigned int selectedFrame = 1;
// by default, the UI has selected the angled option from the dropdown
bool angled = true;

bool p2Found = true;
fs::path p2Path;

std::optional<fspp::FileSystem> gamefs = std::nullopt;
fspp::FileSystemOptions options;
Fl_Preferences prefs(Fl_Preferences::USER_L, "this_is_not_available", "monitor_maker");

int materialName = 0;

// Add new default skin to the end of the list
void AddNewSkin() {
    Skin newSkin;

    newSkin.animated = false;
    newSkin.fps = 24; // Default fps

    // Skins start out with no frames, so there is no width or height
    newSkin.width = 0; 
    newSkin.height = 0;

    newSkin.materialName = "default_screen_material_";
    newSkin.materialName.append(std::to_string(materialName++));

    AddEmptyFrameToSkin(&newSkin);

    skins.push_back(newSkin);
}

// Remove 0-based skin
void RemoveSkin(unsigned char skin) {
    skins.erase(skins.begin() + skin);
}

// Add empty frame to the end of the current skin
void AddEmptyFrameToSkin(Skin* skin) {
    RGBATexture newFrame;
    skin->frames.push_back(newFrame);
}

// Add empty frame at the specified point to the current skin
void AddEmptyFrameToSkin(Skin* skin, unsigned int frame) {
    RGBATexture newFrame;
    skin->frames.insert(skin->frames.begin() + frame, newFrame);
}

// Remove 0-based frame from provided skin
void RemoveFrameFromSkin(Skin* skin, unsigned int frame) {
    skin->imageLoaded[frame] = false;
    skin->frames.erase(skin->frames.begin() + frame);
}

// Load image from provided filename to the current frame
// Resizes it to fit previous frames' dimensions 
// Does not touch preview images
// Returns amount of added frames
int LoadImageForCurrentFrame(const char* filename) {
    vtfpp::VTF::CreationOptions options;
    options.outputFormat = vtfpp::ImageFormat::RGBA8888;
    vtfpp::VTF vtf;

    // TODO: do not trust file extension
    if (((std::string)filename).ends_with(".vtf")) {
        vtf = vtfpp::VTF(filename);
        // Fix some vtfs not being powers of 2
        vtf.setSize(std::bit_ceil(vtf.getWidth()), std::bit_ceil(vtf.getHeight()), vtfpp::ImageConversion::ResizeFilter::DEFAULT);
    } else {
        vtf = vtfpp::VTF::create (filename, options);
    }

    Skin *currentSkin = &skins[selectedSkin];

    // If replacing the only frame in a skin, always use the maximum resolution
    if (currentSkin->frames.size() == 1) {
        currentSkin->width = 0;
        currentSkin->height = 0;
    }

    uint16_t width = vtf.getWidth();
    uint16_t height = vtf.getHeight();

    if (width == 0 || height == 0) {
        currentSkin->imageLoaded[selectedFrame - 1] = false;
        return 0;
    }

    if (currentSkin->width == 0 && currentSkin->height == 0) {
        currentSkin->width = width;
        currentSkin->height = height;
    }

    if (currentSkin->width != width || currentSkin->height != height) {
        vtf.setSize(currentSkin->width, currentSkin->height, vtfpp::ImageConversion::ResizeFilter::DEFAULT);
    }
    
    for (unsigned int i = 0; i < vtf.getFrameCount(); i++) {
        unsigned int f = selectedFrame - 1 + i;
        if (i > 0) {
            AddEmptyFrameToSkin(currentSkin, f);
            currentSkin->imageLoaded[f + vtf.getFrameCount() - 1] = currentSkin->imageLoaded[f];
        }
        currentSkin->frames[f] = vtf.getImageDataAsRGBA8888(0,i,0,0);
        currentSkin->imageLoaded[f] = true;
    }

    return vtf.getFrameCount();
}

bool LoadPortal2FromPath(fs::path path, fspp::FileSystemOptions options) {
     if (!fs::exists(path)) {
        return false;
    }

    if (!fs::exists(path / "portal2/gameinfo.txt")) {
        return false;
    }

    gamefs = fspp::FileSystem::load((path / "portal2").string(), options);

    if (!gamefs.has_value()) {
        return false;
    }

    p2Path = path;
    p2Found = true;
    return true;
}

bool LoadPortal2FromPrefs() {
    char *savedDir = nullptr;
    prefs.get("P2Dir", savedDir, "");

    fs::path savedPath = savedDir;
    free(savedDir);
    if (LoadPortal2FromPath(savedPath, options)) {
        return true;
    }
    return false;
}

bool LoadPortal2FromUserInput() {
    const char *input = fl_input("Your installation of Portal 2 was not automatically found. Some features will not work unless you enter the path to your Portal 2 installation", DEFAULT_PORTAL2_DIRECTORY_EXAMPLE);

    if (input == NULL) {
       return false;
    }

    fs::path inputPath = input;
    if (LoadPortal2FromPath(inputPath, options)) {
        return true;
    }
    return false;
}

// Tries to find Portal 2, asking user to input path to it if not automatically found
// Loads p2Path, p2Found and gamefs with initialized values if successful
void LoadPortal2() {
    steampp::Steam *steam = new steampp::Steam();

    p2Found = true;
    if (!steam->isAppInstalled(PORTAL2_STEAM_APPID)) {
        p2Found = false;
    }

    fs::path steamP2InstallPath;
    if (p2Found) {
        steamP2InstallPath = steam->getAppInstallDir(PORTAL2_STEAM_APPID);
        if (steamP2InstallPath.empty())
            p2Found = false;
    }
    
    delete steam;

    if (p2Found) {
        gamefs = fspp::FileSystem::load(PORTAL2_STEAM_APPID, "portal2", options);

        if (gamefs.has_value()) {
            p2Path = steamP2InstallPath;
        } else {
            p2Found = false;
        }
    } else {
        // loads variables internally
        if (LoadPortal2FromPrefs()) {
            return;
        }

        // loads variables internally
        if (LoadPortal2FromUserInput()) {
            prefs.set("P2Dir", p2Path.string().c_str());
            return;
        }
    }
}

void ExportModel(fs::path outputPath) {
    // Redirect stdout
    std::streambuf* stdout_buf = std::cout.rdbuf();
    std::stringstream stream;
    std::cout.rdbuf(stream.rdbuf());

    bool success = false;
    try
    {
        success = WriteMonitor(skins, outputPath, modelName, angled, OutputFormat::FolderOutput);
    }
    catch(const std::exception& e)
    {
        // !! will not catch synchronous exceptions !!
        success = false;
        std::cout << "***Crash***\nwhat(): " << e.what() << '\n';
    }

    // Restore stdout
    std::cout.rdbuf(stdout_buf);
    

    if (success) {
        fl_message("Success!");
    } else {
        std::string log_str = stream.str();
        fl_alert("An issue occurred while exporting!\nLog:\n%s", log_str.c_str());
        std::cout << stream.str() << std::endl;
    }
}

// Entry point
int main(int argc, char** argv) {
    Fl_Double_Window* window = init();

    LoadPortal2();
    AddNewSkin();
    UpdateInterface();

    window->show();

#ifdef _WIN32
    // Windows on 150% scale looks huge by default
    float current_scale = Fl::screen_scale(window->screen_num());
    Fl::screen_scale(window->screen_num(), current_scale / 1.333f);
#endif

    return Fl::run();;
}