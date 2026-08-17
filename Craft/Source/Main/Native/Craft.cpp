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
                 "=== OMNI CRAFT AAA CRASH HANDLER ===\n"
                 "Signal: %d, Fault Addr: %p\n"
                 "Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                 sig, info->si_addr,
                 timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        write(fd, buf, strlen(buf));
        close(fd);
    }
    LOGE("Yerel motor çökmesi yakalandı: %s", filePath);
    _exit(1);
}

static inline uint32_t makeRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (static_cast<uint32_t>(a) << 24) |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8)  |
            static_cast<uint32_t>(r);
}

static constexpr BlockDef BLOCK_TABLE[BLOCK_COUNT] = {
    {"hava",        false, true,  false, 0.0f, 0,  0,  0,  AIR},
    {"cimen",       true,  false, false, 0.6f, 0,  1,  2,  DIRT},
    {"toprak",      true,  false, false, 0.5f, 2,  2,  2,  DIRT},
    {"tas",         true,  false, false, 1.5f, 3,  3,  3,  COBBLESTONE},
    {"kiriktas",    true,  false, false, 2.0f, 4,  4,  4,  COBBLESTONE},
    {"kum",         true,  false, false, 0.5f, 13, 13, 13, SAND},
    {"cakil",       true,  false, false, 0.6f, 4,  4,  4,  GRAVEL},
    {"mese_kutuk",  true,  false, false, 2.0f, 7,  6,  7,  OAK_LOG},
    {"mese_yaprak", true,  true,  false, 0.2f, 8,  8,  8,  AIR},
    {"mese_tahta",  true,  false, false, 2.0f, 5,  5,  5,  OAK_PLANKS},
    {"ny_kutuk",    true,  false, false, 2.0f, 7,  6,  7,  BIRCH_LOG},
    {"ny_tahta",    true,  false, false, 2.0f, 5,  5,  5,  BIRCH_PLANKS},
    {"ny_yaprak",   true,  true,  false, 0.2f, 8,  8,  8,  AIR},
    {"cam_kutuk",   true,  false, false, 2.0f, 7,  6,  7,  SPRUCE_LOG},
    {"cam_tahta",   true,  false, false, 2.0f, 5,  5,  5,  SPRUCE_PLANKS},
    {"cam_yaprak",  true,  true,  false, 0.2f, 8,  8,  8,  AIR},
    {"komur_cevh",  true,  false, false, 3.0f, 10, 10, 10, ITEM_COAL},
    {"demir_cevh",  true,  false, false, 3.0f, 3,  3,  3,  IRON_ORE},
    {"altin_cevh",  true,  false, false, 3.0f, 3,  3,  3,  GOLD_ORE},
    {"elmas_cevh",  true,  false, false, 3.0f, 9,  9,  9,  ITEM_DIAMOND},
    {"zumrut_cevh", true,  false, false, 3.0f, 3,  3,  3,  EMERALD_ORE},
    {"kiziltas_cv", true,  false, false, 3.0f, 3,  3,  3,  REDSTONE_ORE},
    {"lapis_cevh",  true,  false, false, 3.0f, 3,  3,  3,  LAPIS_ORE},
    {"komur_blok",  true,  false, false, 5.0f, 10, 10, 10, COAL_BLOCK},
    {"demir_blok",  true,  false, false, 5.0f, 3,  3,  3,  IRON_BLOCK},
    {"altin_blok",  true,  false, false, 3.0f, 3,  3,  3,  GOLD_BLOCK},
    {"elmas_blok",  true,  false, false, 5.0f, 9,  9,  9,  DIAMOND_BLOCK},
    {"su",          false, true,  true,  100.f,14, 14, 14, AIR},
    {"lav",         false, true,  true,  100.f,14, 14, 14, AIR},
    {"cam",         true,  true,  false, 0.3f, 8,  8,  8,  AIR},
    {"isiktasi",    true,  true,  false, 0.3f, 13, 13, 13, GLOWSTONE},
    {"nether_tasi", true,  false, false, 0.4f, 4,  4,  4,  NETHERRACK},
    {"uretim_masa", true,  false, false, 2.5f, 12, 12, 5,  CRAFTING_TABLE},
    {"firin",       true,  false, false, 3.5f, 4,  4,  4,  FURNACE},
    {"sandik",      true,  false, false, 2.5f, 5,  5,  5,  CHEST},
    {"merdiven",    false, true,  false, 0.4f, 5,  5,  5,  LADDER},
    {"mesale",      false, true,  false, 0.0f, 5,  5,  5,  TORCH},
    {"bedrock",     true,  false, false, -1.f, 11, 11, 11, BEDROCK},
    {"kar_katmani", true,  false, false, 0.2f, 13, 13, 13, SNOW_LAYER},
    {"buz",         true,  true,  false, 0.5f, 14, 14, 14, AIR},
    {"kil",         true,  false, false, 0.6f, 2,  2,  2,  CLAY},
    {"tugla",       true,  false, false, 2.0f, 4,  4,  4,  BRICK},
    {"tas_tugla",   true,  false, false, 1.5f, 3,  3,  3,  STONE_BRICKS},
    {"yosun_tas",   true,  false, false, 2.0f, 4,  4,  4,  MOSSY_COBBLE},
    {"tnt",         true,  false, false, 0.0f, 0,  1,  2,  TNT},
    {"kitaplik",    true,  false, false, 1.5f, 5,  5,  5,  OAK_PLANKS},
    {"balkabagi",   true,  false, false, 1.0f, 13, 13, 13, PUMPKIN},
    {"karpuz",      true,  false, false, 1.0f, 0,  0,  0,  MELON},
    {"kumtasi",     true,  false, false, 0.8f, 13, 13, 13, SANDSTONE},
    {"kaktus",      true,  true,  false, 0.4f, 0,  0,  0,  CACTUS},
    {"sunger",      true,  false, false, 0.6f, 13, 13, 13, SPONGE}
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

static uint32_t gWorldSeed = 0x5EED1234u;

static inline float getTerrainHeight(float x, float z) {
    const float sx = static_cast<float>((gWorldSeed & 0xFFFFu)) * 0.00001f;
    const float sz = static_cast<float>((gWorldSeed >> 16) & 0xFFFFu) * 0.00001f;
    return 30.0f + smoothNoise(x * 0.012f + sx, z * 0.012f + sz) * 22.0f +
                   smoothNoise(x * 0.045f - sz, z * 0.045f + sx) * 9.0f;
}

static Vec3 getRayDirection(const Player& player, float screenX, float screenY, int screenW, int screenH) {
    float aspect = static_cast<float>(screenW) / static_cast<float>(screenH);
    float fovRad = player.fov * 0.01745329252f;
    float halfFovTan = tanf(fovRad * 0.5f);

    float ndcX = (2.0f * screenX / static_cast<float>(screenW)) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY / static_cast<float>(screenH));

    Vec3 fwd = player.lookDir().norm();
    Vec3 upWorld = {0.0f, 1.0f, 0.0f};
    Vec3 right = fwd.cross(upWorld).norm();
    Vec3 up = right.cross(fwd).norm();

    return (fwd + right * (ndcX * aspect * halfFovTan) + up * (ndcY * halfFovTan)).norm();
}

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

