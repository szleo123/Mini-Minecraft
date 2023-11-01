#include "chunk.h"


Chunk::Chunk(int x, int z, OpenGLContext* context) : Drawable(context), m_blocks(), minX(x), minZ(z),
    m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}, vbo_created(false)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

std::vector<glm::vec4> Chunk::findFace(glm::ivec3 n) {
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

glm::vec3 Chunk::findColor(BlockType t) {
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
        default:
            // Other block types are not yet handled, so we default to debug purple
            return glm::vec3(1.f, 0.f, 1.f);
    }
}

std::vector<glm::vec4> Chunk::findUV(BlockType t, glm::ivec3 n) {
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
        default:
            results.push_back(glm::vec4(7.f, 2.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(7.f, 1.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(8.f, 1.f, 0, 0) / 16.f);
            results.push_back(glm::vec4(8.f, 2.f, 0, 0) / 16.f);
    }
    return results;
}

bool Chunk::checkBound(int x, int y, int z) {
    if (x < 0 || x > 15) {
        return false;
    } else if (y < 0 || y > 255) {
        return false;
    } else if (z < 0 || z > 15) {
        return false;
    }
    return true;
}

bool Chunk::checkNeighbor(int x, int y, int z, BlockType t) {
    if (t == WATER) return false;
    if (x == -1) {
        if (m_neighbors[XNEG]) {
            if (m_neighbors[XNEG]->getBlockAt(15, y, z) == EMPTY ||
                m_neighbors[XNEG]->getBlockAt(15, y, z) == WATER) return true;
            return false;
        } else {
            return true;
        }
    }
    if (x == 16) {
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
            if (m_neighbors[ZNEG]->getBlockAt(x, y, 15) == EMPTY ||
                m_neighbors[ZNEG]->getBlockAt(x, y, 15) == WATER) return true;
            return false;
        } else {
            return true;
        }
    }
    if (z == 16) {
        if (m_neighbors[ZPOS]) {
            if (m_neighbors[ZPOS]->getBlockAt(x, y, 0) == EMPTY ||
                m_neighbors[ZPOS]->getBlockAt(x, y, 0)  == WATER) return true;
            return false;
        } else {
            return true;
        }
    }
    if (y == -1 || y == 256) {
        return true;
    }
    return false;
}

bool Chunk::checkConidtions(int x, int y, int z, const glm::ivec3& n, BlockType t) {
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

void Chunk::bindBuffer(const std::vector<glm::vec4> &d, const std::vector<glm::vec4> &d_trans,
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

glm::vec2 Chunk::getMins() {
    return glm::vec2(minX, minZ);
}


void Chunk::createVBOdata() {
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

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                BlockType t = getBlockAt(x, y, z);
                if (t != EMPTY) {
                    for (auto const& n:neighbors) {
                        if (checkConidtions(x, y, z, n, t)) {
                            auto offsets = findFace(n);
                            auto UVs = findUV(t, n);
                            if (t == WATER && n.y != 1) continue; // ensure only draw the water level
                            if (t == WATER) {
                                for (int i = 0; i < 4; i++) {
                                    data_trans.push_back(glm::vec4(x+ this->minX, y, z + this->minZ, 0.) + offsets[i]);
                                    data_trans.push_back(glm::vec4(n, 1.));
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
                                    data.push_back(glm::vec4(x + this->minX, y, z + this->minZ, 0.) + offsets[i]);
                                    data.push_back(glm::vec4(n, 1.));
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
    vbo_created  = true;
    bindBuffer(data, data_trans, indices, indices_trans);
}

void Chunk::generateVBO(std::vector<ChunkVBOData> &vboData, std::mutex &mu) {
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

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                BlockType t = getBlockAt(x, y, z);
                if (t != EMPTY) {
                    for (auto const& n:neighbors) {
                        if (checkConidtions(x, y, z, n, t)) {
                            auto offsets = findFace(n);
                            auto UVs = findUV(t, n);
                            if (t == WATER && n.y != 1) continue; // ensure only draw the water level
                            if (t == WATER) {
                                for (int i = 0; i < 4; i++) {
                                    data_trans.push_back(glm::vec4(x+ this->minX, y, z + this->minZ, 0.) + offsets[i]);
                                    data_trans.push_back(glm::vec4(n, 1.));
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
                                    data.push_back(glm::vec4(x + this->minX, y, z + this->minZ, 0.) + offsets[i]);
                                    data.push_back(glm::vec4(n, 1.));
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
    ChunkVBOData storedData;
    storedData.chunk = this;
    storedData.d = data;
    storedData.d_trans = data_trans;
    storedData.idx = indices;
    storedData.idx_trans = indices_trans;

    mu.lock();
    vboData.push_back(storedData);
    mu.unlock();
}
