#pragma once

/*
 * Game Hack Core - Advanced Version
 * Features: ESP, Aimbot, Silent Aim, Bullet Tracer, Kick Player
 */

#include <Windows.h>
#include <Psapi.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <map>

#pragma comment(lib, "Psapi.lib")

namespace GameHack
{
    //=============================================================================
    // SDK Offsets
    //=============================================================================
    namespace Offsets
    {
        // Global
        constexpr uintptr_t GObjects      = 0x0A5092B0;
        constexpr uintptr_t GWorld        = 0x0A5140F0;
        constexpr uintptr_t ProcessEvent  = 0x00AE0BE0;

        // UWorld
        constexpr uintptr_t World_GameState          = 0x0158;
        constexpr uintptr_t World_OwningGameInstance = 0x01B8;
        constexpr uintptr_t World_Levels             = 0x0170;

        // UGameInstance
        constexpr uintptr_t GameInstance_LocalPlayers = 0x0038;

        // UPlayer
        constexpr uintptr_t Player_PlayerController = 0x0030;

        // AGameStateBase
        constexpr uintptr_t GameState_PlayerArray = 0x02C8;

        // AController
        constexpr uintptr_t Controller_PlayerState     = 0x02B8;
        constexpr uintptr_t Controller_Pawn            = 0x02F0;
        constexpr uintptr_t Controller_Character       = 0x0300;
        constexpr uintptr_t Controller_ControlRotation = 0x0328;

        // APlayerController
        constexpr uintptr_t PlayerController_PlayerCameraManager = 0x0368;
        constexpr uintptr_t PlayerController_MyHUD               = 0x0360;
        constexpr uintptr_t PlayerController_CheatManager        = 0x0418;

        // APlayerCameraManager
        constexpr uintptr_t CameraManager_DefaultFOV         = 0x02C8;
        constexpr uintptr_t CameraManager_CameraCachePrivate = 0x2480;

        // AActor
        constexpr uintptr_t Actor_RootComponent = 0x0198;

        // USceneComponent
        constexpr uintptr_t SceneComponent_RelativeLocation = 0x0128;
        constexpr uintptr_t SceneComponent_ComponentToWorld = 0x01C0;

        // ACharacter
        constexpr uintptr_t Character_Mesh = 0x0328;

        // AP2BaseCharacter
        constexpr uintptr_t P2BaseCharacter_AttributeSetHealth = 0x07C8;

        // UP2HealthSet
        constexpr uintptr_t HealthSet_Health    = 0x0060;
        constexpr uintptr_t HealthSet_MaxHealth = 0x0080;
        constexpr uintptr_t AttributeData_BaseValue = 0x0008;

        // APlayerState
        constexpr uintptr_t PlayerState_TeamId = 0x04A4;
        constexpr uintptr_t PlayerState_Pawn   = 0x02F0;
        constexpr uintptr_t PlayerState_PlayerName = 0x0310;

        // USkeletalMeshComponent
        constexpr uintptr_t SkeletalMesh_ComponentSpaceTransforms = 0x0590;

        // UProjectileMovementComponent (Engine_classes.hpp:1512)
        constexpr uintptr_t Projectile_InitialSpeed      = 0x0128;
        constexpr uintptr_t Projectile_MaxSpeed          = 0x012C;
        constexpr uintptr_t Projectile_GravityScale      = 0x0150;
        constexpr uintptr_t Projectile_HomingAccel       = 0x0188;
        constexpr uintptr_t Projectile_HomingTarget      = 0x018C;
        constexpr uintptr_t Projectile_bIsHomingProjectile = 0x0130; // Bit 7

        // ULevel
        constexpr uintptr_t Level_Actors = 0x00A0;

        // AP2Projectile
        constexpr uintptr_t Projectile_ProjectileMovement = 0x02B0;

        // AActor
        constexpr uintptr_t Actor_Owner = 0x0160;

        // AP2BruteProjectile (AICommon_classes.hpp:484)
        constexpr uintptr_t BruteProjectile_Velocity       = 0x0390;
        constexpr uintptr_t BruteProjectile_TargetLocation = 0x03A8;

        // AdminCheatExtension (CommonCheatsRuntime)
        constexpr uintptr_t TestKickPlayer_Func = 0x431C7C8;
    }

    //=============================================================================
    // Basic Types
    //=============================================================================
    class UObject;

    //=============================================================================
    // Math Structures
    //=============================================================================
    struct FVector
    {
        float X, Y, Z;

        FVector() : X(0), Y(0), Z(0) {}
        FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

        FVector operator-(const FVector& o) const { return FVector(X - o.X, Y - o.Y, Z - o.Z); }
        FVector operator+(const FVector& o) const { return FVector(X + o.X, Y + o.Y, Z + o.Z); }
        FVector operator*(float s) const { return FVector(X * s, Y * s, Z * s); }

