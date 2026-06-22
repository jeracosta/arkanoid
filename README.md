# Arkanoid 3D

*A 3D reimagining of the classic Arkanoid arcade game, built as a demo project
showcasing a custom (from-scratch) game engine.*

> **Tech stack:** C++26, OpenGL 1.3 (fixed-function), SDL2, ImGui, assimp, glm, Boost.MP11

Video: <https://youtu.be/PJRcgD1OAC0>

## Architecture and features

### Node system

Every game entity is a node in a tree scenegraph. This approach provides clear code structure, entity and transform hierarchy, lifecycle hooks, and utilities such as logging.

Standard nodes are included for features like lights and hitboxes, and mixins for features like tick slowdown or automatic despawn. User can extend and mix those to code their custom logic.

Dangerous tree-mutations during mounting get defered to end of frame.

```cpp
// Example of a custom node:
class RootNode : public Node {
  public:
    RootNode() : Node("Root") {
        emplace_child<CameraNode>().rename("Camera");
        emplace_child<WindowNode>().rename("Window");
        // ...
    }

  private:
    void on_mount_() override {
        auto log_tree = [this] { this->log(tree_string(*this)); };
        this->hold(this->game()->input.bind(Action::PrintTree, log_tree));
    }

    void on_ready_() override {
        this->log("Aye captain, we're ready!");
    }

    void on_tick_() override {
        log("Hi!", LogLevel::Debug);
    }

    void on_render_  (RenderFrame &) override;
    void on_unmount_ () override;

};
```

---

### Time tracking

Nodes can access `game()->time`, contaning the frame/delta time, total elapsed time, time scale (for slow/fast-motion), and the unscaled times (ignoring time scale).

---

### Event system

A compile-time-typed, multi-cast event bus with RAII connections.

```cpp
auto bus = EventBus<FooEvent, BarEvent>;

auto connection = bus.bind([&](const FooEvent &event){/* handler code*/};
this->hold(std::move(connection)); // disconnects when this object is destroyed

bus.emit(FooEvent{"example"});
```

This class is leveraged for keyboard input handling, inter-node communication, game-state propagation, setting changes, etc.

---

### Input mapping

Physical keys are bound to actions, then game code subscribes to action events. Action polling is also avaliable for per-frame checks.

```cpp
enum class Action { Quit, TogglePause, ...};

// During controls setup:
input_mapper->bind({ SDLK_r, Press }, Action::PlayerShoot);

// On a node during initialization:
this->hold(this->game().input.bind(Action::PlayerShoot, [this] { shoot_(); }));

// On a node during tick:
if (this->game().input.is_pressed(Action::PlayerShoot)) {
    shoot_();
}

```

Continuous input from the mouse can be directly bound to a handler by the mapper.

---

### Curves

A generic tweening system built on `Curve<T>` (maps `[0,1] → T`)
driven by `CurveProcess<T>` over a duration:

```cpp
auto curve    = std::make_shared<Interpolation<CameraShot>>(from, to, EasingCurve::smoothstep());
auto process  = CurveProcess<CameraShot>(curve, 0.22f); // completed in 0.22 seconds

// Each frame:
process.update(delta_time); // value moves along the curve
if (!process.is_completed()) {
    camera->apply(process.value());
}
```

It's leveraged for animations, transitions, trajectories, and any time-based interpolation.
The engine provides convenience curve types like harmonic oscillators and key-framed splines.

---

### Particle system

Particle behavior is defined by a **`ParticleScheme`** — a policy object where
each property can be a constant, a `Curve<T>`, a `Region + RNG` (for random initialization), or any arbitrary function:

```cpp
const ParticleScheme scheme = {
    .initial_position  = { Box::from_size(Vec3f{ 0.8f }), rng },
    .initial_velocity  = { Sphere<3>({}, 0.1f), rng },
    .acceleration      = Vec3f{ 0, 0, 0 },
    .time_to_live      = 5.0f,
    .color             = Interpolation{ Color::white(), Color::black() },
    .scale             = [] (auto &context) { ... },
    .blend_mode        = BlendMode::additive(),
};
```

**`ParticleEmitterNode`** interacts with the system server, emitting batches of particles at configurable intervals.
Particles outlive their emitter, and their properties get interpolated to consider frame latency.

### Collision system

**`HitboxNode`** defines a local AABB, auto-registers with the collision server
on mount, and unregisters on unmount. Every frame the server checks overlapping
pairs and calls `on_collision_(HitboxNode &other)` on both participants:

```cpp
class ProjectileHitbox : public HitboxNode {
    void on_collision_(HitboxNode &other) override {
        if (dyamic_cast<ObstacleHitbox*>(&other)) {
            emit(ObstacleDestroyed{});
        }
    }
};
```

---

### Rendering

Each tick, nodes can alter a `RenderFrame` passed to `on_render_()`, which contains meshes, lights, and particles to draw, as well as global render settings like the skybox and fog. Before the next frame, the engine updates the screen based on that object. It performs several additive light passes to bypass the limitations of fixed-function OpenGL.

---

### Others

- **Geometry classes** like vectors, orientations, intervals, spheres, and cones; reused in different contexts.
- **Camera** with orbit controls and projection management.
- **Assets** like textures, meshes, and materials.
- **Color** abstraction to easily work with sRGB, linear, HSV and Hex formats.
- **Logging** with tree-structured context and log levels.
- **Settings** with type-safe keys, event notifications, and ImGui integration.

---

## Building

Linux compile and run instructions:

```bash
./scripts/build.sh game
./build/arkanoid/bin/game
```

It may need some dependencies installed, such as SDL2, OpenGL, and assimp.