static Mat4 matLookAt(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = (center - eye).norm();
    Vec3 r = f.cross(up).norm();
    Vec3 u = r.cross(f);

    Mat4 res;
    res.m[0] = r.x; res.m[1] = u.x; res.m[2] = -f.x;
    res.m[4] = r.y; res.m[5] = u.y; res.m[6] = -f.y;
    res.m[8] = r.z; res.m[9] = u.z; res.m[10]= -f.z;
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
    if (waterVao) glDeleteVertexArrays(1, &waterVao);
    if (waterVbo) glDeleteBuffers(1, &waterVbo);
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

static inline float calcAO(bool side1, bool side2, bool corner) {
    if (side1 && side2) return 0.35f;
    int count = (side1 ? 1 : 0) + (side2 ? 1 : 0) + (corner ? 1 : 0);
    if (count == 2) return 0.55f;
    if (count == 1) return 0.78f;
    return 1.0f;
}

void Chunk::buildMesh(const World& world) {
    std::vector<Vertex> opaqueVerts;
    std::vector<Vertex> waterVerts;
    opaqueVerts.reserve(6144);
    waterVerts.reserve(1024);

    auto getBlock = [&](int lx, int ly, int lz) -> uint8_t {
        if (ly < 0 || ly >= WH) return AIR;
        if (lx >= 0 && lx < CS && lz >= 0 && lz < CS) return get(lx, ly, lz);
        return world.blockAt(cx * CS + lx, ly, cz * CS + lz);
    };

    auto isSolid = [&](int lx, int ly, int lz) -> bool {
        uint8_t b = getBlock(lx, ly, lz);
        return b != AIR && b != WATER && BD(b).solid;
    };

    auto addQuad = [&](std::vector<Vertex>& target, int tile, float baseLight,
                       float ao0, float ao1, float ao2, float ao3,
                       float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2,
                       float x3, float y3, float z3) {
        float tx = (tile % 4) * 0.25f;
        float ty = (tile / 4) * 0.25f;
        float tw = 0.25f;
        float tId = static_cast<float>(tile);

        target.push_back({x0, y0, z0, tx,      ty+tw, baseLight, ao0, tId});
        target.push_back({x1, y1, z1, tx+tw,   ty+tw, baseLight, ao1, tId});
        target.push_back({x2, y2, z2, tx+tw,   ty,    baseLight, ao2, tId});

        target.push_back({x0, y0, z0, tx,      ty+tw, baseLight, ao0, tId});
        target.push_back({x2, y2, z2, tx+tw,   ty,    baseLight, ao2, tId});
        target.push_back({x3, y3, z3, tx,      ty,    baseLight, ao3, tId});
    };

    for (int x = 0; x < CS; ++x) {
        for (int y = 0; y < WH; ++y) {
            for (int z = 0; z < CS; ++z) {
                uint8_t id = get(x, y, z);
                if (id == AIR) continue;

                float wx = static_cast<float>(cx * CS + x);
                float wy = static_cast<float>(y);
                float wz = static_cast<float>(cz * CS + z);

                if (id == WATER) {
                    uint8_t bTop = getBlock(x, y + 1, z);
                    if (bTop == AIR) {
                        addQuad(waterVerts, 14, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                wx+0, wy+0.90f, wz+0, wx+0, wy+0.90f, wz+1,
                                wx+1, wy+0.90f, wz+1, wx+1, wy+0.90f, wz+0);
                    }
                    continue;
                }

                // +Y (Üst Yüzey)
                uint8_t bTop = getBlock(x, y + 1, z);
                if (!BD(bTop).solid) {
                    float ao0 = calcAO(isSolid(x-1, y+1, z), isSolid(x, y+1, z-1), isSolid(x-1, y+1, z-1));
                    float ao1 = calcAO(isSolid(x-1, y+1, z), isSolid(x, y+1, z+1), isSolid(x-1, y+1, z+1));
                    float ao2 = calcAO(isSolid(x+1, y+1, z), isSolid(x, y+1, z+1), isSolid(x+1, y+1, z+1));
                    float ao3 = calcAO(isSolid(x+1, y+1, z), isSolid(x, y+1, z-1), isSolid(x+1, y+1, z-1));
                    addQuad(opaqueVerts, BD(id).topTile, 1.0f, ao0, ao1, ao2, ao3,
                            wx+0, wy+1, wz+0, wx+0, wy+1, wz+1,
                            wx+1, wy+1, wz+1, wx+1, wy+1, wz+0);
                }
                // -Y (Alt Yüzey)
                uint8_t bBot = getBlock(x, y - 1, z);
                if (!BD(bBot).solid) {
                    addQuad(opaqueVerts, BD(id).botTile, 0.55f, 0.8f, 0.8f, 0.8f, 0.8f,
                            wx+0, wy+0, wz+1, wx+0, wy+0, wz+0,
                            wx+1, wy+0, wz+0, wx+1, wy+0, wz+1);
                }
                // +Z (Ön Yüzey)
                uint8_t bFwd = getBlock(x, y, z + 1);
                if (!BD(bFwd).solid) {
                    float ao0 = calcAO(isSolid(x-1, y, z+1), isSolid(x, y-1, z+1), isSolid(x-1, y-1, z+1));
                    float ao1 = calcAO(isSolid(x+1, y, z+1), isSolid(x, y-1, z+1), isSolid(x+1, y-1, z+1));
                    float ao2 = calcAO(isSolid(x+1, y, z+1), isSolid(x, y+1, z+1), isSolid(x+1, y+1, z+1));
                    float ao3 = calcAO(isSolid(x-1, y, z+1), isSolid(x, y+1, z+1), isSolid(x-1, y+1, z+1));
                    addQuad(opaqueVerts, BD(id).sideTile, 0.85f, ao0, ao1, ao2, ao3,
                            wx+0, wy+0, wz+1, wx+1, wy+0, wz+1,
                            wx+1, wy+1, wz+1, wx+0, wy+1, wz+1);
                }
                // -Z (Arka Yüzey)
                uint8_t bBack = getBlock(x, y, z - 1);
                if (!BD(bBack).solid) {
                    float ao0 = calcAO(isSolid(x+1, y, z-1), isSolid(x, y-1, z-1), isSolid(x+1, y-1, z-1));
                    float ao1 = calcAO(isSolid(x-1, y, z-1), isSolid(x, y-1, z-1), isSolid(x-1, y-1, z-1));
                    float ao2 = calcAO(isSolid(x-1, y, z-1), isSolid(x, y+1, z-1), isSolid(x-1, y+1, z-1));
                    float ao3 = calcAO(isSolid(x+1, y, z-1), isSolid(x, y+1, z-1), isSolid(x+1, y+1, z-1));
                    addQuad(opaqueVerts, BD(id).sideTile, 0.85f, ao0, ao1, ao2, ao3,
                            wx+1, wy+0, wz+0, wx+0, wy+0, wz+0,
                            wx+0, wy+1, wz+0, wx+1, wy+1, wz+0);
                }
                // +X (Sağ Yüzey)
                uint8_t bRight = getBlock(x + 1, y, z);
                if (!BD(bRight).solid) {
                    float ao0 = calcAO(isSolid(x+1, y, z+1), isSolid(x+1, y-1, z), isSolid(x+1, y-1, z+1));
                    float ao1 = calcAO(isSolid(x+1, y, z-1), isSolid(x+1, y-1, z), isSolid(x+1, y-1, z-1));
                    float ao2 = calcAO(isSolid(x+1, y, z-1), isSolid(x+1, y+1, z), isSolid(x+1, y+1, z-1));
                    float ao3 = calcAO(isSolid(x+1, y, z+1), isSolid(x+1, y+1, z), isSolid(x+1, y+1, z+1));
                    addQuad(opaqueVerts, BD(id).sideTile, 0.72f, ao0, ao1, ao2, ao3,
                            wx+1, wy+0, wz+1, wx+1, wy+0, wz+0,
                            wx+1, wy+1, wz+0, wx+1, wy+1, wz+1);
                }
                // -X (Sol Yüzey)
                uint8_t bLeft = getBlock(x - 1, y, z);
                if (!BD(bLeft).solid) {
                    float ao0 = calcAO(isSolid(x-1, y, z-1), isSolid(x-1, y-1, z), isSolid(x-1, y-1, z-1));
                    float ao1 = calcAO(isSolid(x-1, y, z+1), isSolid(x-1, y-1, z), isSolid(x-1, y-1, z+1));
                    float ao2 = calcAO(isSolid(x-1, y, z+1), isSolid(x-1, y+1, z), isSolid(x-1, y+1, z+1));
                    float ao3 = calcAO(isSolid(x-1, y, z-1), isSolid(x-1, y+1, z), isSolid(x-1, y+1, z-1));
                    addQuad(opaqueVerts, BD(id).sideTile, 0.72f, ao0, ao1, ao2, ao3,
                            wx+0, wy+0, wz+0, wx+0, wy+0, wz+1,
                            wx+0, wy+1, wz+1, wx+0, wy+1, wz+0);
                }
            }
        }
    }

    vertexCount = opaqueVerts.size();
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, opaqueVerts.size() * sizeof(Vertex), opaqueVerts.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, light));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, ao));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tileId));

    waterVertexCount = waterVerts.size();
    if (waterVao == 0) {
        glGenVertexArrays(1, &waterVao);
        glGenBuffers(1, &waterVbo);
    }
    glBindVertexArray(waterVao);
    glBindBuffer(GL_ARRAY_BUFFER, waterVbo);
    glBufferData(GL_ARRAY_BUFFER, waterVerts.size() * sizeof(Vertex), waterVerts.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, light));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, ao));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tileId));

    glBindVertexArray(0);
    dirty = false;
}

