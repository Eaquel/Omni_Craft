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
#include <algorithm>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <string>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>

#define LOG_TAG "OmniCraft_Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static constexpr int CS        = 16;
static constexpr int WH        = 128;
static constexpr int RD        = 5;
static constexpr int SEA_LEVEL = 32;
static constexpr float GRAVITY = -26.0f;
static constexpr float JUMP_VEL= 8.6f;
static constexpr float WALK_SPD= 4.317f;
static constexpr float SPRINT_SPD= 6.2f;
static constexpr float REACH   = 5.5f;
static constexpr int INV_SIZE  = 36;
static constexpr int HOTBAR_SZ = 9;
static constexpr int MAX_STACK = 64;

enum BlockID : uint8_t {
    AIR=0, GRASS, DIRT, STONE, COBBLESTONE, SAND, GRAVEL,
    OAK_LOG, OAK_LEAVES, OAK_PLANKS, BIRCH_LOG, BIRCH_PLANKS, BIRCH_LEAVES,
    SPRUCE_LOG, SPRUCE_PLANKS, SPRUCE_LEAVES, COAL_ORE, IRON_ORE, GOLD_ORE,
    DIAMOND_ORE, EMERALD_ORE, REDSTONE_ORE, LAPIS_ORE, COAL_BLOCK, IRON_BLOCK,
    GOLD_BLOCK, DIAMOND_BLOCK, WATER, LAVA, GLASS, GLOWSTONE, NETHERRACK,
    CRAFTING_TABLE, FURNACE, CHEST, LADDER, TORCH, BEDROCK, SNOW_LAYER,
    ICE, CLAY, BRICK, STONE_BRICKS, MOSSY_COBBLE, TNT, BOOKSHELF, PUMPKIN,
    MELON, SANDSTONE, CACTUS, SPONGE, BLOCK_COUNT
};

enum ItemID : uint16_t {
    ITEM_NONE=0, ITEM_STICK=256, ITEM_RAW_PORK, ITEM_COOKED_PORK,
    ITEM_RAW_BEEF, ITEM_COOKED_BEEF, ITEM_RAW_MUTTON, ITEM_WOOL,
    ITEM_COAL, ITEM_IRON_INGOT, ITEM_GOLD_INGOT, ITEM_DIAMOND,
    ITEM_SWORD_IRON, ITEM_PICKAXE_IRON
};

enum EntityType : uint8_t {
    ENT_PIG=0, ENT_COW, ENT_SHEEP, ENT_ITEM_DROP
};

struct BlockDef {
    const char* name;
    bool solid;
    bool transparent;
    bool fluid;
    float hardness;
    uint8_t topTile, sideTile, botTile;
    uint16_t dropId;
};

struct Vec3 {
    float x, y, z;
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    Vec3& operator+=(const Vec3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
    float dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o) const { return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x}; }
    float len() const { return sqrtf(x*x + y*y + z*z); }
    Vec3 norm() const { float l = len() + 1e-7f; return {x/l, y/l, z/l}; }
};

struct Vec3i { int x, y, z; bool operator==(const Vec3i& o) const { return x==o.x && y==o.y && z==o.z; } };

struct Mat4 {
    float m[16] = {0};
    Mat4 operator*(const Mat4& b) const {
        Mat4 res;
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                res.m[c * 4 + r] = m[0 * 4 + r] * b.m[c * 4 + 0] +
                                   m[1 * 4 + r] * b.m[c * 4 + 1] +
                                   m[2 * 4 + r] * b.m[c * 4 + 2] +
                                   m[3 * 4 + r] * b.m[c * 4 + 3];
            }
        }
        return res;
    }
};

struct Vertex {
    float x, y, z;
    float u, v;
    float light;
};

struct Particle {
    Vec3 pos;
    Vec3 vel;
    Vec3 color;
    float life;
    float maxLife;
};

struct ItemStack {
    uint16_t id = 0;
    uint8_t count = 0;
    ItemStack() = default;
    ItemStack(uint16_t i, uint8_t c) : id(i), count(c) {}
    bool empty() const { return id == 0 || count == 0; }
};

