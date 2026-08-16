#include "Craft.hpp"

static char gCrashLogDir[512] = {0};

static void nativeSignalCrashHandler(int sig, siginfo_t* info, void*) {
    char filePath[600];
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    snprintf(filePath, sizeof(filePath), "%s/crash_native_%04d%02d%02d_%02d%02d%02d.log",
             gCrashLogDir[0] ? gCrashLogDir : "/sdcard/Documents/Craft_Log",
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    int fd = open(filePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd >= 0) {
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "=== OMNI CRAFT AAA CRASH RAPORU ===\n"
                 "Sinyal: %d\n"
                 "Hata Adresi: %p\n"
                 "Tarih: %04d-%02d-%02d %02d:%02d:%02d\n"
                 "Durum: Is parcacigi kritik sinyal ile sonlandi.\n",
                 sig, info->si_addr,
                 timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        write(fd, buf, strlen(buf));
        close(fd);
    }
    LOGE("Yerel motor cokmesi kaydedildi: %s", filePath);
    _exit(1);
}

static constexpr BlockDef BLOCK_TABLE[BLOCK_COUNT] = {
    {"hava",           false,true, false,false,false,false, 0.0f,  0.0f,  0,0,  0, 0, 0,  0,0,  0.6f, AIR},
    {"cimen",          true, false,false,false,false,false, 0.6f,  0.6f,  1,0,  1, 2, 3,  0,0,  0.6f, DIRT},
    {"toprak",         true, false,false,false,false,false, 0.5f,  0.5f,  1,0,  3, 3, 3,  0,0,  0.6f, DIRT},
    {"tas",            true, false,false,false,false,false, 1.5f,  6.0f,  2,1,  4, 4, 4,  0,0,  0.6f, COBBLESTONE},
    {"kiriktas",       true, false,false,false,false,false, 2.0f,  6.0f,  2,1,  5, 5, 5,  0,0,  0.6f, COBBLESTONE},
    {"kum",            true, false,false,true, false,false, 0.5f,  0.5f,  1,0,  6, 6, 6,  0,0,  0.4f, SAND},
    {"cakil",          true, false,false,true, false,false, 0.6f,  0.6f,  1,0,  7, 7, 7,  0,0,  0.4f, GRAVEL},
    {"mese_kutugu",    true, false,false,false,false,false, 2.0f,  2.0f,  3,0,  8, 9, 8,  0,0,  0.6f, OAK_LOG},
    {"mese_yapragi",   true, true, false,false,false,false, 0.2f,  0.2f,  1,0, 10,10,10,  0,1,  0.6f, AIR},
    {"mese_tahtasi",   true, false,false,false,false,false, 2.0f,  3.0f,  3,0, 11,11,11,  0,0,  0.6f, OAK_PLANKS},
    {"ny_kutugu",      true, false,false,false,false,false, 2.0f,  2.0f,  3,0, 12,13,12,  0,0,  0.6f, BIRCH_LOG},
    {"ny_tahtasi",     true, false,false,false,false,false, 2.0f,  3.0f,  3,0, 14,14,14,  0,0,  0.6f, BIRCH_PLANKS},
    {"ny_yapragi",     true, true, false,false,false,false, 0.2f,  0.2f,  1,0, 15,15,15,  0,1,  0.6f, AIR},
    {"cam_kutugu",     true, false,false,false,false,false, 2.0f,  2.0f,  3,0, 16,17,16,  0,0,  0.6f, SPRUCE_LOG},
    {"cam_tahtasi",    true, false,false,false,false,false, 2.0f,  3.0f,  3,0, 18,18,18,  0,0,  0.6f, SPRUCE_PLANKS},
    {"cam_yapragi",    true, true, false,false,false,false, 0.2f,  0.2f,  1,0, 19,19,19,  0,1,  0.6f, AIR},
    {"komur_cevheri",  true, false,false,false,false,false, 3.0f,  3.0f,  2,1, 20,20,20,  0,0,  0.6f, COAL_ORE},
    {"demir_cevheri",  true, false,false,false,false,false, 3.0f,  3.0f,  2,2, 21,21,21,  0,0,  0.6f, IRON_ORE},
    {"altin_cevheri",  true, false,false,false,false,false, 3.0f,  3.0f,  2,3, 22,22,22,  0,0,  0.6f, GOLD_ORE},
    {"elmas_cevheri",  true, false,false,false,false,false, 3.0f,  3.0f,  2,3, 23,23,23,  0,0,  0.6f, DIAMOND_ORE},
    {"zumrut_cevheri", true, false,false,false,false,false, 3.0f,  3.0f,  2,3, 24,24,24,  0,0,  0.6f, EMERALD_ORE},
    {"kiziltas_cevh",  true, false,false,false,false,false, 3.0f,  3.0f,  2,2, 25,25,25,  0,0,  0.6f, REDSTONE_ORE},
    {"lapis_cevheri",  true, false,false,false,false,false, 3.0f,  3.0f,  2,2, 26,26,26,  0,0,  0.6f, LAPIS_ORE},
    {"komur_bloku",    true, false,false,false,false,false, 5.0f,  6.0f,  2,1, 27,27,27,  0,0,  0.6f, COAL_BLOCK},
    {"demir_bloku",    true, false,false,false,false,false, 5.0f, 10.0f,  2,2, 28,28,28,  0,0,  0.6f, IRON_BLOCK},
    {"altin_bloku",    true, false,false,false,false,false, 3.0f, 10.0f,  2,3, 29,29,29,  0,0,  0.6f, GOLD_BLOCK},
    {"elmas_bloku",    true, false,false,false,false,false, 5.0f, 10.0f,  2,3, 30,30,30,  0,0,  0.6f, DIAMOND_BLOCK},
    {"su",             false,true, true, false,false,false,100.f,100.f,  0,0, 31,31,31,  0,2,  0.2f, AIR},
    {"lav",            false,true, true, false,false,true, 100.f,100.f,  0,0, 32,32,32, 15,2,  0.2f, AIR},
    {"cam",            true, true, false,false,false,false, 0.3f,  1.5f,  1,0, 33,33,33,  0,0,  0.6f, AIR},
    {"isiktasi",       true, true, false,false,false,true,  0.3f,  1.5f,  1,0, 34,34,34, 15,0,  0.6f, GLOWSTONE},
    {"nether_tasi",    true, false,false,false,false,false, 0.4f,  0.4f,  2,1, 35,35,35,  0,0,  0.6f, NETHERRACK},
    {"uretim_masasi",  true, false,false,false,false,false, 2.5f,  2.5f,  3,0, 36,37,11,  0,0,  0.6f, CRAFTING_TABLE},
    {"firin",          true, false,false,false,false,false, 3.5f,  3.5f,  2,1, 38,39,38,  0,0,  0.6f, FURNACE},
    {"sandik",         true, false,false,false,false,false, 2.5f,  2.5f,  3,0, 40,40,40,  0,0,  0.6f, CHEST},
    {"merdiven",       false,true, false,false,true, false, 0.4f,  0.4f,  3,0, 41,41,41,  0,0,  0.6f, LADDER},
    {"mesale",         false,true, false,false,false,true,  0.0f,  0.0f,  0,0, 42,42,42, 14,0,  0.6f, TORCH},
    {"katman_kayasi",  true, false,false,false,false,false, -1.f,3600.f,  0,0, 43,43,43,  0,0,  0.6f, BEDROCK},
    {"kar_katmani",    true, false,false,false,false,false, 0.2f,  0.2f,  1,0, 44,44,44,  0,0,  0.3f, SNOW_LAYER},
    {"buz",            true, true, false,false,false,false, 0.5f,  0.5f,  1,0, 45,45,45,  0,1,  0.02f,AIR},
    {"kil",            true, false,false,false,false,false, 0.6f,  0.6f,  1,0, 46,46,46,  0,0,  0.6f, CLAY},
    {"tugla",          true, false,false,false,false,false, 2.0f,  6.0f,  2,1, 47,47,47,  0,0,  0.6f, BRICK},
    {"tas_tugla",      true, false,false,false,false,false, 1.5f,  6.0f,  2,1, 48,48,48,  0,0,  0.6f, STONE_BRICKS},
    {"yosunlu_tas",    true, false,false,false,false,false, 2.0f,  6.0f,  2,1, 49,49,49,  0,0,  0.6f, MOSSY_COBBLE},
    {"tnt",            true, false,false,false,false,false, 0.0f,  0.0f,  1,0, 50,51,50,  0,0,  0.6f, TNT},
    {"kitaplik",       true, false,false,false,false,false, 1.5f,  1.5f,  3,0, 11,52,11,  0,0,  0.6f, OAK_PLANKS},
    {"balkabagi",      true, false,false,false,false,false, 1.0f,  1.0f,  1,0, 53,54,53,  0,0,  0.6f, PUMPKIN},
    {"karpuz",         true, false,false,false,false,false, 1.0f,  1.0f,  1,0, 55,56,55,  0,0,  0.6f, MELON},
    {"kumtasi",        true, false,false,false,false,false, 0.8f,  4.0f,  2,1, 57,58,59,  0,0,  0.6f, SANDSTONE},
    {"kaktus",         true, true, false,false,false,false, 0.4f,  0.4f,  1,0, 60,61,62,  0,1,  0.6f, CACTUS},
    {"sunger",         true, false,false,false,false,false, 0.6f,  0.6f,  1,0, 63,63,63,  0,0,  0.6f, SPONGE},
    {"bugday_0",       false,true, false,false,false,false, 0.0f,  0.0f,  0,0, 64,64,64,  0,0,  0.6f, AIR},
    {"bugday_1",       false,true, false,false,false,false, 0.0f,  0.0f,  0,0, 65,65,65,  0,0,  0.6f, AIR},
    {"bugday_2",       false,true, false,false,false,false, 0.0f,  0.0f,  0,0, 66,66,66,  0,0,  0.6f, AIR},
    {"bugday_3",       false,true, false,false,false,false, 0.0f,  0.0f,  0,0, 67,67,67,  0,0,  0.6f, AIR},
    {"bugday_4",       false,true, false,false,false,false, 0.0f,  0.0f,  0,0, 68,68,68,  0,0,  0.6f, AIR},
    {"bugday_5",       false,true, false,false,false,false, 0.0f,  0.0f,  0,0, 69,69,69,  0,0,  0.6f, AIR},
    {"bugday_6",       false,true, false,false,false,false, 0.0f,  0.0f,  0,0, 70,70,70,  0,0,  0.6f, AIR},
    {"bugday_7",       false,true, false,false,false,false, 0.0f,  0.0f,  0,0, 71,71,71,  0,0,  0.6f, ITEM_WHEAT_ITEM},
};

static inline const BlockDef& BD(uint8_t id) { return BLOCK_TABLE[id < BLOCK_COUNT ? id : 0]; }

static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline int flr(float f) { return static_cast<int>(std::floor(f)); }

static Mat4 matPerspective(float fov, float asp, float near_, float far_) {
    Mat4 m;
    float f = 1.0f / tanf(fov * 0.5f);
    m.m[0] = f / asp;
    m.m[5] = f;
    m.m[10] = (far_ + near_) / (near_ - far_);
    m.m[11] = -1.0f;
    m.m[14] = (2.0f * far_ * near_) / (near_ - far_);
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

static Mat4 matOrtho(float l, float r, float b, float t, float n, float fa) {
    Mat4 m;
    m.m[0] = 2.0f / (r - l);
    m.m[5] = 2.0f / (t - b);
    m.m[10] = -2.0f / (fa - n);
    m.m[12] = -(r + l) / (r - l);
    m.m[13] = -(t + b) / (t - b);
    m.m[14] = -(fa + n) / (fa - n);
    m.m[15] = 1.0f;
    return m;
}

static Frustum buildFrustum(const Mat4& vp) {
    Frustum f;
    const auto& m = vp.m;
    auto norm = [](Plane& p) {
        float l = sqrtf(p.a * p.a + p.b * p.b + p.c * p.c) + 1e-8f;
        p.a /= l; p.b /= l; p.c /= l; p.d /= l;
    };
    f.planes[0] = {m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]}; norm(f.planes[0]);
    f.planes[1] = {m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]}; norm(f.planes[1]);
    f.planes[2] = {m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]}; norm(f.planes[2]);
    f.planes[3] = {m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]}; norm(f.planes[3]);
    f.planes[4] = {m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]}; norm(f.planes[4]);
    f.planes[5] = {m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]}; norm(f.planes[5]);
    return f;
}

