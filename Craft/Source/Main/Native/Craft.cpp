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
                 "=== OMNI CRAFT AAA PROCEDURAL CRASH ===\n"
                 "Sinyal: %d\n"
                 "Hata Adresi: %p\n"
                 "Zaman: %04d-%02d-%02d %02d:%02d:%02d\n",
                 sig, info->si_addr,
                 timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        write(fd, buf, strlen(buf));
        close(fd);
    }
    LOGE("Yerel motor çökmesi kaydedildi: %s", filePath);
    _exit(1);
}

// Narrowing hatasını tamamen engelleyen net struct başlatıcı dizisi
static constexpr BlockDef BLOCK_TABLE[BLOCK_COUNT] = {
    {"hava",           false, true,  false, false, 0.0f, 0.6f, AIR},
    {"cimen",          true,  false, false, false, 0.6f, 0.6f, DIRT},
    {"toprak",         true,  false, false, false, 0.5f, 0.6f, DIRT},
    {"tas",            true,  false, false, false, 1.5f, 0.6f, COBBLESTONE},
    {"kiriktas",       true,  false, false, false, 2.0f, 0.6f, COBBLESTONE},
    {"kum",            true,  false, false, true,  0.5f, 0.4f, SAND},
    {"cakil",          true,  false, false, true,  0.6f, 0.4f, GRAVEL},
    {"mese_kutuk",     true,  false, false, false, 2.0f, 0.6f, OAK_LOG},
    {"mese_yaprak",    true,  true,  false, false, 0.2f, 0.6f, AIR},
    {"mese_tahta",     true,  false, false, false, 2.0f, 0.6f, OAK_PLANKS},
    {"ny_kutuk",       true,  false, false, false, 2.0f, 0.6f, BIRCH_LOG},
    {"ny_tahta",       true,  false, false, false, 2.0f, 0.6f, BIRCH_PLANKS},
    {"ny_yaprak",      true,  true,  false, false, 0.2f, 0.6f, AIR},
    {"cam_kutuk",      true,  false, false, false, 2.0f, 0.6f, SPRUCE_LOG},
    {"cam_tahta",      true,  false, false, false, 2.0f, 0.6f, SPRUCE_PLANKS},
    {"cam_yaprak",     true,  true,  false, false, 0.2f, 0.6f, AIR},
    {"komur_cevh",     true,  false, false, false, 3.0f, 0.6f, ITEM_COAL},
    {"demir_cevh",     true,  false, false, false, 3.0f, 0.6f, IRON_ORE},
    {"altin_cevh",     true,  false, false, false, 3.0f, 0.6f, GOLD_ORE},
    {"elmas_cevh",     true,  false, false, false, 3.0f, 0.6f, ITEM_DIAMOND},
    {"zumrut_cevh",    true,  false, false, false, 3.0f, 0.6f, EMERALD_ORE},
    {"kiziltas_cv",    true,  false, false, false, 3.0f, 0.6f, REDSTONE_ORE},
    {"lapis_cevh",     true,  false, false, false, 3.0f, 0.6f, LAPIS_ORE},
    {"komur_blok",     true,  false, false, false, 5.0f, 0.6f, COAL_BLOCK},
    {"demir_blok",     true,  false, false, false, 5.0f, 0.6f, IRON_BLOCK},
    {"altin_blok",     true,  false, false, false, 3.0f, 0.6f, GOLD_BLOCK},
    {"elmas_blok",     true,  false, false, false, 5.0f, 0.6f, DIAMOND_BLOCK},
    {"su",             false, true,  true,  false, 100.f,0.2f, AIR},
    {"lav",            false, true,  true,  false, 100.f,0.2f, AIR},
    {"cam",            true,  true,  false, false, 0.3f, 0.6f, AIR},
    {"isiktasi",       true,  true,  false, false, 0.3f, 0.6f, GLOWSTONE},
    {"nether_tasi",    true,  false, false, false, 0.4f, 0.6f, NETHERRACK},
    {"uretim_masa",    true,  false, false, false, 2.5f, 0.6f, CRAFTING_TABLE},
    {"firin",          true,  false, false, false, 3.5f, 0.6f, FURNACE},
    {"sandik",         true,  false, false, false, 2.5f, 0.6f, CHEST},
    {"merdiven",       false, true,  false, false, 0.4f, 0.6f, LADDER},
    {"mesale",         false, true,  false, false, 0.0f, 0.6f, TORCH},
    {"bedrock",        true,  false, false, false, -1.f, 0.6f, BEDROCK},
    {"kar_katmani",    true,  false, false, false, 0.2f, 0.3f, SNOW_LAYER},
    {"buz",            true,  true,  false, false, 0.5f, 0.02f,AIR},
    {"kil",            true,  false, false, false, 0.6f, 0.6f, CLAY},
    {"tugla",          true,  false, false, false, 2.0f, 0.6f, BRICK},
    {"tas_tugla",      true,  false, false, false, 1.5f, 0.6f, STONE_BRICKS},
    {"yosun_tas",      true,  false, false, false, 2.0f, 0.6f, MOSSY_COBBLE},
    {"tnt",            true,  false, false, false, 0.0f, 0.6f, TNT},
    {"kitaplik",       true,  false, false, false, 1.5f, 0.6f, OAK_PLANKS},
    {"balkabagi",      true,  false, false, false, 1.0f, 0.6f, PUMPKIN},
    {"karpuz",         true,  false, false, false, 1.0f, 0.6f, MELON},
    {"kumtasi",        true,  false, false, false, 0.8f, 0.6f, SANDSTONE},
    {"kaktus",         true,  true,  false, false, 0.4f, 0.6f, CACTUS},
    {"sunger",         true,  false, false, false, 0.6f, 0.6f, SPONGE}
};

