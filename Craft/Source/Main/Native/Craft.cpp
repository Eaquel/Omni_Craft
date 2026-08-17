#include "Craft.hpp"

static char gCrashLogDir[512] = {0};

static void nativeSignalHandler(int sig, siginfo_t* info, void*) {
    char filePath[600];
    time_t rawtime;
    time(&rawtime);
    struct tm* timeinfo = localtime(&rawtime);

    snprintf(filePath, sizeof(filePath), "%s/crash_native_%04d%02d%02d_%02d%02d%02d.log",
             gCrashLogDir[0] ? gCrashLogDir : "/sdcard/Documents/Craft_Log",
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    int fd = open(filePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd >= 0) {
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "=== OMNI CRAFT CRASH RAPORU ===\n"
                 "Sinyal: %d\n"
                 "Bellek: %p\n"
                 "Zaman: %04d-%02d-%02d %02d:%02d:%02d\n",
                 sig, info->si_addr,
                 timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        write(fd, buf, strlen(buf));
        close(fd);
    }
    LOGE("Kritik motor hatasi: %s", filePath);
    _exit(1);
}

static constexpr BlockDef BLOCK_TABLE[BLOCK_COUNT] = {
    {"hava",        false, true,  false, false, 0.0f, 0.6f, 0,  0,  0,  AIR},
    {"cimen",       true,  false, false, false, 0.6f, 0.6f, 0,  1,  2,  DIRT},
    {"toprak",      true,  false, false, false, 0.5f, 0.6f, 2,  2,  2,  DIRT},
    {"tas",         true,  false, false, false, 1.5f, 0.6f, 3,  3,  3,  COBBLESTONE},
    {"kiriktas",    true,  false, false, false, 2.0f, 0.6f, 4,  4,  4,  COBBLESTONE},
    {"kum",         true,  false, false, true,  0.5f, 0.4f, 13, 13, 13, SAND},
    {"cakil",       true,  false, false, true,  0.6f, 0.4f, 4,  4,  4,  GRAVEL},
    {"mese_kutuk",  true,  false, false, false, 2.0f, 0.6f, 7,  6,  7,  OAK_LOG},
    {"mese_yaprak", true,  true,  false, false, 0.2f, 0.6f, 8,  8,  8,  AIR},
    {"mese_tahta",  true,  false, false, false, 2.0f, 0.6f, 5,  5,  5,  OAK_PLANKS},
    {"ny_kutuk",    true,  false, false, false, 2.0f, 0.6f, 7,  6,  7,  BIRCH_LOG},
    {"ny_tahta",    true,  false, false, false, 2.0f, 0.6f, 5,  5,  5,  BIRCH_PLANKS},
    {"ny_yaprak",   true,  true,  false, false, 0.2f, 0.6f, 8,  8,  8,  AIR},
    {"cam_kutuk",   true,  false, false, false, 2.0f, 0.6f, 7,  6,  7,  SPRUCE_LOG},
    {"cam_tahta",   true,  false, false, false, 2.0f, 0.6f, 5,  5,  5,  SPRUCE_PLANKS},
    {"cam_yaprak",  true,  true,  false, false, 0.2f, 0.6f, 8,  8,  8,  AIR},
    {"komur_cevh",  true,  false, false, false, 3.0f, 0.6f, 10, 10, 10, ITEM_COAL},
    {"demir_cevh",  true,  false, false, false, 3.0f, 0.6f, 3,  3,  3,  IRON_ORE},
    {"altin_cevh",  true,  false, false, false, 3.0f, 0.6f, 3,  3,  3,  GOLD_ORE},
    {"elmas_cevh",  true,  false, false, false, 3.0f, 0.6f, 9,  9,  9,  ITEM_DIAMOND},
    {"zumrut_cevh", true,  false, false, false, 3.0f, 0.6f, 3,  3,  3,  EMERALD_ORE},
    {"kiziltas_cv", true,  false, false, false, 3.0f, 0.6f, 3,  3,  3,  REDSTONE_ORE},
    {"lapis_cevh",  true,  false, false, false, 3.0f, 0.6f, 3,  3,  3,  LAPIS_ORE},
    {"komur_blok",  true,  false, false, false, 5.0f, 0.6f, 10, 10, 10, COAL_BLOCK},
    {"demir_blok",  true,  false, false, false, 5.0f, 0.6f, 3,  3,  3,  IRON_BLOCK},
    {"altin_blok",  true,  false, false, false, 3.0f, 0.6f, 3,  3,  3,  GOLD_BLOCK},
    {"elmas_blok",  true,  false, false, false, 5.0f, 0.6f, 9,  9,  9,  DIAMOND_BLOCK},
    {"su",          false, true,  true,  false, 100.f,0.2f, 14, 14, 14, AIR},
    {"lav",         false, true,  true,  false, 100.f,0.2f, 14, 14, 14, AIR},
    {"cam",         true,  true,  false, false, 0.3f, 0.6f, 8,  8,  8,  AIR},
    {"isiktasi",    true,  true,  false, false, 0.3f, 0.6f, 13, 13, 13, GLOWSTONE},
    {"nether_tasi", true,  false, false, false, 0.4f, 0.6f, 4,  4,  4,  NETHERRACK},
    {"uretim_masa", true,  false, false, false, 2.5f, 0.6f, 12, 12, 5,  CRAFTING_TABLE},
    {"firin",       true,  false, false, false, 3.5f, 0.6f, 4,  4,  4,  FURNACE},
    {"sandik",      true,  false, false, false, 2.5f, 0.6f, 5,  5,  5,  CHEST},
    {"merdiven",    false, true,  false, false, 0.4f, 0.6f, 5,  5,  5,  LADDER},
    {"mesale",      false, true,  false, false, 0.0f, 0.6f, 15, 15, 15, TORCH},
    {"bedrock",     true,  false, false, false, -1.f, 0.6f, 11, 11, 11, BEDROCK},
    {"kar_katmani", true,  false, false, false, 0.2f, 0.3f, 13, 13, 13, SNOW_LAYER},
    {"buz",         true,  true,  false, false, 0.5f, 0.02f,14, 14, 14, AIR},
    {"kil",         true,  false, false, false, 0.6f, 0.6f, 2,  2,  2,  CLAY},
    {"tugla",       true,  false, false, false, 2.0f, 0.6f, 4,  4,  4,  BRICK},
    {"tas_tugla",   true,  false, false, false, 1.5f, 0.6f, 3,  3,  3,  STONE_BRICKS},
    {"yosun_tas",   true,  false, false, false, 2.0f, 0.6f, 4,  4,  4,  MOSSY_COBBLE},
    {"tnt",         true,  false, false, false, 0.0f, 0.6f, 0,  1,  2,  TNT},
    {"kitaplik",    true,  false, false, false, 1.5f, 0.6f, 5,  5,  5,  OAK_PLANKS},
    {"balkabagi",   true,  false, false, false, 1.0f, 0.6f, 13, 13, 13, PUMPKIN},
    {"karpuz",      true,  false, false, false, 1.0f, 0.6f, 0,  0,  0,  MELON},
    {"kumtasi",     true,  false, false, false, 0.8f, 0.6f, 13, 13, 13, SANDSTONE},
    {"kaktus",      true,  true,  false, false, 0.4f, 0.6f, 0,  0,  0,  CACTUS},
    {"sunger",      true,  false, false, false, 0.6f, 0.6f, 13, 13, 13, SPONGE}
};

static inline const BlockDef& BD(uint8_t id) { return BLOCK_TABLE[id < BLOCK_COUNT ? id : 0]; }
static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline int flr(float f) { return static_cast<int>(std::floor(f)); }

static inline float hash2D(int x, int z) {
    int n = x * 374761393 + z * 668265263;
    n = (n ^ (n >> 13)) * 1274126177;
    return static_cast<float>(n & 0x7fffffff) / static_cast<float>(0x7fffffff);
}

static inline float smoothNoise(float x, float z) {
    int ix = flr(x), iz = flr(z);
    float fx = x - ix, fz = z - iz;
    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uz = fz * fz * (3.0f - 2.0f * fz);

    float v00 = hash2D(ix, iz);
    float v10 = hash2D(ix + 1, iz);
    float v01 = hash2D(ix, iz + 1);
    float v11 = hash2D(ix + 1, iz + 1);

    return (v00 * (1.0f - ux) + v10 * ux) * (1.0f - uz) +
           (v01 * (1.0f - ux) + v11 * ux) * uz;
}

static inline float getTerrainHeight(float x, float z) {
    float h = 0.0f;
    h += smoothNoise(x * 0.015f, z * 0.015f) * 18.0f;
    h += smoothNoise(x * 0.05f,  z * 0.05f)  * 8.0f;
    return 30.0f + h;
}

// Standart OpenGL ES 3.2 Projeksiyon Matrisi (Column-Major)
static Mat4 matPerspective(float fovRad, float aspect, float nearZ, float farZ) {
    Mat4 res;
    float f = 1.0f / tanf(fovRad * 0.5f);
    res.m[0]  = f / aspect;
    res.m[5]  = f;
    res.m[10] = (farZ + nearZ) / (nearZ - farZ);
    res.m[11] = -1.0f;
    res.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    res.m[15] = 0.0f;
    return res;
}

// Standart OpenGL ES 3.2 Görünüm Matrisi (Column-Major)
static Mat4 matLookAt(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = (center - eye).norm();
    Vec3 r = f.cross(up).norm();
    Vec3 u = r.cross(f);

    Mat4 res;
    res.m[0] = r.x;
    res.m[1] = u.x;
    res.m[2] = -f.x;
    res.m[3] = 0.0f;

    res.m[4] = r.y;
    res.m[5] = u.y;
    res.m[6] = -f.y;
    res.m[7] = 0.0f;

    res.m[8] = r.z;
    res.m[9] = u.z;
    res.m[10] = -f.z;
    res.m[11] = 0.0f;

    res.m[12] = -(r.x * eye.x + r.y * eye.y + r.z * eye.z);
    res.m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    res.m[14] =  (f.x * eye.x + f.y * eye.y + f.z * eye.z);
    res.m[15] = 1.0f;
    return res;
}

bool Inventory::add(uint16_t id, int cnt) {
    for (int i = 0; i < INV_SIZE && cnt > 0; ++i) {
        if (slots[i].id == id && slots[i].count < MAX_STACK) {
            int a = std::min(cnt, static_cast<int>(MAX_STACK - slots[i].count));
            slots[i].count += a;
            cnt -= a;
        }
    }
    for (int i = 0; i < INV_SIZE && cnt > 0; ++i) {
        if (slots[i].empty()) {
            int a = std::min(cnt, MAX_STACK);
            slots[i] = ItemStack(id, a);
            cnt -= a;
        }
    }
    return cnt == 0;
}

Chunk::Chunk(int x, int z) : cx(x), cz(z) {
    blocks.fill(0);
}

Chunk::~Chunk() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
}

