#pragma once

#include <filesystem>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/texture.hpp"

namespace soccernoid {

static constexpr float gravity_strength = 9.81f;

static constexpr float despawn_distance = 60.0f;

static constexpr struct
{
    float start = 25.0f;
    float end   = 50.0f;
} fog;

struct ColorPalette
{
    ome::Color fog;
    ome::Color ball;
    ome::Color grass;
    ome::Color dirt;
    ome::Color goal;
    ome::Color red_kit;
    ome::Color skin;
};

static const inline ColorPalette colors = {
    .fog     = ome::Color::rgb(0, 0, 0),
    .ball    = ome::Color::rgb(210, 50, 255),
    .grass   = ome::Color::rgb(60, 175, 45),
    .dirt    = ome::Color::rgb(105, 65, 35),
    .goal    = ome::Color::rgb(230, 242, 217),
    .red_kit = ome::Color::rgb(200, 30, 30),
    .skin    = ome::Color::rgb(230, 190, 150),
};

#ifndef SOCCERNOID_ASSETS_DIRECTORY_RELATIVE_PATH
#define SOCCERNOID_ASSETS_DIRECTORY_RELATIVE_PATH "../assets/"
#endif

struct FilesystemPaths
{
    inline static const std::filesystem::path executable = []
    {
#ifdef _WIN32
        char path[MAX_PATH];
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        return std::filesystem::path(path).parent_path();

#elif __linux__
        return std::filesystem::canonical("/proc/self/exe").parent_path();

#elif __APPLE__
        char     path[1024];
        uint32_t size = sizeof(path);
        _NSGetExecutablePath(path, &size);
        return std::filesystem::canonical(path).parent_path();
#endif
    }();

    inline static const std::filesystem::path assets
        = executable / SOCCERNOID_ASSETS_DIRECTORY_RELATIVE_PATH;

    inline static const std::filesystem::path textures = assets / "textures";
};

struct TexturePalette
{
    class Item
    {
      private:
        std::filesystem::path file_name_;

        mutable std::shared_ptr<ome::Texture> texture_;

      public:
        Item(std::filesystem::path file_name)
            : file_name_(std::move(file_name))
        {
        }

        operator const ome::Texture &() const
        {
            if (!texture_)
            {
                texture_ = ome::Texture::load(FilesystemPaths::textures / file_name_);
            }

            return *texture_;
        }
    };
};

static const inline TexturePalette textures = {};

} // namespace soccernoid
