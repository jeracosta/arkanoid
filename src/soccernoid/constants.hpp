#pragma once

#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/mesh.hpp"
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

    inline static const std::filesystem::path meshes = assets / "meshes";
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
        std::string name;
        Item        front;
        Item        back;
        Item        left;
        Item        right;
        Item        top;
        Item        bottom;

        // `directory` is relative to the textures root (e.g. "skybox/dawn").
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
                .name   = directory.filename().string(),
                .front  = { directory / "front.png", config },
                .back   = { directory / "back.png", config },
                .left   = { directory / "left.png", config },
                .right  = { directory / "right.png", config },
                .top    = { directory / "top.png", config },
                .bottom = { directory / "bottom.png", config },
            };
        }

        // Scans the skybox assets directory and builds one entry per
        // subdirectory found, sorted by name for a deterministic order.
        static std::vector<SkyboxFaces>
        load_all()
        {
            std::vector<SkyboxFaces> skyboxes;

            const auto root = FilesystemPaths::textures / "skybox";

            if (std::filesystem::is_directory(root))
            {
                for (const auto &entry : std::filesystem::directory_iterator(root))
                {
                    if (entry.is_directory())
                    {
                        skyboxes.push_back(
                            from_directory(std::filesystem::path("skybox")
                                            / entry.path().filename()));
                    }
                }
            }

            std::sort(skyboxes.begin(), skyboxes.end(),
                      [](const SkyboxFaces &a, const SkyboxFaces &b)
            { return a.name < b.name; });

            return skyboxes;
        }
    };

    Item                     column;
    Item                     dirt;
    Item                     metal;
    Item                     cobblestone;
    Item                     snail;
    Item                     barrel;
    Item                     transformer;
    Item                     moai;
    Item                     racoon;
    std::vector<SkyboxFaces> skybox;

    // Looks a skybox up by its directory name, falling back to the first one.
    const SkyboxFaces &
    skybox_named(std::string_view name) const
    {
        for (const auto &candidate : skybox)
        {
            if (candidate.name == name)
            {
                return candidate;
            }
        }

        return skybox.at(0);
    }
};

static const inline TexturePalette textures = {
    .column      = { "column.tga.png" },
    .dirt        = { "dirt.png" },
    .metal       = { "metal.jpg" },
    .cobblestone = { "cobblestone.jpg" },
    .snail       = { "snail.webp" },
    .barrel      = { "explosive_barrel.png" },
    .transformer = { "transformer.png" },
    .moai        = { "moai.png" },
    .racoon      = { "racoon_colors.png" },
    .skybox      = TexturePalette::SkyboxFaces::load_all(),
};

struct MeshPalette
{
    class Item
    {
      private:
        std::filesystem::path              file_name_;
        mutable std::shared_ptr<ome::Mesh> mesh_;

        std::shared_ptr<ome::Mesh>
        load_() const
        {
            if (!mesh_)
            {
                mesh_ = ome::Mesh::load(FilesystemPaths::meshes / file_name_);
            }
            return mesh_;
        }

      public:
        Item(std::filesystem::path file_name)
            : file_name_(std::move(file_name))
        {
        }

        ome::Mesh &
        get() const
        {
            return *load_();
        }

        operator const ome::Mesh &() const
        {
            return get();
        }

        operator std::shared_ptr<ome::Mesh>() const
        {
            return load_();
        }
    };

    Item characters_g;
    Item characters_h;
    Item column;
    Item dragon;
    Item arrow;
    Item barrel;
    Item teapot;
    Item transformer;
    Item moai;
    Item skateboard;
    Item racoon;
};

static const inline MeshPalette meshes = {
    .characters_g = { "characters/character-g.fbx" },
    .characters_h = { "characters/character-h.fbx" },
    .column       = { "column.fbx" },
    .dragon       = { "dragon/Dragon.fbx" },
    .arrow        = { "arrow.glb" },
    .barrel       = { "barrel.glb" },
    .teapot       = { "teapot/teapot.obj" },
    .transformer  = { "transformer/tranformer.gltf" },
    .moai         = { "moai.fbx" },
    .skateboard   = { "board.obj" },
    .racoon       = { "racoon/scene.gltf" },
};

} // namespace soccernoid
