#pragma once

#include <jni.h>
#include <android/log.h>
#include <GLES3/gl32.h>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <functional>
#include <string>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>

#define LOG_TAG "OmniCraft_Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)

static constexpr int CS = 16;
static constexpr int WH = 256;
static constexpr int RD = 10;
static constexpr int MESH_RD = 9;
static constexpr float GRAVITY = -28.0f;
static constexpr float JUMP_VEL = 9.5f;
static constexpr float WALK_SPD = 4.317f;
static constexpr float SPRINT_SPD = 5.612f;
static constexpr float SNEAK_SPD = 1.295f;
static constexpr float REACH = 4.8f;
static constexpr int INV_SIZE = 36;
static constexpr int HOTBAR_SZ = 9;
static constexpr int MAX_STACK = 64;
static constexpr int ATLAS_DIM = 16;

enum BlockID : uint8_t {
    AIR = 0,
    GRASS, DIRT, STONE, COBBLESTONE, SAND, GRAVEL,
    OAK_LOG, OAK_LEAVES, OAK_PLANKS,
    BIRCH_LOG, BIRCH_PLANKS, BIRCH_LEAVES,
    SPRUCE_LOG, SPRUCE_PLANKS, SPRUCE_LEAVES,
    COAL_ORE, IRON_ORE, GOLD_ORE, DIAMOND_ORE, EMERALD_ORE, REDSTONE_ORE, LAPIS_ORE,
    COAL_BLOCK, IRON_BLOCK, GOLD_BLOCK, DIAMOND_BLOCK,
    WATER, LAVA,
    GLASS, GLOWSTONE, NETHERRACK,
    CRAFTING_TABLE, FURNACE, CHEST,
    LADDER, TORCH, BEDROCK,
    SNOW_LAYER, ICE, CLAY,
    BRICK, STONE_BRICKS, MOSSY_COBBLE,
    TNT, BOOKSHELF, PUMPKIN, MELON,
    SANDSTONE, CACTUS, SPONGE,
    WHEAT_0, WHEAT_1, WHEAT_2, WHEAT_3, WHEAT_4, WHEAT_5, WHEAT_6, WHEAT_7,
    BLOCK_COUNT
};

enum ItemID : uint16_t {
    ITEM_NONE = 0,
    ITEM_STICK = 256, ITEM_COAL, ITEM_RAW_IRON, ITEM_IRON_INGOT,
    ITEM_RAW_GOLD, ITEM_GOLD_INGOT, ITEM_DIAMOND, ITEM_EMERALD,
    ITEM_REDSTONE, ITEM_LAPIS,
    ITEM_WOOD_SWORD, ITEM_WOOD_PICKAXE, ITEM_WOOD_AXE, ITEM_WOOD_SHOVEL, ITEM_WOOD_HOE,
    ITEM_STONE_SWORD, ITEM_STONE_PICKAXE, ITEM_STONE_AXE, ITEM_STONE_SHOVEL, ITEM_STONE_HOE,
    ITEM_IRON_SWORD, ITEM_IRON_PICKAXE, ITEM_IRON_AXE, ITEM_IRON_SHOVEL, ITEM_IRON_HOE,
    ITEM_GOLD_SWORD, ITEM_GOLD_PICKAXE, ITEM_GOLD_AXE, ITEM_GOLD_SHOVEL, ITEM_GOLD_HOE,
    ITEM_DIAMOND_SWORD, ITEM_DIAMOND_PICKAXE, ITEM_DIAMOND_AXE, ITEM_DIAMOND_SHOVEL, ITEM_DIAMOND_HOE,
    ITEM_BUCKET, ITEM_WATER_BUCKET, ITEM_LAVA_BUCKET,
    ITEM_BREAD, ITEM_APPLE, ITEM_COOKED_BEEF, ITEM_RAW_BEEF,
    ITEM_SEEDS, ITEM_WHEAT_ITEM,
    ITEM_COUNT
};

