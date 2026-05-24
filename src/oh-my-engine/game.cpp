#include "game.hpp"

#include <SDL2/SDL.h>
#include <memory>

#include "input.hpp"
#include "node.hpp"
#include "oh-my-engine/open_gl/matrix_guard.hpp"
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
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
        // note: std::make_shared can't be used if Enviroment constructor is private.
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
      window(config.window),
      camera(config.camera)
{
}

Game::~Game()
{
    // Shut ImGui down while the SDL GL context (owned by `window`) is alive.
    imgui_.reset();
}

void
Game::initialize_()
{
    imgui_.emplace(window);

    if (config_.make_logger)
    {
        logger_ = config_.make_logger();
    }

    if (config_.make_input_mapper)
    {
        input = config_.make_input_mapper(*this);
    }

    if (config_.make_root_node)
    {
        mount_(config_.make_root_node(*this));
    }

    if (config_.on_init)
    {
        config_.on_init(*this);
    }

    hold(camera.bind(&Game::on_projection_update_, this));

    hold(window.bind(&Game::on_window_resize_, this));

    on_projection_update_(ProjectionUpdated{ camera.projection() });

    if (config_.fog.enabled)
    {
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, config_.fog.start);
        glFogf(GL_FOG_END, config_.fog.end);

        auto [r, g, b, a] = config_.fog.color.rgba_f();
        float fog_col[4]  = { r, g, b, a };

        glFogfv(GL_FOG_COLOR, fog_col);
        glClearColor(r, g, b, a);
    }
    else
    {
        glDisable(GL_FOG);
    }

    if (config_.lighting.enabled)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        GLfloat global_ambient[] = { 0, 0, 0, 1 }; // no global ambient
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

        glEnable(GL_NORMALIZE);
    }
    else
    {
        glDisable(GL_LIGHTING);
    }
}

void
Game::mount_(std::shared_ptr<Node> node_owner)
{
    assert(node_owner);

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
Game::run_()
{
    initialize_();

    running_ = true;

    while (running_)
    {
        time.update_();

        process_sdl_events_();

        imgui_->begin_frame();

        config_.on_update ? config_.on_update(*this) : void();

        // Render and logic update are intertwined, as some nodes use OpenGL directly in on_tick_.
        // TODO: Make all nodes render thru the render_to method, and extract a render_ method here.

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        ome::open_gl::look_at(camera);

        [[likely]]
        if (root_node_)
        {
            visit_dfs(*root_node_, &Node::tick, [](Node &) {});
        }

        collision_server.process_collisions();

        auto render_frame = RenderFrame{};

        [[likely]]
        if (root_node_)
        {
            visit_dfs(*root_node_, [&render_frame](Node &node) {
                node.render_to(render_frame);
            }, [](Node &) {});
        }

        ome::open_gl::render(render_frame);

        resolve_tasks_();

        imgui_->end_frame();

        SDL_GL_SwapWindow(window);

        frame_count_++;
    }
}

void
Game::process_sdl_events_()
{
    auto event = SDL_Event{};
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            running_ = false;
        }

        event | *imgui_ | input | window; // piped to chain of handlers
    }
}

void
Game::resolve_tasks_()
{
    for (auto &task : tasks_)
    {
        task();
    }

    tasks_.clear();
}

std::shared_ptr<Node>
Game::unmount_tree()
{
    root_node_->unmount_();
    return std::move(root_node_);
}

void
Game::on_projection_update_(const ProjectionUpdated &event)
{
    auto &projection = event.new_projection;

    auto matrix_guard = ome::open_gl::MatrixGuard();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(projection.fov_degrees, projection.aspect_ratio, camera.near(), camera.far());
}

void
Game::on_window_resize_(const WindowResized &)
{
    camera.aspect_ratio(window.aspect_ratio());
}

} // namespace ome