static inline const BlockDef& BD(uint8_t id) { return BLOCK_TABLE[id < BLOCK_COUNT ? id : 0]; }
static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline int flr(float f) { return static_cast<int>(std::floor(f)); }

static Mat4 matPerspective(float fov, float asp, float nearZ, float farZ) {
    Mat4 m; float f = 1.0f / tanf(fov * 0.5f);
    m.m[0] = f / asp; m.m[5] = f;
    m.m[10] = (farZ + nearZ) / (nearZ - farZ); m.m[11] = -1.0f;
    m.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    return m;
}

static Mat4 matLookAt(Vec3 eye, Vec3 ctr, Vec3 up) {
    Vec3 f = (ctr - eye).norm();
    Vec3 r = f.cross(up).norm();
    Vec3 u = r.cross(f);
    Mat4 m;
    m.m[0] = r.x;  m.m[4] = r.y;  m.m[8]  = r.z;  m.m[12] = -(r.x * eye.x + r.y * eye.y + r.z * eye.z);
    m.m[1] = u.x;  m.m[5] = u.y;  m.m[9]  = u.z;  m.m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    m.m[2] = -f.x; m.m[6] = -f.y; m.m[10] = -f.z; m.m[14] =  (f.x * eye.x + f.y * eye.y + f.z * eye.z);
    m.m[15] = 1.0f;
    return m;
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
    heightMap.fill(0);
}
uint8_t Chunk::get(int x, int y, int z) const {
    if (x < 0 || x >= CS || y < 0 || y >= WH || z < 0 || z >= CS) return AIR;
    return blocks[idx(x, y, z)];
}
void Chunk::set(int x, int y, int z, uint8_t b) {
    if (x < 0 || x >= CS || y < 0 || y >= WH || z < 0 || z >= CS) return;
    blocks[idx(x, y, z)] = b;
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
            int h = 64 + static_cast<int>(sinf(wx * 0.05f) * cosf(wz * 0.05f) * 10.0f);
            c.heightMap[x * CS + z] = h;
            for (int y = 1; y < h - 4; ++y) c.set(x, y, z, STONE);
            for (int y = h - 4; y < h; ++y) c.set(x, y, z, DIRT);
            c.set(x, h, z, GRASS);
        }
    }
    if ((c.cx + c.cz) % 4 == 0) {
        spawnMob(ENT_PIG,   {(float)bx + 8, (float)c.heightMap[8*CS+8] + 1.2f, (float)bz + 8});
        spawnMob(ENT_COW,   {(float)bx + 4, (float)c.heightMap[4*CS+4] + 1.2f, (float)bz + 4});
        spawnMob(ENT_SHEEP, {(float)bx + 12,(float)c.heightMap[12*CS+12] + 1.2f, (float)bz + 12});
    }
    c.generated = true;
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
    if (ch) ch->set(lx, wy, lz, b);
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
                e.pos.y = flr(e.pos.y) + 1.1f;
            }
        } else {
            e.pos.y += GRAVITY * dt * 0.3f;
            uint8_t bBelow = blockAt(flr(e.pos.x), flr(e.pos.y - 0.2f), flr(e.pos.z));
            if (bBelow != AIR) e.pos.y = flr(e.pos.y) + 1.0f;

            if (fmodf(e.animTime, 3.0f) < dt) {
                e.yaw = static_cast<float>(rand() % 360);
            }
            float rad = e.yaw * 0.01745329f;
            e.pos.x += sinf(rad) * 0.8f * dt;
            e.pos.z += cosf(rad) * 0.8f * dt;
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
    Vec3i prev = {ix, iy, iz};

    for (int s = 0; s < 250; ++s) {
        float t = std::min({tX, tY, tZ});
        if (t > maxD) break;
        uint8_t b = blockAt(ix, iy, iz);
        if (b != AIR && BD(b).solid) {
            r.hit = true; r.block = {ix, iy, iz}; r.dist = t;
            r.face = {ix - prev.x, iy - prev.y, iz - prev.z};
            return r;
        }
        prev = {ix, iy, iz};
        if (tX < tY && tX < tZ) { tX += tDX; ix += (int)sX; }
        else if (tY < tZ)        { tY += tDY; iy += (int)sY; }
        else                    { tZ += tDZ; iz += (int)sZ; }
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
    LOGI("Dünya diske yazıldı: %s (%u chunk)", file.c_str(), chunkCount);
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
        chunks[key(cx, cz)] = ch;
    }
    close(fd);
    LOGI("Dünya diskten yüklendi: %s (%u chunk)", file.c_str(), chunkCount);
}