        float Length() const { return sqrtf(X * X + Y * Y + Z * Z); }
        float Length2D() const { return sqrtf(X * X + Y * Y); }
        float Distance(const FVector& o) const { return (*this - o).Length(); }
        float Dot(const FVector& o) const { return X * o.X + Y * o.Y + Z * o.Z; }

        FVector Normalize() const {
            float len = Length();
            return len > 0 ? FVector(X / len, Y / len, Z / len) : FVector();
        }
    };

    struct FRotator
    {
        float Pitch, Yaw, Roll;

        FRotator() : Pitch(0), Yaw(0), Roll(0) {}
        FRotator(float p, float y, float r) : Pitch(p), Yaw(y), Roll(r) {}

        FRotator Clamp() const {
            FRotator r = *this;
            if (r.Pitch > 89.0f) r.Pitch = 89.0f;
            if (r.Pitch < -89.0f) r.Pitch = -89.0f;
            while (r.Yaw > 180.0f) r.Yaw -= 360.0f;
            while (r.Yaw < -180.0f) r.Yaw += 360.0f;
            r.Roll = 0;
            return r;
        }

        FRotator operator-(const FRotator& o) const {
            return FRotator(Pitch - o.Pitch, Yaw - o.Yaw, Roll - o.Roll);
        }
    };

    struct FMatrix { 
        float M[4][4]; 
        FMatrix() { 
            std::memset(M, 0, sizeof(M)); 
        } 
    };

    struct FTransform {
        float Rotation[4];
        FVector Translation;
        float Pad1;
        FVector Scale3D;
        float Pad2;
    };

    // UE FString structure
    struct FString {
        wchar_t* Data;
        int32_t Count;
        int32_t Max;
    };

    //=============================================================================
    // Memory Helper
    //=============================================================================
    class HackMemory
    {
    public:
        static inline uintptr_t BaseAddress = 0;
        static inline HANDLE hProcess = NULL;
        static inline DWORD ProcessId = 0;

        static void Initialize() {}

        static bool Attach(const wchar_t* windowName) {
            HWND hWindow = FindWindowW(NULL, windowName);
            if (!hWindow) return false;

            GetWindowThreadProcessId(hWindow, &ProcessId);
            if (!ProcessId) return false;

            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
            if (!hProcess) return false;

            // Get Base Address (Simple method for main module)
            HMODULE hMods[1024];
            DWORD cbNeeded;
            if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
                BaseAddress = (uintptr_t)hMods[0];
            }
            
            return (BaseAddress != 0);
        }

        static void Close() {
            if (hProcess) CloseHandle(hProcess);
        }

        template<typename T>
        static T Read(uintptr_t addr) {
            if (!addr || !hProcess) return T();
            T val;
            if (ReadProcessMemory(hProcess, (LPCVOID)addr, &val, sizeof(T), NULL)) {
                return val;
            }
            return T();
        }

        template<typename T>
        static void Write(uintptr_t addr, const T& val) {
            if (!addr || !hProcess) return;
            WriteProcessMemory(hProcess, (LPVOID)addr, &val, sizeof(T), NULL);
        }
        
        // Read raw bytes
        static bool ReadRaw(uintptr_t addr, void* buffer, size_t size) {
            if (!addr || !hProcess) return false;
            return ReadProcessMemory(hProcess, (LPCVOID)addr, buffer, size, NULL);
        }

        static bool IsValid(uintptr_t addr) {
            return (addr > 0x10000 && addr < 0x7FFFFFFFFFFF);
        }