static bool aabbInFrustum(const Frustum& fr, float x, float y, float z, float sz) {
    for (const auto& p : fr.planes) {
        float d = p.a * (p.a > 0 ? x + sz : x) + p.b * (p.b > 0 ? y + sz : y) + p.c * (p.c > 0 ? z + sz : z) + p.d;
        if (d < 0) return false;
    }
    return true;
}

Noise::Noise(uint64_t seed) {
    std::mt19937_64 rng(seed);
    for (int i = 0; i < 256; ++i) perm[i] = i;
    for (int i = 255; i > 0; --i) {
        int j = static_cast<int>(rng() % (i + 1));
        std::swap(perm[i], perm[j]);
    }
    for (int i = 0; i < 256; ++i) perm[256 + i] = perm[i];
}
float Noise::fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float Noise::grad(int h, float x, float y, float z) {
    int hh = h & 15;
    float u = hh < 8 ? x : y, v = hh < 4 ? y : (hh == 12 || hh == 14 ? x : z);
    return ((hh & 1) ? -u : u) + ((hh & 2) ? -v : v);
}
float Noise::at(float x, float y, float z) {
    int X = flr(x) & 255, Y = flr(y) & 255, Z = flr(z) & 255;
    x -= std::floor(x); y -= std::floor(y); z -= std::floor(z);
    float u = fade(x), v = fade(y), w = fade(z);
    int A = perm[X] + Y, AA = perm[A] + Z, AB = perm[A + 1] + Z;
    int B = perm[X + 1] + Y, BA = perm[B] + Z, BB = perm[B + 1] + Z;
    return lerp(lerp(lerp(grad(perm[AA], x, y, z), grad(perm[BA], x - 1, y, z), u),
                     lerp(grad(perm[AB], x, y - 1, z), grad(perm[BB], x - 1, y - 1, z), u), v),
                lerp(lerp(grad(perm[AA + 1], x, y, z - 1), grad(perm[BA + 1], x - 1, y, z - 1), u),
                     lerp(grad(perm[AB + 1], x, y - 1, z - 1), grad(perm[BB + 1], x - 1, y - 1, z - 1), u), v), w);
}
float Noise::fbm(float x, float y, float z, int oct, float per) {
    float v = 0, a = 1, f = 1, mx = 0;
    for (int i = 0; i < oct; ++i) {
        v += at(x * f, y * f, z * f) * a;
        mx += a;
        a *= per;
        f *= 2.0f;
    }
    return v / mx;
}

