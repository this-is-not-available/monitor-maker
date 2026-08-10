#ifndef MONITOR_GENERATE_H
#define MONITOR_GENERATE_H


#include "main.h"

#include <vector>

/*#define DEFAULT_MDL_PATH "models/props/lab_monitor_pose01/lab_monitor_pose01.mdl"
#define DEFAULT_PHY_PATH "models/props/lab_monitor_pose01/lab_monitor_pose01.phy"
#define DEFAULT_VVD_PATH "models/props/lab_monitor_pose01/lab_monitor_pose01.vvd"
#define DEFAULT_VTX_PATH "models/props/lab_monitor_pose01/lab_monitor_pose01.vtx"
#define DEFAULT_VTX_DX90_PATH "models/props/lab_monitor_pose01/lab_monitor_pose01.dx90.vtx"*/

const std::string_view DEFAULT_MDL_PATH = "models/props/lab_monitor_pose01/lab_monitor_pose01.mdl";
const std::string_view DEFAULT_PHY_PATH = "models/props/lab_monitor_pose01/lab_monitor_pose01.phy";
const std::string_view DEFAULT_VVD_PATH = "models/props/lab_monitor_pose01/lab_monitor_pose01.vvd";
const std::string_view DEFAULT_VTX_PATH = "models/props/lab_monitor_pose01/lab_monitor_pose01.vtx";
const std::string_view DEFAULT_VTX_DX90_PATH = "models/props/lab_monitor_pose01/lab_monitor_pose01.dx90.vtx";

enum OutputFormat {
    FolderOutput,
    // OutputZip
};

void WriteMonitor(std::vector<Skin> skins, std::filesystem::path outputDirectory, std::string modelPath, bool angled, OutputFormat outputFormat);

#endif