        static uintptr_t GetAddr(uintptr_t off) { return BaseAddress + off; }
    };

    //=============================================================================
    // GUObjectArray Helper
    //=============================================================================
    class GUObjectArray
    {
    public:
        static int32_t GetSerialNumber(uintptr_t objectAddr) {
            if (!HackMemory::IsValid(objectAddr)) return 0;

            int32_t index = HackMemory::Read<int32_t>(objectAddr + 0xC); // InternalIndex

            uintptr_t gObjects = HackMemory::Read<uintptr_t>(HackMemory::GetAddr(Offsets::GObjects));
            uintptr_t objectsPtr = HackMemory::Read<uintptr_t>(gObjects); // Objects

            int32_t chunkIndex = index / 65536;
            int32_t withinChunkIndex = index % 65536;

            uintptr_t chunk = HackMemory::Read<uintptr_t>(objectsPtr + chunkIndex * 8);
            if (!chunk) return 0;

            // FUObjectItem size 24
            uintptr_t item = chunk + withinChunkIndex * 24;
            return HackMemory::Read<int32_t>(item + 16); // SerialNumber
        }

        static int32_t GetIndex(uintptr_t objectAddr) {
            if (!HackMemory::IsValid(objectAddr)) return 0;
            return HackMemory::Read<int32_t>(objectAddr + 0xC);
        }
    };


    //=============================================================================
    // Math Functions
    //=============================================================================
    constexpr float PI = 3.14159265358979323846f;
    constexpr float GRAVITY = 981.0f; // UE uses cm/s^2

    inline float ToRad(float d) { return d * (PI / 180.0f); }
    inline float ToDeg(float r) { return r * (180.0f / PI); }

    inline FRotator CalcAngle(const FVector& from, const FVector& to) {
        FVector d = to - from;
        float dist = d.Length();
        if (dist == 0) return FRotator();
        return FRotator(-asinf(d.Z / dist) * (180.0f / PI), atan2f(d.Y, d.X) * (180.0f / PI), 0).Clamp();
    }

    inline float GetFOVAngle(const FRotator& v, const FRotator& t) {
        float pd = v.Pitch - t.Pitch;
        float yd = v.Yaw - t.Yaw;
        while (yd > 180.0f) yd -= 360.0f;
        while (yd < -180.0f) yd += 360.0f;
        return sqrtf(pd * pd + yd * yd);
    }

    inline FMatrix RotatorToMatrix(const FRotator& rot) {
        FMatrix m;
        float rP = ToRad(rot.Pitch), rY = ToRad(rot.Yaw), rR = ToRad(rot.Roll);
        float SP = sinf(rP), CP = cosf(rP), SY = sinf(rY), CY = cosf(rY), SR = sinf(rR), CR = cosf(rR);
        m.M[0][0] = CP * CY; m.M[0][1] = CP * SY; m.M[0][2] = SP;
        m.M[1][0] = SR * SP * CY - CR * SY; m.M[1][1] = SR * SP * SY + CR * CY; m.M[1][2] = -SR * CP;
        m.M[2][0] = -(CR * SP * CY + SR * SY); m.M[2][1] = CY * SR - CR * SP * SY; m.M[2][2] = CR * CP;
        m.M[3][3] = 1.0f;
        return m;
    }

    inline bool WorldToScreen(const FVector& world, const FVector& cam, const FRotator& rot,
                               float fov, int sw, int sh, FVector& screen) {
        FVector d = world - cam;
        FMatrix m = RotatorToMatrix(rot);
        FVector t;
        t.X = d.X * m.M[0][0] + d.Y * m.M[0][1] + d.Z * m.M[0][2];
        t.Y = d.X * m.M[1][0] + d.Y * m.M[1][1] + d.Z * m.M[1][2];
        t.Z = d.X * m.M[2][0] + d.Y * m.M[2][1] + d.Z * m.M[2][2];
        if (t.X < 1.0f) return false;
        float tanFov = tanf(ToRad(fov) / 2.0f);
        float cx = sw / 2.0f, cy = sh / 2.0f;
        screen.X = cx + (t.Y / t.X) * (cx / tanFov);
        screen.Y = cy - (t.Z / t.X) * (cx / tanFov);
        screen.Z = t.X;
        return true;
    }

    //=============================================================================
    // Prediction for Bullet Drop/Travel Time
    //=============================================================================
    inline FVector PredictTargetPosition(const FVector& targetPos, const FVector& targetVel,
                                          const FVector& shooterPos, float bulletSpeed, float gravity) {
        FVector delta = targetPos - shooterPos;
        float dist = delta.Length();
        float travelTime = dist / bulletSpeed;

        // Predict where target will be
        FVector predicted = targetPos + targetVel * travelTime;

        // Compensate for bullet drop (gravity affects Z)
        float dropCompensation = 0.5f * gravity * travelTime * travelTime;
        predicted.Z += dropCompensation;

        return predicted;
    }

    inline FRotator CalcAngleWithPrediction(const FVector& from, const FVector& targetPos,
                                             const FVector& targetVel, float bulletSpeed, float gravity) {
        FVector predicted = PredictTargetPosition(targetPos, targetVel, from, bulletSpeed, gravity);
        return CalcAngle(from, predicted);
    }

    //=============================================================================
    // Game Class Wrappers
    //=============================================================================
    class UWorld {
        uintptr_t addr;
    public:
        UWorld(uintptr_t a) : addr(a) {}
        static UWorld Get() { return UWorld(HackMemory::Read<uintptr_t>(HackMemory::GetAddr(Offsets::GWorld))); }
        bool IsValid() const { return HackMemory::IsValid(addr); }
        uintptr_t GetGameState() const { return IsValid() ? HackMemory::Read<uintptr_t>(addr + Offsets::World_GameState) : 0; }
        uintptr_t GetGameInstance() const { return IsValid() ? HackMemory::Read<uintptr_t>(addr + Offsets::World_OwningGameInstance) : 0; }
    };

    class UGameInstance {
        uintptr_t addr;
    public:
        UGameInstance(uintptr_t a) : addr(a) {}
        bool IsValid() const { return HackMemory::IsValid(addr); }
        uintptr_t GetLocalPlayer(int i = 0) const {
            if (!IsValid()) return 0;
            uintptr_t arr = HackMemory::Read<uintptr_t>(addr + Offsets::GameInstance_LocalPlayers);
            int32_t cnt = HackMemory::Read<int32_t>(addr + Offsets::GameInstance_LocalPlayers + 0x08);
            if (i >= cnt || !arr) return 0;
            return HackMemory::Read<uintptr_t>(arr + i * 0x08);
        }
    };

    class ULocalPlayer {
        uintptr_t addr;
    public:
        ULocalPlayer(uintptr_t a) : addr(a) {}
        bool IsValid() const { return HackMemory::IsValid(addr); }
        uintptr_t GetController() const { return IsValid() ? HackMemory::Read<uintptr_t>(addr + Offsets::Player_PlayerController) : 0; }
    };

    class AGameState {
        uintptr_t addr;
    public:
        AGameState(uintptr_t a) : addr(a) {}
        bool IsValid() const { return HackMemory::IsValid(addr); }
        int GetPlayerCount() const { return IsValid() ? HackMemory::Read<int32_t>(addr + Offsets::GameState_PlayerArray + 0x08) : 0; }
        uintptr_t GetPlayerState(int i) const {
            if (!IsValid()) return 0;
            uintptr_t arr = HackMemory::Read<uintptr_t>(addr + Offsets::GameState_PlayerArray);
            if (i >= GetPlayerCount() || !arr) return 0;
            return HackMemory::Read<uintptr_t>(arr + i * 0x08);
        }
    };

    class APlayerState {
        uintptr_t addr;
    public:
        APlayerState(uintptr_t a) : addr(a) {}
        bool IsValid() const { return HackMemory::IsValid(addr); }
        uintptr_t GetAddr() const { return addr; }
        int32_t GetTeamId() const { return IsValid() ? HackMemory::Read<int32_t>(addr + Offsets::PlayerState_TeamId) : -1; }
        uintptr_t GetPawn() const { return IsValid() ? HackMemory::Read<uintptr_t>(addr + Offsets::PlayerState_Pawn) : 0; }
    };

    class UP2HealthSet {
        uintptr_t addr;
    public:
        UP2HealthSet(uintptr_t a) : addr(a) {}
        bool IsValid() const { return HackMemory::IsValid(addr); }
        float GetHealth() const { return IsValid() ? HackMemory::Read<float>(addr + Offsets::HealthSet_Health + Offsets::AttributeData_BaseValue) : 0; }
        float GetMaxHealth() const { return IsValid() ? HackMemory::Read<float>(addr + Offsets::HealthSet_MaxHealth + Offsets::AttributeData_BaseValue) : 0; }
    };

    class AActor {
    protected:
        uintptr_t addr;
    public:
        AActor(uintptr_t a) : addr(a) {}
        bool IsValid() const { return HackMemory::IsValid(addr); }
        uintptr_t GetAddr() const { return addr; }
        FVector GetLocation() const {
            if (!IsValid()) return FVector();
            uintptr_t root = HackMemory::Read<uintptr_t>(addr + Offsets::Actor_RootComponent);
            return HackMemory::IsValid(root) ? HackMemory::Read<FVector>(root + Offsets::SceneComponent_RelativeLocation) : FVector();
        }
        FVector GetVelocity() const {
            // AActor+0x0170 typically contains velocity
            if (!IsValid()) return FVector();
            return HackMemory::Read<FVector>(addr + 0x0170);
        }
        uintptr_t GetOwner() const {
            if (!IsValid()) return 0;
            return HackMemory::Read<uintptr_t>(addr + Offsets::Actor_Owner);
        }
    };

    class ACharacter : public AActor {
    public:
        ACharacter(uintptr_t a) : AActor(a) {}
        uintptr_t GetMesh() const { return IsValid() ? HackMemory::Read<uintptr_t>(addr + Offsets::Character_Mesh) : 0; }
    };

    struct FWeakObjectPtr
    {
        int32_t ObjectIndex;
        int32_t ObjectSerialNumber;
    };

    class UProjectileMovementComponent
    {
    private:
        uintptr_t address;

    public:
        UProjectileMovementComponent(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return HackMemory::IsValid(address); }

        void EnableHoming(bool enable) {
            if (!IsValid()) return;
            uint8_t value = HackMemory::Read<uint8_t>(address + Offsets::Projectile_bIsHomingProjectile);
            if (enable) value |= 0x80; // Bit 7
            else value &= ~0x80;
            HackMemory::Write<uint8_t>(address + Offsets::Projectile_bIsHomingProjectile, value);
        }

        void SetHomingMagnitude(float magnitude) {
            if (!IsValid()) return;
            HackMemory::Write<float>(address + Offsets::Projectile_HomingAccel, magnitude);
        }

        void SetHomingTarget(const FWeakObjectPtr& target) {
            if (!IsValid()) return;
            HackMemory::Write<FWeakObjectPtr>(address + Offsets::Projectile_HomingTarget, target);
        }
    };

    class AP2Projectile : public AActor
    {
    public:
        AP2Projectile(uintptr_t addr) : AActor(addr) {}

        UProjectileMovementComponent GetMovementComponent() const {
            if (!IsValid()) return UProjectileMovementComponent(0);
            return UProjectileMovementComponent(HackMemory::Read<uintptr_t>(addr + Offsets::Projectile_ProjectileMovement));
        }
    };

    class ULevel
    {
    private:
        uintptr_t address;

    public:
        ULevel(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return HackMemory::IsValid(address); }

        std::vector<AActor> GetActors() const {
            std::vector<AActor> actors;
            if (!IsValid()) return actors;

            uintptr_t actorsArray = address + Offsets::Level_Actors;
            uintptr_t data = HackMemory::Read<uintptr_t>(actorsArray);
            int32_t count = HackMemory::Read<int32_t>(actorsArray + 0x08);

            if (!HackMemory::IsValid(data) || count <= 0 || count > 10000) return actors;

            for (int i = 0; i < count; i++) {
                uintptr_t actorAddr = HackMemory::Read<uintptr_t>(data + i * 0x08);
                if (HackMemory::IsValid(actorAddr)) {
                    actors.push_back(AActor(actorAddr));
                }
            }
            return actors;
        }
    };


    class AP2BaseCharacter : public ACharacter {
    public:
        AP2BaseCharacter(uintptr_t a) : ACharacter(a) {}
        UP2HealthSet GetHealthSet() const { return UP2HealthSet(IsValid() ? HackMemory::Read<uintptr_t>(addr + Offsets::P2BaseCharacter_AttributeSetHealth) : 0); }
        float GetHealth() const { return GetHealthSet().GetHealth(); }
        float GetMaxHealth() const { return GetHealthSet().GetMaxHealth(); }
        bool IsAlive() const { return GetHealth() > 0; }
    };

    class APlayerCameraManager {
        uintptr_t addr;
    public:
        APlayerCameraManager(uintptr_t a) : addr(a) {}
        bool IsValid() const { return HackMemory::IsValid(addr); }
        float GetFOV() const { return IsValid() ? HackMemory::Read<float>(addr + Offsets::CameraManager_DefaultFOV) : 90.0f; }
        FVector GetLocation() const { return IsValid() ? HackMemory::Read<FVector>(addr + Offsets::CameraManager_CameraCachePrivate) : FVector(); }
        FRotator GetRotation() const { return IsValid() ? HackMemory::Read<FRotator>(addr + Offsets::CameraManager_CameraCachePrivate + 0x18) : FRotator(); }
    };

    class APlayerController : public AActor {
    public:
        APlayerController(uintptr_t a) : AActor(a) {}
        FRotator GetControlRotation() const { return IsValid() ? HackMemory::Read<FRotator>(addr + Offsets::Controller_ControlRotation) : FRotator(); }
        void SetControlRotation(const FRotator& r) { if (IsValid()) HackMemory::Write<FRotator>(addr + Offsets::Controller_ControlRotation, r); }
        APlayerCameraManager GetCameraManager() const { return APlayerCameraManager(IsValid() ? HackMemory::Read<uintptr_t>(addr + Offsets::PlayerController_PlayerCameraManager) : 0); }
        AP2BaseCharacter GetCharacter() const { return AP2BaseCharacter(IsValid() ? HackMemory::Read<uintptr_t>(addr + Offsets::Controller_Character) : 0); }
        APlayerState GetPlayerState() const { return APlayerState(IsValid() ? HackMemory::Read<uintptr_t>(addr + Offsets::Controller_PlayerState) : 0); }
    };

    class USkeletalMesh {
        uintptr_t addr;
    public:
        USkeletalMesh(uintptr_t a) : addr(a) {}
        bool IsValid() const { return HackMemory::IsValid(addr); }
        int GetBoneCount() const { return IsValid() ? HackMemory::Read<int32_t>(addr + Offsets::SkeletalMesh_ComponentSpaceTransforms + 0x08) : 0; }
        FVector GetBonePos(int idx) const {
            if (!IsValid() || idx < 0 || idx >= GetBoneCount()) return FVector();
            uintptr_t arr = HackMemory::Read<uintptr_t>(addr + Offsets::SkeletalMesh_ComponentSpaceTransforms);
            if (!HackMemory::IsValid(arr)) return FVector();
            FTransform t = HackMemory::Read<FTransform>(arr + idx * sizeof(FTransform));
            FMatrix m = HackMemory::Read<FMatrix>(addr + Offsets::SceneComponent_ComponentToWorld);
            FVector l = t.Translation;
            return FVector(
                m.M[0][0] * l.X + m.M[1][0] * l.Y + m.M[2][0] * l.Z + m.M[3][0],
                m.M[0][1] * l.X + m.M[1][1] * l.Y + m.M[2][1] * l.Z + m.M[3][1],
                m.M[0][2] * l.X + m.M[1][2] * l.Y + m.M[2][2] * l.Z + m.M[3][2]
            );
        }
    };

    // Common bone indices (verify for your specific game)
    namespace BoneID {
        constexpr int Head = 98;
        constexpr int Neck = 97;
        constexpr int Chest = 66;
        constexpr int Spine = 65;
        constexpr int Pelvis = 0;
        constexpr int ShoulderL = 78, ElbowL = 79, HandL = 80;
        constexpr int ShoulderR = 49, ElbowR = 50, HandR = 51;
        constexpr int HipL = 9, KneeL = 10, FootL = 11;
        constexpr int HipR = 4, KneeR = 5, FootR = 6;
    }

    //=============================================================================
    // Bullet Tracer Data
    //=============================================================================
    struct BulletTrace {
        FVector start;
        FVector end;
        float spawnTime;
        float lifetime;
        bool isHit;
    };

    //=============================================================================
    // ESP Player Data
    //=============================================================================
    struct ESPPlayer {
        uintptr_t addr;
        uintptr_t playerStateAddr;
        FVector pos, headPos, screenPos, headScreen;
        FVector velocity;
        float health, maxHealth, distance;
        int32_t teamId;
        int playerIndex;
        bool visible, enemy, alive;
        std::wstring name;
    };

    //=============================================================================
    // Settings
    //=============================================================================
    struct Settings {
        // ESP
        bool espEnabled = true;
        bool espBox = true;
        bool espHealth = true;
        bool espDistance = true;
        bool espName = true;
        bool espHeadCircle = true;
        bool espSnapline = false;
        bool espSkeleton = false;
        float espMaxDist = 500.0f;

        // Aimbot
        bool aimbotEnabled = false;
        bool aimbotVisOnly = true;
        float aimbotFOV = 15.0f;
        float aimbotSmooth = 5.0f;
        int aimbotBone = BoneID::Head;
        int aimbotKey = VK_RBUTTON;

        // Silent Aim (server-side angle manipulation)
        bool silentAimEnabled = false;
        float silentAimFOV = 10.0f;

        // Magic Bullet
        bool magicBulletEnabled = false;

        // Bullet Prediction
        bool bulletPrediction = false;
        float bulletSpeed = 50000.0f;  // cm/s
        float bulletGravity = 0.0f;     // 0 = hitscan

        // Bullet Tracer
        bool bulletTracerEnabled = false;
        float bulletTracerLifetime = 1.0f;

        // Kick Player (requires admin/host)
        int kickPlayerIndex = -1;
    };

    //=============================================================================
    // Game Manager
    //=============================================================================
    class GameManager {
        APlayerController localCtrl;
        std::vector<ESPPlayer> players;
        std::vector<BulletTrace> bulletTraces;
        Settings settings;
        int sw = 1920, sh = 1080;
        bool init = false;

        // Silent aim cached target
        FRotator silentAimAngle;
        bool hasSilentTarget = false;

    public:
        GameManager() : localCtrl(0) {}
        Settings& GetSettings() { return settings; }
        const std::vector<ESPPlayer>& GetPlayers() const { return players; }
        const std::vector<BulletTrace>& GetBulletTraces() const { return bulletTraces; }
        void SetScreenSize(int w, int h) { sw = w; sh = h; }

        bool Initialize() { HackMemory::Initialize(); init = HackMemory::BaseAddress != 0; return init; }

        APlayerController GetLocalController() {
            UWorld w = UWorld::Get();
            if (!w.IsValid()) return APlayerController(0);
            UGameInstance gi(w.GetGameInstance());
            if (!gi.IsValid()) return APlayerController(0);
            ULocalPlayer lp(gi.GetLocalPlayer(0));
            return APlayerController(lp.IsValid() ? lp.GetController() : 0);
        }

        // Get silent aim angle for hooking fire functions
        bool GetSilentAimAngle(FRotator& outAngle) const {
            if (hasSilentTarget) {
                outAngle = silentAimAngle;
                return true;
            }
            return false;
        }

        void AddBulletTrace(const FVector& start, const FVector& end, bool hit) {
            if (!settings.bulletTracerEnabled) return;

            BulletTrace trace;
            trace.start = start;
            trace.end = end;
            trace.spawnTime = (float)GetTickCount64() / 1000.0f;
            trace.lifetime = settings.bulletTracerLifetime;
            trace.isHit = hit;
            bulletTraces.push_back(trace);
        }

        void UpdateBulletTraces() {
            float currentTime = (float)GetTickCount64() / 1000.0f;
            bulletTraces.erase(
                std::remove_if(bulletTraces.begin(), bulletTraces.end(),
                    [currentTime](const BulletTrace& t) {
                        return (currentTime - t.spawnTime) > t.lifetime;
                    }),
                bulletTraces.end()
            );
        }

        void Update() {
            if (!init) return;
            players.clear();
            hasSilentTarget = false;

            UpdateBulletTraces();

            localCtrl = GetLocalController();
            if (!localCtrl.IsValid()) return;

            AP2BaseCharacter localChar = localCtrl.GetCharacter();
            if (!localChar.IsValid()) return;

            APlayerCameraManager cam = localCtrl.GetCameraManager();
            if (!cam.IsValid()) return;

            FVector camPos = cam.GetLocation();
            FRotator camRot = cam.GetRotation();
            float fov = cam.GetFOV();
            int32_t localTeam = localCtrl.GetPlayerState().GetTeamId();
            FVector localPos = localChar.GetLocation();

            UWorld w = UWorld::Get();
            AGameState gs(w.GetGameState());
            if (!gs.IsValid()) return;

            int cnt = gs.GetPlayerCount();
            for (int i = 0; i < cnt; i++) {
                APlayerState ps(gs.GetPlayerState(i));
                if (!ps.IsValid()) continue;

                uintptr_t pawn = ps.GetPawn();
                if (!HackMemory::IsValid(pawn) || pawn == localChar.GetAddr()) continue;

                AP2BaseCharacter ch(pawn);
                if (!ch.IsValid() || !ch.IsAlive()) continue;

                ESPPlayer p = {};
                p.addr = pawn;
                p.playerStateAddr = ps.GetAddr();
                p.playerIndex = i;
                p.pos = ch.GetLocation();
                p.velocity = ch.GetVelocity();
                p.health = ch.GetHealth();
                p.maxHealth = ch.GetMaxHealth();
                p.teamId = ps.GetTeamId();
                p.enemy = p.teamId != localTeam;
                p.alive = p.health > 0;
                p.distance = p.pos.Distance(localPos) / 100.0f;

                if (p.distance > settings.espMaxDist) continue;

                USkeletalMesh mesh(ch.GetMesh());
                p.headPos = mesh.IsValid() ? mesh.GetBonePos(BoneID::Head) : p.pos + FVector(0, 0, 80);

                p.visible = WorldToScreen(p.pos, camPos, camRot, fov, sw, sh, p.screenPos);
                if (p.visible) WorldToScreen(p.headPos, camPos, camRot, fov, sw, sh, p.headScreen);

                players.push_back(p);
            }

            // Update silent aim target
            if (settings.silentAimEnabled) {
                UpdateSilentAim(camPos, camRot);
            }
        }

        void UpdateSilentAim(const FVector& camPos, const FRotator& camRot) {
            hasSilentTarget = false;
            float bestFov = settings.silentAimFOV;
            ESPPlayer* bestTarget = nullptr;

            for (auto& p : players) {
                if (!p.enemy || !p.alive) continue;
                if (settings.aimbotVisOnly && !p.visible) continue;

                FVector targetPos = p.headPos;

                // Apply prediction if enabled
                if (settings.bulletPrediction && settings.bulletSpeed > 0) {
                    targetPos = PredictTargetPosition(p.headPos, p.velocity, camPos,
                                                       settings.bulletSpeed, settings.bulletGravity);
                }

                FRotator targetRot = CalcAngle(camPos, targetPos);
                float fovAngle = GetFOVAngle(camRot, targetRot);

                if (fovAngle < bestFov) {
                    bestFov = fovAngle;
                    bestTarget = &p;
                    silentAimAngle = targetRot;
                }
            }

            hasSilentTarget = (bestTarget != nullptr);
        }

        void RunAimbot() {
            if (!settings.aimbotEnabled || !(GetAsyncKeyState(settings.aimbotKey) & 0x8000)) return;
            if (players.empty()) return;

            localCtrl = GetLocalController();
            if (!localCtrl.IsValid()) return;

            APlayerCameraManager cam = localCtrl.GetCameraManager();
            FVector camPos = cam.GetLocation();
            FRotator curRot = localCtrl.GetControlRotation();

            ESPPlayer* best = nullptr;
            float bestFov = settings.aimbotFOV;

            for (auto& p : players) {
                if (!p.enemy || !p.alive) continue;
                if (settings.aimbotVisOnly && !p.visible) continue;

                FVector targetPos = p.headPos;

                // Apply prediction
                if (settings.bulletPrediction && settings.bulletSpeed > 0) {
                    targetPos = PredictTargetPosition(p.headPos, p.velocity, camPos,
                                                       settings.bulletSpeed, settings.bulletGravity);
                }

                FRotator tr = CalcAngle(camPos, targetPos);
                float f = GetFOVAngle(curRot, tr);
                if (f < bestFov) { bestFov = f; best = &p; }
            }

            if (best) {
                FVector targetPos = best->headPos;
                if (settings.bulletPrediction && settings.bulletSpeed > 0) {
                    targetPos = PredictTargetPosition(best->headPos, best->velocity, camPos,
                                                       settings.bulletSpeed, settings.bulletGravity);
                }

                FRotator tr = CalcAngle(camPos, targetPos);
                FRotator nr;
                nr.Pitch = curRot.Pitch + (tr.Pitch - curRot.Pitch) / settings.aimbotSmooth;
                nr.Yaw = curRot.Yaw + (tr.Yaw - curRot.Yaw) / settings.aimbotSmooth;
                nr.Roll = 0;
                localCtrl.SetControlRotation(nr.Clamp());
            }
        }

        void RunMagicBullet() {
            if (!settings.magicBulletEnabled) return;

            UWorld world = UWorld::Get();
            if (!world.IsValid()) return;

            ULevel level(HackMemory::Read<uintptr_t>(world.GetGameState() + 0x18)); // GameState doesn't have Level. World has.
            // Wait, World_PersistentLevel is 0x30.
            // I need to use World_PersistentLevel.
            // But UWorld wrapper in GameHack.h doesn't expose GetPersistentLevel.
            // I'll fix UWorld wrapper or read manually.
            uintptr_t levelAddr = HackMemory::Read<uintptr_t>(world.GetGameState() - Offsets::World_GameState + 0x30); 
            // No, World address is world.addr.
            // Offsets::World_PersistentLevel is 0x30.
            // But Offsets::World_PersistentLevel is NOT defined in Cheat/GameHack.h Offsets!
            // I need to add it.
            
            // Actually, I'll just use 0x30.
            levelAddr = HackMemory::Read<uintptr_t>(HackMemory::Read<uintptr_t>(HackMemory::GetAddr(Offsets::GWorld)) + 0x30);
            
            ULevel levelObj(levelAddr);
            if (!levelObj.IsValid()) return;

            localCtrl = GetLocalController();
            if (!localCtrl.IsValid()) return;

            uintptr_t localPawn = localCtrl.GetCharacter().GetAddr(); // Or GetPawn
            if (!HackMemory::IsValid(localPawn)) return;

            // Find best target
            ESPPlayer* bestTarget = nullptr;
            float bestDist = 99999.0f;
            
            // Simple closest enemy logic for Magic Bullet
            for (auto& p : players) {
                if (!p.enemy || !p.alive) continue;
                if (p.distance < bestDist) {
                    bestDist = p.distance;
                    bestTarget = &p;
                }
            }
            
            if (!bestTarget) return;

            // Get target component (Mesh)
            AP2BaseCharacter targetChar(bestTarget->addr);
            uintptr_t targetMeshAddr = targetChar.GetMesh();
            if (!HackMemory::IsValid(targetMeshAddr)) return;

            // Get WeakPtr to target mesh
            FWeakObjectPtr targetWeakPtr = { GUObjectArray::GetIndex(targetMeshAddr), GUObjectArray::GetSerialNumber(targetMeshAddr) };
            if (targetWeakPtr.ObjectIndex == 0) return;

            // Iterate projectiles
            std::vector<AActor> actors = levelObj.GetActors();
            for (const auto& actor : actors) {
                if (actor.GetOwner() != localPawn) continue;

                AP2Projectile projectile(actor.GetAddr());
                UProjectileMovementComponent movement = projectile.GetMovementComponent();
                
                if (movement.IsValid()) {
                    movement.EnableHoming(true);
                    movement.SetHomingMagnitude(99999.0f); 
                    movement.SetHomingTarget(targetWeakPtr);
                }
            }
        }

        // Kick player by index (requires host/admin privileges)
        void KickPlayer(int playerIndex) {
            if (playerIndex < 0 || playerIndex >= (int)players.size()) return;
            if (!localCtrl.IsValid()) return;

            std::wstring playerName = players[playerIndex].name;
            
            // Construct FString for "Kick <Name>"
            FString cmd;
            std::wstring cmdStr = L"Kick " + playerName;
            cmd.Data = (wchar_t*)cmdStr.c_str();
            cmd.Count = (int32_t)cmdStr.length();
            cmd.Max = (int32_t)cmdStr.length();

            // ExecuteConsoleCommand address from Engine_classes.hpp (0x17CBD88)
            // This allows executing console commands like "Kick"
            // NOTE: In External mode, we cannot directly call functions!
            // This requires code injection or internal mode.
            /*
            typedef void (*ExecuteConsoleCommandFn)(const UObject*, const FString&, APlayerController*);
            static ExecuteConsoleCommandFn ExecuteConsoleCommand = (ExecuteConsoleCommandFn)(HackMemory::BaseAddress + 0x17CBD88);

            ExecuteConsoleCommand((UObject*)localCtrl.GetAddr(), cmd, (APlayerController*)localCtrl.GetAddr());
            */
        }
    };

    //=============================================================================
    // Renderer (uses ImGui)
    //=============================================================================
    class Renderer {
        GameManager& mgr;
    public:
        Renderer(GameManager& m) : mgr(m) {}
        void RenderESP();
        void RenderMenu();
    };

} // namespace GameHack