struct BlockDef {
    const char* name;
    bool solid, transparent, fluid, gravity, climbable, emitter;
    float hardness, resistance;
    uint8_t toolType, toolLevel;
    uint8_t texTop, texSide, texBottom;
    uint8_t lightEmit, lightAbsorb;
    float friction;
    uint16_t dropId;
};

struct Vec2 { float x, y; };
struct Vec3 {
    float x, y, z;
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const { return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x}; }
    float len() const { return sqrtf(x * x + y * y + z * z); }
    Vec3 norm() const { float l = len() + 1e-8f; return {x / l, y / l, z / l}; }
};
struct Vec3i {
    int x, y, z;
    bool operator==(const Vec3i& o) const { return x == o.x && y == o.y && z == o.z; }
};
struct Mat4 {
    float m[16] = {};
    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                for (int k = 0; k < 4; ++k)
                    r.m[i * 4 + j] += m[i * 4 + k] * o.m[k * 4 + j];
        return r;
    }
};
struct AABB { float x0, y0, z0, x1, y1, z1; };
struct Plane { float a, b, c, d; };
struct Frustum { Plane planes[6]; };

struct ItemStack {
    uint16_t id = 0;
    uint8_t count = 0;
    uint16_t damage = 0;
    ItemStack() = default;
    ItemStack(uint16_t i, uint8_t c, uint16_t d = 0) : id(i), count(c), damage(d) {}
    bool empty() const { return id == 0 || count == 0; }
};

struct Inventory {
    ItemStack slots[INV_SIZE];
    int selected = 0;
    bool add(uint16_t id, int cnt);
    ItemStack& active() { return slots[selected % HOTBAR_SZ]; }
};

struct Chunk {
    int cx, cz;
    std::array<uint8_t, CS * WH * CS> blocks;
    std::array<uint8_t, CS * WH * CS> skyLight;
    std::array<uint8_t, CS * WH * CS> blockLight;
    std::array<uint8_t, CS * CS> biome;
    std::array<int, CS * CS> heightMap;

    std::atomic<bool> generated{false};
    std::atomic<bool> meshDirty{true};
    std::atomic<bool> meshBuilding{false};
    std::atomic<bool> lightDirty{true};

    GLuint vao = 0, vbo = 0, ebo = 0;
    GLuint tVao = 0, tVbo = 0, tEbo = 0;
    std::atomic<int> indexCount{0};
    std::atomic<int> tIndexCount{0};

    std::vector<float> pendingVerts, pendingTVerts;
    std::vector<uint32_t> pendingIdx, pendingTIdx;
    std::mutex meshMutex;
    bool meshReady = false;

    Chunk(int x, int z);
    inline int idx(int x, int y, int z) const { return x * WH * CS + y * CS + z; }
    uint8_t get(int x, int y, int z) const;
    void set(int x, int y, int z, uint8_t b);
    uint8_t sl(int x, int y, int z) const;
    uint8_t bl(int x, int y, int z) const;
};

class Noise {
    int perm[512];
public:
    explicit Noise(uint64_t seed);
    float fade(float t);
    float grad(int h, float x, float y, float z);
    float at(float x, float y, float z);
    float fbm(float x, float y, float z, int oct, float per);
};

class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stopping{false};
    std::atomic<int> pending{0};
public:
    explicit ThreadPool(int n);
    ~ThreadPool();
    template<typename F> void enqueue(F&& f);
    int pendingCount() { return pending.load(); }
};

class World {
public:
    using ChunkPtr = std::shared_ptr<Chunk>;
    std::unordered_map<uint64_t, ChunkPtr> chunks;
    mutable std::shared_mutex chunkMtx;
    uint64_t seed;
    std::unique_ptr<Noise> hNoise, cNoise, biNoise, caveNoise, treeNoise, humNoise;
    int tick = 0;
    float timeOfDay = 6000.0f;
    bool raining = false;

