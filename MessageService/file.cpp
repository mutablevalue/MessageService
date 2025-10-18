#include "file.h"
#include "profile.h"
#include <sstream>

void FileSystem::WriteProfile(const Profile& profile) {
    const Path path = GetPath() / "profile.json";

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open())
        throw std::runtime_error("Failed to open profile.json ");

    file << "{\n"
        << "  \"username\": \"" << profile.Name << "\",\n"
        << "  \"color\": " << profile.Color << "\n"
        << "}";
    file.close();
}

Profile FileSystem::LoadProfile() {
    const Path path = GetPath() / "profile.json";
    std::ifstream file(path);
    if (!file.is_open()) throw std::runtime_error("Failed to open profile.json");

    char nameBuf[16] = {};
    u32 color = 0;

    std::string label;
    char quote;

    file >> label >> quote;                         
    file.getline(nameBuf, sizeof(nameBuf), '"');      // read until next quote
    file >> label >> color;                          

    const Profile TempProfile{ nameBuf, color };
    return TempProfile;
}
