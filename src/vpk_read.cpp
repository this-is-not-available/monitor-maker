#include "vpk_read.h"
#include "main.h"

#include <iostream>

#include <vpkpp/vpkpp.h>

// Needed until fspp supports useUpdate (and useDLCFolders?) in fspp::FileSystemOptions
// By default, most Portal 2 props appear as if they come from the update folder
std::vector<std::byte> ReadFile(std::string filename) {
    if (gamefs.has_value()) {
        std::optional<std::vector<std::byte>> file = gamefs.value().read(filename, "GAME", true);

        if (file.has_value()) {
            return file.value();
        }

        std::unique_ptr<vpkpp::PackFile> vpk = vpkpp::VPK::open((p2Path / "portal2/pak01_dir.vpk").string());
        file = vpk->readEntry(filename);
        if (file.has_value()) {
            return file.value();
        }
    }

    std::cout << "ReadFile: File not found through fspp nor pak01_dir.vpk" << std::endl;
    throw new std::exception();
}