    explicit World(uint64_t s);
    static uint64_t key(int cx, int cz) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32) |
               static_cast<uint64_t>(static_cast<uint32_t>(cz));
    }
    ChunkPtr getChunk(int cx, int cz) const;
    ChunkPtr getOrCreate(int cx, int cz);
    enum Biome { PLAINS = 0, FOREST, DESERT, MOUNTAINS, OCEAN, TAIGA, TUNDRA, JUNGLE };
    Biome biomeAt(int wx, int wz);
    int surfaceAt(int wx, int wz, Biome b);
    void generateChunk(Chunk& c);
    void placeOres(Chunk& c, uint8_t ore, int sz, int count, int minY, int maxY);
    void placeTree(Chunk& c, int x, int y, int z, int bio, std::mt19937_64& rng);
    void propagateSkyLight(Chunk& c);
    uint8_t blockAt(int wx, int wy, int wz) const;
    uint8_t lightAt(int wx, int wy, int wz) const;
    void setBlock(int wx, int wy, int wz, uint8_t b);

    struct RayHit { bool hit; Vec3i block, face; int faceIdx; float dist; };
    RayHit raycast(Vec3 origin, Vec3 dir, float maxD) const;
};

struct Player {
    Vec3 pos{0, 80, 0}, vel{};
    float yaw = 0, pitch = 0;
    float health = 20, maxHp = 20, hunger = 20, maxHunger = 20, saturation = 5;
    float eyeH = 1.62f, fallDist = 0, foodExhaust = 0, sens = 0.25f;
    bool ground = false, inWater = false, inLava = false;
    bool sneaking = false, sprinting = false, flying = false;
    bool creative = false, dead = false;
    int hurtTimer = 0, invulTimer = 0, deathTimer = 0, selSlot = 0;
    float breakProg = 0;
    Vec3i breakTarget{};
    bool breaking = false;
    Inventory inv;
    Vec3i respawn{0, 80, 0};

    AABB bounds() const { return {pos.x - 0.3f, pos.y, pos.z - 0.3f, pos.x + 0.3f, pos.y + 1.8f, pos.z + 0.3f}; }
    Vec3 eyePos() const { return {pos.x, pos.y + eyeH, pos.z}; }
    Vec3 lookDir() const {
        float y2 = yaw * (3.14159265f / 180.0f), p = pitch * (3.14159265f / 180.0f);
        return {cosf(p) * sinf(-y2), sinf(p), cosf(p) * cosf(-y2)};
    }
};

struct Particle {
    Vec3 pos, vel;
    float r, g, b, a, life, maxLife, size;
    bool active;
};

struct ChatMsg { std::string text; float timer; uint32_t color; };
struct UIVertex { float x, y, u, v, r, g, b, a; };

class Renderer {
public:
    GLuint worldProg = 0, waterProg = 0, skyProg = 0, uiProg = 0;
    GLuint atlasTex = 0;
    GLuint skyVAO = 0, skyVBO = 0;
    GLuint uiVAO = 0, uiVBO = 0;
    int screenW = 1, screenH = 1;
    std::vector<UIVertex> uiVerts;
    std::vector<Particle> particles;

    void init(int w, int h);
    void resize(int w, int h);
    void frame(World& world, Player& player, float time);
    void spawnParticles(Vec3 pos, int n = 8);
    void updateParticles(float dt);
    void addRect(float x0, float y0, float x1, float y1, float r, float g, float b, float a);
    void flushUI();
};

struct GameState {
    std::unique_ptr<World> world;
    Player player;
    Renderer renderer;
    std::unique_ptr<ThreadPool> threadPool;
    std::vector<ChatMsg> chatLog;
    std::atomic<bool> initialized{false};
    int screenW = 1, screenH = 1;
    float joySX = 0, joySY = 0;
    float time = 0;
    std::unordered_set<uint64_t> genInFlight;
    std::unordered_set<uint64_t> meshInFlight;
    std::mutex genMtx, meshMtx;
};