void Chunk::renderOpaque() {
    if (vertexCount > 0 && vao != 0) {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
}

void Chunk::renderWater() {
    if (waterVertexCount > 0 && waterVao != 0) {
        glBindVertexArray(waterVao);
        glDrawArrays(GL_TRIANGLES, 0, waterVertexCount);
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

            if (h > SEA_LEVEL + 1 && (hash2D(wx * 5, wz * 11) > 0.965f) && x >= 2 && x <= CS - 3 && z >= 2 && z <= CS - 3) {
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

    if ((c.cx + c.cz) % 3 == 0) {
        int h = flr(getTerrainHeight(bx + 8.0f, bz + 8.0f));
        if (h > SEA_LEVEL) {
            spawnMob(ENT_PIG, {(float)bx + 8, (float)h + 0.1f, (float)bz + 8});
            spawnMob(ENT_COW, {(float)bx + 4, (float)h + 0.1f, (float)bz + 4});
            spawnMob(ENT_SHEEP, {(float)bx + 12, (float)h + 0.1f, (float)bz + 12});
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
    e.maxHealth = e.health;
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

void World::spawnBreakParticles(Vec3 pos, uint8_t blockId) {
    std::lock_guard lk(particleMtx);
    Vec3 col{0.5f, 0.5f, 0.5f};
    if (blockId == DIRT || blockId == GRASS) col = {0.45f, 0.30f, 0.18f};
    else if (blockId == STONE || blockId == COBBLESTONE) col = {0.55f, 0.55f, 0.55f};
    else if (blockId == OAK_PLANKS || blockId == OAK_LOG) col = {0.65f, 0.48f, 0.28f};
    else if (blockId == OAK_LEAVES) col = {0.20f, 0.58f, 0.15f};
    else if (blockId == DIAMOND_ORE) col = {0.20f, 0.88f, 0.88f};

    for (int i = 0; i < 12; ++i) {
        Particle p;
        p.pos = pos + Vec3{((rand()%100)/100.f - 0.5f)*0.6f, ((rand()%100)/100.f - 0.5f)*0.6f, ((rand()%100)/100.f - 0.5f)*0.6f};
        p.vel = Vec3{((rand()%100)/100.f - 0.5f)*3.2f, ((rand()%100)/100.f)*3.8f + 1.2f, ((rand()%100)/100.f - 0.5f)*3.2f};
        p.color = col;
        p.life = 0.0f;
        p.maxLife = 0.5f + ((rand()%100)/100.f) * 0.4f;
        p.size = 0.12f;
        particles.push_back(p);
    }
}

void World::spawnSplashParticles(Vec3 pos) {
    std::lock_guard lk(particleMtx);
    for (int i = 0; i < 10; ++i) {
        Particle p;
        p.pos = pos + Vec3{((rand()%100)/100.f - 0.5f)*0.5f, 0.1f, ((rand()%100)/100.f - 0.5f)*0.5f};
        p.vel = Vec3{((rand()%100)/100.f - 0.5f)*2.0f, ((rand()%100)/100.f)*2.5f + 1.0f, ((rand()%100)/100.f - 0.5f)*2.0f};
        p.color = {0.6f, 0.85f, 1.0f};
        p.life = 0.0f;
        p.maxLife = 0.45f;
        p.size = 0.09f;
        particles.push_back(p);
    }
}

void World::updateParticles(float dt) {
    std::lock_guard lk(particleMtx);
    for (size_t i = 0; i < particles.size();) {
        particles[i].life += dt;
        if (particles[i].life >= particles[i].maxLife) {
            particles[i] = particles.back();
            particles.pop_back();
        } else {
            particles[i].pos += particles[i].vel * dt;
            particles[i].vel.y += GRAVITY * dt * 0.7f;
            ++i;
        }
    }
}

void World::streamAround(int centerCx, int centerCz, int radius, int budget) {
    int generated = 0;
    for (int ring = 0; ring <= radius && generated < budget; ++ring) {
        for (int dx = -ring; dx <= ring && generated < budget; ++dx) {
            for (int dz = -ring; dz <= ring && generated < budget; ++dz) {
                if (std::max(std::abs(dx), std::abs(dz)) != ring) continue;
                auto ch = getOrCreate(centerCx + dx, centerCz + dz);
                if (!ch->generated) { generateChunk(*ch); ++generated; }
            }
        }
    }
}

void World::trimFarChunks(int centerCx, int centerCz, int keepRadius) {
    std::unique_lock lk(chunkMtx);
    for (auto it = chunks.begin(); it != chunks.end();) {
        auto ch = it->second;
        if (std::max(std::abs(ch->cx - centerCx), std::abs(ch->cz - centerCz)) > keepRadius) it = chunks.erase(it);
        else ++it;
    }
}

static bool checkCollision(const World& world, Vec3 pos);

void World::updateEntities(float dt, const Vec3& playerPos) {
    std::lock_guard lk(entityMtx);
    for (auto& e : entities) {
        if (!e.alive) continue;
        e.animTime += dt;

        if (e.type == ENT_ITEM_DROP) {
            e.pos += e.vel * dt;
            e.vel.y += GRAVITY * dt * 0.4f;
            uint8_t bCurrent = blockAt(flr(e.pos.x), flr(e.pos.y), flr(e.pos.z));
            if (bCurrent == WATER) {
                e.vel.y = 0.8f;
                e.vel.x *= 0.85f;
                e.vel.z *= 0.85f;
            } else if (blockAt(flr(e.pos.x), flr(e.pos.y - 0.1f), flr(e.pos.z)) != AIR) {
                e.vel = {0, 0, 0};
                e.pos.y = flr(e.pos.y) + 0.15f;
            }
        } else {
            e.vel.y += GRAVITY * dt;
            Vec3 nextPos = e.pos;
            nextPos.y += e.vel.y * dt;

            if (checkCollision(*this, nextPos)) {
                e.vel.y = 0;
                e.pos.y = flr(e.pos.y) + 0.02f;
            } else {
                e.pos.y = nextPos.y;
            }

            Vec3 diff = playerPos - e.pos;
            float dist = diff.len();
            if (dist < 8.0f) {
                float targetYaw = atan2f(diff.x, -diff.z) * 57.29578f;
                float diffYaw = fmodf(targetYaw - e.yaw + 540.0f, 360.0f) - 180.0f;
                e.yaw += diffYaw * dt * 3.0f;
            } else {
                if (fmodf(e.animTime, 4.0f) < dt) {
                    e.yaw = static_cast<float>(rand() % 360);
                }
            }

            float rad = e.yaw * 0.01745329f;
            Vec3 stepMove = {sinf(rad) * 0.9f * dt, 0, -cosf(rad) * 0.9f * dt};
            Vec3 testP = e.pos + stepMove;
            if (!checkCollision(*this, testP)) {
                e.pos = testP;
                e.walkSpeed = 1.0f;
            } else {
                e.walkSpeed = 0.0f;
            }
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
    if (path.empty()) return;
    std::string file = path + "/world_save.dat";
    int fd = open(file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0) return;
    const uint32_t magic = 0x4F4D4E49u;
    const uint32_t version = 3;
    uint32_t chunkCount = static_cast<uint32_t>(chunks.size());
    write(fd, &magic, sizeof(magic)); write(fd, &version, sizeof(version));
    write(fd, &gWorldSeed, sizeof(gWorldSeed)); write(fd, &chunkCount, sizeof(chunkCount));
    std::shared_lock lk(chunkMtx);
    for (auto& [k, ch] : chunks) {
        write(fd, &ch->cx, sizeof(ch->cx)); write(fd, &ch->cz, sizeof(ch->cz));
        write(fd, ch->blocks.data(), ch->blocks.size());
    }
    close(fd);
    LOGI("Dünya AAA formatında kaydedildi: %s (%u chunk)", file.c_str(), chunkCount);
}

void World::loadWorld(const std::string& path) {
    if (path.empty()) return;
    std::string file = path + "/world_save.dat";
    int fd = open(file.c_str(), O_RDONLY);
    if (fd < 0) return;
    uint32_t magic = 0, version = 0, chunkCount = 0;
    if (read(fd, &magic, sizeof(magic)) != sizeof(magic)) { close(fd); return; }
    if (magic == 0x4F4D4E49u) {
        if (read(fd, &version, sizeof(version)) != sizeof(version) || version < 2 ||
            read(fd, &gWorldSeed, sizeof(gWorldSeed)) != sizeof(gWorldSeed) ||
            read(fd, &chunkCount, sizeof(chunkCount)) != sizeof(chunkCount)) { close(fd); return; }
    } else chunkCount = magic;
    if (chunkCount > 10000u) { close(fd); return; }
    std::unique_lock lk(chunkMtx); chunks.clear();
    for (uint32_t i=0; i<chunkCount; ++i) {
        int cx=0, cz=0;
        if (read(fd,&cx,sizeof(cx)) != sizeof(cx) || read(fd,&cz,sizeof(cz)) != sizeof(cz)) break;
        auto ch=std::make_shared<Chunk>(cx,cz);
        if (read(fd,ch->blocks.data(),ch->blocks.size()) != static_cast<ssize_t>(ch->blocks.size())) break;
        ch->generated=true; ch->dirty=true; chunks[key(cx,cz)]=ch;
    }
    close(fd); LOGI("Dünya yüklendi: %s (%u chunk)", file.c_str(), chunkCount);
}

void Renderer::generateProceduralAtlas() {
    const int AT_SZ = 128;
    std::vector<uint32_t> pixels(AT_SZ * AT_SZ);

    auto putTilePixel = [&](int tileIdx, int px, int py, uint32_t color) {
        int tx = (tileIdx % 4) * 32;
        int ty = (tileIdx / 4) * 32;
        pixels[(ty + py) * AT_SZ + (tx + px)] = color;
    };

    for (int t = 0; t < 16; ++t) {
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                int noise = ((x * 11 + y * 19 + t * 23) % 28) - 14;
                uint32_t c = makeRGBA(140, 140, 140);
                switch(t) {
                    case 0: // Grass Top (AAA Canlı Çimen)
                        c = makeRGBA(78 + noise, 185 + noise, 45 + noise);
                        break;
                    case 1: // Grass Side
                        if (y < 7 + (x % 3)) c = makeRGBA(78 + noise, 185 + noise, 45 + noise);
                        else c = makeRGBA(118 + noise, 82 + noise, 52 + noise);
                        break;
                    case 2: // Dirt (Toprak)
                        c = makeRGBA(118 + noise, 82 + noise, 52 + noise);
                        break;
                    case 3: // Stone (Taş)
                        c = makeRGBA(135 + noise, 135 + noise, 138 + noise);
                        break;
                    case 4: // Cobblestone (Kırıktaş)
                        noise = ((x * 13 ^ y * 23) % 40) - 20;
                        c = makeRGBA(115 + noise, 115 + noise, 115 + noise);
                        break;
                    case 5: // Planks (Tahta)
                        c = makeRGBA(178 + (y%8==0?-30:noise), 138 + noise, 80 + noise);
                        break;
                    case 6: // Oak Log Side (Kütük)
                        c = makeRGBA(108 + (x%8==0?-25:noise), 78 + noise, 48 + noise);
                        break;
                    case 7: // Oak Log Top
                        c = makeRGBA(165 + noise, 128 + noise, 82 + noise);
                        break;
                    case 8: // Oak Leaves (Şeffaf Yaprak)
                        if ((x%4==0 && y%4==0) && ((x+y)%5==0)) {
                            c = makeRGBA(0, 0, 0, 0);
                        } else {
                            int leafVar = ((x * 7 + y * 13) % 36) - 18;
                            c = makeRGBA(28 + leafVar, 155 + leafVar, 32 + leafVar, 255);
                        }
                        break;
                    case 9: // Diamond Ore (Elmas Cevheri)
                        if ((x>=8 && x<=23 && y>=8 && y<=23) && (noise > -3)) c = makeRGBA(42, 235, 235);
                        else c = makeRGBA(135 + noise, 135 + noise, 138 + noise);
                        break;
                    case 10: // Coal Ore
                        if ((x>=8 && x<=23 && y>=8 && y<=23) && (noise > -3)) c = makeRGBA(28, 28, 28);
                        else c = makeRGBA(135 + noise, 135 + noise, 138 + noise);
                        break;
                    case 11: // Bedrock
                        noise = ((x * 37 + y * 53) % 70) - 35;
                        c = makeRGBA(38 + noise, 38 + noise, 38 + noise);
                        break;
                    case 12: // Crafting Table
                        c = makeRGBA(185 + noise, 140 + noise, 85 + noise);
                        break;
                    case 13: // Sand (Kum)
                        c = makeRGBA(232 + noise, 220 + noise, 158 + noise);
                        break;
                    case 14: // Canlı Şeffaf Su
                        c = makeRGBA(24 + noise, 120 + noise, 235 + noise, 205);
                        break;
                    case 15: // Kırılma Çatlak Maskesi
                        if ((x == y || x == (31 - y) || (x % 5 == 0 && y > 6)) && ((x + y) % 2 == 0)) {
                            c = makeRGBA(20, 20, 20, 220);
                        } else {
                            c = makeRGBA(0, 0, 0, 0);
                        }
                        break;
                    default:
                        c = makeRGBA(140, 140, 140);
                }
                putTilePixel(t, x, y, c);
            }
        }
    }

    glGenTextures(1, &atlasTex);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, AT_SZ, AT_SZ, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
}

static const float UNIT_BOX_VERTS[] = {
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
layout(location=3) in float aAO;
layout(location=4) in float aTileId;
uniform mat4 uMVP;
uniform vec3 uSunDir;
uniform float uDaylight;
out vec2 vUV;
out float vLight;
out float vFogDist;
void main(){
    vUV = aUV;
    vLight = aLight * aAO * (0.25 + 0.75 * uDaylight);
    vec4 pos4 = uMVP * vec4(aPos, 1.0);
    vFogDist = length(pos4.xyz);
    gl_Position = pos4;
})";

static const char* FS_WORLD = R"(#version 300 es
precision mediump float;
in vec2 vUV;
in float vLight;
in float vFogDist;
uniform sampler2D uTexture;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
out vec4 FragColor;
void main(){
    vec4 tex = texture(uTexture, vUV);
    if(tex.a < 0.1) discard;
    vec3 shaded = tex.rgb * vLight;
    float fogFactor = clamp((vFogDist - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
    FragColor = vec4(mix(shaded, uFogColor, fogFactor), tex.a);
})";

static const char* VS_WATER = R"(#version 300 es
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in float aLight;
layout(location=3) in float aAO;
layout(location=4) in float aTileId;
uniform mat4 uMVP;
uniform float uTime;
uniform float uDaylight;
out vec2 vUV;
out float vLight;
out float vFogDist;
out vec3 vWorldPos;
void main(){
    vUV = aUV;
    vec3 wavePos = aPos;
    wavePos.y += sin(aPos.x * 1.5 + uTime * 2.8) * cos(aPos.z * 1.5 + uTime * 2.2) * 0.06;
    vWorldPos = wavePos;
    vLight = aLight * (0.3 + 0.7 * uDaylight);
    vec4 pos4 = uMVP * vec4(wavePos, 1.0);
    vFogDist = length(pos4.xyz);
    gl_Position = pos4;
})";

static const char* FS_WATER = R"(#version 300 es
precision mediump float;
in vec2 vUV;
in float vLight;
in float vFogDist;
in vec3 vWorldPos;
uniform sampler2D uTexture;
uniform vec3 uFogColor;
uniform float uTime;
uniform vec3 uEyePos;
out vec4 FragColor;
void main(){
    vec4 baseTex = texture(uTexture, vUV);
    vec3 viewDir = normalize(uEyePos - vWorldPos);
    vec3 normal = vec3(0.0, 1.0, 0.0);
    float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), 2.5);
    vec3 waterDeep = vec3(0.06, 0.38, 0.78);
    vec3 waterShallow = vec3(0.20, 0.75, 0.95);
    vec3 finalColor = mix(waterDeep, waterShallow, fresnel * 0.75) * vLight;
    float fogFactor = clamp((vFogDist - 40.0) / 80.0, 0.0, 1.0);
    FragColor = vec4(mix(finalColor, uFogColor, fogFactor), 0.78);
})";

static const char* VS_ENTITY = R"(#version 300 es
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
uniform mat4 uMVP;
uniform mat4 uModel;
out vec3 vNorm;
void main(){
    vNorm = aNorm;
    gl_Position = uMVP * vec4(aPos, 1.0);
})";

static const char* FS_ENTITY = R"(#version 300 es
precision mediump float;
in vec3 vNorm;
uniform vec3 uColor;
uniform float uDaylight;
out vec4 FragColor;
void main(){
    float diffuse = max(dot(vNorm, normalize(vec3(0.4, 0.8, 0.3))), 0.0) * 0.45 + 0.55;
    vec3 lit = uColor * diffuse * (0.3 + 0.7 * uDaylight);
    FragColor = vec4(lit, 1.0);
})";

static const char* VS_SKY = R"(#version 300 es
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
out vec3 vPos;
void main(){
    vPos = aPos;
    gl_Position = uMVP * vec4(aPos, 1.0);
})";

