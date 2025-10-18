#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include <memory>
#include <iomanip>
#include <cstdlib>     
#include "types.h"

inline static Path GetPath() {

    char* buffer = nullptr;
    size_t size = 0;

    if (!(_dupenv_s(&buffer, &size, "APPDATA") == 0 && buffer != nullptr)) {
        return std::filesystem::path(".");
    } // have to use dupenv instead the syntax is god awful
    std::filesystem::path base(buffer);
    free(buffer);
    std::filesystem::path dir = base / "Data";
    std::filesystem::create_directories(dir);
    return dir;
}


struct Profile;

struct FileSystem {
    std::unique_ptr<std::ofstream> KeyFile;
    std::unique_ptr<std::ofstream> ProfileFile;

    inline static bool KeyExists() {
        return std::filesystem::exists(GetPath() / "MS.bin");
    }
    inline static bool ProfileExists() {
        return std::filesystem::exists(GetPath() / "profile.json");
    }

    void OpenKeyFile() {
        Path path = GetPath() / "MS.bin";
        KeyFile = std::make_unique<std::ofstream>(path, std::ios::binary | std::ios::trunc);
        if (!KeyFile || !KeyFile->is_open())
            throw std::runtime_error("Failed to open: " + path.string());
    }

    void OpenProfileFile() {
        Path path = GetPath() / "profile.json";
        ProfileFile = std::make_unique<std::ofstream>(path, std::ios::trunc);
        if (!ProfileFile || !ProfileFile->is_open())
            throw std::runtime_error("Failed to open: " + path.string());
    }

    void const WriteKey(const KArray& Key) const {
        if (!KeyFile || !KeyFile->is_open()) throw std::logic_error("Wrong order: OpenKeyFile() first.");
        KeyFile->write(Key.data(), static_cast<std::streamsize>(Key.size()));
        if (!*KeyFile) throw std::runtime_error("Write failed");
        KeyFile->flush();
    }

    void WriteProfile(const Profile& profile);

    static KArray LoadKey() {
        const Path path = GetPath() / "MS.bin";
        KArray key{};
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("No key file found at: " + path.string());
        file.read(key.data(), static_cast<std::streamsize>(key.size()));
        if (!file) throw std::runtime_error("Failed to read full key from: " + path.string());
        return key;
    }

    Profile LoadProfile();

    void CreateKey(const KArray& key) {
        if (KeyExists()) return;
        OpenKeyFile();
        WriteKey(key);
    }

    void CreateProfile(const Profile& Profile) {
        if (ProfileExists()) return;
        OpenProfileFile();
        WriteProfile(Profile);
    }

    void Close() {
        if (KeyFile) {
            KeyFile->flush();
            KeyFile->close();
            KeyFile.reset();
        }
        if (ProfileFile) {
            ProfileFile->flush();
            ProfileFile->close();
            ProfileFile.reset();
        }
    }

    ~FileSystem() { Close(); }

private:
    static std::string color_to_hex(u32 rgba) {
        std::ostringstream oss;
        oss << "0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << rgba;
        return oss.str();
    }
};
