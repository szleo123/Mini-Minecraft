#ifndef PLANET_H
#define PLANET_H

#include "glm_includes.h"
#include "drawable.h"
#include "chunk.h"
#include "shaderprogram.h"

class Planet_Chunk: public Drawable
{
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 262144> m_blocks;
    int minX, minY, minZ;
    glm::vec3 center;
    std::unordered_map<Direction, Planet_Chunk*, EnumHash> m_neighbors;

public:
    bool vbo_created=false;
    bool buffer_created=false;

    Planet_Chunk();
    Planet_Chunk(int, int, int, glm::vec3, OpenGLContext*);
    void createVBOdata();
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Planet_Chunk>& neighbor, Direction dir);
    std::vector<glm::vec4> findFace(glm::ivec3);
    glm::vec3 findColor(BlockType);
    std::vector<glm::vec4> findUV(BlockType, glm::ivec3);
    bool checkBound(int, int, int);
    bool checkNeighbor(int, int, int, BlockType);
    bool checkConidtions(int, int, int, const glm::ivec3&, BlockType);
    void bindBuffer(const std::vector<glm::vec4>&, const std::vector<glm::vec4>&,
                    const std::vector<GLuint>&, const std::vector<GLuint>&);
};

struct PlanetVBOData {
    Planet_Chunk* chunk;
    std::vector<glm::vec4> d;
    std::vector<glm::vec4> d_trans;
    std::vector<GLuint> idx;
    std::vector<GLuint> idx_trans;
};

// helper functions to convert (x, y, z) to and from the hash map key
uint32_t toKey_planet(int x, int y, int z);
glm::ivec3 toCoords_planet(uint32_t);

class Planet
{
private:
    std::unordered_map<uint32_t, uPtr<Planet_Chunk>> m_chunks;
    std::unordered_set<uint32_t> m_generatedTerrain;
    OpenGLContext* mp_context;
    int radius;

public:
    glm::vec3 center;


    Planet(OpenGLContext *context, glm::vec3 center, int radius);
    ~Planet();
    // Instantiates a new Planet_Chunk and stores it in
    // our chunk map at the given coordinates.
    // Returns a pointer to the created Chunk.
    Planet_Chunk* instantiateChunkAt(int x, int y, int z);
    // Do these world-space coordinates lie within
    // a Chunk that exists?
    bool hasChunkAt(int x, int y, int z) const;
    // Assuming a Chunk exists at these coords,
    // return a mutable reference to it
    uPtr<Planet_Chunk>& getChunkAt(int x, int y, int z);
    // Assuming a Chunk exists at these coords,
    // return a const reference to it
    const uPtr<Planet_Chunk>& getChunkAt(int x, int y, int z) const;
    // Given a world-space coordinate (which may have negative
    // values) return the block stored at that point in space.
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType getBlockAt(glm::vec3 p) const;
    // Given a world-space coordinate (which may have negative
    // values) set the block at that point in space to the
    // given type.
    void setBlockAt(int x, int y, int z, BlockType t);
    // Draws every Chunk that falls within the bounding box
    // described by the min and max coords, using the provided
    // ShaderProgram
    void draw(ShaderProgram *shaderProgram);
    // Initializes the Planet_Chunks
    void createPlanet();
    void move(float time);

};

#endif // PLANET_H
