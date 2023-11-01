#include "planet.h"

Planet_Chunk::Planet_Chunk(int x, int y, int z, glm::vec3 center, OpenGLContext* context): Drawable(context),
    minX(x), minY(y), minZ(z), center(center), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}, {YPOS, nullptr}, {YNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 262144, EMPTY);
}

// Does bounds checking with at()
BlockType Planet_Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 64 * y + 64 * 64 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Planet_Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Planet_Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 64 * y + 64 * 64 * z) = t;
}

const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Planet_Chunk::linkNeighbor(uPtr<Planet_Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

std::vector<glm::vec4> Planet_Chunk::findFace(glm::ivec3 n) {
    std::vector<glm::vec4> result;
    if (n.x == 1) {
        result.push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        result.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        result.push_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        result.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    if (n.x == -1) {
        result.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    }
    if (n.y == 1) {
        result.push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        result.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    }
    if (n.y == -1) {
        result.push_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        result.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    }
    if (n.z == 1) {
        result.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        result.push_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
    }
    if (n.z == -1) {
        result.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        result.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        result.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        result.push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    }
    return result;
}

glm::vec3 Planet_Chunk::findColor(BlockType t) {
    switch(t) {
        case GRASS:
            return glm::vec3(95.f, 159.f, 53.f) / 255.f;
        case DIRT:
            return glm::vec3(121.f, 85.f, 58.f) / 255.f;
        case STONE:
            return glm::vec3(0.5f);
        case WATER:
            return glm::vec3(0.f, 0.f, 0.75f);
        case SNOW:
            return glm::vec3(1.f, 1.f, 1.f);
        case LAVA:
            return glm::vec3(207.f, 16.f, 32.f) / 255.f;
        default:
            // Other block types are not yet handled, so we default to debug purple
            return glm::vec3(1.f, 0.f, 1.f);
    }
}

std::vector<glm::vec4> Planet_Chunk::findUV(BlockType t, glm::ivec3 n) {
    std::vector<glm::vec4> results;
    switch(t) {
        case GRASS:
            if (n.y == 1) {
                results.push_back(glm::vec4(8.f, 14.f, 0, 0) / 16.f);
                results.push_back(glm::vec4(8.f, 13.f, 0, 0) / 16.f);
                results.push_back(glm::vec4(9.f, 13.f, 0, 0) / 16.f);
                results.push_back(glm::vec4(9.f, 14.f, 0, 0) / 16.f);
            }
            else if (n.y == -1) {
                results.push_back(glm::vec4(2.f, 16.f, 0, 0) / 16.f);
                results.push_back(glm::vec4(2.f, 15.f, 0, 0) / 16.f);
                results.push_back(glm::vec4(3.f, 15.f, 0, 0) / 16.f);
                results.push_back(glm::vec4(3.f, 16.f, 0, 0) / 16.f);
            }
            else {
                results.push_back(glm::vec4(3.f, 16.f, 0, 0) / 16.f);
                results.push_back(glm::vec4(3.f, 15.f, 0, 0) / 16.f);
                results.push_back(glm::vec4(4.f, 15.f, 0, 0) / 16.f);
                results.push_back(glm::vec4(4.f, 16.f, 0, 0) / 16.f);
            }
        case DIRT:
            results.push_back(glm::vec4(3.f, 16.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(3.f, 15.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(4.f, 15.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(4.f, 16.f, 0, 0) / 16.f);
        case STONE:
            results.push_back(glm::vec4(1.f, 16.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(1.f, 15.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(2.f, 15.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(2.f, 16.f, 0, 0) / 16.f);
        case WATER:
            results.push_back(glm::vec4(13.f, 4.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(13.f, 3.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(14.f, 3.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(14.f, 4.f, 0, 0) / 16.f);
        case SNOW:
            results.push_back(glm::vec4(2.f, 12.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(2.f, 11.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(3.f, 11.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(3.f, 12.f, 0, 0) / 16.f);
        case LAVA:
            results.push_back(glm::vec4(13.f, 2.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(13.f, 1.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(14.f, 1.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(14.f, 2.f, 0, 0) / 16.f);
        default:
            results.push_back(glm::vec4(7.f, 2.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(7.f, 1.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(8.f, 1.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(8.f, 2.f, 0, 0) / 16.f);
    }
    return results;
}

bool Planet_Chunk::checkBound(int x, int y, int z) {
    if (x < 0 || x > 63) {
        return false;
    } else if (y < 0 || y > 63) {
        return false;
    } else if (z < 0 || z > 63) {
        return false;
    }
    return true;
}

bool Planet_Chunk::checkNeighbor(int x, int y, int z, BlockType t) {
    if (t == WATER) return false;
    if (x == -1) {
        if (m_neighbors[XNEG]) {
            if (m_neighbors[XNEG]->getBlockAt(63, y, z) == EMPTY ||
                m_neighbors[XNEG]->getBlockAt(63, y, z) == WATER) return true;
            return false;
        } else {
            return true;
        }
    }
    if (x == 64) {
        if (m_neighbors[XPOS]) {
            if (m_neighbors[XPOS]->getBlockAt(0, y, z) == EMPTY ||
                m_neighbors[XPOS]->getBlockAt(0, y, z) == WATER) return true;
            return false;
        } else {
            return true;
        }
    }
    if (z == -1) {
        if (m_neighbors[ZNEG]) {
            if (m_neighbors[ZNEG]->getBlockAt(x, y, 63) == EMPTY ||
                m_neighbors[ZNEG]->getBlockAt(x, y, 63) == WATER) return true;
            return false;
        } else {
            return true;
        }
    }
    if (z == 64) {
        if (m_neighbors[ZPOS]) {
            if (m_neighbors[ZPOS]->getBlockAt(x, y, 0) == EMPTY ||
                m_neighbors[ZPOS]->getBlockAt(x, y, 0)  == WATER) return true;
            return false;
        } else {
            return true;
        }
    }
    if (y == -1) {
        if (m_neighbors[YNEG]) {
            if (m_neighbors[YNEG]->getBlockAt(x, 63, z) == EMPTY ||
                m_neighbors[YNEG]->getBlockAt(x, 63, z) == WATER) return true;
            return false;
        } else {
            return true;
        }
    }
    if (y == 64) {
        if (m_neighbors[YPOS]) {
            if (m_neighbors[YPOS]->getBlockAt(x, 0, z) == EMPTY ||
                m_neighbors[YPOS]->getBlockAt(x, 0, z) == WATER) return true;
            return false;
        } else {
            return true;
        }
    }
    return false;
}

bool Planet_Chunk::checkConidtions(int x, int y, int z, const glm::ivec3& n, BlockType t) {
    if (checkNeighbor(x+n.x, y+n.y, z+n.z, t)) {
        return true;
    } else if (checkBound(x+n.x, y+n.y, z+n.z)) {
        if (getBlockAt(x+n.x, y+n.y, z+n.z) == EMPTY) {
            return true;
        } else if (t != WATER && getBlockAt(x+n.x, y+n.y, z+n.z) == WATER) {
            return true;
        }
    }
    return false;
}

void Planet_Chunk::bindBuffer(const std::vector<glm::vec4> &d, const std::vector<glm::vec4> &d_trans,
                       const std::vector<GLuint> &i, const std::vector<GLuint> &i_trans) {
    m_count = i.size();
    m_count_trans = i_trans.size();
    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint), i.data(), GL_STATIC_DRAW);

    generateIdxTrans();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTrans);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count_trans * sizeof(GLuint), i_trans.data(), GL_STATIC_DRAW);

    generateInter();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInter);
    mp_context->glBufferData(GL_ARRAY_BUFFER, d.size() * sizeof(glm::vec4), d.data(), GL_STATIC_DRAW);

    generateInterTrans();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterTrans);
    mp_context->glBufferData(GL_ARRAY_BUFFER, d_trans.size() * sizeof(glm::vec4), d_trans.data(), GL_STATIC_DRAW);

    buffer_created = true;
}

void Planet_Chunk::createVBOdata() {
    std::vector<glm::vec4> data;
    std::vector<glm::vec4> data_trans;
    std::vector<GLuint> indices;
    std::vector<GLuint> indices_trans;
    int curSize = 0;
    int curSize_trans = 0;
    std::vector<glm::ivec3> neighbors = {glm::ivec3(1, 0, 0),
                                        glm::ivec3(-1, 0, 0),
                                        glm::ivec3(0, 1, 0),
                                        glm::ivec3(0, -1, 0),
                                        glm::ivec3(0, 0, 1),
                                        glm::ivec3(0, 0, -1)};

    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            for (int z = 0; z < 64; z++) {
                BlockType t = getBlockAt(x, y, z);
                if (t != EMPTY) {
                    for (auto const& n:neighbors) {
                        if (checkConidtions(x, y, z, n, t)) {
                            auto offsets = findFace(n);
                            auto UVs = findUV(t, n);
                            if (t == WATER && n.y != 1) continue; // ensure only draw the water level
                            if (t == WATER) {
                                for (int i = 0; i < 4; i++) {
                                    data_trans.push_back(glm::vec4(x+ this->minX + center.x, y + this->minY + center.y, z + this->minZ + center.z, 0.) + offsets[i]);
                                    data.push_back(glm::vec4(-glm::normalize(glm::vec3(x + this->minX, y + this->minY, z + this->minZ)), 1));
                                    data_trans.push_back(glm::vec4(findColor(t), 1.));
                                    data_trans.push_back(UVs[i]);
                                }
                                indices_trans.push_back(curSize_trans);
                                indices_trans.push_back(curSize_trans + 1);
                                indices_trans.push_back(curSize_trans + 2);
                                indices_trans.push_back(curSize_trans);
                                indices_trans.push_back(curSize_trans + 2);
                                indices_trans.push_back(curSize_trans + 3);
                                curSize_trans += 4;
                            } else {
                                for (int i = 0; i < 4; i++) {
                                    data.push_back(glm::vec4(x + this->minX + center.x, y + this->minY + center.y, z + this->minZ + center.z, 0.) + offsets[i]);
                                    data.push_back(glm::vec4(-glm::normalize(glm::vec3(x + this->minX, y + this->minY, z + this->minZ)), 1));
                                    data.push_back(glm::vec4(findColor(t), 1.));
                                    data.push_back(UVs[i]);
                                }
                                indices.push_back(curSize);
                                indices.push_back(curSize + 1);
                                indices.push_back(curSize + 2);
                                indices.push_back(curSize);
                                indices.push_back(curSize + 2);
                                indices.push_back(curSize + 3);
                                curSize += 4;
                            }
                        } else {
                            continue;
                        }
                    }
                } else {
                    continue;
                }
            }
        }
    }

    bindBuffer(data, data_trans, indices, indices_trans);
    vbo_created = true;
    buffer_created = true;
//    PlanetVBOData storedData;
//    storedData.chunk = this;
//    storedData.d = data;
//    storedData.d_trans = data_trans;
//    storedData.idx = indices;
//    storedData.idx_trans = indices_trans;

//    mu.lock();
//    vboData.push_back(storedData);
//    mu.unlock();
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/// The following are planet class

uint32_t toKey_planet(int x, int y , int z) {
    uint32_t hash_val = ((x & 0xFF) << 16) | ((y & 0xFF) << 8) | (z & 0xFF);
    return hash_val;
}

glm::ivec3 toCoords_planet(uint32_t co) {
    int32_t x = static_cast<int32_t>((co >> 16) & 0xFF);
    int32_t y = static_cast<int32_t>((co >> 8) & 0xFF);
    int32_t z = static_cast<int32_t>(co & 0xFF);
    return glm::ivec3(x, y, z);
}

Planet::Planet(OpenGLContext *context, glm::vec3 center, int radius)
    :m_chunks(), m_generatedTerrain(), mp_context(context),
     center(center), radius(radius)
{}

Planet::~Planet()
{}

bool Planet::hasChunkAt(int x, int y, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 64.f));
    int yFloor = static_cast<int>(glm::floor(y / 64.f));
    int zFloor = static_cast<int>(glm::floor(z / 64.f));
    return m_chunks.find(toKey_planet(64 * xFloor, 64 * yFloor, 64 * zFloor)) != m_chunks.end();
}

uPtr<Planet_Chunk>& Planet::getChunkAt(int x, int y, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 64.f));
    int yFloor = static_cast<int>(glm::floor(y / 64.f));
    int zFloor = static_cast<int>(glm::floor(z / 64.f));
    return m_chunks[toKey_planet(64 * xFloor, 64 * yFloor, 64 * zFloor)];
}

const uPtr<Planet_Chunk>& Planet::getChunkAt(int x, int y, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 64.f));
    int yFloor = static_cast<int>(glm::floor(y / 64.f));
    int zFloor = static_cast<int>(glm::floor(z / 64.f));
    return m_chunks.at(toKey_planet(64 * xFloor, 64 * yFloor, 64 * zFloor));
}


BlockType Planet::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, y, z)) {
        const uPtr<Planet_Chunk> &c = getChunkAt(x, y, z);
        glm::vec3 chunkOrigin = glm::vec3(floor(x / 64.f) * 64, floor(y / 64.f) * 64, floor(z / 64.f) * 64);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y - chunkOrigin.y),
                             static_cast<unsigned int>(z - chunkOrigin.z));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Planet::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, y, z)) {
        uPtr<Planet_Chunk> &c = getChunkAt(x, y, z);
        glm::vec3 chunkOrigin = glm::vec3(floor(x / 64.f) * 64, floor(y / 64.f) * 64, floor(z / 64.f) * 64);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y - chunkOrigin.y),
                      static_cast<unsigned int>(z - chunkOrigin.z),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Planet_Chunk* Planet::instantiateChunkAt(int x, int y, int z) {
    uPtr<Planet_Chunk> chunk = mkU<Planet_Chunk>(x, y, z, center, mp_context);
    Planet_Chunk *cPtr = chunk.get();
    m_chunks[toKey_planet(x, y, z)] = std::move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, y, z + 64)) {
        auto &chunkNorth = m_chunks[toKey_planet(x, y, z + 64)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, y, z - 64)) {
        auto &chunkSouth = m_chunks[toKey_planet(x, y, z - 64)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 64, y, z)) {
        auto &chunkEast = m_chunks[toKey_planet(x + 64, y, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 64, y, z)) {
        auto &chunkWest = m_chunks[toKey_planet(x - 64, y, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    if(hasChunkAt(x, y + 64, z)) {
        auto &chunkUp = m_chunks[toKey_planet(x, y + 64, z)];
        cPtr->linkNeighbor(chunkUp, YPOS);
    }
    if(hasChunkAt(x, y - 64, z)) {
        auto &chunkDown = m_chunks[toKey_planet(x, y - 64, z)];
        cPtr->linkNeighbor(chunkDown, YNEG);
    }
    return cPtr;
}

void Planet::createPlanet() {
    for(int x = -radius; x < radius; x += 64) {
        for(int y = -radius; y < radius; y += 64) {
            for (int z = -radius; z < radius; z += 64){
                instantiateChunkAt(x, y, z);
            }
        }
    }

    for(int x = -radius; x < radius; x++) {
        for(int y = -radius; y < radius; y++) {
            for (int z = -radius; z < radius; z++){
                if (glm::abs(glm::length(glm::vec3(x, y, z))) < radius) {
                    setBlockAt(x, y, z, LAVA);
                }
            }
        }
    }

    for (const auto &[key, chunk] : m_chunks) {
        chunk->createVBOdata();
    }
}

void Planet::draw(ShaderProgram *shaderProgram) {
    shaderProgram->setModelMatrix(glm::mat4());
    for (const auto &[key, chunk] : m_chunks) {
        shaderProgram->draw(*(chunk.get()), false);
    }
}

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

void Planet::move(float time) {
    float v = ((int)time % 1000) / 1000.f * TWO_PI;
    this->center = glm::vec3(800. * glm::cos(v), 800. * glm::sin(v), 0.);
}