static const float PROCEDURAL_CUBE[] = {
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
     0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f,-0.5f, 0, 1,0,
};

// Tamamen matematikle (Procedural Noise ve Işıklandırma) resimsiz blok dokuları üreten Shader
static const char* VS_PROC = R"(#version 300 es
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
uniform mat4 uMVP;
uniform vec3 uWorldPos;
out vec3 vWorldPos;
out vec3 vNorm;
void main(){
    vWorldPos = uWorldPos + aPos;
    vNorm = aNorm;
    gl_Position = uMVP * vec4(aPos, 1.0);
})";

static const char* FS_PROC = R"(#version 300 es
precision highp float;
in vec3 vWorldPos;
in vec3 vNorm;
uniform vec3 uBaseColor;
uniform int uBlockType;
out vec4 FragColor;

float hash(vec3 p) {
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

void main(){
    float n = hash(floor(vWorldPos * 16.0)) * 0.15;
    vec3 col = uBaseColor + vec3(n - 0.075);

    // Yönsel Gölgelendirme (Directional Diffuse Lighting)
    float light = max(dot(vNorm, normalize(vec3(0.4, 0.8, 0.3))), 0.0) * 0.4 + 0.6;
    if(vNorm.y < -0.5) light *= 0.5; // Alt yüzey gölgesi
    if(abs(vNorm.x) > 0.5 || abs(vNorm.z) > 0.5) light *= 0.8;

    FragColor = vec4(col * light, 1.0);
})";