uint8_t Chunk::get(int x, int y, int z) const {
    if (x < 0 || x >= CS || y < 0 || y >= WH || z < 0 || z >= CS) return AIR;
    return blocks[idx(x, y, z)];
}

void Chunk::set(int x, int y, int z, uint8_t b) {
    if (x < 0 || x >= CS || y < 0 || y >= WH || z < 0 || z >= CS) return;
    blocks[idx(x, y, z)] = b;
    dirty = true;
}

void Chunk::buildMesh(const World& world) {
    std::vector<Vertex> verts;
    verts.reserve(4096);

    auto getBlock = [&](int lx, int ly, int lz) -> uint8_t {
        if (ly < 0 || ly >= WH) return AIR;
        if (lx >= 0 && lx < CS && lz >= 0 && lz < CS) return get(lx, ly, lz);
        return world.blockAt(cx * CS + lx, ly, cz * CS + lz);
    };

    auto addQuad = [&](int tile, float light,
                       float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2,
                       float x3, float y3, float z3) {
        float tx = (tile % 4) * 0.25f;
        float ty = (tile / 4) * 0.25f;
        float tw = 0.25f;

        // Tri 1: (0, 1, 2)
        verts.push_back({x0, y0, z0, tx,      ty+tw, light});
        verts.push_back({x1, y1, z1, tx+tw,   ty+tw, light});
        verts.push_back({x2, y2, z2, tx+tw,   ty,    light});

        // Tri 2: (0, 2, 3)
        verts.push_back({x0, y0, z0, tx,      ty+tw, light});
        verts.push_back({x2, y2, z2, tx+tw,   ty,    light});
        verts.push_back({x3, y3, z3, tx,      ty,    light});
    };

    for (int x = 0; x < CS; ++x) {
        for (int y = 0; y < WH; ++y) {
            for (int z = 0; z < CS; ++z) {
                uint8_t id = get(x, y, z);
                if (id == AIR) continue;

                float wx = static_cast<float>(cx * CS + x);
                float wy = static_cast<float>(y);
                float wz = static_cast<float>(cz * CS + z);

                // +Y (Üst Yüzey - CCW)
                uint8_t bTop = getBlock(x, y + 1, z);
                if (!BD(bTop).solid || (bTop == WATER && id != WATER)) {
                    addQuad(BD(id).topTile, 1.0f,
                            wx+0, wy+1, wz+0,
                            wx+0, wy+1, wz+1,
                            wx+1, wy+1, wz+1,
                            wx+1, wy+1, wz+0);
                }
                // -Y (Alt Yüzey - CCW)
                uint8_t bBot = getBlock(x, y - 1, z);
                if (!BD(bBot).solid) {
                    addQuad(BD(id).botTile, 0.5f,
                            wx+0, wy+0, wz+1,
                            wx+0, wy+0, wz+0,
                            wx+1, wy+0, wz+0,
                            wx+1, wy+0, wz+1);
                }
                // +Z (Ön Yüzey - CCW)
                uint8_t bFwd = getBlock(x, y, z + 1);
                if (!BD(bFwd).solid) {
                    addQuad(BD(id).sideTile, 0.85f,
                            wx+0, wy+0, wz+1,
                            wx+1, wy+0, wz+1,
                            wx+1, wy+1, wz+1,
                            wx+0, wy+1, wz+1);
                }
                // -Z (Arka Yüzey - CCW)
                uint8_t bBack = getBlock(x, y, z - 1);
                if (!BD(bBack).solid) {
                    addQuad(BD(id).sideTile, 0.85f,
                            wx+1, wy+0, wz+0,
                            wx+0, wy+0, wz+0,
                            wx+0, wy+1, wz+0,
                            wx+1, wy+1, wz+0);
                }
                // +X (Sağ Yüzey - CCW)
                uint8_t bRight = getBlock(x + 1, y, z);
                if (!BD(bRight).solid) {
                    addQuad(BD(id).sideTile, 0.70f,
                            wx+1, wy+0, wz+1,
                            wx+1, wy+0, wz+0,
                            wx+1, wy+1, wz+0,
                            wx+1, wy+1, wz+1);
                }
                // -X (Sol Yüzey - CCW)
                uint8_t bLeft = getBlock(x - 1, y, z);
                if (!BD(bLeft).solid) {
                    addQuad(BD(id).sideTile, 0.70f,
                            wx+0, wy+0, wz+0,
                            wx+0, wy+0, wz+1,
                            wx+0, wy+1, wz+1,
                            wx+0, wy+1, wz+0);
                }
            }
        }
    }

    vertexCount = verts.size();
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, light));

    glBindVertexArray(0);
    dirty = false;
}

