#include "monitor_generate.h"
#include "vpk_read.h"
#include "mdl.h"
#include "vtx.h"
#include "vvd.h"
#include "phy.h"
#include "vmt_template.h"
#include <vtfpp/vtfpp.h>

#include <vector>

// Please forgive me for my sins writing this code,
// this was the simplest way to implement it without hardcoding every permutation
// also these path operations are super sketchy and prone to errors
// ..atleast I refactored the major sections into their own functions

namespace fs = std::filesystem;

void ReplaceString(std::string* string, std::string replace, std::string with) {
    *string = string->replace(string->find(replace), replace.length(), with);
}

std::vector<std::byte> ReadMonitorFile(std::string_view filename, bool pose03) {
    if (pose03) {
        std::string altered {filename};
        size_t pos = altered.rfind("pose01");
        
        if (pos != std::string::npos) {
            altered.replace(pos, 6, "pose03");
        }
        return ReadFile(std::move(altered));
    }
    return ReadFile(std::string(filename));
}

void WriteSkin(Skin skin, fs::path path) {
    vtfpp::VTF::CreationOptions options;
    options.outputFormat = vtfpp::ImageFormat::DXT5;
    vtfpp::VTF vtf = vtfpp::VTF::create(skin.frames[0], vtfpp::ImageFormat::RGBA8888, skin.width, skin.height, options);
    if (skin.frames.size() == 1) {
    }
    else {
        vtf.setMipCount(1);
        vtf.setFrameCount((uint16_t)skin.frames.size());
        for (int i = 1; i < skin.frames.size(); i++) {
            vtf.setImage(skin.frames[i], vtfpp::ImageFormat::RGBA8888, skin.width, skin.height, options.filter, 0, i, 0, 0, -1.0f);
        }
    }
    vtf.bake(path);
}

std::string GenerateVMT(Skin skin, fs::path modelFilename) {
    std::string target_shader = "UnlitGeneric";
    std::string vmt_content {vmt_template};
    std::string proxies_content {proxy_template};
    bool proxies_used = false;

    // Scanline
    if (skin.scanline) {
        // requires two textures
        // can't use all the time, otherwise $texture2 is blank
        target_shader = "UnlitTwoTexture";
        proxies_used = true;
        ReplaceString(&proxies_content, "{APPEND_PROXIES}", (std::string)scanline_proxies_template + "\n\t{APPEND_PROXIES}");
        ReplaceString(&vmt_content, "{APPEND_VARIABLES}", (std::string)scanline_variables_template + "\n\t{APPEND_VARIABLES}");
    }

    // Animated
    if (skin.animated) {
        proxies_used = true;
        ReplaceString(&proxies_content, "{APPEND_PROXIES}", (std::string)animated_proxies_template + "\n\t{APPEND_PROXIES}");
        ReplaceString(&proxies_content, "{ANIMATED_REPLACE_FPS}", std::to_string(skin.fps));
    }

    // Scrolling
    if (skin.scrolling) {
        proxies_used = true;
        bool is_vertical = !skin.horizontalScrolling;
        ReplaceString(&proxies_content, "{APPEND_PROXIES}", (std::string)scrolling_proxies_template + "\n\t{APPEND_PROXIES}");
        ReplaceString(&proxies_content, "{SCROLLING_REPLACE_RATE}", std::to_string(skin.scrollingRate));
        ReplaceString(&proxies_content, "{SCROLLING_REPLACE_AXIS}", is_vertical ? "1" : "0");
    }

    // Write proxies
    if (proxies_used) {
        ReplaceString(&proxies_content, "{APPEND_PROXIES}", "");
        ReplaceString(&vmt_content, "{REPLACE_PROXIES}", proxies_content);
    } else {
        ReplaceString(&vmt_content, "{REPLACE_PROXIES}", "");
    }

    // Write variables
    ReplaceString(&vmt_content, "{APPEND_VARIABLES}", "");

    ReplaceString(&vmt_content, "{REPLACE_VTF}", "models/props/" + modelFilename.replace_extension("").string() + "/" + skin.materialName);
    ReplaceString(&vmt_content, "{REPLACE_SHADER}", target_shader);

    // TODO: trim whitespace and/or newlines?
    return vmt_content;
}