ThreadPool::ThreadPool(int n) {
    for (int i = 0; i < n; ++i) workers.emplace_back([this] {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lk(mtx);
                cv.wait(lk, [this] { return stopping || !tasks.empty(); });
                if (stopping && tasks.empty()) return;
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
            pending--;
        }
    });
}
ThreadPool::~ThreadPool() {
    {
        std::unique_lock lk(mtx);
        stopping = true;
    }
    cv.notify_all();
    for (auto& w : workers) if (w.joinable()) w.join();
}
template<typename F>
void ThreadPool::enqueue(F&& f) {
    pending++;
    {
        std::lock_guard lk(mtx);
        tasks.emplace(std::forward<F>(f));
    }
    cv.notify_one();
}

bool Inventory::add(uint16_t id, int cnt) {
    for (int i = 0; i < INV_SIZE && cnt > 0; ++i)
        if (slots[i].id == id && slots[i].count < MAX_STACK) {
            int a = std::min(cnt, static_cast<int>(MAX_STACK - slots[i].count));
            slots[i].count += a;
            cnt -= a;
        }
    for (int i = 0; i < INV_SIZE && cnt > 0; ++i)
        if (slots[i].empty()) {
            int a = std::min(cnt, MAX_STACK);
            slots[i] = ItemStack(id, a);
            cnt -= a;
        }
    return cnt == 0;
}