void Chunk::render() {
    if (vertexCount > 0 && vao != 0) {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
}

std::shared_ptr<Chunk> World::getChunk(int cx, int cz) const {
    uint64_t k = key(cx, cz);
    std::shared_lock lk(chunkMtx);
    auto it = chunks.find(k);
    return it != chunks.end() ? it->second : nullptr;
}

std::shared_ptr<Chunk> World::getOrCreate(int cx, int cz) {
    uint64_t k = key(cx, cz);
    {
        std::shared_lock lk(chunkMtx);
        auto it = chunks.find(k);
        if (it != chunks.end()) return it->second;
    }
    auto c = std::make_shared<Chunk>(cx, cz);
    {
        std::unique_lock lk(chunkMtx);
        chunks[k] = c;
    }
    return c;
}

void World::generateChunk(Chunk& c) {
    int bx = c.cx * CS, bz = c.cz * CS;
    for (int x = 0; x < CS; ++x) {
        for (int z = 0; z < CS; ++z) {
            int wx = bx + x, wz = bz + z;
            c.set(x, 0, z, BEDROCK);

            int h = flr(getTerrainHeight(static_cast<float>(wx), static_cast<float>(wz)));
            if (h >= WH - 10) h = WH - 10;

            for (int y = 1; y < h - 3; ++y) {
                if (y < 16 && (wx * 13 + y * 7 + wz * 17) % 31 == 0) c.set(x, y, z, DIAMOND_ORE);
                else if (y < 35 && (wx * 11 + y * 5 + wz * 13) % 21 == 0) c.set(x, y, z, IRON_ORE);
                else if (y < 50 && (wx * 9 + y * 3 + wz * 7) % 15 == 0) c.set(x, y, z, COAL_ORE);
                else c.set(x, y, z, STONE);
            }

            if (h <= SEA_LEVEL + 1) {
                for (int y = std::max(1, h - 3); y <= h; ++y) c.set(x, y, z, SAND);
            } else {
                for (int y = std::max(1, h - 3); y < h; ++y) c.set(x, y, z, DIRT);
                c.set(x, h, z, GRASS);
            }

            for (int y = h + 1; y <= SEA_LEVEL; ++y) {
                c.set(x, y, z, WATER);
            }

            if (h > SEA_LEVEL + 1 && (hash2D(wx * 5, wz * 11) > 0.96f) && x >= 2 && x <= CS - 3 && z >= 2 && z <= CS - 3) {
                for (int ty = h + 1; ty <= h + 5; ++ty) c.set(x, ty, z, OAK_LOG);
                for (int lx = x - 2; lx <= x + 2; ++lx) {
                    for (int lz = z - 2; lz <= z + 2; ++lz) {
                        for (int ly = h + 3; ly <= h + 5; ++ly) {
                            if (c.get(lx, ly, lz) == AIR) c.set(lx, ly, lz, OAK_LEAVES);
                        }
                    }
                }
                c.set(x, h + 6, z, OAK_LEAVES);
            }
        }
    }

    if ((c.cx + c.cz) % 4 == 0) {
        int h = flr(getTerrainHeight(bx + 8.0f, bz + 8.0f));
        if (h > SEA_LEVEL) {
            spawnMob(ENT_PIG, {(float)bx + 8, (float)h + 1.2f, (float)bz + 8});
            spawnMob(ENT_COW, {(float)bx + 4, (float)h + 1.2f, (float)bz + 4});
        }
    }

    c.generated = true;
    c.dirty = true;
}

int World::getHighestBlock(int wx, int wz) const {
    for (int y = WH - 1; y >= 0; --y) {
        uint8_t b = blockAt(wx, y, wz);
        if (b != AIR && b != WATER) return y;
    }
    return SEA_LEVEL;
}

uint8_t World::blockAt(int wx, int wy, int wz) const {
    if (wy < 0) return BEDROCK;
    if (wy >= WH) return AIR;
    int cx = flr(static_cast<float>(wx) / CS), cz = flr(static_cast<float>(wz) / CS);
    int lx = ((wx % CS) + CS) % CS, lz = ((wz % CS) + CS) % CS;
    auto ch = getChunk(cx, cz);
    return ch ? ch->get(lx, wy, lz) : AIR;
}

void World::setBlock(int wx, int wy, int wz, uint8_t b) {
    if (wy < 0 || wy >= WH) return;
    int cx = flr(static_cast<float>(wx) / CS), cz = flr(static_cast<float>(wz) / CS);
    int lx = ((wx % CS) + CS) % CS, lz = ((wz % CS) + CS) % CS;
    auto ch = getChunk(cx, cz);
    if (ch) {
        ch->set(lx, wy, lz, b);
        if (lx == 0)  { auto c = getChunk(cx - 1, cz); if (c) c->dirty = true; }
        if (lx == 15) { auto c = getChunk(cx + 1, cz); if (c) c->dirty = true; }
        if (lz == 0)  { auto c = getChunk(cx, cz - 1); if (c) c->dirty = true; }
        if (lz == 15) { auto c = getChunk(cx, cz + 1); if (c) c->dirty = true; }
    }
}

void World::spawnMob(EntityType type, Vec3 pos) {
    std::lock_guard lk(entityMtx);
    Entity e;
    e.id = nextEntityId++;
    e.type = type;
    e.pos = pos;
    e.health = (type == ENT_COW ? 15.0f : 10.0f);
    entities.push_back(e);
}

void World::spawnItemDrop(uint16_t itemId, Vec3 pos, Vec3 vel) {
    std::lock_guard lk(entityMtx);
    Entity e;
    e.id = nextEntityId++;
    e.type = ENT_ITEM_DROP;
    e.pos = pos;
    e.vel = vel;
    e.itemId = itemId;
    entities.push_back(e);
}

void World::updateEntities(float dt) {
    std::lock_guard lk(entityMtx);
    for (auto& e : entities) {
        if (!e.alive) continue;
        e.animTime += dt;

        if (e.type == ENT_ITEM_DROP) {
            e.pos += e.vel * dt;
            e.vel.y += GRAVITY * dt * 0.5f;
            if (blockAt(flr(e.pos.x), flr(e.pos.y), flr(e.pos.z)) != AIR) {
                e.vel = {0, 0, 0};
                e.pos.y = flr(e.pos.y) + 1.05f;
            }
        } else {
            e.pos.y += GRAVITY * dt * 0.3f;
            uint8_t bBelow = blockAt(flr(e.pos.x), flr(e.pos.y - 0.2f), flr(e.pos.z));
            if (bBelow != AIR && BD(bBelow).solid) e.pos.y = flr(e.pos.y) + 1.0f;

            if (fmodf(e.animTime, 4.0f) < dt) {
                e.yaw = static_cast<float>(rand() % 360);
            }
            float rad = e.yaw * 0.01745329f;
            e.pos.x += sinf(rad) * 0.7f * dt;
            e.pos.z += cosf(rad) * 0.7f * dt;
        }
    }
}

World::RayHit World::raycast(Vec3 origin, Vec3 dir, float maxD) const {
    RayHit r{false};
    dir = dir.norm();
    int ix = flr(origin.x), iy = flr(origin.y), iz = flr(origin.z);
    float sX = dir.x > 0 ? 1.f : -1.f, sY = dir.y > 0 ? 1.f : -1.f, sZ = dir.z > 0 ? 1.f : -1.f;
    float tDX = fabsf(dir.x) < 1e-6f ? 1e30f : 1.0f / fabsf(dir.x);
    float tDY = fabsf(dir.y) < 1e-6f ? 1e30f : 1.0f / fabsf(dir.y);
    float tDZ = fabsf(dir.z) < 1e-6f ? 1e30f : 1.0f / fabsf(dir.z);
    float tX = fabsf(((dir.x > 0 ? (ix + 1) : ix) - origin.x) / (dir.x + 1e-8f));
    float tY = fabsf(((dir.y > 0 ? (iy + 1) : iy) - origin.y) / (dir.y + 1e-8f));
    float tZ = fabsf(((dir.z > 0 ? (iz + 1) : iz) - origin.z) / (dir.z + 1e-8f));
    Vec3i normal = {0, 1, 0};

    for (int s = 0; s < 120; ++s) {
        float t = std::min({tX, tY, tZ});
        if (t > maxD) break;
        uint8_t b = blockAt(ix, iy, iz);
        if (b != AIR && b != WATER && BD(b).solid) {
            r.hit = true; r.block = {ix, iy, iz}; r.dist = t;
            r.face = normal;
            return r;
        }
        if (tX < tY && tX < tZ) { tX += tDX; ix += (int)sX; normal = {-(int)sX, 0, 0}; }
        else if (tY < tZ)        { tY += tDY; iy += (int)sY; normal = {0, -(int)sY, 0}; }
        else                    { tZ += tDZ; iz += (int)sZ; normal = {0, 0, -(int)sZ}; }
    }
    return r;
}

void World::saveWorld(const std::string& path) {
    std::string file = path + "/world_save.dat";
    int fd = open(file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0) return;
    std::shared_lock lk(chunkMtx);
    uint32_t chunkCount = chunks.size();
    write(fd, &chunkCount, sizeof(chunkCount));
    for (auto& [k, ch] : chunks) {
        write(fd, &ch->cx, sizeof(ch->cx));
        write(fd, &ch->cz, sizeof(ch->cz));
        write(fd, ch->blocks.data(), ch->blocks.size());
    }
    close(fd);
    LOGI("Dunya kaydedildi: %s (%u chunk)", file.c_str(), chunkCount);
}

void World::loadWorld(const std::string& path) {
    std::string file = path + "/world_save.dat";
    int fd = open(file.c_str(), O_RDONLY);
    if (fd < 0) return;
    uint32_t chunkCount = 0;
    if (read(fd, &chunkCount, sizeof(chunkCount)) <= 0) { close(fd); return; }
    std::unique_lock lk(chunkMtx);
    chunks.clear();
    for (uint32_t i = 0; i < chunkCount; ++i) {
        int cx = 0, cz = 0;
        read(fd, &cx, sizeof(cx));
        read(fd, &cz, sizeof(cz));
        auto ch = std::make_shared<Chunk>(cx, cz);
        read(fd, ch->blocks.data(), ch->blocks.size());
        ch->generated = true;
        ch->dirty = true;
        chunks[key(cx, cz)] = ch;
    }
    close(fd);
    LOGI("Dunya yuklendi: %s (%u chunk)", file.c_str(), chunkCount);
}

void Renderer::generateProceduralAtlas() {
    const int AT_SZ = 64;
    std::vector<uint32_t> pixels(AT_SZ * AT_SZ);

    auto putTilePixel = [&](int tileIdx, int px, int py, uint32_t color) {
        int tx = (tileIdx % 4) * 16;
        int ty = (tileIdx / 4) * 16;
        pixels[(ty + py) * AT_SZ + (tx + px)] = color;
    };

    for (int t = 0; t < 16; ++t) {
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int noise = ((x * 7 + y * 13 + t * 19) % 20) - 10;
                uint32_t c = 0xFF888888;
                switch(t) {
                    case 0: // Grass Top
                        c = 0xFF000000 | ((70 + noise) << 16) | ((160 + noise) << 8) | (40 + noise);
                        break;
                    case 1: // Grass Side
                        if (y < 4) c = 0xFF000000 | ((70 + noise) << 16) | ((160 + noise) << 8) | (40 + noise);
                        else c = 0xFF000000 | ((90 + noise) << 16) | ((60 + noise) << 8) | (40 + noise);
                        break;
                    case 2: // Dirt
                        c = 0xFF000000 | ((90 + noise) << 16) | ((60 + noise) << 8) | (40 + noise);
                        break;
                    case 3: // Stone
                        c = 0xFF000000 | ((115 + noise) << 16) | ((115 + noise) << 8) | (115 + noise);
                        break;
                    case 4: // Cobblestone
                        noise = ((x * 11 ^ y * 17) % 35) - 17;
                        c = 0xFF000000 | ((100 + noise) << 16) | ((100 + noise) << 8) | (100 + noise);
                        break;
                    case 5: // Planks
                        c = 0xFF000000 | ((150 + (y%4==0?-25:noise)) << 16) | ((110 + noise) << 8) | (65 + noise);
                        break;
                    case 6: // Oak Log Side
                        c = 0xFF000000 | ((95 + (x%4==0?-20:noise)) << 16) | ((65 + noise) << 8) | (35 + noise);
                        break;
                    case 7: // Oak Log Top
                        c = 0xFF000000 | ((140 + noise) << 16) | ((110 + noise) << 8) | (70 + noise);
                        break;
                    case 8: // Leaves
                        c = ((x + y) % 2 == 0) ? 0x00000000 : (0xFF000000 | ((20 + noise) << 16) | ((120 + noise) << 8) | (20 + noise));
                        break;
                    case 9: // Diamond Ore
                        if ((x>=4 && x<=11 && y>=4 && y<=11) && (noise > 0)) c = 0xFFE0D030;
                        else c = 0xFF000000 | ((115 + noise) << 16) | ((115 + noise) << 8) | (115 + noise);
                        break;
                    case 10: // Coal Ore
                        if ((x>=4 && x<=11 && y>=4 && y<=11) && (noise > 0)) c = 0xFF222222;
                        else c = 0xFF000000 | ((115 + noise) << 16) | ((115 + noise) << 8) | (115 + noise);
                        break;
                    case 11: // Bedrock
                        noise = ((x * 31 + y * 47) % 60) - 30;
                        c = 0xFF000000 | ((40 + noise) << 16) | ((40 + noise) << 8) | (40 + noise);
                        break;
                    case 12: // Crafting Table
                        c = 0xFF000000 | ((160 + noise) << 16) | ((120 + noise) << 8) | (70 + noise);
                        break;
                    case 13: // Sand
                        c = 0xFF000000 | ((215 + noise) << 16) | ((205 + noise) << 8) | (145 + noise);
                        break;
                    case 14: // Water
                        c = 0xBB000000 | ((40 + noise) << 16) | ((80 + noise) << 8) | (210 + noise);
                        break;
                    case 15: // Torch
                        if (x>=6 && x<=9 && y>=4 && y<=15) c = 0xFF000000 | (120<<16) | (80<<8) | 40;
                        else if (x>=6 && x<=9 && y<=3) c = 0xFF00FFFF;
                        else c = 0x00000000;
                        break;
                    default:
                        c = 0xFF888888;
                }
                putTilePixel(t, x, y, c);
            }
        }
    }

    glGenTextures(1, &atlasTex);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, AT_SZ, AT_SZ, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static const float ENTITY_BOX_VERTS[] = {
    -0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f, 0.5f,-0.5f, 0,0,-1,
     0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f,-0.5f,-0.5f, 0,0,-1,
    -0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f, 0.5f, 0.5f, 0,0, 1,
     0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f,-0.5f, 0.5f, 0,0, 1,
    -0.5f, 0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f,-0.5f,-1,0, 0,
    -0.5f,-0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f, 0.5f,-1,0, 0,
     0.5f, 0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f,-0.5f, 1,0, 0,
     0.5f,-0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f, 0.5f, 1,0, 0,
    -0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f, 0.5f, 0,-1,0,
     0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f,-0.5f, 0,-1,0,
    -0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f, 0.5f, 0, 1,0,
     0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f,-0.5f, 0, 1,0
};