struct Inventory {
    ItemStack slots[INV_SIZE];
    int selected = 0;
    bool add(uint16_t id, int cnt);
    ItemStack& active() { return slots[selected % HOTBAR_SZ]; }
};

struct Entity {
    uint32_t id = 0;
    EntityType type;
    Vec3 pos{}, vel{};
    float yaw = 0.0f;
    float health = 10.0f;
    float animTime = 0.0f;
    uint16_t itemId = 0;
    bool alive = true;
};

class World;

struct Chunk {
    int cx, cz;
    std::array<uint8_t, CS * WH * CS> blocks;
    std::atomic<bool> dirty{true};
    std::atomic<bool> generated{false};

    GLuint vao = 0, vbo = 0;
    GLsizei vertexCount = 0;

    Chunk(int x, int z);
    ~Chunk();
    inline int idx(int x, int y, int z) const { return x * WH * CS + y * CS + z; }
    uint8_t get(int x, int y, int z) const;
    void set(int x, int y, int z, uint8_t b);
    void buildMesh(const World& world);
    void render();
};

class World {
public:
    std::unordered_map<uint64_t, std::shared_ptr<Chunk>> chunks;
    mutable std::shared_mutex chunkMtx;
    std::vector<Entity> entities;
    std::mutex entityMtx;
    std::vector<Particle> particles;
    std::mutex particleMtx;
    uint32_t nextEntityId = 1;
    std::string worldPath = "";

    static uint64_t key(int cx, int cz) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32) |
               static_cast<uint64_t>(static_cast<uint32_t>(cz));
    }

    std::shared_ptr<Chunk> getChunk(int cx, int cz) const;
    std::shared_ptr<Chunk> getOrCreate(int cx, int cz);
    void generateChunk(Chunk& c);
    uint8_t blockAt(int wx, int wy, int wz) const;
    void setBlock(int wx, int wy, int wz, uint8_t b);
    int getHighestBlock(int wx, int wz) const;
    
    void saveWorld(const std::string& path);
    void loadWorld(const std::string& path);
    void spawnMob(EntityType type, Vec3 pos);
    void spawnItemDrop(uint16_t itemId, Vec3 pos, Vec3 vel);
    void spawnBreakParticles(Vec3 pos, uint8_t blockId);
    void updateEntities(float dt);
    void updateParticles(float dt);

    struct RayHit { bool hit; Vec3i block, face; float dist; };
    RayHit raycast(Vec3 origin, Vec3 dir, float maxD) const;
};

struct Player {
    Vec3 pos{0.5f, 60.0f, 0.5f}, vel{};
    float yaw = 0, pitch = 0;
    float eyeH = 1.62f;
    bool onGround = false;
    bool inWater = false;
    Inventory inv;

    bool isBreaking = false;
    Vec3i breakingBlock{-999, -999, -999};
    float breakProgress = 0.0f;

    Vec3 lookDir() const {
        float y2 = yaw * (3.14159265f / 180.0f), p = pitch * (3.14159265f / 180.0f);
        return {sinf(y2) * cosf(p), sinf(p), -cosf(y2) * cosf(p)};
    }
};

class Renderer {
public:
    GLuint worldProg = 0;
    GLuint entityProg = 0;
    GLuint skyProg = 0;
    GLuint atlasTex = 0;
    GLuint boxVAO = 0, boxVBO = 0;
    int screenW = 1920, screenH = 1080;

    void init(int w, int h);
    void resize(int w, int h);
    void frame(World& world, Player& player, float time);
    void generateProceduralAtlas();
    void drawBox(const Mat4& vp, Vec3 pos, Vec3 scale, Vec3 color, float yaw = 0, float pitch = 0);
    void drawSunAndClouds(const Mat4& vp, Vec3 playerPos, float time);
    void drawBreakingOverlay(const Mat4& vp, Vec3i pos, float progress);
    void drawMob(const Mat4& vp, const Entity& e);
};

struct GameState {
    std::unique_ptr<World> world;
    Player player;
    Renderer renderer;
    std::atomic<bool> initialized{false};
    int screenW = 1920, screenH = 1080;
    float inputX = 0, inputZ = 0;
    float time = 0;
    std::string saveDirectory = "";
};
