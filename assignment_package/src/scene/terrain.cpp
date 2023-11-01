#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), m_geomCube(context), mp_context(context),
      blocktype_threads(), vbo_threads(), block_mutex(), vbo_mutex(), chunk_vbos()
{}

Terrain::~Terrain() {
    m_geomCube.destroyVBOdata();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(x, z, mp_context);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = std::move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(ShaderProgram *shaderProgram, glm::vec3 pos) {
    int xFloor = static_cast<int>(glm::floor(pos.x / 64.f));
    int zFloor = static_cast<int>(glm::floor(pos.z / 64.f));
    shaderProgram->setModelMatrix(glm::mat4());
    for (int i = xFloor - DRAW_RADIUS; i < xFloor + DRAW_RADIUS + 1; i++) {
        for (int j = zFloor - DRAW_RADIUS; j < zFloor + DRAW_RADIUS + 1; j++){
            for (int k = i*64; k < (i+1)*64; k += 16) {
                for (int l = j*64; l < (j+1)*64; l +=16) {
                    if (hasChunkAt(k, l)){
                        uPtr<Chunk> &c = getChunkAt(k, l);
                        if (c->buffer_created){
                            shaderProgram->draw(*(c.get()), false);
                        }
                    }
                }
            }
        }
    }
    for (int i = xFloor - DRAW_RADIUS; i < xFloor + DRAW_RADIUS + 1; i++) {
        for (int j = zFloor - DRAW_RADIUS; j < zFloor + DRAW_RADIUS + 1; j++){
            for (int k = i*64; k < (i+1)*64; k += 16) {
                for (int l = j*64; l < (j+1)*64; l +=16) {
                    if (hasChunkAt(k, l)){
                        uPtr<Chunk> &c = getChunkAt(k, l);
                        if (c->buffer_created){
                            shaderProgram->draw(*(c.get()), true);
                        }
                    }
                }
            }
        }
    }
}


void Terrain::CreateTestScene()
{
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));


//     Create the basic terrain floor
    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            if((x + z) % 2 == 0) {
                setBlockAt(x, 128, z, STONE);
            }
            else {
                setBlockAt(x, 128, z, DIRT);
            }
        }
    }
//     Add "walls" for collision testing
    for(int x = 0; x < 64; ++x) {
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
//     Add a central column
    for(int y = 129; y < 140; ++y) {
        setBlockAt(32, y, 32, GRASS);
    }

//    for (const auto &[key, chunk] : m_chunks) {
//        chunk->createVBOdata();
//    }
}

//////// Noise functions //////////////////////////////////////

float random1( glm::vec2 p ) {
    return glm::fract(glm::sin(glm::dot(p,glm::vec2(127.1,311.7)))*43758.5453f);
}

float mySmootherStep(float a, float b, float t) {
    t = t*t*t*(t*(t*6.0 - 15.0) + 10.0);
    return glm::mix(a, b, t);
}

float bilerpNoise(glm::vec2 uv) {
    glm::vec2 uvFract = glm::fract(uv);
    float ll = random1(glm::floor(uv));
    float lr = random1(glm::floor(uv) + glm::vec2(1,0));
    float ul = random1(glm::floor(uv) + glm::vec2(0,1));
    float ur = random1(glm::floor(uv) + glm::vec2(1,1));

    float lerpXL = mySmootherStep(ll, lr, uvFract.x);
    float lerpXU = mySmootherStep(ul, ur, uvFract.x);

    return mySmootherStep(lerpXL, lerpXU, uvFract.y);
}

float fbm(glm::vec2 uv) {
    float amp = 0.5;
    float freq = 8.0;
    float sum = 0.0;
    for(int i = 0; i < 6; i++) {
        sum += bilerpNoise(uv * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

glm::vec2 random2( glm::vec2 p ) {
    return glm::fract(glm::sin(glm::vec2(glm::dot(p,glm::vec2(127.1,311.7)),glm::dot(p,glm::vec2(269.5,183.3))))*43758.5453f);
}

float surflet(glm::vec2 P, glm::vec2 gridPoint) {
    // Compute falloff function by converting linear distance to a polynomial (quintic smootherstep function)
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1.0 - 6.0 * pow(distX, 5.0) + 15.0 * pow(distX, 4.0) - 10.0 * pow(distX, 3.0);
    float tY = 1.0 - 6.0 * pow(distY, 5.0) + 15.0 * pow(distY, 4.0) - 10.0 * pow(distY, 3.0);

    // Get the random vector for the grid point
    glm::vec2 gradient = random2(gridPoint);
    // Get the vector from the grid point to P
    glm::vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}

float PerlinNoise(glm::vec2 uv) {
    // Tile the space
    glm::vec2 uvXLYL = glm::floor(uv);
    glm::vec2 uvXHYL = uvXLYL + glm::vec2(1,0);
    glm::vec2 uvXHYH = uvXLYL + glm::vec2(1,1);
    glm::vec2 uvXLYH = uvXLYL + glm::vec2(0,1);

    return surflet(uv, uvXLYL) + surflet(uv, uvXHYL) + surflet(uv, uvXHYH) + surflet(uv, uvXLYH);
}


float fractalNoise(glm::vec2 uv, float o, float l, float p, float s) {
    float value = 0.;
    float amp = 2;
    float x1 = uv.x;
    float z1 = uv.y;
    for (int i = 0; i < o; i++) {
        value += abs(PerlinNoise(glm::vec2(x1, z1) / s)) * amp;
        x1 *= l;
        z1 *= l;
        amp *= p;
    }
    value = pow(value, 2);
    return glm::clamp(value, -1.f, 1.f);
}

float WorleyNoise(glm::vec2 uv) {

    // Tile the space
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y));

            // Random point inside current neighboring cell
            glm::vec2 point = random2(uvInt + neighbor);

            // Animate the point
//            point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            glm::vec2 diff = neighbor + point - uvFract;
            float dist = glm::length(diff);
            minDist = glm::min(minDist, dist);
        }
    }
    return minDist;
}