void WriteMonitor(std::vector<Skin> skins, std::filesystem::path outputDirectory, std::string modelPath, bool angled, OutputFormat outputFormat) {
    if (!(skins.size() > 0)) {
        std::cout << "No skins provided" << std::endl;
        return;
    }
    if (outputFormat == OutputFormat::FolderOutput && !modelPath.ends_with(".mdl")) {
        std::cout << "Invalid model path: no .mdl" << std::endl;
        return;
    }
    if (outputFormat == OutputFormat::FolderOutput && !modelPath.starts_with("models/")) {
        std::cout << "Invalid model path: no models/" << std::endl;
        return;
    }
    if (!std::filesystem::exists(outputDirectory)) {
        std::cout << "Output directory doesn't exist" << std::endl;
        return;
    }

    fs::path modelFilename = ((fs::path)modelPath).filename();
    std::string modelName = ((fs::path)modelPath).filename().string();

    // /mnt/g/SteamLibrary/steamapps/common/Portal 2/portal2_dlc3/models/props/lab_monitor_test/lab_monitor_test_pose03.mdl
    fs::path modelDir = outputDirectory / ((fs::path)modelPath).parent_path(); //"models/props" / "monitor_test_change_me";
    fs::path textureDir = outputDirectory / "materials/models/props" / modelFilename.replace_extension("");

    bool worked = fs::create_directories(modelDir);
    worked &= fs::create_directories(textureDir);

    if (worked) {
        std::cout << "Directories created successfully.\n";
    } else {
        if (fs::exists(modelDir) && fs::exists(textureDir)) {
            std::cout << "Directories already exists.\n";
        } else {
            std::cout << "Directories could not be created.\n";
            return;
        }
    }


    // Write .mdl file
    std::vector<std::byte> mdl = ReadMonitorFile(DEFAULT_MDL_PATH, angled);
    MDL model = ParseMDL(mdl);

    model.model_name = modelPath.replace(0, std::string_view("models/").length(), "");//(fs::path)"props" / "monitor_test_change_me" / modelName;
    model.skin_table = {};
    model.material_names.resize(skins.size() + 1);
    model.cdmaterials.push_back(("models/props" / modelFilename.replace_extension("") / "").string());

    // Assume lab_monitor comes first, since we are not trying to replace that
    for (short i = 1; i <= skins.size(); i++) {
        model.skin_table.push_back({0, i});
        model.material_names[i] = skins[i-1].materialName;
    }

    std::ofstream outputMDLFile(modelDir / modelName, std::ios::out | std::ios::binary);
    WriteMDL(model, outputMDLFile);


    // Write .vtx files
    std::vector<std::byte> vtx = ReadMonitorFile(DEFAULT_VTX_PATH, angled);
    VTX vtx_parsed = ReadVTX(vtx);
    vtx_parsed.checksum = model.checksum;
    
    std::ofstream outputVTXFile((modelDir / modelName).replace_extension(".vtx"), std::ios::out | std::ios::binary);
    WriteVTX(vtx_parsed, outputVTXFile);

    // .dx90.vtx seems to be identical to the normal one, produce a copy if it is necessary work model to work
    // Portal 2 does not care if it doesn't exist, I think it's unnecessary
    //std::ofstream outputVTX90File((modelDir / modelName).replace_extension(".dx90.vtx"), std::ios::out | std::ios::binary);
    //WriteVTX(vtx_parsed, outputVTX90File);


    // Write .vvd file
    std::vector<std::byte> vvd = ReadMonitorFile(DEFAULT_VVD_PATH, angled);
    VVD vvd_parsed = ReadVVD(vvd);
    vvd_parsed.checksum = model.checksum;
    
    std::ofstream outputVVDFile((modelDir / modelName).replace_extension(".vvd"), std::ios::out | std::ios::binary);
    WriteVVD(vvd_parsed, outputVVDFile);


    // Write .phy file
    std::vector<std::byte> phy = ReadMonitorFile(DEFAULT_PHY_PATH, angled);
    PHY phy_parsed = ReadPHY(phy);
    phy_parsed.checksum = model.checksum;
    
    std::ofstream outputPHYFile((modelDir / modelName).replace_extension(".phy"), std::ios::out | std::ios::binary);
    WritePHY(phy_parsed, outputPHYFile);


    // Write .vtf/.vmt files
    for (Skin skin : skins) {
        std::cout << "Writing " << skin.materialName << ".vtf/.vmt" << std::endl;

        WriteSkin(skin, (textureDir / skin.materialName).replace_extension(".vtf"));
        
        std::ofstream outputVMTFile((textureDir / skin.materialName).replace_extension(".vmt"), std::ios::out);

        if (!outputVMTFile.is_open()) {
            std::cerr << "Error opening the VMT file!" << std::endl;
            return;
        }

        std::string vmt_content = GenerateVMT(skin, modelFilename);

        // Write to file
        outputVMTFile << vmt_content;
        outputVMTFile.close();
    }
    
    std::cout << "Finished" << std::endl;
}