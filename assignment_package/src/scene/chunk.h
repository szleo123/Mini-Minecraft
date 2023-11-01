#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>
#include "drawable.h"

#include <thread>
#include <mutex>


//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, LAVA, BEDROCK
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

struct ChunkVBOData;

class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    int minX, minZ;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;



public:
    bool vbo_created=false;
    bool buffer_created=false;

    Chunk();
    Chunk(int, int, OpenGLContext*);
    void createVBOdata();
    void generateVBO(std::vector<ChunkVBOData>&, std::mutex&);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    std::vector<glm::vec4> findFace(glm::ivec3);
    glm::vec3 findColor(BlockType);
    std::vector<glm::vec4> findUV(BlockType, glm::ivec3);
    bool checkBound(int, int, int);
    bool checkNeighbor(int, int, int, BlockType);
    bool checkConidtions(int, int, int, const glm::ivec3&, BlockType);
    void bindBuffer(const std::vector<glm::vec4>&, const std::vector<glm::vec4>&,
                    const std::vector<GLuint>&, const std::vector<GLuint>&);
    glm::vec2 getMins();
};

struct ChunkVBOData {
    Chunk* chunk;
    std::vector<glm::vec4> d;
    std::vector<glm::vec4> d_trans;
    std::vector<GLuint> idx;
    std::vector<GLuint> idx_trans;
};
