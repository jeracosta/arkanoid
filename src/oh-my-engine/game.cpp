#include "game.hpp"

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <memory>

#include "input.hpp"
#include "node.hpp"
#include "oh-my-engine/entity-component-system/system_store.hpp"
#include "time.hpp"
#include "window.hpp"

namespace ome {

Game::Enviroment::Enviroment()
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
}

std::shared_ptr<Game::Enviroment>
Game::Enviroment::instance()
{
    static std::weak_ptr<Enviroment> enviroment;

    if (auto ref = enviroment.lock())
    {
        return ref;
    }
    else
    {
        // note: std::make_shared can't used because Enviroment constructor is private.
        auto new_enviroment = std::shared_ptr<Enviroment>(new Enviroment{});
        enviroment          = new_enviroment;
        return new_enviroment;
    }
}

Game::Enviroment::~Enviroment()
{
    SDL_Quit();
}

Game::Game(const Configuration &config)
    : config_(config),
      window(config.window)
{
    if (config_.configure_input)
    {
        config_.configure_input(input_mapper_, *this);
    }

    if (config_.configure_systems)
    {
        config_.configure_systems(systems, *this);
    }

    if (config_.make_root_node)
    {
        mount_(config_.make_root_node(*this));
    }

    if (config_.on_init)
    {
        config_.on_init(*this);
    }
}

Game::~Game() = default;

void
Game::mount_(std::unique_ptr<Node> node_owner)
{
    assert(node_owner);

    auto node = node_owner.get();

    [[unlikely]]
    if (node->is_mounted())
    {
        throw std::runtime_error("Cannot mount a node that is already mounted.");
    }

    if (root_node_)
    {
        root_node_->add_child(std::move(node_owner));
    }
    else
    {
        root_node_ = std::move(node_owner);
        root_node_->mount_to_(this);
    }
}

void
Game::update_()
{
    time.update_();

    auto event = SDL_Event{};
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            running_ = false;
        }
        input_mapper_.handle(event);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    config_.on_update(*this);

    for (auto entity : entities.living())
    {
        systems.update(entity, *this);
    }

    visit_dfs(*root_node_, &Node::tick, [](Node &) {});

    for (auto &task : tasks_)
    {
        task();
    }

    tasks_.clear();

    SDL_GL_SwapWindow(window);

    frame_count_++;
}

} // namespace ome