static const char* VS_WORLD = R"(#version 300 es
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in float aLight;
uniform mat4 uMVP;
out vec2 vUV;
out float vLight;
void main(){
    vUV = aUV;
    vLight = aLight;
    gl_Position = uMVP * vec4(aPos, 1.0);
})";

static const char* FS_WORLD = R"(#version 300 es
precision mediump float;
in vec2 vUV;
in float vLight;
uniform sampler2D uTexture;
out vec4 FragColor;
void main(){
    vec4 tex = texture(uTexture, vUV);
    if(tex.a < 0.1) discard;
    FragColor = vec4(tex.rgb * vLight, tex.a);
})";

static const char* VS_ENTITY = R"(#version 300 es
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
uniform mat4 uMVP;
out vec3 vNorm;
void main(){
    vNorm = aNorm;
    gl_Position = uMVP * vec4(aPos, 1.0);
})";

static const char* FS_ENTITY = R"(#version 300 es
precision mediump float;
in vec3 vNorm;
uniform vec3 uColor;
out vec4 FragColor;
void main(){
    float light = max(dot(vNorm, normalize(vec3(0.4, 0.8, 0.3))), 0.0) * 0.4 + 0.6;
    FragColor = vec4(uColor * light, 1.0);
})";

static GLuint compileShader(const char* vs, const char* fs) {
    GLuint v = glCreateShader(GL_VERTEX_SHADER); glShaderSource(v,1,&vs,nullptr); glCompileShader(v);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(f,1,&fs,nullptr); glCompileShader(f);
    GLuint p = glCreateProgram(); glAttachShader(p,v); glAttachShader(p,f); glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

void Renderer::init(int w, int h) {
    screenW = w; screenH = h;
    worldProg = compileShader(VS_WORLD, FS_WORLD);
    entityProg = compileShader(VS_ENTITY, FS_ENTITY);
    generateProceduralAtlas();

    glGenVertexArrays(1, &entityVAO);
    glGenBuffers(1, &entityVBO);
    glBindVertexArray(entityVAO);
    glBindBuffer(GL_ARRAY_BUFFER, entityVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ENTITY_BOX_VERTS), ENTITY_BOX_VERTS, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::resize(int w, int h) {
    screenW = w; screenH = h;
    glViewport(0, 0, w, h);
}

void Renderer::drawEntityBox(const Mat4& vp, Vec3 pos, Vec3 scale, Vec3 color, float yaw) {
    Mat4 model;
    float rad = yaw * 0.01745329f;
    model.m[0]  = scale.x * cosf(rad); model.m[2]  = scale.x * sinf(rad);
    model.m[5]  = scale.y;
    model.m[8]  = scale.z * -sinf(rad);model.m[10] = scale.z * cosf(rad);
    model.m[12] = pos.x; model.m[13] = pos.y; model.m[14] = pos.z; model.m[15] = 1.0f;
    Mat4 mvp = vp * model;

    glUseProgram(entityProg);
    glUniformMatrix4fv(glGetUniformLocation(entityProg, "uMVP"), 1, GL_FALSE, mvp.m);
    glUniform3f(glGetUniformLocation(entityProg, "uColor"), color.x, color.y, color.z);

    glBindVertexArray(entityVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Renderer::frame(World& world, Player& player) {
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Mat4 proj = matPerspective(1.222f, (float)screenW / (float)screenH, 0.1f, 350.0f);
    Vec3 eye = player.pos + Vec3{0, player.eyeH, 0};
    Mat4 view = matLookAt(eye, eye + player.lookDir(), {0, 1, 0});
    Mat4 vp = proj * view;

    glUseProgram(worldProg);
    glUniformMatrix4fv(glGetUniformLocation(worldProg, "uMVP"), 1, GL_FALSE, vp.m);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glUniform1i(glGetUniformLocation(worldProg, "uTexture"), 0);

    int pcx = flr(player.pos.x / CS), pcz = flr(player.pos.z / CS);
    for (int dx = -RD; dx <= RD; ++dx) {
        for (int dz = -RD; dz <= RD; ++dz) {
            auto ch = world.getChunk(pcx + dx, pcz + dz);
            if (ch && ch->generated) {
                if (ch->dirty) ch->buildMesh(world);
                ch->render();
            }
        }
    }

    std::lock_guard lk(world.entityMtx);
    for (const auto& e : world.entities) {
        if (!e.alive) continue;
        if (e.type == ENT_PIG) {
            drawEntityBox(vp, e.pos, {0.9f, 0.8f, 1.2f}, {0.95f, 0.65f, 0.65f}, e.yaw);
            drawEntityBox(vp, e.pos + Vec3{0, 0.5f, 0.5f}, {0.6f, 0.6f, 0.6f}, {0.95f, 0.65f, 0.65f}, e.yaw);
        } else if (e.type == ENT_COW) {
            drawEntityBox(vp, e.pos, {1.0f, 1.1f, 1.4f}, {0.45f, 0.30f, 0.15f}, e.yaw);
            drawEntityBox(vp, e.pos + Vec3{0, 0.7f, 0.6f}, {0.7f, 0.7f, 0.7f}, {0.35f, 0.20f, 0.10f}, e.yaw);
        } else if (e.type == ENT_SHEEP) {
            drawEntityBox(vp, e.pos, {1.0f, 1.0f, 1.3f}, {0.9f, 0.9f, 0.9f}, e.yaw);
            drawEntityBox(vp, e.pos + Vec3{0, 0.6f, 0.6f}, {0.6f, 0.6f, 0.6f}, {0.8f, 0.75f, 0.7f}, e.yaw);
        } else if (e.type == ENT_ITEM_DROP) {
            drawEntityBox(vp, e.pos, {0.35f, 0.35f, 0.35f}, {0.3f, 0.8f, 0.8f}, e.animTime * 90.0f);
        }
    }
}

static bool checkCollision(const World& world, Vec3 pos) {
    float r = 0.3f;
    float h = 1.8f;
    int minX = flr(pos.x - r), maxX = flr(pos.x + r);
    int minY = flr(pos.y),       maxY = flr(pos.y + h);
    int minZ = flr(pos.z - r), maxZ = flr(pos.z + r);

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                uint8_t b = world.blockAt(x, y, z);
                if (b != AIR && b != WATER && BD(b).solid) return true;
            }
        }
    }
    return false;
}

static GameState* gState = nullptr;

extern "C" {

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetupCrashHandler(JNIEnv* env, jclass, jstring logPath) {
    const char* str = env->GetStringUTFChars(logPath, nullptr);
    snprintf(gCrashLogDir, sizeof(gCrashLogDir), "%s", str);
    env->ReleaseStringUTFChars(logPath, str);

    struct sigaction sa{};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = nativeSignalHandler;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);
    sigaction(SIGILL,  &sa, nullptr);
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeInit(JNIEnv* env, jclass, jint w, jint h, jstring saveDir) {
    if (gState) { delete gState; gState = nullptr; }
    gState = new GameState();
    gState->screenW = w; gState->screenH = h;
    const char* s = env->GetStringUTFChars(saveDir, nullptr);
    gState->saveDirectory = s;
    env->ReleaseStringUTFChars(saveDir, s);

    gState->world = std::make_unique<World>();
    gState->world->worldPath = gState->saveDirectory;
    gState->renderer.init(w, h);

    for (int dx = -RD; dx <= RD; ++dx) {
        for (int dz = -RD; dz <= RD; ++dz) {
            auto ch = gState->world->getOrCreate(dx, dz);
            if (!ch->generated) gState->world->generateChunk(*ch);
        }
    }

    int spawnY = gState->world->getHighestBlock(0, 0);
    gState->player.pos = {0.5f, static_cast<float>(spawnY) + 1.2f, 0.5f};
    
    // Varsayılan Envanter Eşyaları
    gState->player.inv.slots[0] = ItemStack(OAK_PLANKS, 64);
    gState->player.inv.slots[1] = ItemStack(COBBLESTONE, 64);
    gState->player.inv.slots[2] = ItemStack(DIAMOND_ORE, 64);
    gState->player.inv.slots[3] = ItemStack(CRAFTING_TABLE, 16);
    gState->player.inv.slots[4] = ItemStack(TORCH, 64);
    gState->player.inv.slots[5] = ItemStack(GRASS, 64);
    gState->player.inv.slots[6] = ItemStack(DIRT, 64);
    gState->player.inv.slots[7] = ItemStack(OAK_LOG, 64);
    gState->player.inv.slots[8] = ItemStack(TNT, 64);

    gState->initialized = true;
    LOGI("Minecraft Engine Baslatildi. Spawn: [0, %d, 0]", spawnY);
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeResize(JNIEnv*, jclass, jint w, jint h) {
    if (gState) { gState->screenW = w; gState->screenH = h; gState->renderer.resize(w, h); }
}

JNIEXPORT jboolean JNICALL Java_com_omni_craft_Engine_nativeIsInitialized(JNIEnv*, jclass) {
    return gState && gState->initialized.load();
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeFrame(JNIEnv*, jclass, jfloat dt) {
    if (!gState || !gState->initialized) return;
    GameState& gs = *gState;
    float sdt = clampf(dt, 0.0f, 0.033f);
    gs.time += sdt;

    float yaw = gs.player.yaw * (3.14159265f / 180.0f);
    float spd = gs.player.sprinting ? SPRINT_SPD : (gs.player.sneaking ? SNEAK_SPD : WALK_SPD);
    
    float fwd = gs.inputZ * spd;
    float str = gs.inputX * spd;

    Vec3 moveDir = {
        sinf(yaw) * fwd + cosf(-yaw) * str,
        0.0f,
        -cosf(yaw) * fwd + sinf(-yaw) * str
    };

    gs.player.vel.x = moveDir.x;
    gs.player.vel.z = moveDir.z;
    gs.player.vel.y += GRAVITY * sdt;

    Vec3 p = gs.player.pos;
    p.x += gs.player.vel.x * sdt;
    if (checkCollision(*gs.world, p)) {
        p.x = gs.player.pos.x;
        gs.player.vel.x = 0;
    }

    p.z += gs.player.vel.z * sdt;
    if (checkCollision(*gs.world, p)) {
        p.z = gs.player.pos.z;
        gs.player.vel.z = 0;
    }

    p.y += gs.player.vel.y * sdt;
    if (checkCollision(*gs.world, p)) {
        if (gs.player.vel.y < 0) {
            gs.player.onGround = true;
        }
        p.y = gs.player.pos.y;
        gs.player.vel.y = 0;
    } else {
        gs.player.onGround = false;
    }

    gs.player.pos = p;

    {
        std::lock_guard lk(gs.world->entityMtx);
        for (auto& e : gs.world->entities) {
            if (e.alive && e.type == ENT_ITEM_DROP) {
                float dist = (e.pos - gs.player.pos).len();
                if (dist < 1.8f) {
                    gs.player.inv.add(e.itemId, 1);
                    e.alive = false;
                }
            }
        }
    }

    gs.world->updateEntities(sdt);
    gs.renderer.frame(*gs.world, gs.player);
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeInput(JNIEnv*, jclass, jfloat x, jfloat z) {
    if (gState) { gState->inputX = x; gState->inputZ = z; }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeCameraInput(JNIEnv*, jclass, jfloat dx, jfloat dy) {
    if (gState) {
        gState->player.yaw += dx;
        gState->player.pitch = clampf(gState->player.pitch + dy, -89.0f, 89.0f);
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeTap(JNIEnv*, jclass, jint type) {
    if (!gState) return;
    auto hit = gState->world->raycast(gState->player.pos + Vec3{0, 1.62f, 0}, gState->player.lookDir(), REACH);
    if (type == 0 && hit.hit) {
        uint8_t oldB = gState->world->blockAt(hit.block.x, hit.block.y, hit.block.z);
        gState->world->setBlock(hit.block.x, hit.block.y, hit.block.z, AIR);
        uint16_t drop = BD(oldB).dropId;
        if (drop != AIR) {
            gState->world->spawnItemDrop(drop, {(float)hit.block.x + 0.5f, (float)hit.block.y + 0.5f, (float)hit.block.z + 0.5f}, {0, 2.5f, 0});
        }
    } else if (type == 1 && hit.hit) {
        ItemStack& held = gState->player.inv.active();
        if (!held.empty() && held.id < BLOCK_COUNT) {
            Vec3i target = {hit.block.x + hit.face.x, hit.block.y + hit.face.y, hit.block.z + hit.face.z};
            Vec3 bCenter = {(float)target.x + 0.5f, (float)target.y + 0.5f, (float)target.z + 0.5f};
            if ((bCenter - (gState->player.pos + Vec3{0, 0.9f, 0})).len() > 0.8f) {
                gState->world->setBlock(target.x, target.y, target.z, (uint8_t)held.id);
                held.count--;
                if (held.count == 0) held = ItemStack();
            }
        }
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeJump(JNIEnv*, jclass) {
    if (gState && gState->player.onGround) {
        gState->player.vel.y = JUMP_VEL;
        gState->player.onGround = false;
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSneak(JNIEnv*, jclass, jboolean on) {
    if (gState) gState->player.sneaking = on;
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSprint(JNIEnv*, jclass, jboolean on) {
    if (gState) gState->player.sprinting = on;
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSelectSlot(JNIEnv*, jclass, jint slot) {
    if (gState) gState->player.inv.selected = slot % HOTBAR_SZ;
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSaveWorld(JNIEnv*, jclass) {
    if (gState && gState->world) gState->world->saveWorld(gState->saveDirectory);
}

JNIEXPORT jstring JNICALL Java_com_omni_craft_Engine_nativeGetInventory(JNIEnv* env, jclass) {
    if (!gState) return env->NewStringUTF("[]");
    std::string json = "[";
    for (int i = 0; i < INV_SIZE; ++i) {
        if (i > 0) json += ",";
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"id\":%d,\"count\":%d}", gState->player.inv.slots[i].id, gState->player.inv.slots[i].count);
        json += buf;
    }
    json += "]";
    return env->NewStringUTF(json.c_str());
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeDestroy(JNIEnv*, jclass) {
    if (gState) {
        if (gState->world) gState->world->saveWorld(gState->saveDirectory);
        gState->initialized = false;
        delete gState;
        gState = nullptr;
    }
}

} // extern "C"
