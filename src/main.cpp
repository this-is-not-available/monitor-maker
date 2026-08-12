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

// Remove 0-based frame from provided skin
void RemoveFrameFromSkin(Skin* skin, unsigned int frame) {
    skin->imageLoaded[frame] = false;
    skin->frames.erase(skin->frames.begin() + frame);
}

// Load image from provided filename to the current frame
// Resizes it to fit previous frames' dimensions 
// Does not touch preview images
void LoadImageForCurrentFrame(const char* filename) {
    vtfpp::VTF::CreationOptions options;
    options.outputFormat = vtfpp::ImageFormat::RGBA8888;
    vtfpp::VTF vtf = vtfpp::VTF::create (filename, options);

    Skin *currentSkin = &skins[selectedSkin];

    uint16_t width = vtf.getWidth();
    uint16_t height = vtf.getHeight();

    if (width == 0 || height == 0) {
        currentSkin->imageLoaded[selectedFrame - 1] = false;
        return;
    }

    if (currentSkin->width == 0 && currentSkin->height == 0) {
        currentSkin->width = width;
        currentSkin->height = height;
    }

    if (currentSkin->width != width || currentSkin->height != height) {
        vtf.setSize(currentSkin->width, currentSkin->height, vtfpp::ImageConversion::ResizeFilter::DEFAULT);
    }
    
    currentSkin->frames[selectedFrame - 1] = vtf.getImageDataAsRGBA8888(0,0,0,0);
    currentSkin->imageLoaded[selectedFrame - 1] = true;
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
    // "lab_monitor_test.mdl"
    WriteMonitor(skins, outputPath, modelName, angled, OutputFormat::FolderOutput);
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