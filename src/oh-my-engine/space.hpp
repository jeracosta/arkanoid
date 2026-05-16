namespace ome {

// Coordinate space used for spacial properties.
enum Space
{
    World, // Absolute, shared by all nodes in the scene.
    Local  // Relative to the parent node. Each node has its own local space.
};

}