static GLuint compileProg(const char* vs, const char* fs) {
    GLuint v = glCreateShader(GL_VERTEX_SHADER); glShaderSource(v,1,&vs,nullptr); glCompileShader(v);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(f,1,&fs,nullptr); glCompileShader(f);
    GLuint p = glCreateProgram(); glAttachShader(p,v); glAttachShader(p,f); glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

void Renderer::init(int w, int h) {
    screenW = w; screenH = h;
    cubeProg = compileProg(VS_PROC, FS_PROC);
    glGenVertexArrays(1, &cubeVAO); glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PROCEDURAL_CUBE), PROCEDURAL_CUBE, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void Renderer::resize(int w, int h) {
    screenW = w; screenH = h;
    glViewport(0, 0, w, h);
}

void Renderer::drawBlockProcedural(const Mat4& vp, Vec3 pos, Vec3 scale, uint8_t blockId, float yaw) {
    Mat4 model;
    float rad = yaw * 0.01745329f;
    model.m[0] = scale.x * cosf(rad); model.m[2] = scale.x * sinf(rad);
    model.m[5] = scale.y;
    model.m[8] = scale.z * -sinf(rad); model.m[10] = scale.z * cosf(rad);
    model.m[12] = pos.x; model.m[13] = pos.y; model.m[14] = pos.z; model.m[15] = 1.0f;
    Mat4 mvp = vp * model;

    Vec3 color{0.5f, 0.5f, 0.5f};
    switch (blockId) {
        case GRASS:       color = {0.35f, 0.72f, 0.28f}; break;
        case DIRT:        color = {0.52f, 0.35f, 0.22f}; break;
        case STONE:       color = {0.55f, 0.55f, 0.55f}; break;
        case COBBLESTONE: color = {0.42f, 0.42f, 0.42f}; break;
        case SAND:        color = {0.88f, 0.82f, 0.56f}; break;
        case OAK_LOG:     color = {0.40f, 0.26f, 0.13f}; break;
        case OAK_PLANKS:  color = {0.65f, 0.48f, 0.28f}; break;
        case OAK_LEAVES:  color = {0.20f, 0.55f, 0.15f}; break;
        case DIAMOND_BLOCK:color= {0.32f, 0.90f, 0.88f}; break;
        case GOLD_BLOCK:  color = {0.95f, 0.82f, 0.20f}; break;
        case IRON_BLOCK:  color = {0.85f, 0.85f, 0.85f}; break;
        case BEDROCK:     color = {0.15f, 0.15f, 0.15f}; break;
        default:          color = {0.60f, 0.60f, 0.60f}; break;
    }

    glUseProgram(cubeProg);
    glUniformMatrix4fv(glGetUniformLocation(cubeProg, "uMVP"), 1, GL_FALSE, mvp.m);
    glUniform3f(glGetUniformLocation(cubeProg, "uWorldPos"), pos.x, pos.y, pos.z);
    glUniform3f(glGetUniformLocation(cubeProg, "uBaseColor"), color.x, color.y, color.z);
    glUniform1i(glGetUniformLocation(cubeProg, "uBlockType"), blockId);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Renderer::drawMobProcedural(const Mat4& vp, const Entity& e) {
    if (e.type == ENT_PIG) {
        // Gövde ve Kafa (Pembe Domuz)
        drawBlockProcedural(vp, e.pos, {0.9f, 0.8f, 1.2f}, 0, e.yaw);
        drawBlockProcedural(vp, e.pos + Vec3{0, 0.5f, 0.6f}, {0.6f, 0.6f, 0.6f}, 0, e.yaw);
    } else if (e.type == ENT_COW) {
        // Gövde (Kahverengi/Beyaz İnek)
        drawBlockProcedural(vp, e.pos, {1.0f, 1.1f, 1.4f}, 0, e.yaw);
        drawBlockProcedural(vp, e.pos + Vec3{0, 0.7f, 0.7f}, {0.7f, 0.7f, 0.7f}, 0, e.yaw);
    } else if (e.type == ENT_SHEEP) {
        // Gövde (Yünlü Koyun)
        drawBlockProcedural(vp, e.pos, {1.0f, 1.0f, 1.3f}, 0, e.yaw);
        drawBlockProcedural(vp, e.pos + Vec3{0, 0.6f, 0.65f}, {0.6f, 0.6f, 0.6f}, 0, e.yaw);
    } else if (e.type == ENT_ITEM_DROP) {
        // Yerde dönen 3D eşya
        drawBlockProcedural(vp, e.pos, {0.35f, 0.35f, 0.35f}, static_cast<uint8_t>(e.itemId), e.animTime * 60.0f);
    }
}

void Renderer::frame(World& world, Player& player, float) {
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    Mat4 proj = matPerspective(1.222f, (float)screenW / (float)screenH, 0.05f, 600.0f);
    Vec3 eye = player.pos + Vec3{0, player.eyeH, 0};
    Mat4 view = matLookAt(eye, eye + player.lookDir(), {0, 1, 0});
    Mat4 vp = proj * view;

    int pcx = flr(player.pos.x / CS), pcz = flr(player.pos.z / CS);
    for (int dx = -RD; dx <= RD; ++dx) {
        for (int dz = -RD; dz <= RD; ++dz) {
            auto ch = world.getChunk(pcx + dx, pcz + dz);
            if (ch && ch->generated) {
                for (int x = 0; x < CS; x += 2) {
                    for (int z = 0; z < CS; z += 2) {
                        int h = ch->heightMap[x * CS + z];
                        drawBlockProcedural(vp, {(float)(ch->cx * CS + x), (float)h, (float)(ch->cz * CS + z)},
                                            {1.0f, 1.0f, 1.0f}, GRASS, 0.0f);
                    }
                }
            }
        }
    }

    std::lock_guard lk(world.entityMtx);
    for (const auto& e : world.entities) {
        if (e.alive) drawMobProcedural(vp, e);
    }
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

    gState->world->loadWorld(gState->saveDirectory);
    for (int dx = -3; dx <= 3; ++dx) {
        for (int dz = -3; dz <= 3; ++dz) {
            auto ch = gState->world->getOrCreate(dx, dz);
            if (!ch->generated) gState->world->generateChunk(*ch);
        }
    }

    gState->player.pos = {0, 75, 0};
    gState->player.inv.add(OAK_PLANKS, 64);
    gState->player.inv.add(COBBLESTONE, 64);
    gState->player.inv.add(DIAMOND_BLOCK, 64);
    gState->initialized = true;
    LOGI("AAA OmniCraft Başarıyla Başlatıldı [%dx%d]", w, h);
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
    float sdt = clampf(dt, 0.0f, 0.05f);
    gs.time += sdt;

    float yaw = gs.player.yaw * (3.14159265f / 180.0f);
    float spd = gs.player.sprinting ? SPRINT_SPD : (gs.player.sneaking ? SNEAK_SPD : WALK_SPD);
    float fwd = -gs.joySY * spd, str = gs.joySX * spd;
    gs.player.vel.x += (cosf(-yaw) * str + sinf(yaw) * fwd) * sdt * 10.0f;
    gs.player.vel.z += (sinf(-yaw) * str - cosf(yaw) * fwd) * sdt * 10.0f;
    gs.player.pos += gs.player.vel * sdt;
    gs.player.vel.x *= 0.82f; gs.player.vel.z *= 0.82f;

    // Yerdeki eşyaları otomatik toplama
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
    gs.renderer.frame(*gs.world, gs.player, gs.time);
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeJoystick(JNIEnv*, jclass, jfloat x, jfloat y) {
    if (gState) { gState->joySX = x; gState->joySY = y; }
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
        // Blok Kır ve Yere 3D Eşya Düşür
        uint8_t oldB = gState->world->blockAt(hit.block.x, hit.block.y, hit.block.z);
        gState->world->setBlock(hit.block.x, hit.block.y, hit.block.z, AIR);
        uint16_t drop = BD(oldB).dropId;
        if (drop != AIR) {
            gState->world->spawnItemDrop(drop, {(float)hit.block.x + 0.5f, (float)hit.block.y + 0.5f, (float)hit.block.z + 0.5f}, {0, 2.5f, 0});
        }
    } else if (type == 1 && hit.hit) {
        // Blok Yerleştir
        ItemStack& held = gState->player.inv.active();
        if (!held.empty() && held.id < BLOCK_COUNT) {
            gState->world->setBlock(hit.block.x + hit.face.x, hit.block.y + hit.face.y, hit.block.z + hit.face.z, (uint8_t)held.id);
            held.count--;
            if (held.count == 0) held = ItemStack();
        }
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeDropItem(JNIEnv*, jclass) {
    if (!gState) return;
    ItemStack& held = gState->player.inv.active();
    if (!held.empty()) {
        Vec3 p = gState->player.pos + Vec3{0, 1.4f, 0};
        Vec3 v = gState->player.lookDir() * 5.0f;
        gState->world->spawnItemDrop(held.id, p, v);
        held.count--;
        if (held.count == 0) held = ItemStack();
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeJump(JNIEnv*, jclass) {
    if (gState) gState->player.pos.y += 1.3f;
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