Chunk::Chunk(int x, int z) : cx(x), cz(z) {
    blocks.fill(0);
    skyLight.fill(15);
    blockLight.fill(0);
    biome.fill(0);
    heightMap.fill(0);
}
uint8_t Chunk::get(int x, int y, int z) const {
    if (x < 0 || x >= CS || y < 0 || y >= WH || z < 0 || z >= CS) return AIR;
    return blocks[idx(x, y, z)];
}
void Chunk::set(int x, int y, int z, uint8_t b) {
    if (x < 0 || x >= CS || y < 0 || y >= WH || z < 0 || z >= CS) return;
    blocks[idx(x, y, z)] = b;
    meshDirty = true;
    if (y > heightMap[x * CS + z]) heightMap[x * CS + z] = y;
}
uint8_t Chunk::sl(int x, int y, int z) const {
    if (x < 0 || x >= CS || y < 0 || y >= WH || z < 0 || z >= CS) return 15;
    return skyLight[idx(x, y, z)];
}
uint8_t Chunk::bl(int x, int y, int z) const {
    if (x < 0 || x >= CS || y < 0 || y >= WH || z < 0 || z >= CS) return 0;
    return blockLight[idx(x, y, z)];
}

World::World(uint64_t s) : seed(s) {
    hNoise    = std::make_unique<Noise>(s);
    cNoise    = std::make_unique<Noise>(s + 1);
    biNoise   = std::make_unique<Noise>(s + 2);
    caveNoise = std::make_unique<Noise>(s + 3);
    treeNoise = std::make_unique<Noise>(s + 4);
    humNoise  = std::make_unique<Noise>(s + 5);
}

World::ChunkPtr World::getChunk(int cx, int cz) const {
    uint64_t k = key(cx, cz);
    std::shared_lock lk(chunkMtx);
    auto it = chunks.find(k);
    return it != chunks.end() ? it->second : nullptr;
}