static const char* FS_SKY = R"(#version 300 es
precision mediump float;
in vec3 vPos;
uniform vec4 uSkyColor;
uniform vec4 uHorizonColor;
out vec4 FragColor;
void main(){
    float h = normalize(vPos).y;
    float factor = clamp(h * 2.0, 0.0, 1.0);
    FragColor = mix(uHorizonColor, uSkyColor, factor);
})";

static GLuint compileShader(const char* vs, const char* fs) {
    auto compile=[](GLenum type,const char* src)->GLuint {
        GLuint sh=glCreateShader(type); glShaderSource(sh,1,&src,nullptr); glCompileShader(sh);
        GLint ok=GL_FALSE; glGetShaderiv(sh,GL_COMPILE_STATUS,&ok);
        if(!ok){ char log[2048]={}; glGetShaderInfoLog(sh,sizeof(log),nullptr,log); LOGE("Shader derleme hatası: %s",log); }
        return sh;
    };
    GLuint v=compile(GL_VERTEX_SHADER,vs), f=compile(GL_FRAGMENT_SHADER,fs);
    GLuint prog=glCreateProgram(); glAttachShader(prog,v); glAttachShader(prog,f); glLinkProgram(prog);
    GLint linked=GL_FALSE; glGetProgramiv(prog,GL_LINK_STATUS,&linked);
    if(!linked){ char log[2048]={}; glGetProgramInfoLog(prog,sizeof(log),nullptr,log); LOGE("Program bağlama hatası: %s",log); glDeleteProgram(prog); prog=0; }
    glDeleteShader(v); glDeleteShader(f); return prog;
}

