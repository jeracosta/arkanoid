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
    ome::Color projectile;
    ome::Color grass;
    ome::Color dirt;
    ome::Color goal;
    ome::Color red_kit;
    ome::Color skin;
};

static const inline ColorPalette colors = {
    .fog        = ome::Color::rgb(0, 0, 0),
    .ball       = ome::Color::rgb(210, 50, 255),
    .projectile = ome::Color::rgb(0, 255, 0),
    .grass      = ome::Color::rgb(60, 175, 45),
    .dirt       = ome::Color::rgb(105, 65, 35),
    .goal       = ome::Color::rgb(230, 242, 217),
    .red_kit    = ome::Color::rgb(200, 30, 30),
    .skin       = ome::Color::rgb(230, 190, 150),
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
    // #region Texture palette item class

    class Item
    {
      private:
        std::filesystem::path               file_name_;
        std::function<void(ome::Texture &)> config_;

        mutable std::shared_ptr<ome::Texture> texture_;

        std::shared_ptr<ome::Texture>
        load_() const
        {
            if (!texture_)
            {
                texture_ = ome::Texture::load(FilesystemPaths::textures / file_name_);
            }

            if (config_)
            {
                config_(*texture_);
            }

            return texture_;
        }

      public:
        Item(std::filesystem::path file_name, std::function<void(ome::Texture &)> config = nullptr)
            : file_name_(std::move(file_name)),
              config_(std::move(config))
        {
        }

        ome::Texture &
        get() const
        {
            return *load_();
        }

        operator const ome::Texture &() const
        {
            return get();
        }

        operator std::shared_ptr<ome::Texture>() const
        {
            return load_();
        }
    };

    // #endregion

    struct SkyboxFaces
    {
        Item front;
        Item back;
        Item left;
        Item right;
        Item top;
        Item bottom;

        static SkyboxFaces
        from_directory(const std::filesystem::path &directory)
        {
            auto config = [](ome::Texture &texture)
            {
                texture.set_wrap({ GL_CLAMP_TO_EDGE });
                texture.set_filters(GL_LINEAR, GL_LINEAR);
                texture.set_blend_mode(GL_REPLACE);
            };

            return {
                .front  = { directory / "front.png", config },
                .back   = { directory / "back.png", config },
                .left   = { directory / "left.png", config },
                .right  = { directory / "right.png", config },
                .top    = { directory / "top.png", config },
                .bottom = { directory / "bottom.png", config },
            };
        }
    };

    struct SkyboxPalette
    {
        SkyboxFaces ablaze;
        SkyboxFaces blink;
        SkyboxFaces blood;
        SkyboxFaces dawn;
        SkyboxFaces earth;
        SkyboxFaces night;
        SkyboxFaces space;
        SkyboxFaces space2;
    };

    Item          dirt;
    Item          floor;
    Item          snail;
    SkyboxPalette skybox;
};

static const inline TexturePalette textures = {
    .dirt = { "dirt.png" },
    .floor = { "floor.jpg" },
    .snail = { "snail.webp" },
    .skybox = { //TODO: Generate faces from cubemap
        .ablaze = TexturePalette::SkyboxFaces::from_directory("skybox/ablaze"),
        .blink  = TexturePalette::SkyboxFaces::from_directory("skybox/blink"),
        .blood  = TexturePalette::SkyboxFaces::from_directory("skybox/blood"),
        .dawn   = TexturePalette::SkyboxFaces::from_directory("skybox/dawn"),
        .earth  = TexturePalette::SkyboxFaces::from_directory("skybox/earth"),
        .night  = TexturePalette::SkyboxFaces::from_directory("skybox/night"),
        .space  = TexturePalette::SkyboxFaces::from_directory("skybox/space"),
        .space2 = TexturePalette::SkyboxFaces::from_directory("skybox/space2"),
    },
};

}; // namespace soccernoid