World::ChunkPtr World::getOrCreate(int cx, int cz) {
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

World::Biome World::biomeAt(int wx, int wz) {
    float t = biNoise->fbm(wx * 0.0015f, 0, wz * 0.0015f, 3, 0.5f);
    float h = humNoise->fbm(wx * 0.0015f + 200, 0, wz * 0.0015f + 200, 3, 0.5f);
    float el = hNoise->fbm(wx * 0.003f, 0, wz * 0.003f, 6, 0.5f);
    if (el > 0.55f)  return MOUNTAINS;
    if (el < -0.35f) return OCEAN;
    if (t > 0.45f && h < -0.1f) return DESERT;
    if (t < -0.4f && h > 0.1f)  return TAIGA;
    if (t < -0.55f)          return TUNDRA;
    if (t > 0.3f && h > 0.3f)   return JUNGLE;
    return h > 0.05f ? FOREST : PLAINS;
}

int World::surfaceAt(int wx, int wz, Biome b) {
    float base = hNoise->fbm(wx * 0.003f, 0, wz * 0.003f, 6, 0.5f);
    float cont = cNoise->fbm(wx * 0.001f, 0, wz * 0.001f, 4, 0.5f);
    float h;
    switch (b) {
        case MOUNTAINS: h = 72 + (base * 90 + cont * 40); break;
        case OCEAN:     h = 48 + (base * 16);             break;
        case DESERT:    h = 63 + (base * 12 + cont * 5);  break;
        case TAIGA:
        case TUNDRA:    h = 62 + (base * 18 + cont * 8);  break;
        default:        h = 64 + (base * 22 + cont * 10); break;
    }
    return static_cast<int>(clampf(h, 2.0f, 220.0f));
}

void World::generateChunk(Chunk& c) {
    int bx = c.cx * CS, bz = c.cz * CS;
    for (int x = 0; x < CS; ++x) {
        for (int z = 0; z < CS; ++z) {
            int wx = bx + x, wz = bz + z;
            Biome bio = static_cast<Biome>(biomeAt(wx, wz));
            c.biome[x * CS + z] = static_cast<uint8_t>(bio);
            int surf = surfaceAt(wx, wz, bio);
            c.heightMap[x * CS + z] = surf;
            c.blocks[c.idx(x, 0, z)] = BEDROCK;
            for (int y = 1; y < 4; ++y) c.blocks[c.idx(x, y, z)] = BEDROCK;
            for (int y = 4; y < surf - 3 && y < WH; ++y) {
                float cv = caveNoise->fbm(wx * 0.04f, y * 0.04f, wz * 0.04f, 3, 0.5f);
                c.blocks[c.idx(x, y, z)] = (cv > 0.18f) ? AIR : STONE;
            }
            for (int y = std::max(4, surf - 3); y < surf && y < WH; ++y)
                c.blocks[c.idx(x, y, z)] = (bio == DESERT || bio == OCEAN) ? SAND : DIRT;
            if (surf > 0 && surf < WH) {
                switch (bio) {
                    case DESERT: c.blocks[c.idx(x, surf, z)] = SAND; break;
                    case OCEAN:  c.blocks[c.idx(x, surf, z)] = (surf < 60) ? SAND : DIRT; break;
                    case TUNDRA: c.blocks[c.idx(x, surf, z)] = SNOW_LAYER; break;
                    default:     c.blocks[c.idx(x, surf, z)] = GRASS; break;
                }
            }
            for (int y = surf + 1; y < 62 && y < WH; ++y)
                if (c.blocks[c.idx(x, y, z)] == AIR) c.blocks[c.idx(x, y, z)] = WATER;
        }
    }
    placeOres(c, COAL_ORE, 17, 24, 0, 128);
    placeOres(c, IRON_ORE, 9, 10, 0, 64);
    placeOres(c, GOLD_ORE, 9, 4, 0, 34);
    placeOres(c, DIAMOND_ORE, 8, 2, 0, 18);

    std::mt19937_64 rng(seed ^ ((static_cast<uint64_t>(c.cx) * 0x9E3779B97F4A7C15ULL) ^
                                (static_cast<uint64_t>(c.cz) * 0x6C62272E07BB0142ULL)));
    for (int x = 2; x < CS - 2; ++x) {
        for (int z = 2; z < CS - 2; ++z) {
            Biome bio = static_cast<Biome>(c.biome[x * CS + z]);
            int surf = c.heightMap[x * CS + z];
            if (surf < 4 || surf >= WH - 8) continue;
            float rnd = static_cast<float>(rng() % 1000) / 1000.0f;
            float treeDens = (bio == FOREST) ? 0.06f : (bio == JUNGLE) ? 0.10f : (bio == TAIGA) ? 0.05f :
                             (bio == PLAINS) ? 0.012f : 0.0f;
            if (rnd < treeDens) placeTree(c, x, surf + 1, z, static_cast<int>(bio), rng);
        }
    }
    propagateSkyLight(c);
    c.generated = true;
}

void World::placeOres(Chunk& c, uint8_t ore, int sz, int count, int minY, int maxY) {
    std::mt19937_64 rng(seed ^ (static_cast<uint64_t>(c.cx) * 12345 + ore) ^ static_cast<uint64_t>(c.cz) * 67891);
    for (int i = 0; i < count; ++i) {
        int ox = static_cast<int>(rng() % CS), oy = minY + static_cast<int>(rng() % (maxY - minY + 1)), oz = static_cast<int>(rng() % CS);
        for (int j = 0; j < sz; ++j) {
            int bx = ox + static_cast<int>(rng() % 3) - 1, by = oy + static_cast<int>(rng() % 3) - 1, bz = oz + static_cast<int>(rng() % 3) - 1;
            if (bx >= 0 && bx < CS && by > 4 && by < WH && bz >= 0 && bz < CS && c.blocks[c.idx(bx, by, bz)] == STONE)
                c.blocks[c.idx(bx, by, bz)] = ore;
        }
    }
}

void World::placeTree(Chunk& c, int x, int y, int z, int bio, std::mt19937_64& rng) {
    uint8_t log = OAK_LOG, leaf = OAK_LEAVES;
    int h = 5 + static_cast<int>(rng() % 3);
    if (bio == 4 || bio == 6) { log = SPRUCE_LOG; leaf = SPRUCE_LEAVES; h += 2; }
    else if (bio == 1) { log = BIRCH_LOG; leaf = BIRCH_LEAVES; }
    for (int i = 0; i < h && y + i < WH; ++i) c.blocks[c.idx(x, y + i, z)] = log;
    int lh = y + h;
    for (int dy = -2; dy <= 1; ++dy) {
        for (int dx = -2; dx <= 2; ++dx) {
            for (int dz = -2; dz <= 2; ++dz) {
                if (std::abs(dx) + std::abs(dz) + (dy == 1 ? 1 : 0) > 2) continue;
                int lx = x + dx, ly = lh + dy, lz = z + dz;
                if (lx >= 0 && lx < CS && ly >= 0 && ly < WH && lz >= 0 && lz < CS && c.blocks[c.idx(lx, ly, lz)] == AIR)
                    c.blocks[c.idx(lx, ly, lz)] = leaf;
            }
        }
    }
}

void World::propagateSkyLight(Chunk& c) {
    c.skyLight.fill(0);
    for (int x = 0; x < CS; ++x) {
        for (int z = 0; z < CS; ++z) {
            int lv = 15;
            for (int y = WH - 1; y >= 0; --y) {
                uint8_t b = c.blocks[c.idx(x, y, z)];
                lv = std::max(0, lv - BD(b).lightAbsorb);
                c.skyLight[c.idx(x, y, z)] = static_cast<uint8_t>(std::max(static_cast<int>(BD(b).lightEmit), lv));
            }
        }
    }
}

uint8_t World::blockAt(int wx, int wy, int wz) const {
    if (wy < 0) return BEDROCK;
    if (wy >= WH) return AIR;
    int cx2 = flr(static_cast<float>(wx) / CS), cz2 = flr(static_cast<float>(wz) / CS);
    int lx = ((wx % CS) + CS) % CS, lz = ((wz % CS) + CS) % CS;
    auto c = getChunk(cx2, cz2);
    return c ? c->blocks[c->idx(lx, wy, lz)] : AIR;
}

uint8_t World::lightAt(int wx, int wy, int wz) const {
    if (wy < 0 || wy >= WH) return 0;
    int cx2 = flr(static_cast<float>(wx) / CS), cz2 = flr(static_cast<float>(wz) / CS);
    int lx = ((wx % CS) + CS) % CS, lz = ((wz % CS) + CS) % CS;
    auto c = getChunk(cx2, cz2);
    if (!c) return 15;
    int i = c->idx(lx, wy, lz);
    return std::max(c->skyLight[i], c->blockLight[i]);
}

void World::setBlock(int wx, int wy, int wz, uint8_t b) {
    if (wy < 0 || wy >= WH) return;
    int cx2 = flr(static_cast<float>(wx) / CS), cz2 = flr(static_cast<float>(wz) / CS);
    int lx = ((wx % CS) + CS) % CS, lz = ((wz % CS) + CS) % CS;
    auto c = getChunk(cx2, cz2);
    if (!c) return;
    c->set(lx, wy, lz, b);
    propagateSkyLight(*c);
}

World::RayHit World::raycast(Vec3 origin, Vec3 dir, float maxD) const {
    RayHit r{false};
    float len = dir.len();
    if (len < 1e-6f) return r;
    dir = dir.norm();
    int ix = flr(origin.x), iy = flr(origin.y), iz = flr(origin.z);
    float sX = dir.x > 0 ? 1.0f : -1.0f, sY = dir.y > 0 ? 1.0f : -1.0f, sZ = dir.z > 0 ? 1.0f : -1.0f;
    float tDX = fabsf(dir.x) < 1e-6f ? 1e30f : 1.0f / fabsf(dir.x);
    float tDY = fabsf(dir.y) < 1e-6f ? 1e30f : 1.0f / fabsf(dir.y);
    float tDZ = fabsf(dir.z) < 1e-6f ? 1e30f : 1.0f / fabsf(dir.z);
    float tX = fabsf(((dir.x > 0 ? (ix + 1) : ix) - origin.x) / (dir.x + 1e-8f));
    float tY = fabsf(((dir.y > 0 ? (iy + 1) : iy) - origin.y) / (dir.y + 1e-8f));
    float tZ = fabsf(((dir.z > 0 ? (iz + 1) : iz) - origin.z) / (dir.z + 1e-8f));
    Vec3i prev = {ix, iy, iz};
    for (int step = 0; step < 300; ++step) {
        float t = std::min({tX, tY, tZ});
        if (t > maxD) break;
        uint8_t b = blockAt(ix, iy, iz);
        if (b != AIR && BD(b).solid) {
            r.hit = true;
            r.block = {ix, iy, iz};
            r.dist = t;
            r.face = {ix - prev.x, iy - prev.y, iz - prev.z};
            return r;
        }
        prev = {ix, iy, iz};
        if (tX < tY && tX < tZ) { tX += tDX; ix += static_cast<int>(sX); }
        else if (tY < tZ)        { tY += tDY; iy += static_cast<int>(sY); }
        else                    { tZ += tDZ; iz += static_cast<int>(sZ); }
    }
    return r;
}

static const char* VS_WORLD = R"(#version 300 es
precision highp float;
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in float aAO;
layout(location=3) in float aLight;
uniform mat4 uMVP;
uniform vec3 uCamPos;
uniform float uFogStart, uFogEnd;
out vec2 vUV;
out float vAO, vLight, vFog;
void main(){
    gl_Position = uMVP * vec4(aPos, 1.0);
    vUV = aUV; vAO = aAO; vLight = aLight;
    float dist = length(aPos - uCamPos);
    vFog = clamp((dist - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
})";

static const char* FS_WORLD = R"(#version 300 es
precision highp float;
in vec2 vUV;
in float vAO, vLight, vFog;
uniform sampler2D uAtlas;
uniform vec3 uFogColor;
uniform float uDayLight;
out vec4 fragColor;
void main(){
    vec4 tex = texture(uAtlas, vUV);
    if(tex.a < 0.1) discard;
    float ao = mix(0.4, 1.0, vAO);
    float light = max(vLight * uDayLight * ao, 0.05);
    vec3 col = mix(tex.rgb * light, uFogColor, vFog);
    fragColor = vec4(col, tex.a);
})";

static const char* VS_UI = R"(#version 300 es
precision mediump float;
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in vec4 aColor;
uniform mat4 uProj;
out vec2 vUV;
out vec4 vColor;
void main(){ gl_Position = uProj * vec4(aPos, 0, 1); vUV = aUV; vColor = aColor; })";