/////////////////////////////////////////////////////////////////////

void Terrain::generateBlocks(int minX, int minZ, std::mutex& mu) {
    mu.lock();
    Chunk* c = instantiateChunkAt(minX, minZ);
    mu.unlock();
    for(int x = 0; x < 16; x++) {
        for(int z = 0; z < 16; z++) {
            int truex = minX + x;
            int truez = minZ + z;
            glm::vec2 xz = glm::vec2(float(truex), float(truez));
            float mountain = glm::floor((150.f + glm::abs(fractalNoise(xz, 5, 3, 0.2, 80)) * 200));
            glm::vec2 offset = glm::vec2(fbm(xz / 256.f), fbm(xz / 128.f)) + glm::vec2(1000.f);
            float grass =  128 + (1. - WorleyNoise((xz + offset * 50.f) / 70.f)) * 30.f;
            float lerp =  0.5 * (PerlinNoise(xz / 200.f) + 1.f);
            lerp = glm::smoothstep(0.4, 0.6, (double)lerp);
            float y_final = glm::mix(grass, mountain, lerp);
            y_final = glm::min(y_final, 255.f);

            if (lerp < 0.45) {
                // grass
                for (int y = 128; y <= 128; y++) {
                    c->setBlockAt(x, y, z, STONE);
                }
                for (int y = 129; y < y_final; y++) {
                    c->setBlockAt(x, y, z, DIRT);
                }
                c->setBlockAt(x, y_final, z, GRASS);
            } else {
                // moutain
                for (int y = 128; y <= 128; y++) {
                    c->setBlockAt(x, y, z, STONE);
                }
                if (mountain <= 180) {
                    // Fill above y's with STONE blocks
                    for (int y = 129; y <= y_final; y++) {
                        c->setBlockAt(x, y, z, STONE);
                    }
                } else {
                    // Fill above y's with STONE blocks except top
                    for (int y = 129; y < y_final; y++) {
                        c->setBlockAt(x, y, z, STONE);
                    }
                    // Fill top y with SNOW blocks
                    c->setBlockAt(x, y_final, z, SNOW);
                }
            }

            // Fill any EMPTY blocks between height of [128, 138] with WATER
            for (int y = 128; y <= 142; ++y) {
                if (c->getBlockAt(x, y, z) == EMPTY) {
                    c->setBlockAt(x, y, z, WATER);
                }
            }
        }
    }
}

void Terrain::CreateNewScene() {
//    // Create the Chunks that will
//    // store the blocks for our
//    // initial world space
//    for(int x = 0; x < 64; x += 16) {
//        for(int z = 0; z < 64; z += 16) {
//            instantiateChunkAt(x, z);
//        }
//    }
//    // Tell our existing terrain set that
//    // the "generated terrain zone" at (0,0)
//    // now exists.
//    m_generatedTerrain.insert(toKey(0, 0));
//    generateBlocks(0, 0);
}

void Terrain::checkTerrain(glm::vec3 pos) {
    int xFloor = static_cast<int>(glm::floor(pos.x / 64.f));
    int zFloor = static_cast<int>(glm::floor(pos.z / 64.f));
    for (int i = xFloor - TERRAIN_RADIUS; i < xFloor + TERRAIN_RADIUS + 1; i++) {
        for (int j = zFloor - TERRAIN_RADIUS; j < zFloor + TERRAIN_RADIUS + 1; j++) {
            if (m_generatedTerrain.find(toKey(i, j)) == m_generatedTerrain.end()) {
                m_generatedTerrain.insert(toKey(i, j));
                // create chunks
                for (int k = i*64; k < (i+1)*64; k += 16) {
                    for (int l = j*64; l < (j+1)*64; l +=16) {
                        blocktype_threads.push_back(std::thread(&Terrain::generateBlocks, this, k, l, std::ref(block_mutex)));
                    }
                }
            } else {
                for (int k = i*64; k < (i+1)*64; k += 16) {
                    for (int l = j*64; l < (j+1)*64; l +=16) {
                        if (hasChunkAt(k, l)) {
                            uPtr<Chunk> &c = getChunkAt(k, l);
                            if (!c->vbo_created) {
                                vbo_threads.push_back(std::thread(&Chunk::generateVBO, c.get(),
                                                                  std::ref(chunk_vbos), std::ref(vbo_mutex)));
                                c->vbo_created = true;
                            }
                        }
                    }
                }
            }
        }
    }
    vbo_mutex.lock();
    for (auto const& c:chunk_vbos) {
        c.chunk->bindBuffer(c.d, c.d_trans, c.idx, c.idx_trans);

    }
    chunk_vbos.clear();
    vbo_mutex.unlock();



}