void Renderer::init(int w, int h) {
    screenW = w; screenH = h;
    worldProg = compileShader(VS_WORLD, FS_WORLD);
    waterProg = compileShader(VS_WATER, FS_WATER);
    entityProg = compileShader(VS_ENTITY, FS_ENTITY);
    skyProg = compileShader(VS_SKY, FS_SKY);
    generateProceduralAtlas();

    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);
    glBindVertexArray(boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(UNIT_BOX_VERTS), UNIT_BOX_VERTS, GL_STATIC_DRAW);
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

void Renderer::drawBox(const Mat4& vp, Vec3 pos, Vec3 scale, Vec3 color, float yaw, float pitch, float roll) {
    Mat4 model = Mat4::identity();
    float radY = yaw * 0.01745329f;
    float radP = pitch * 0.01745329f;
    
    model.m[0]  = scale.x * cosf(radY); 
    model.m[2]  = scale.x * sinf(radY);
    model.m[5]  = scale.y * cosf(radP); 
    model.m[6]  = scale.y * sinf(radP);
    model.m[8]  = scale.z * -sinf(radY);
    model.m[10] = scale.z * cosf(radY);
    model.m[12] = pos.x; model.m[13] = pos.y; model.m[14] = pos.z; model.m[15] = 1.0f;
    Mat4 mvp = vp * model;

    glUseProgram(entityProg);
    glUniformMatrix4fv(glGetUniformLocation(entityProg, "uMVP"), 1, GL_FALSE, mvp.m);
    glUniform3f(glGetUniformLocation(entityProg, "uColor"), color.x, color.y, color.z);
    glUniform1f(glGetUniformLocation(entityProg, "uDaylight"), 1.0f);

    glBindVertexArray(boxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Renderer::drawBreakingOverlay(const Mat4& vp, Vec3i pos, float progress) {
    if (progress <= 0.0f) return;
    
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    Mat4 model = Mat4::identity();
    float expand = 1.003f;
    model.m[0] = expand; model.m[5] = expand; model.m[10] = expand;
    model.m[12] = pos.x + 0.5f; model.m[13] = pos.y + 0.5f; model.m[14] = pos.z + 0.5f;
    Mat4 mvp = vp * model;

    glUseProgram(worldProg);
    glUniformMatrix4fv(glGetUniformLocation(worldProg, "uMVP"), 1, GL_FALSE, mvp.m);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glUniform1i(glGetUniformLocation(worldProg, "uTexture"), 0);

    glBindVertexArray(boxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void Renderer::drawAtmosphere(const Mat4& vp, Vec3 playerPos, float worldTime, float time) {
    glDisable(GL_CULL_FACE);
    glUseProgram(skyProg);
    
    float sunAngle = (worldTime / 24000.0f) * 6.2831853f - 1.5707963f;
    float sunY = sinf(sunAngle);
    float sunX = cosf(sunAngle);

    // Güneş ve Ay pozisyonları
    Vec3 sunPos = playerPos + Vec3{sunX * 140.0f, sunY * 140.0f, 0.0f};
    Vec3 moonPos = playerPos + Vec3{-sunX * 140.0f, -sunY * 140.0f, 0.0f};

    // Güneş Çizimi (Glowing Solar Disc)
    if (sunY > -0.2f) {
        Mat4 sm = Mat4::identity();
        sm.m[0] = 30.0f; sm.m[5] = 30.0f; sm.m[10] = 30.0f;
        sm.m[12] = sunPos.x; sm.m[13] = sunPos.y; sm.m[14] = sunPos.z;
        Mat4 smvp = vp * sm;
        glUniformMatrix4fv(glGetUniformLocation(skyProg, "uMVP"), 1, GL_FALSE, smvp.m);
        glUniform4f(glGetUniformLocation(skyProg, "uSkyColor"), 1.0f, 0.98f, 0.82f, 1.0f);
        glUniform4f(glGetUniformLocation(skyProg, "uHorizonColor"), 1.0f, 0.85f, 0.45f, 1.0f);
        glBindVertexArray(boxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Ay Çizimi (Lunar Disc)
    if (sunY < 0.2f) {
        Mat4 mm = Mat4::identity();
        mm.m[0] = 22.0f; mm.m[5] = 22.0f; mm.m[10] = 22.0f;
        mm.m[12] = moonPos.x; mm.m[13] = moonPos.y; mm.m[14] = moonPos.z;
        Mat4 mmvp = vp * mm;
        glUniformMatrix4fv(glGetUniformLocation(skyProg, "uMVP"), 1, GL_FALSE, mmvp.m);
        glUniform4f(glGetUniformLocation(skyProg, "uSkyColor"), 0.90f, 0.92f, 1.0f, 1.0f);
        glUniform4f(glGetUniformLocation(skyProg, "uHorizonColor"), 0.70f, 0.75f, 0.95f, 1.0f);
        glBindVertexArray(boxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 3B Hacimsel Bulutlar (Volumetric Cloud Layer)
    float cloudShift = fmodf(time * 2.2f, 96.0f);
    int cx = flr(playerPos.x / 48.0f);
    int cz = flr(playerPos.z / 48.0f);

    float cloudLight = clampf(0.2f + 0.8f * std::max(0.0f, sunY), 0.25f, 1.0f);

    for (int dx = -4; dx <= 4; ++dx) {
        for (int dz = -4; dz <= 4; ++dz) {
            Vec3 cpos{(cx + dx) * 48.0f + cloudShift, 108.0f, (cz + dz) * 48.0f};
            Mat4 cm = Mat4::identity();
            cm.m[0] = 42.0f; cm.m[5] = 6.5f; cm.m[10] = 42.0f;
            cm.m[12] = cpos.x; cm.m[13] = cpos.y; cm.m[14] = cpos.z;
            Mat4 cmvp = vp * cm;
            glUniformMatrix4fv(glGetUniformLocation(skyProg, "uMVP"), 1, GL_FALSE, cmvp.m);
            glUniform4f(glGetUniformLocation(skyProg, "uSkyColor"), cloudLight, cloudLight, cloudLight * 1.05f, 0.85f);
            glUniform4f(glGetUniformLocation(skyProg, "uHorizonColor"), cloudLight * 0.9f, cloudLight * 0.9f, cloudLight, 0.85f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
}

void Renderer::drawMob(const Mat4& vp, const Entity& e, float daylight) {
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    float legAngle = sinf(e.animTime * 7.5f) * 28.0f * e.walkSpeed;
    float rad = e.yaw * 0.01745329f;

    if (e.type == ENT_PIG) {
        Vec3 pink{0.96f, 0.65f, 0.65f};
        Vec3 snoutCol{0.92f, 0.48f, 0.48f};
        drawBox(vp, e.pos + Vec3{0, 0.6f, 0}, {0.9f, 0.7f, 1.2f}, pink, e.yaw);
        Vec3 headOffset{sinf(rad)*0.65f, 0.85f, -cosf(rad)*0.65f};
        drawBox(vp, e.pos + headOffset, {0.6f, 0.6f, 0.6f}, pink, e.yaw);
        Vec3 snoutOffset{sinf(rad)*0.98f, 0.76f, -cosf(rad)*0.98f};
        drawBox(vp, e.pos + snoutOffset, {0.3f, 0.2f, 0.2f}, snoutCol, e.yaw);
        drawBox(vp, e.pos + Vec3{-0.25f, 0.25f, -0.35f}, {0.25f, 0.5f, 0.25f}, pink, e.yaw, legAngle);
        drawBox(vp, e.pos + Vec3{0.25f, 0.25f, -0.35f},  {0.25f, 0.5f, 0.25f}, pink, e.yaw, -legAngle);
        drawBox(vp, e.pos + Vec3{-0.25f, 0.25f, 0.35f},  {0.25f, 0.5f, 0.25f}, pink, e.yaw, -legAngle);
        drawBox(vp, e.pos + Vec3{0.25f, 0.25f, 0.35f},   {0.25f, 0.5f, 0.25f}, pink, e.yaw, legAngle);
    } else if (e.type == ENT_COW) {
        Vec3 cowCol{0.40f, 0.25f, 0.15f};
        Vec3 hornCol{0.90f, 0.90f, 0.85f};
        drawBox(vp, e.pos + Vec3{0, 0.85f, 0}, {1.05f, 1.0f, 1.45f}, cowCol, e.yaw);
        Vec3 headOffset{sinf(rad)*0.80f, 1.15f, -cosf(rad)*0.80f};
        drawBox(vp, e.pos + headOffset, {0.6f, 0.6f, 0.6f}, cowCol, e.yaw);
        // Boynuzlar
        drawBox(vp, e.pos + headOffset + Vec3{0.3f, 0.35f, 0}, {0.12f, 0.25f, 0.12f}, hornCol, e.yaw);
        drawBox(vp, e.pos + headOffset + Vec3{-0.3f, 0.35f, 0}, {0.12f, 0.25f, 0.12f}, hornCol, e.yaw);
        drawBox(vp, e.pos + Vec3{-0.3f, 0.3f, -0.4f}, {0.28f, 0.6f, 0.28f}, cowCol, e.yaw, legAngle);
        drawBox(vp, e.pos + Vec3{0.3f, 0.3f, -0.4f},  {0.28f, 0.6f, 0.28f}, cowCol, e.yaw, -legAngle);
        drawBox(vp, e.pos + Vec3{-0.3f, 0.3f, 0.4f},  {0.28f, 0.6f, 0.28f}, cowCol, e.yaw, -legAngle);
        drawBox(vp, e.pos + Vec3{0.3f, 0.3f, 0.4f},   {0.28f, 0.6f, 0.28f}, cowCol, e.yaw, legAngle);
    } else if (e.type == ENT_SHEEP) {
        Vec3 wool{0.94f, 0.94f, 0.94f};
        Vec3 headSkin{0.86f, 0.76f, 0.70f};
        drawBox(vp, e.pos + Vec3{0, 0.78f, 0}, {1.15f, 0.95f, 1.35f}, wool, e.yaw);
        Vec3 headOffset{sinf(rad)*0.72f, 0.98f, -cosf(rad)*0.72f};
        drawBox(vp, e.pos + headOffset, {0.55f, 0.55f, 0.55f}, headSkin, e.yaw);
        drawBox(vp, e.pos + Vec3{-0.28f, 0.3f, -0.38f}, {0.24f, 0.6f, 0.24f}, wool, e.yaw, legAngle);
        drawBox(vp, e.pos + Vec3{0.28f, 0.3f, -0.38f},  {0.24f, 0.6f, 0.24f}, wool, e.yaw, -legAngle);
        drawBox(vp, e.pos + Vec3{-0.28f, 0.3f, 0.38f},  {0.24f, 0.6f, 0.24f}, wool, e.yaw, -legAngle);
        drawBox(vp, e.pos + Vec3{0.28f, 0.3f, 0.38f},   {0.24f, 0.6f, 0.24f}, wool, e.yaw, legAngle);
    } else if (e.type == ENT_ITEM_DROP) {
        float bob = sinf(e.animTime * 3.8f) * 0.12f;
        drawBox(vp, e.pos + Vec3{0, 0.25f + bob, 0}, {0.35f, 0.35f, 0.35f}, {0.2f, 0.85f, 0.95f}, e.animTime * 95.0f);
    }
    glEnable(GL_CULL_FACE);
}

void Renderer::drawFirstPersonHand(const Mat4& proj, const Player& player, float time) {
    glDisable(GL_DEPTH_TEST);
    float swing = sinf(player.handSwingProgress * 3.1415926f) * 0.4f;
    float bobX = cosf(time * 6.0f) * 0.02f * (player.vel.len() > 0.1f ? 1.0f : 0.2f);
    float bobY = fabsf(sinf(time * 6.0f)) * 0.02f * (player.vel.len() > 0.1f ? 1.0f : 0.2f);

    Mat4 handModel = Mat4::identity();
    handModel.m[0] = 0.22f; handModel.m[5] = 0.45f; handModel.m[10] = 0.22f;
    handModel.m[12] = 0.45f + bobX + swing * 0.1f;
    handModel.m[13] = -0.42f - bobY - swing * 0.2f;
    handModel.m[14] = -0.65f - swing * 0.2f;

    glUseProgram(entityProg);
    glUniformMatrix4fv(glGetUniformLocation(entityProg, "uMVP"), 1, GL_FALSE, handModel.m);
    glUniform3f(glGetUniformLocation(entityProg, "uColor"), 0.88f, 0.68f, 0.52f);
    glUniform1f(glGetUniformLocation(entityProg, "uDaylight"), 1.0f);

    glBindVertexArray(boxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::frame(World& world, Player& player, float time) {
    float sunAngle = (player.worldTime / 24000.0f) * 6.2831853f - 1.5707963f;
    float sunY = sinf(sunAngle);
    float daylight = clampf(0.18f + 0.82f * std::max(0.0f, sunY), 0.18f, 1.0f);

    Vec3 skyColorDay{0.35f, 0.65f, 0.98f};
    Vec3 horizonColorDay{0.72f, 0.88f, 1.0f};
    Vec3 skyColorNight{0.03f, 0.05f, 0.12f};
    Vec3 horizonColorNight{0.08f, 0.10f, 0.20f};

    Vec3 curSky = skyColorNight + (skyColorDay - skyColorNight) * daylight;
    Vec3 curHorizon = horizonColorNight + (horizonColorDay - horizonColorNight) * daylight;

    if (player.inWater) {
        glClearColor(0.05f * daylight, 0.24f * daylight, 0.52f * daylight, 1.0f);
    } else {
        glClearColor(curHorizon.x, curHorizon.y, curHorizon.z, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float fovRad = player.fov * 0.01745329252f;
    Mat4 proj = matPerspective(fovRad, (float)screenW / (float)screenH, 0.08f, 500.0f);
    Vec3 eye = player.pos + Vec3{0, player.eyeH, 0};
    Mat4 view = matLookAt(eye, eye + player.lookDir(), {0, 1, 0});
    Mat4 vp = proj * view;

    drawAtmosphere(vp, player.pos, player.worldTime, time);

    // 1. Opak Bloklar Çizimi
    glUseProgram(worldProg);
    glUniformMatrix4fv(glGetUniformLocation(worldProg, "uMVP"), 1, GL_FALSE, vp.m);
    glUniform1f(glGetUniformLocation(worldProg, "uDaylight"), daylight);
    glUniform3f(glGetUniformLocation(worldProg, "uFogColor"), curHorizon.x, curHorizon.y, curHorizon.z);
    glUniform1f(glGetUniformLocation(worldProg, "uFogStart"), (player.renderDistance - 2) * 16.0f);
    glUniform1f(glGetUniformLocation(worldProg, "uFogEnd"), player.renderDistance * 16.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glUniform1i(glGetUniformLocation(worldProg, "uTexture"), 0);

    int pcx = flr(player.pos.x / CS), pcz = flr(player.pos.z / CS);
    world.streamAround(pcx, pcz, player.renderDistance, 2);
    world.trimFarChunks(pcx, pcz, player.renderDistance + 2);

    for (int dx = -player.renderDistance; dx <= player.renderDistance; ++dx) {
        for (int dz = -player.renderDistance; dz <= player.renderDistance; ++dz) {
            auto ch = world.getChunk(pcx + dx, pcz + dz);
            if (ch && ch->generated) {
                if (ch->dirty) ch->buildMesh(world);
                ch->renderOpaque();
            }
        }
    }

    // 2. Canlı Şeffaf Su Çizimi (Fresnel & Wave Displacement)
    glUseProgram(waterProg);
    glUniformMatrix4fv(glGetUniformLocation(waterProg, "uMVP"), 1, GL_FALSE, vp.m);
    glUniform1f(glGetUniformLocation(waterProg, "uTime"), time);
    glUniform1f(glGetUniformLocation(waterProg, "uDaylight"), daylight);
    glUniform3f(glGetUniformLocation(waterProg, "uEyePos"), eye.x, eye.y, eye.z);
    glUniform3f(glGetUniformLocation(waterProg, "uFogColor"), curHorizon.x, curHorizon.y, curHorizon.z);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glUniform1i(glGetUniformLocation(waterProg, "uTexture"), 0);

    for (int dx = -player.renderDistance; dx <= player.renderDistance; ++dx) {
        for (int dz = -player.renderDistance; dz <= player.renderDistance; ++dz) {
            auto ch = world.getChunk(pcx + dx, pcz + dz);
            if (ch && ch->generated) {
                ch->renderWater();
            }
        }
    }

    if (player.isBreaking && player.breakProgress > 0.0f) {
        drawBreakingOverlay(vp, player.breakingBlock, player.breakProgress);
    }

    std::lock_guard lk(world.entityMtx);
    for (const auto& e : world.entities) {
        if (e.alive) drawMob(vp, e, daylight);
    }

    std::lock_guard plk(world.particleMtx);
    for (const auto& p : world.particles) {
        drawBox(vp, p.pos, {p.size, p.size, p.size}, p.color);
    }

    drawFirstPersonHand(proj, player, time);
}

static bool checkCollision(const World& world, Vec3 pos) {
    float r = 0.30f;
    float h = 1.80f;
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

static constexpr uint32_t SAVE_FOOTER=0x504C5952u;
struct PlayerFooter { uint32_t magic; float x,y,z,yaw,pitch,worldTime; int32_t selected; uint8_t slots[INV_SIZE][3]; };
static void savePlayerFooter(const GameState& gs){ if(gs.saveDirectory.empty())return; std::string f=gs.saveDirectory+"/world_save.dat"; int fd=open(f.c_str(),O_WRONLY|O_APPEND); if(fd<0)return; PlayerFooter p{}; p.magic=SAVE_FOOTER; p.x=gs.player.pos.x;p.y=gs.player.pos.y;p.z=gs.player.pos.z;p.yaw=gs.player.yaw;p.pitch=gs.player.pitch;p.worldTime=gs.player.worldTime;p.selected=gs.player.inv.selected; for(int i=0;i<INV_SIZE;++i){p.slots[i][0]=gs.player.inv.slots[i].id&255;p.slots[i][1]=(gs.player.inv.slots[i].id>>8)&255;p.slots[i][2]=gs.player.inv.slots[i].count;} write(fd,&p,sizeof(p)); close(fd); }
static bool loadPlayerFooter(GameState& gs){ if(gs.saveDirectory.empty())return false; std::string f=gs.saveDirectory+"/world_save.dat"; int fd=open(f.c_str(),O_RDONLY); if(fd<0)return false; PlayerFooter p{}; if(lseek(fd,-(off_t)sizeof(p),SEEK_END)<0||read(fd,&p,sizeof(p))!=(ssize_t)sizeof(p)){close(fd);return false;} close(fd); if(p.magic!=SAVE_FOOTER)return false; gs.player.pos={p.x,p.y,p.z};gs.player.yaw=p.yaw;gs.player.pitch=p.pitch;gs.player.worldTime=fmodf(p.worldTime,24000.0f);gs.player.inv.selected=std::clamp(p.selected,0,HOTBAR_SZ-1);for(int i=0;i<INV_SIZE;++i){uint16_t id=(uint16_t)p.slots[i][0]|((uint16_t)p.slots[i][1]<<8);gs.player.inv.slots[i]=ItemStack(id,p.slots[i][2]);}return true;}

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
    gState->world->loadWorld(gState->saveDirectory);
    if (gState->world->chunks.empty()) gWorldSeed = 0x5EED1234u;
    gState->world->streamAround(0, 0, 3, 64);

    int spawnY = gState->world->getHighestBlock(0, 0);
    gState->player.pos = {0.5f, static_cast<float>(spawnY) + 1.2f, 0.5f};
    
    gState->player.inv.slots[0] = ItemStack(OAK_PLANKS, 64);
    gState->player.inv.slots[1] = ItemStack(COBBLESTONE, 64);
    gState->player.inv.slots[2] = ItemStack(DIAMOND_ORE, 64);
    gState->player.inv.slots[3] = ItemStack(CRAFTING_TABLE, 16);
    gState->player.inv.slots[4] = ItemStack(TORCH, 64);
    gState->player.inv.slots[5] = ItemStack(GRASS, 64);
    gState->player.inv.slots[6] = ItemStack(DIRT, 64);
    gState->player.inv.slots[7] = ItemStack(OAK_LOG, 64);
    gState->player.inv.slots[8] = ItemStack(TNT, 64);
    loadPlayerFooter(*gState);
    gState->initialized = true;
    LOGI("Omni Craft AAA Motoru Başlatıldı. Spawn: [0, %d, 0]", spawnY);
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
    float sdt = clampf(dt, 0.001f, 0.033f);
    gs.time += sdt;
    gs.autoSaveTimer += sdt;
    gs.fps = 1.0f / sdt;

    if (gs.player.handSwingProgress > 0.0f) {
        gs.player.handSwingProgress -= sdt * 3.5f;
        if (gs.player.handSwingProgress < 0.0f) gs.player.handSwingProgress = 0.0f;
    }

    if (gs.player.dayNightEnabled) {
        gs.player.worldTime = fmodf(gs.player.worldTime + sdt * 20.0f, 24000.0f);
    }

    if (gs.autoSaveTimer >= 60.0f) {
        if (gs.world) { gs.world->saveWorld(gs.saveDirectory); savePlayerFooter(gs); }
        gs.autoSaveTimer = 0.0f;
    }

    uint8_t bFeet = gs.world->blockAt(flr(gs.player.pos.x), flr(gs.player.pos.y), flr(gs.player.pos.z));
    uint8_t bHead = gs.world->blockAt(flr(gs.player.pos.x), flr(gs.player.pos.y + 1.2f), flr(gs.player.pos.z));
    bool wasInWater = gs.player.inWater;
    gs.player.inWater = (bFeet == WATER || bHead == WATER);

    if (!wasInWater && gs.player.inWater) {
        gs.world->spawnSplashParticles(gs.player.pos);
    }

    float yawRad = gs.player.yaw * (3.14159265f / 180.0f);
    float currentSpeed = gs.player.inWater ? WALK_SPD * 0.6f : 
                        (gs.player.sprinting ? SPRINT_SPD : (gs.player.sneaking ? SNEAK_SPD : WALK_SPD));

    float fwd = gs.inputZ * currentSpeed;
    float str = gs.inputX * currentSpeed;

    Vec3 moveDir = {
        sinf(yawRad) * fwd + cosf(yawRad) * str,
        0.0f,
        -cosf(yawRad) * fwd + sinf(yawRad) * str
    };

    gs.player.vel.x = moveDir.x;
    gs.player.vel.z = moveDir.z;

    // Yüzme ve Zıplama Fiziği
    if (gs.player.inWater) {
        if (gs.player.jumpHolding) {
            gs.player.vel.y = SWIM_VEL;
        } else {
            gs.player.vel.y += GRAVITY * sdt * 0.25f;
            gs.player.vel.y *= 0.88f;
        }
    } else {
        if (gs.player.jumpHolding && gs.player.onGround) {
            gs.player.vel.y = JUMP_VEL;
            gs.player.onGround = false;
        }
        gs.player.vel.y += GRAVITY * sdt;
    }

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

    // Blok Kırma İşlemi
    if (gs.player.isBreaking) {
        gs.player.handSwingProgress = 1.0f;
        auto hit = gs.world->raycast(gs.player.pos + Vec3{0, gs.player.eyeH, 0}, gs.player.lookDir(), REACH);
        if (hit.hit) {
            if (hit.block == gs.player.breakingBlock) {
                uint8_t targetB = gs.world->blockAt(hit.block.x, hit.block.y, hit.block.z);
                float hVal = std::max(0.2f, BD(targetB).hardness);
                gs.player.breakProgress += sdt / (hVal * 0.75f);

                if (gs.player.breakProgress >= 1.0f) {
                    gs.world->setBlock(hit.block.x, hit.block.y, hit.block.z, AIR);
                    gs.world->spawnBreakParticles({hit.block.x+0.5f, hit.block.y+0.5f, hit.block.z+0.5f}, targetB);
                    uint16_t drop = BD(targetB).dropId;
                    if (drop != AIR) {
                        gs.world->spawnItemDrop(drop, {hit.block.x+0.5f, hit.block.y+0.5f, hit.block.z+0.5f}, {0, 2.5f, 0});
                    }
                    gs.player.breakProgress = 0.0f;
                    gs.player.breakingBlock = {-999, -999, -999};
                }
            } else {
                gs.player.breakingBlock = hit.block;
                gs.player.breakProgress = 0.0f;
            }
        } else {
            gs.player.breakProgress = 0.0f;
        }
    } else {
        gs.player.breakProgress = 0.0f;
    }

    // Eşya Toplama
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

    gs.world->updateEntities(sdt, gs.player.pos);
    gs.world->updateParticles(sdt);
    gs.renderer.frame(*gs.world, gs.player, gs.time);
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeInput(JNIEnv*, jclass, jfloat x, jfloat z) {
    if (gState) { gState->inputX = x; gState->inputZ = z; }
}

// DÜZELTME: Y-Ekseni tersliği giderildi (dy değeri terslenerek doğru doğal bakış sağlandı)
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeCameraInput(JNIEnv*, jclass, jfloat dx, jfloat dy) {
    if (gState) {
        gState->player.yaw += dx;
        gState->player.pitch = clampf(gState->player.pitch - dy, -89.5f, 89.5f);
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetBreaking(JNIEnv*, jclass, jboolean active) {
    if (gState) {
        gState->player.isBreaking = active;
        if (!active) {
            gState->player.breakProgress = 0.0f;
            gState->player.breakingBlock = {-999, -999, -999};
        }
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeTapPlaceAt(JNIEnv*, jclass, jfloat screenX, jfloat screenY) {
    if (!gState) return;
    gState->player.handSwingProgress = 1.0f;
    Vec3 rayDir = getRayDirection(gState->player, screenX, screenY, gState->screenW, gState->screenH);
    auto hit = gState->world->raycast(gState->player.pos + Vec3{0, gState->player.eyeH, 0}, rayDir, REACH);
    if (hit.hit) {
        ItemStack& held = gState->player.inv.active();
        if (!held.empty() && held.id < BLOCK_COUNT) {
            Vec3i target = {hit.block.x + hit.face.x, hit.block.y + hit.face.y, hit.block.z + hit.face.z};
            Vec3 bCenter = {(float)target.x + 0.5f, (float)target.y + 0.5f, (float)target.z + 0.5f};
            if ((bCenter - (gState->player.pos + Vec3{0, 0.9f, 0})).len() > 0.75f) {
                gState->world->setBlock(target.x, target.y, target.z, (uint8_t)held.id);
                held.count--;
                if (held.count == 0) held = ItemStack();
            }
        }
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetJumpState(JNIEnv*, jclass, jboolean holding) {
    if (!gState) return;
    gState->player.jumpHolding = holding;
    if (holding && gState->player.onGround && !gState->player.inWater) {
        gState->player.vel.y = JUMP_VEL;
        gState->player.onGround = false;
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetSneakState(JNIEnv*, jclass, jboolean holding) {
    if (gState) gState->player.sneaking = holding;
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetSprintState(JNIEnv*, jclass, jboolean active) {
    if (gState) gState->player.sprinting = active;
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSelectSlot(JNIEnv*, jclass, jint slot) {
    if (gState) gState->player.inv.selected = slot % HOTBAR_SZ;
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSaveWorld(JNIEnv*, jclass) {
    if (gState && gState->world) {
        gState->world->saveWorld(gState->saveDirectory);
        savePlayerFooter(*gState);
    }
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

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetInventorySlot(JNIEnv*, jclass, jint slot, jint id, jint count) {
    if (gState && slot >= 0 && slot < INV_SIZE) {
        gState->player.inv.slots[slot] = ItemStack(static_cast<uint16_t>(id), static_cast<uint8_t>(count));
    }
}

JNIEXPORT jstring JNICALL Java_com_omni_craft_Engine_nativeGetPlayerStats(JNIEnv* env, jclass) {
    if (!gState) return env->NewStringUTF("{}");
    char buf[256];
    snprintf(buf, sizeof(buf),
             "{\"fps\":%.1f,\"x\":%.1f,\"y\":%.1f,\"z\":%.1f,\"hp\":%.1f,\"hunger\":%.1f,\"time\":%.0f}",
             gState->fps, gState->player.pos.x, gState->player.pos.y, gState->player.pos.z,
             gState->player.health, gState->player.hunger, gState->player.worldTime);
    return env->NewStringUTF(buf);
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetRenderDistance(JNIEnv*,jclass,jint d){if(gState)gState->player.renderDistance=std::clamp((int)d,3,RD);}
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetFov(JNIEnv*,jclass,jfloat f){if(gState)gState->player.fov=clampf(f,55.0f,95.0f);}
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetDayNight(JNIEnv*,jclass,jboolean e){if(gState)gState->player.dayNightEnabled=e;}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeDestroy(JNIEnv*, jclass) {
    if (gState) {
        if (gState->world) { gState->world->saveWorld(gState->saveDirectory); savePlayerFooter(*gState); }
        gState->initialized = false;
        delete gState;
        gState = nullptr;
    }
}

} // extern "C"