static const char* FS_UI = R"(#version 300 es
precision mediump float;
in vec2 vUV;
in vec4 vColor;
out vec4 fragColor;
void main(){ fragColor = vColor; })";

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}
static GLuint linkProg(const char* vs, const char* fs) {
    GLuint v = compileShader(GL_VERTEX_SHADER, vs), f = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f); glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

static uint32_t generateAtlasTexture() {
    const int TS = 16, AS = TS * ATLAS_DIM;
    std::vector<uint32_t> data(AS * AS, 0xFF808080u);
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, AS, AS, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}

void Renderer::init(int w, int h) {
    screenW = w; screenH = h;
    worldProg = linkProg(VS_WORLD, FS_WORLD);
    uiProg    = linkProg(VS_UI, FS_UI);
    atlasTex  = generateAtlasTexture();
    glGenVertexArrays(1, &uiVAO); glGenBuffers(1, &uiVBO);
    particles.resize(512);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void Renderer::resize(int w, int h) {
    screenW = w; screenH = h;
    glViewport(0, 0, w, h);
}

void Renderer::addRect(float x0, float y0, float x1, float y1, float r, float g, float b, float a) {
    uiVerts.push_back({x0, y0, 0, 0, r, g, b, a}); uiVerts.push_back({x1, y0, 1, 0, r, g, b, a});
    uiVerts.push_back({x1, y1, 1, 1, r, g, b, a}); uiVerts.push_back({x0, y0, 0, 0, r, g, b, a});
    uiVerts.push_back({x1, y1, 1, 1, r, g, b, a}); uiVerts.push_back({x0, y1, 0, 1, r, g, b, a});
}

void Renderer::flushUI() {
    if (uiVerts.empty()) return;
    Mat4 ortho = matOrtho(0, static_cast<float>(screenW), static_cast<float>(screenH), 0, -1, 1);
    glUseProgram(uiProg);
    glUniformMatrix4fv(glGetUniformLocation(uiProg, "uProj"), 1, GL_FALSE, ortho.m);
    glBindVertexArray(uiVAO); glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(uiVerts.size() * sizeof(UIVertex)), uiVerts.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(UIVertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(UIVertex), reinterpret_cast<void*>(8));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(UIVertex), reinterpret_cast<void*>(16));
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(uiVerts.size()));
    uiVerts.clear();
    glBindVertexArray(0);
}

void Renderer::frame(World& world, Player& player, float) {
    glClearColor(0.5f, 0.75f, 0.95f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    Mat4 proj = matPerspective(1.222f, static_cast<float>(screenW) / static_cast<float>(screenH), 0.05f, 800.0f);
    Vec3 eye = player.eyePos();
    Vec3 tgt = eye + player.lookDir();
    Mat4 view = matLookAt(eye, tgt, {0, 1, 0});
    Mat4 vp = proj * view;

    glUseProgram(worldProg);
    glUniformMatrix4fv(glGetUniformLocation(worldProg, "uMVP"), 1, GL_FALSE, vp.m);
    glUniform3f(glGetUniformLocation(worldProg, "uCamPos"), eye.x, eye.y, eye.z);
    glUniform3f(glGetUniformLocation(worldProg, "uFogColor"), 0.5f, 0.75f, 0.95f);
    glUniform1f(glGetUniformLocation(worldProg, "uFogStart"), static_cast<float>(MESH_RD - 2) * CS);
    glUniform1f(glGetUniformLocation(worldProg, "uFogEnd"),   static_cast<float>(MESH_RD) * CS);
    glUniform1f(glGetUniformLocation(worldProg, "uDayLight"), 1.0f);

    int pcx = flr(player.pos.x / CS), pcz = flr(player.pos.z / CS);
    for (int dx = -RD; dx <= RD; ++dx) {
        for (int dz = -RD; dz <= RD; ++dz) {
            auto ch = world.getChunk(pcx + dx, pcz + dz);
            if (ch && ch->vao && ch->indexCount > 0) {
                glBindVertexArray(ch->vao);
                glDrawElements(GL_TRIANGLES, ch->indexCount, GL_UNSIGNED_INT, nullptr);
            }
        }
    }
    glBindVertexArray(0);
}

static GameState* gState = nullptr;

extern "C" {

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSetupCrashHandler(JNIEnv* env, jclass, jstring logPath) {
    const char* str = env->GetStringUTFChars(logPath, nullptr);
    snprintf(gCrashLogDir, sizeof(gCrashLogDir), "%s", str);
    env->ReleaseStringUTFChars(logPath, str);

    struct sigaction sa{};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = nativeSignalCrashHandler;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);
    sigaction(SIGILL,  &sa, nullptr);
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeInit(JNIEnv*, jclass, jint w, jint h, jlong seed) {
    if (gState) { delete gState; gState = nullptr; }
    gState = new GameState();
    gState->screenW = w; gState->screenH = h;
    gState->world = std::make_unique<World>(static_cast<uint64_t>(seed));
    gState->threadPool = std::make_unique<ThreadPool>(std::max(2, static_cast<int>(std::thread::hardware_concurrency())));
    gState->renderer.init(w, h);

    for (int dx = -3; dx <= 3; ++dx) {
        for (int dz = -3; dz <= 3; ++dz) {
            auto c = gState->world->getOrCreate(dx, dz);
            gState->world->generateChunk(*c);
        }
    }
    for (int y = WH - 1; y >= 0; --y) {
        if (gState->world->blockAt(0, y, 0) != AIR) {
            gState->player.pos.y = y + 1.62f;
            break;
        }
    }
    gState->initialized = true;
    LOGI("OmniCraft AAA Engine Initialized [%dx%d]", w, h);
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeResize(JNIEnv*, jclass, jint w, jint h) {
    if (gState) {
        gState->screenW = w; gState->screenH = h;
        gState->renderer.resize(w, h);
    }
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
    gs.player.vel.x *= 0.85f; gs.player.vel.z *= 0.85f;

    gs.renderer.frame(*gs.world, gs.player, gs.time);

    float cx = gs.screenW * 0.5f, cy = gs.screenH * 0.5f;
    gs.renderer.addRect(cx - 15, cy - 2, cx + 15, cy + 2, 1, 1, 1, 0.8f);
    gs.renderer.addRect(cx - 2, cy - 15, cx + 2, cy + 15, 1, 1, 1, 0.8f);
    gs.renderer.flushUI();
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
    auto hit = gState->world->raycast(gState->player.eyePos(), gState->player.lookDir(), REACH);
    if (type == 1 && hit.hit) {
        gState->world->setBlock(hit.block.x + hit.face.x, hit.block.y + hit.face.y, hit.block.z + hit.face.z, OAK_PLANKS);
    }
}

JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeJump(JNIEnv*, jclass) {
    if (gState) gState->player.pos.y += 1.2f;
}
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSneak(JNIEnv*, jclass, jboolean on) {
    if (gState) gState->player.sneaking = on;
}
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeSprint(JNIEnv*, jclass, jboolean on) {
    if (gState) gState->player.sprinting = on;
}
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeFlyUp(JNIEnv*, jclass, jboolean on) {
    if (gState && on) gState->player.pos.y += 0.8f;
}
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeFlyDown(JNIEnv*, jclass, jboolean on) {
    if (gState && on) gState->player.pos.y -= 0.8f;
}
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeStartBreak(JNIEnv*, jclass) {
    if (!gState) return;
    auto hit = gState->world->raycast(gState->player.eyePos(), gState->player.lookDir(), REACH);
    if (hit.hit) gState->world->setBlock(hit.block.x, hit.block.y, hit.block.z, AIR);
}
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeStopBreak(JNIEnv*, jclass) {}
JNIEXPORT void JNICALL Java_com_omni_craft_Engine_nativeDestroy(JNIEnv*, jclass) {
    if (gState) {
        gState->initialized = false;
        delete gState;
        gState = nullptr;
    }
}

} // extern "C"
