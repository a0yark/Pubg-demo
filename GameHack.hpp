#pragma once

/*
 * Game Reverse Engineering Study Framework
 * Complete Implementation with ImGui
 * For educational purposes only
 */

#include <Windows.h>
#include <cmath>
#include <vector>
#include <string>
#include <d3d11.h>

// ImGui includes - make sure you have imgui in your project
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

namespace GameHack
{
    //=============================================================================
    // SDK Offsets (from SDK analysis)
    //=============================================================================
    namespace Offsets
    {
        // Global Offsets (Basic.hpp)
        constexpr uintptr_t GObjects      = 0x0A5092B0;
        constexpr uintptr_t GWorld        = 0x0A5140F0;
        constexpr uintptr_t ProcessEvent  = 0x00AE0BE0;

        // UWorld (Engine_classes.hpp:4263)
        constexpr uintptr_t World_PersistentLevel    = 0x0030;
        constexpr uintptr_t World_AuthorityGameMode  = 0x0150;
        constexpr uintptr_t World_GameState          = 0x0158;
        constexpr uintptr_t World_Levels             = 0x0170;
        constexpr uintptr_t World_OwningGameInstance = 0x01B8;

        // UGameInstance (Engine_classes.hpp:4412)
        constexpr uintptr_t GameInstance_LocalPlayers = 0x0038;

        // UPlayer / ULocalPlayer (Engine_classes.hpp:6500)
        constexpr uintptr_t Player_PlayerController = 0x0030;

        // AGameStateBase (Engine_classes.hpp:9771)
        constexpr uintptr_t GameState_PlayerArray = 0x02C8;

        // AController (Engine_classes.hpp)
        constexpr uintptr_t Controller_PlayerState     = 0x02B8;
        constexpr uintptr_t Controller_Pawn            = 0x02F0;
        constexpr uintptr_t Controller_Character       = 0x0300;
        constexpr uintptr_t Controller_ControlRotation = 0x0328;

        // APlayerController (Engine_classes.hpp:5326+)
        constexpr uintptr_t PlayerController_Player             = 0x0350;
        constexpr uintptr_t PlayerController_AcknowledgedPawn   = 0x0358;
        constexpr uintptr_t PlayerController_MyHUD              = 0x0360;
        constexpr uintptr_t PlayerController_PlayerCameraManager = 0x0368;
        constexpr uintptr_t PlayerController_TargetViewRotation  = 0x0380;

        // APlayerCameraManager
        constexpr uintptr_t CameraManager_DefaultFOV       = 0x02C8;
        constexpr uintptr_t CameraManager_CameraCache      = 0x22C0;  // FMinimalViewInfo
        constexpr uintptr_t CameraManager_CameraCachePrivate = 0x2480;

        // FMinimalViewInfo offsets
        constexpr uintptr_t MinimalViewInfo_Location = 0x0000;
        constexpr uintptr_t MinimalViewInfo_Rotation = 0x0018;
        constexpr uintptr_t MinimalViewInfo_FOV      = 0x0030;

        // AActor
        constexpr uintptr_t Actor_RootComponent = 0x0198;
        constexpr uintptr_t Actor_Owner = 0x0160;

        // USceneComponent
        constexpr uintptr_t SceneComponent_RelativeLocation = 0x0128;
        constexpr uintptr_t SceneComponent_ComponentToWorld = 0x01C0;

        // ACharacter
        constexpr uintptr_t Character_Mesh = 0x0328;

        // AP2BaseCharacter (Game_classes.hpp)
        constexpr uintptr_t P2BaseCharacter_AttributeSetHealth = 0x07C8;
        constexpr uintptr_t P2BaseCharacter_HealthComponent    = 0x07E0;

        // UP2HealthSet (Game_classes.hpp:8838)
        constexpr uintptr_t HealthSet_Health    = 0x0060;
        constexpr uintptr_t HealthSet_MinHealth = 0x0070;
        constexpr uintptr_t HealthSet_MaxHealth = 0x0080;
        // FGameplayAttributeData BaseValue offset
        constexpr uintptr_t AttributeData_BaseValue = 0x0008;

        // APlayerState
        constexpr uintptr_t PlayerState_PlayerName = 0x0310;
        constexpr uintptr_t PlayerState_TeamId     = 0x04A4;

        // USkeletalMeshComponent
        constexpr uintptr_t SkeletalMesh_BoneSpaceTransforms = 0x0570;
        constexpr uintptr_t SkeletalMesh_ComponentSpaceTransforms = 0x0590;

        // ULevel
        constexpr uintptr_t Level_Actors = 0x00A0; // Assumed for UE 5.2

        // AP2Projectile
        constexpr uintptr_t Projectile_ProjectileMovement = 0x02B0;

        // UProjectileMovementComponent
        constexpr uintptr_t ProjectileMovement_bIsHomingProjectile = 0x0130; // Bit 7
        constexpr uintptr_t ProjectileMovement_HomingAccelerationMagnitude = 0x0188;
        constexpr uintptr_t ProjectileMovement_HomingTargetComponent = 0x018C;
    }

    //=============================================================================
    // Basic Math Structures
    //=============================================================================
    struct FVector
    {
        float X, Y, Z;

        FVector() : X(0), Y(0), Z(0) {}
        FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

        FVector operator-(const FVector& other) const {
            return FVector(X - other.X, Y - other.Y, Z - other.Z);
        }

        FVector operator+(const FVector& other) const {
            return FVector(X + other.X, Y + other.Y, Z + other.Z);
        }

        FVector operator*(float scalar) const {
            return FVector(X * scalar, Y * scalar, Z * scalar);
        }

        float Length() const {
            return sqrtf(X * X + Y * Y + Z * Z);
        }

        float Distance(const FVector& other) const {
            return (*this - other).Length();
        }

        float Dot(const FVector& other) const {
            return X * other.X + Y * other.Y + Z * other.Z;
        }

        FVector Normalize() const {
            float len = Length();
            if (len == 0) return FVector();
            return FVector(X / len, Y / len, Z / len);
        }
    };

    struct FRotator
    {
        float Pitch; // Up/Down (-90 to 90)
        float Yaw;   // Left/Right (-180 to 180)
        float Roll;  // Tilt

        FRotator() : Pitch(0), Yaw(0), Roll(0) {}
        FRotator(float p, float y, float r) : Pitch(p), Yaw(y), Roll(r) {}

        FRotator Clamp() const {
            FRotator result = *this;
            if (result.Pitch > 89.0f) result.Pitch = 89.0f;
            if (result.Pitch < -89.0f) result.Pitch = -89.0f;
            while (result.Yaw > 180.0f) result.Yaw -= 360.0f;
            while (result.Yaw < -180.0f) result.Yaw += 360.0f;
            result.Roll = 0;
            return result;
        }

        FRotator operator-(const FRotator& other) const {
            return FRotator(Pitch - other.Pitch, Yaw - other.Yaw, Roll - other.Roll);
        }
    };

    struct FMatrix
    {
        float M[4][4];

        FMatrix() { memset(M, 0, sizeof(M)); }
    };

    struct FTransform
    {
        float Rotation[4];    // Quaternion (X, Y, Z, W)
        FVector Translation;
        float Pad1;
        FVector Scale3D;
        float Pad2;
    };

    // TArray structure
    template<typename T>
    struct TArray
    {
        T* Data;
        int32_t Count;
        int32_t Max;
    };

    //=============================================================================
    // Memory Read/Write Helpers
    //=============================================================================
    class Memory
    {
    public:
        static uintptr_t BaseAddress;

        static void Initialize() {
            BaseAddress = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
        }

        template<typename T>
        static T Read(uintptr_t address) {
            if (!address) return T();
            __try {
                return *reinterpret_cast<T*>(address);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return T();
            }
        }

        template<typename T>
        static void Write(uintptr_t address, const T& value) {
            if (!address) return;
            __try {
                *reinterpret_cast<T*>(address) = value;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }

        static uintptr_t GetAddress(uintptr_t offset) {
            return BaseAddress + offset;
        }

        static bool IsValidPtr(uintptr_t ptr) {
            return ptr >= 0x10000 && ptr < 0x7FFFFFFFFFFF;
        }
    };

    uintptr_t Memory::BaseAddress = 0;

    //=============================================================================
    // GUObjectArray Helper
    //=============================================================================
    class GUObjectArray
    {
    public:
        static int32_t GetSerialNumber(uintptr_t objectAddr) {
            if (!Memory::IsValidPtr(objectAddr)) return 0;

            int32_t index = Memory::Read<int32_t>(objectAddr + 0xC); // InternalIndex

            uintptr_t gObjects = Memory::Read<uintptr_t>(Memory::GetAddress(Offsets::GObjects));
            uintptr_t objectsPtr = Memory::Read<uintptr_t>(gObjects); // Objects

            int32_t chunkIndex = index / 65536;
            int32_t withinChunkIndex = index % 65536;

            uintptr_t chunk = Memory::Read<uintptr_t>(objectsPtr + chunkIndex * 8);
            if (!chunk) return 0;

            // FUObjectItem size 24
            uintptr_t item = chunk + withinChunkIndex * 24;
            return Memory::Read<int32_t>(item + 16); // SerialNumber
        }

        static int32_t GetIndex(uintptr_t objectAddr) {
            if (!Memory::IsValidPtr(objectAddr)) return 0;
            return Memory::Read<int32_t>(objectAddr + 0xC);
        }
    };


    //=============================================================================
    // Core Math Functions
    //=============================================================================

    constexpr float PI = 3.14159265358979323846f;

    inline float ToRadians(float degrees) {
        return degrees * (PI / 180.0f);
    }

    inline float ToDegrees(float radians) {
        return radians * (180.0f / PI);
    }

    // Calculate angle from one position to another
    inline FRotator CalcAngle(const FVector& from, const FVector& to) {
        FVector delta = to - from;
        float distance = delta.Length();

        if (distance == 0) return FRotator();

        float pitch = -asinf(delta.Z / distance) * (180.0f / PI);
        float yaw = atan2f(delta.Y, delta.X) * (180.0f / PI);

        return FRotator(pitch, yaw, 0).Clamp();
    }

    // Calculate FOV angle between two rotations
    inline float GetFOVAngle(const FRotator& viewAngle, const FRotator& targetAngle) {
        float pitchDiff = viewAngle.Pitch - targetAngle.Pitch;
        float yawDiff = viewAngle.Yaw - targetAngle.Yaw;

        while (yawDiff > 180.0f) yawDiff -= 360.0f;
        while (yawDiff < -180.0f) yawDiff += 360.0f;

        return sqrtf(pitchDiff * pitchDiff + yawDiff * yawDiff);
    }

    // Build rotation matrix from rotator
    inline FMatrix RotatorToMatrix(const FRotator& rot) {
        FMatrix matrix;

        float radPitch = ToRadians(rot.Pitch);
        float radYaw = ToRadians(rot.Yaw);
        float radRoll = ToRadians(rot.Roll);

        float SP = sinf(radPitch);
        float CP = cosf(radPitch);
        float SY = sinf(radYaw);
        float CY = cosf(radYaw);
        float SR = sinf(radRoll);
        float CR = cosf(radRoll);

        matrix.M[0][0] = CP * CY;
        matrix.M[0][1] = CP * SY;
        matrix.M[0][2] = SP;
        matrix.M[0][3] = 0.0f;

        matrix.M[1][0] = SR * SP * CY - CR * SY;
        matrix.M[1][1] = SR * SP * SY + CR * CY;
        matrix.M[1][2] = -SR * CP;
        matrix.M[1][3] = 0.0f;

        matrix.M[2][0] = -(CR * SP * CY + SR * SY);
        matrix.M[2][1] = CY * SR - CR * SP * SY;
        matrix.M[2][2] = CR * CP;
        matrix.M[2][3] = 0.0f;

        matrix.M[3][0] = 0.0f;
        matrix.M[3][1] = 0.0f;
        matrix.M[3][2] = 0.0f;
        matrix.M[3][3] = 1.0f;

        return matrix;
    }

    // World to Screen projection
    inline bool WorldToScreen(const FVector& worldPos, const FVector& cameraPos, const FRotator& cameraRot,
                               float fov, int screenWidth, int screenHeight, FVector& screenPos) {
        FVector delta = worldPos - cameraPos;
        FMatrix rotMatrix = RotatorToMatrix(cameraRot);

        FVector transformed;
        transformed.X = delta.X * rotMatrix.M[0][0] + delta.Y * rotMatrix.M[0][1] + delta.Z * rotMatrix.M[0][2];
        transformed.Y = delta.X * rotMatrix.M[1][0] + delta.Y * rotMatrix.M[1][1] + delta.Z * rotMatrix.M[1][2];
        transformed.Z = delta.X * rotMatrix.M[2][0] + delta.Y * rotMatrix.M[2][1] + delta.Z * rotMatrix.M[2][2];

        if (transformed.X < 1.0f) return false;

        float fovRad = ToRadians(fov);
        float screenCenterX = screenWidth / 2.0f;
        float screenCenterY = screenHeight / 2.0f;
        float tanFov = tanf(fovRad / 2.0f);

        screenPos.X = screenCenterX + (transformed.Y / transformed.X) * (screenCenterX / tanFov);
        screenPos.Y = screenCenterY - (transformed.Z / transformed.X) * (screenCenterX / tanFov);
        screenPos.Z = transformed.X;

        return true;
    }

    //=============================================================================
    // Game Classes Wrappers
    //=============================================================================

    class UWorld
    {
    private:
        uintptr_t address;

    public:
        UWorld(uintptr_t addr) : address(addr) {}

        static UWorld GetWorld() {
            uintptr_t worldPtr = Memory::Read<uintptr_t>(Memory::GetAddress(Offsets::GWorld));
            return UWorld(worldPtr);
        }

        bool IsValid() const { return Memory::IsValidPtr(address); }
        uintptr_t GetAddress() const { return address; }

        uintptr_t GetGameState() const {
            if (!IsValid()) return 0;
            return Memory::Read<uintptr_t>(address + Offsets::World_GameState);
        }

        uintptr_t GetOwningGameInstance() const {
            if (!IsValid()) return 0;
            return Memory::Read<uintptr_t>(address + Offsets::World_OwningGameInstance);
        }
    };

    class UGameInstance
    {
    private:
        uintptr_t address;

    public:
        UGameInstance(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return Memory::IsValidPtr(address); }

        uintptr_t GetLocalPlayer(int index = 0) const {
            if (!IsValid()) return 0;
            uintptr_t arrayData = Memory::Read<uintptr_t>(address + Offsets::GameInstance_LocalPlayers);
            int32_t count = Memory::Read<int32_t>(address + Offsets::GameInstance_LocalPlayers + 0x08);
            if (index >= count || !arrayData) return 0;
            return Memory::Read<uintptr_t>(arrayData + index * 0x08);
        }
    };

    class ULocalPlayer
    {
    private:
        uintptr_t address;

    public:
        ULocalPlayer(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return Memory::IsValidPtr(address); }

        uintptr_t GetPlayerController() const {
            if (!IsValid()) return 0;
            return Memory::Read<uintptr_t>(address + Offsets::Player_PlayerController);
        }
    };

    class AGameStateBase
    {
    private:
        uintptr_t address;

    public:
        AGameStateBase(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return Memory::IsValidPtr(address); }

        int GetPlayerCount() const {
            if (!IsValid()) return 0;
            return Memory::Read<int32_t>(address + Offsets::GameState_PlayerArray + 0x08);
        }

        uintptr_t GetPlayerState(int index) const {
            if (!IsValid()) return 0;
            uintptr_t arrayData = Memory::Read<uintptr_t>(address + Offsets::GameState_PlayerArray);
            int32_t count = GetPlayerCount();
            if (index >= count || !arrayData) return 0;
            return Memory::Read<uintptr_t>(arrayData + index * 0x08);
        }
    };

    class APlayerState
    {
    private:
        uintptr_t address;

    public:
        APlayerState(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return Memory::IsValidPtr(address); }
        uintptr_t GetAddress() const { return address; }

        int32_t GetTeamId() const {
            if (!IsValid()) return -1;
            return Memory::Read<int32_t>(address + Offsets::PlayerState_TeamId);
        }

        uintptr_t GetPawn() const {
            if (!IsValid()) return 0;
            // APlayerState has a PawnPrivate member, typically at offset 0x02F0
            return Memory::Read<uintptr_t>(address + 0x02F0);
        }
    };

    class UP2HealthSet
    {
    private:
        uintptr_t address;

    public:
        UP2HealthSet(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return Memory::IsValidPtr(address); }

        float GetHealth() const {
            if (!IsValid()) return 0;
            return Memory::Read<float>(address + Offsets::HealthSet_Health + Offsets::AttributeData_BaseValue);
        }

        float GetMaxHealth() const {
            if (!IsValid()) return 0;
            return Memory::Read<float>(address + Offsets::HealthSet_MaxHealth + Offsets::AttributeData_BaseValue);
        }

        float GetHealthPercent() const {
            float max = GetMaxHealth();
            if (max <= 0) return 0;
            return GetHealth() / max;
        }
    };

    class AActor
    {
    protected:
        uintptr_t address;

    public:
        AActor(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return Memory::IsValidPtr(address); }
        uintptr_t GetAddress() const { return address; }

        FVector GetLocation() const {
            if (!IsValid()) return FVector();
            uintptr_t rootComp = Memory::Read<uintptr_t>(address + Offsets::Actor_RootComponent);
            if (!Memory::IsValidPtr(rootComp)) return FVector();
            return Memory::Read<FVector>(rootComp + Offsets::SceneComponent_RelativeLocation);
        }

        uintptr_t GetOwner() const {
            if (!IsValid()) return 0;
            return Memory::Read<uintptr_t>(address + Offsets::Actor_Owner);
        }
    };

    class ACharacter : public AActor
    {
    public:
        ACharacter(uintptr_t addr) : AActor(addr) {}

        uintptr_t GetMesh() const {
            if (!IsValid()) return 0;
            return Memory::Read<uintptr_t>(address + Offsets::Character_Mesh);
        }
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

        bool IsValid() const { return Memory::IsValidPtr(address); }

        void EnableHoming(bool enable) {
            if (!IsValid()) return;
            uint8_t value = Memory::Read<uint8_t>(address + Offsets::ProjectileMovement_bIsHomingProjectile);
            if (enable) value |= 0x80; // Bit 7
            else value &= ~0x80;
            Memory::Write<uint8_t>(address + Offsets::ProjectileMovement_bIsHomingProjectile, value);
        }

        void SetHomingMagnitude(float magnitude) {
            if (!IsValid()) return;
            Memory::Write<float>(address + Offsets::ProjectileMovement_HomingAccelerationMagnitude, magnitude);
        }

        void SetHomingTarget(const FWeakObjectPtr& target) {
            if (!IsValid()) return;
            Memory::Write<FWeakObjectPtr>(address + Offsets::ProjectileMovement_HomingTargetComponent, target);
        }
    };

    class AP2Projectile : public AActor
    {
    public:
        AP2Projectile(uintptr_t addr) : AActor(addr) {}

        UProjectileMovementComponent GetMovementComponent() const {
            if (!IsValid()) return UProjectileMovementComponent(0);
            return UProjectileMovementComponent(Memory::Read<uintptr_t>(address + Offsets::Projectile_ProjectileMovement));
        }
    };

    class ULevel
    {
    private:
        uintptr_t address;

    public:
        ULevel(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return Memory::IsValidPtr(address); }

        std::vector<AActor> GetActors() const {
            std::vector<AActor> actors;
            if (!IsValid()) return actors;

            uintptr_t actorsArray = address + Offsets::Level_Actors;
            uintptr_t data = Memory::Read<uintptr_t>(actorsArray);
            int32_t count = Memory::Read<int32_t>(actorsArray + 0x08);

            if (!Memory::IsValidPtr(data) || count <= 0 || count > 10000) return actors;

            for (int i = 0; i < count; i++) {
                uintptr_t actorAddr = Memory::Read<uintptr_t>(data + i * 0x08);
                if (Memory::IsValidPtr(actorAddr)) {
                    actors.push_back(AActor(actorAddr));
                }
            }
            return actors;
        }
    };


    class AP2BaseCharacter : public ACharacter
    {
    public:
        AP2BaseCharacter(uintptr_t addr) : ACharacter(addr) {}

        UP2HealthSet GetHealthSet() const {
            if (!IsValid()) return UP2HealthSet(0);
            uintptr_t healthSet = Memory::Read<uintptr_t>(address + Offsets::P2BaseCharacter_AttributeSetHealth);
            return UP2HealthSet(healthSet);
        }

        float GetHealth() const {
            return GetHealthSet().GetHealth();
        }

        float GetMaxHealth() const {
            return GetHealthSet().GetMaxHealth();
        }

        bool IsAlive() const {
            return GetHealth() > 0;
        }

        float GetHealthPercent() const {
            return GetHealthSet().GetHealthPercent();
        }
    };

    class APlayerCameraManager
    {
    private:
        uintptr_t address;

    public:
        APlayerCameraManager(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return Memory::IsValidPtr(address); }

        float GetFOV() const {
            if (!IsValid()) return 90.0f;
            return Memory::Read<float>(address + Offsets::CameraManager_DefaultFOV);
        }

        FVector GetCameraLocation() const {
            if (!IsValid()) return FVector();
            // Try CameraCachePrivate first
            return Memory::Read<FVector>(address + Offsets::CameraManager_CameraCachePrivate + Offsets::MinimalViewInfo_Location);
        }

        FRotator GetCameraRotation() const {
            if (!IsValid()) return FRotator();
            return Memory::Read<FRotator>(address + Offsets::CameraManager_CameraCachePrivate + Offsets::MinimalViewInfo_Rotation);
        }
    };

    class AController : public AActor
    {
    public:
        AController(uintptr_t addr) : AActor(addr) {}

        FRotator GetControlRotation() const {
            if (!IsValid()) return FRotator();
            return Memory::Read<FRotator>(address + Offsets::Controller_ControlRotation);
        }

        void SetControlRotation(const FRotator& rotation) {
            if (!IsValid()) return;
            Memory::Write<FRotator>(address + Offsets::Controller_ControlRotation, rotation);
        }

        uintptr_t GetPawn() const {
            if (!IsValid()) return 0;
            return Memory::Read<uintptr_t>(address + Offsets::Controller_Pawn);
        }

        APlayerState GetPlayerState() const {
            if (!IsValid()) return APlayerState(0);
            return APlayerState(Memory::Read<uintptr_t>(address + Offsets::Controller_PlayerState));
        }
    };

    class APlayerController : public AController
    {
    public:
        APlayerController(uintptr_t addr) : AController(addr) {}

        APlayerCameraManager GetCameraManager() const {
            if (!IsValid()) return APlayerCameraManager(0);
            return APlayerCameraManager(Memory::Read<uintptr_t>(address + Offsets::PlayerController_PlayerCameraManager));
        }

        AP2BaseCharacter GetCharacter() const {
            if (!IsValid()) return AP2BaseCharacter(0);
            return AP2BaseCharacter(Memory::Read<uintptr_t>(address + Offsets::Controller_Character));
        }

        uintptr_t GetAcknowledgedPawn() const {
            if (!IsValid()) return 0;
            return Memory::Read<uintptr_t>(address + Offsets::PlayerController_AcknowledgedPawn);
        }
    };

    class USkeletalMeshComponent
    {
    private:
        uintptr_t address;

    public:
        USkeletalMeshComponent(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return Memory::IsValidPtr(address); }

        int GetBoneCount() const {
            if (!IsValid()) return 0;
            return Memory::Read<int32_t>(address + Offsets::SkeletalMesh_ComponentSpaceTransforms + 0x08);
        }

        FTransform GetBoneTransform(int boneIndex) const {
            FTransform result = {};
            if (!IsValid()) return result;

            int boneCount = GetBoneCount();
            if (boneIndex < 0 || boneIndex >= boneCount) return result;

            uintptr_t transformArray = Memory::Read<uintptr_t>(address + Offsets::SkeletalMesh_ComponentSpaceTransforms);
            if (!Memory::IsValidPtr(transformArray)) return result;

            return Memory::Read<FTransform>(transformArray + boneIndex * sizeof(FTransform));
        }

        FMatrix GetComponentToWorld() const {
            if (!IsValid()) return FMatrix();
            return Memory::Read<FMatrix>(address + Offsets::SceneComponent_ComponentToWorld);
        }

        FVector GetBoneWorldPosition(int boneIndex) const {
            if (!IsValid()) return FVector();

            FTransform boneTransform = GetBoneTransform(boneIndex);
            FMatrix compToWorld = GetComponentToWorld();

            FVector localPos = boneTransform.Translation;
            FVector worldPos;

            worldPos.X = compToWorld.M[0][0] * localPos.X + compToWorld.M[1][0] * localPos.Y +
                         compToWorld.M[2][0] * localPos.Z + compToWorld.M[3][0];
            worldPos.Y = compToWorld.M[0][1] * localPos.X + compToWorld.M[1][1] * localPos.Y +
                         compToWorld.M[2][1] * localPos.Z + compToWorld.M[3][1];
            worldPos.Z = compToWorld.M[0][2] * localPos.X + compToWorld.M[1][2] * localPos.Y +
                         compToWorld.M[2][2] * localPos.Z + compToWorld.M[3][2];

            return worldPos;
        }
    };

    //=============================================================================
    // Common Bone IDs (may need adjustment for this specific game)
    //=============================================================================
    namespace BoneID
    {
        constexpr int Head = 98;        // Common head bone index
        constexpr int Neck = 97;
        constexpr int Chest = 66;
        constexpr int Spine = 65;
        constexpr int Pelvis = 0;

        constexpr int ShoulderL = 78;
        constexpr int ElbowL = 79;
        constexpr int HandL = 80;

        constexpr int ShoulderR = 49;
        constexpr int ElbowR = 50;
        constexpr int HandR = 51;

        constexpr int HipL = 77;
        constexpr int KneeL = 78;
        constexpr int FootL = 79;

        constexpr int HipR = 48;
        constexpr int KneeR = 49;
        constexpr int FootR = 50;
    }

    //=============================================================================
    // ESP Target Data
    //=============================================================================
    struct ESPPlayer
    {
        uintptr_t address;
        FVector position;
        FVector headPos;
        FVector screenPos;
        FVector headScreenPos;
        float health;
        float maxHealth;
        float distance;
        int32_t teamId;
        bool isVisible;
        bool isEnemy;
        bool isAlive;
    };

    //=============================================================================
    // Settings
    //=============================================================================
    struct Settings
    {
        // ESP Settings
        bool espEnabled = true;
        bool espBox = true;
        bool espHealth = true;
        bool espDistance = true;
        bool espSkeleton = false;
        bool espHeadCircle = true;
        bool espSnapline = false;
        float espMaxDistance = 500.0f;

        // Aimbot Settings
        bool aimbotEnabled = false;
        bool aimbotVisibleOnly = true;
        float aimbotFOV = 15.0f;
        float aimbotSmooth = 5.0f;
        int aimbotBone = BoneID::Head;
        int aimbotKey = VK_RBUTTON;

        // Magic Bullet Settings
        bool magicBulletEnabled = false;

        // Colors (ImGui colors are 0-1 range)
        ImVec4 colorEnemy = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        ImVec4 colorTeam = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 colorHealthFull = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 colorHealthLow = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    };

    //=============================================================================
    // Main Game Manager
    //=============================================================================
    class GameManager
    {
    private:
        APlayerController localController;
        std::vector<ESPPlayer> players;
        Settings settings;
        int screenWidth;
        int screenHeight;
        bool initialized;

    public:
        GameManager() : localController(0), screenWidth(1920), screenHeight(1080), initialized(false) {}

        Settings& GetSettings() { return settings; }
        const std::vector<ESPPlayer>& GetPlayers() const { return players; }

        bool Initialize() {
            Memory::Initialize();

            if (Memory::BaseAddress == 0) {
                return false;
            }

            initialized = true;
            return true;
        }

        void SetScreenSize(int w, int h) {
            screenWidth = w;
            screenHeight = h;
        }

        APlayerController GetLocalController() {
            UWorld world = UWorld::GetWorld();
            if (!world.IsValid()) return APlayerController(0);

            UGameInstance gameInstance(world.GetOwningGameInstance());
            if (!gameInstance.IsValid()) return APlayerController(0);

            ULocalPlayer localPlayer(gameInstance.GetLocalPlayer(0));
            if (!localPlayer.IsValid()) return APlayerController(0);

            return APlayerController(localPlayer.GetPlayerController());
        }

        void Update() {
            if (!initialized) return;

            players.clear();

            localController = GetLocalController();
            if (!localController.IsValid()) return;

            AP2BaseCharacter localChar = localController.GetCharacter();
            if (!localChar.IsValid()) return;

            APlayerCameraManager cameraManager = localController.GetCameraManager();
            if (!cameraManager.IsValid()) return;

            FVector cameraPos = cameraManager.GetCameraLocation();
            FRotator cameraRot = cameraManager.GetCameraRotation();
            float fov = cameraManager.GetFOV();

            int32_t localTeamId = localController.GetPlayerState().GetTeamId();

            // Get all players from GameState
            UWorld world = UWorld::GetWorld();
            AGameStateBase gameState(world.GetGameState());
            if (!gameState.IsValid()) return;

            int playerCount = gameState.GetPlayerCount();
            for (int i = 0; i < playerCount; i++) {
                APlayerState playerState(gameState.GetPlayerState(i));
                if (!playerState.IsValid()) continue;

                uintptr_t pawnAddr = playerState.GetPawn();
                if (!Memory::IsValidPtr(pawnAddr)) continue;

                // Skip local player
                if (pawnAddr == localChar.GetAddress()) continue;

                AP2BaseCharacter character(pawnAddr);
                if (!character.IsValid() || !character.IsAlive()) continue;

                ESPPlayer player = {};
                player.address = pawnAddr;
                player.position = character.GetLocation();
                player.health = character.GetHealth();
                player.maxHealth = character.GetMaxHealth();
                player.teamId = playerState.GetTeamId();
                player.isEnemy = (player.teamId != localTeamId);
                player.isAlive = player.health > 0;
                player.distance = player.position.Distance(localChar.GetLocation()) / 100.0f; // Convert to meters

                if (player.distance > settings.espMaxDistance) continue;

                // Get head position from skeleton
                USkeletalMeshComponent mesh(character.GetMesh());
                if (mesh.IsValid()) {
                    player.headPos = mesh.GetBoneWorldPosition(BoneID::Head);
                } else {
                    player.headPos = player.position;
                    player.headPos.Z += 80.0f; // Approximate head height
                }

                // World to screen
                player.isVisible = WorldToScreen(player.position, cameraPos, cameraRot, fov,
                                                  screenWidth, screenHeight, player.screenPos);

                if (player.isVisible) {
                    WorldToScreen(player.headPos, cameraPos, cameraRot, fov,
                                  screenWidth, screenHeight, player.headScreenPos);
                }

                players.push_back(player);
            }
        }

        ESPPlayer* FindBestAimbotTarget() {
            if (players.empty()) return nullptr;

            localController = GetLocalController();
            if (!localController.IsValid()) return nullptr;

            FRotator currentRot = localController.GetControlRotation();
            AP2BaseCharacter localChar = localController.GetCharacter();
            if (!localChar.IsValid()) return nullptr;

            FVector localPos = localChar.GetLocation();
            APlayerCameraManager cam = localController.GetCameraManager();
            FVector camPos = cam.GetCameraLocation();
            

            ESPPlayer* bestTarget = nullptr;
            float bestFOV = settings.aimbotFOV;

            for (auto& player : players) {
                if (!player.isEnemy || !player.isAlive) continue;
                if (settings.aimbotVisibleOnly && !player.isVisible) continue;

                FVector targetPos = player.headPos;
                FRotator targetRot = CalcAngle(camPos, targetPos);
                float fovAngle = GetFOVAngle(currentRot, targetRot);

                if (fovAngle < bestFOV) {
                    bestFOV = fovAngle;
                    bestTarget = &player;
                }
            }

            return bestTarget;
        }

        void RunAimbot() {
            if (!settings.aimbotEnabled) return;
            if (!(GetAsyncKeyState(settings.aimbotKey) & 0x8000)) return;

            ESPPlayer* target = FindBestAimbotTarget();
            if (!target) return;

            localController = GetLocalController();
            if (!localController.IsValid()) return;

            APlayerCameraManager cam = localController.GetCameraManager();
            FVector camPos = cam.GetCameraLocation();

            FRotator currentRot = localController.GetControlRotation();
            FRotator targetRot = CalcAngle(camPos, target->headPos);

            // Apply smoothing
            FRotator newRot;
            newRot.Pitch = currentRot.Pitch + (targetRot.Pitch - currentRot.Pitch) / settings.aimbotSmooth;
            newRot.Yaw = currentRot.Yaw + (targetRot.Yaw - currentRot.Yaw) / settings.aimbotSmooth;
            newRot.Roll = 0;
            newRot = newRot.Clamp();

            localController.SetControlRotation(newRot);
        }

        void RunMagicBullet() {
            if (!settings.magicBulletEnabled) return;

            UWorld world = UWorld::GetWorld();
            if (!world.IsValid()) return;

            ULevel level(Memory::Read<uintptr_t>(world.GetAddress() + Offsets::World_PersistentLevel));
            if (!level.IsValid()) return;

            localController = GetLocalController();
            if (!localController.IsValid()) return;

            uintptr_t localPawn = localController.GetPawn();
            if (!Memory::IsValidPtr(localPawn)) return;

            // Find best target (closest enemy to crosshair)
            ESPPlayer* bestTarget = FindBestAimbotTarget(); 
            if (!bestTarget) return;

            // Get target component (Mesh)
            AP2BaseCharacter targetChar(bestTarget->address);
            uintptr_t targetMeshAddr = targetChar.GetMesh();
            if (!Memory::IsValidPtr(targetMeshAddr)) return;

            // Get WeakPtr to target mesh
            FWeakObjectPtr targetWeakPtr = GUObjectArray::GetWeakPtr(targetMeshAddr);
            if (targetWeakPtr.ObjectIndex == 0) return;

            // Iterate projectiles
            std::vector<AActor> actors = level.GetActors();
            for (const auto& actor : actors) {
                // Check owner
                if (actor.GetOwner() != localPawn) continue;

                // Try to get projectile movement
                AP2Projectile projectile(actor.GetAddress());
                UProjectileMovementComponent movement = projectile.GetMovementComponent();
                
                if (movement.IsValid()) {
                    // Enable Homing
                    movement.EnableHoming(true);
                    movement.SetHomingMagnitude(99999.0f); 
                    movement.SetHomingTarget(targetWeakPtr);
                }
            }
        }
    };

    //=============================================================================
    // ImGui Renderer
    //=============================================================================
    class Renderer
    {
    private:
        GameManager& manager;

        ImU32 ToImColor(const ImVec4& color) {
            return IM_COL32(
                (int)(color.x * 255),
                (int)(color.y * 255),
                (int)(color.z * 255),
                (int)(color.w * 255)
            );
        }

        ImU32 GetHealthColor(float healthPercent, Settings& s) {
            float r = s.colorHealthLow.x + (s.colorHealthFull.x - s.colorHealthLow.x) * healthPercent;
            float g = s.colorHealthLow.y + (s.colorHealthFull.y - s.colorHealthLow.y) * healthPercent;
            float b = s.colorHealthLow.z + (s.colorHealthFull.z - s.colorHealthLow.z) * healthPercent;
            return IM_COL32((int)(r * 255), (int)(g * 255), (int)(b * 255), 255);
        }

    public:
        Renderer(GameManager& mgr) : manager(mgr) {}

        void RenderESP() {
            Settings& s = manager.GetSettings();
            if (!s.espEnabled) return;

            ImDrawList* draw = ImGui::GetBackgroundDrawList();
            const auto& players = manager.GetPlayers();

            for (const auto& player : players) {
                if (!player.isVisible) continue;

                ImU32 color = player.isEnemy ? ToImColor(s.colorEnemy) : ToImColor(s.colorTeam);

                float boxHeight = fabsf(player.screenPos.Y - player.headScreenPos.Y);
                float boxWidth = boxHeight * 0.5f;

                float boxTop = player.headScreenPos.Y - 10;
                float boxBottom = player.screenPos.Y;
                float boxLeft = player.screenPos.X - boxWidth / 2;
                float boxRight = player.screenPos.X + boxWidth / 2;

                // Box ESP
                if (s.espBox) {
                    draw->AddRect(ImVec2(boxLeft, boxTop), ImVec2(boxRight, boxBottom), color, 0, 0, 2.0f);
                }

                // Health bar
                if (s.espHealth) {
                    float healthPercent = player.health / player.maxHealth;
                    float healthHeight = boxHeight * healthPercent;

                    draw->AddRectFilled(
                        ImVec2(boxLeft - 6, boxBottom),
                        ImVec2(boxLeft - 2, boxTop),
                        IM_COL32(0, 0, 0, 180)
                    );
                    draw->AddRectFilled(
                        ImVec2(boxLeft - 5, boxBottom),
                        ImVec2(boxLeft - 3, boxBottom - healthHeight),
                        GetHealthColor(healthPercent, s)
                    );
                }

                // Distance
                if (s.espDistance) {
                    char distText[32];
                    sprintf_s(distText, "%.0fm", player.distance);
                    draw->AddText(ImVec2(player.screenPos.X - 15, boxBottom + 2), color, distText);
                }

                // Head circle
                if (s.espHeadCircle) {
                    float headRadius = boxWidth * 0.2f;
                    draw->AddCircle(ImVec2(player.headScreenPos.X, player.headScreenPos.Y), headRadius, color, 16, 2.0f);
                }

                // Snapline
                if (s.espSnapline) {
                    ImGuiIO& io = ImGui::GetIO();
                    draw->AddLine(
                        ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y),
                        ImVec2(player.screenPos.X, boxBottom),
                        color, 1.5f
                    );
                }
            }

            // Draw aimbot FOV circle
            if (s.aimbotEnabled) {
                ImGuiIO& io = ImGui::GetIO();
                float fovRadius = tanf(ToRadians(s.aimbotFOV)) * (io.DisplaySize.x / 2);
                draw->AddCircle(
                    ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2),
                    fovRadius,
                    IM_COL32(255, 255, 255, 100),
                    64, 1.5f
                );
            }
        }

        void RenderMenu() {
            Settings& s = manager.GetSettings();

            ImGui::SetNextWindowSize(ImVec2(400, 450), ImGuiCond_FirstUseEver);
            ImGui::Begin("Game Hack Menu", nullptr, ImGuiWindowFlags_NoCollapse);

            if (ImGui::CollapsingHeader("ESP Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Enable ESP", &s.espEnabled);
                ImGui::Checkbox("Box", &s.espBox);
                ImGui::Checkbox("Health Bar", &s.espHealth);
                ImGui::Checkbox("Distance", &s.espDistance);
                ImGui::Checkbox("Skeleton", &s.espSkeleton);
                ImGui::Checkbox("Head Circle", &s.espHeadCircle);
                ImGui::Checkbox("Snaplines", &s.espSnapline);
                ImGui::SliderFloat("Max Distance", &s.espMaxDistance, 50.0f, 1000.0f, "%.0f m");

                ImGui::ColorEdit4("Enemy Color", &s.colorEnemy.x);
                ImGui::ColorEdit4("Team Color", &s.colorTeam.x);
            }

            if (ImGui::CollapsingHeader("Aimbot Settings")) {
                ImGui::Checkbox("Enable Aimbot", &s.aimbotEnabled);
                ImGui::Checkbox("Magic Bullet", &s.magicBulletEnabled);
                ImGui::Checkbox("Visible Only", &s.aimbotVisibleOnly);
                ImGui::SliderFloat("FOV", &s.aimbotFOV, 1.0f, 90.0f, "%.1f");
                ImGui::SliderFloat("Smoothing", &s.aimbotSmooth, 1.0f, 20.0f, "%.1f");

                const char* bones[] = { "Head", "Neck", "Chest" };
                static int selectedBone = 0;
                if (ImGui::Combo("Target Bone", &selectedBone, bones, IM_ARRAYSIZE(bones))) {
                    switch (selectedBone) {
                        case 0: s.aimbotBone = BoneID::Head; break;
                        case 1: s.aimbotBone = BoneID::Neck; break;
                        case 2: s.aimbotBone = BoneID::Chest; break;
                    }
                }

                ImGui::Text("Aim Key: Right Mouse Button");
            }

            ImGui::Separator();
            ImGui::Text("Players Found: %d", (int)manager.GetPlayers().size());

            ImGui::End();
        }
    };

} // namespace GameHack


//=============================================================================
// Usage Example - Main Loop Integration
//=============================================================================
/*
// Global instances
GameHack::GameManager g_GameManager;
GameHack::Renderer* g_Renderer = nullptr;

// Call once after D3D11 hook and ImGui initialization
void InitializeHack() {
    g_GameManager.Initialize();
    g_Renderer = new GameHack::Renderer(g_GameManager);
}

// Call every frame in your Present hook
void OnFrame() {
    ImGuiIO& io = ImGui::GetIO();
    g_GameManager.SetScreenSize((int)io.DisplaySize.x, (int)io.DisplaySize.y);
    g_GameManager.Update();
    g_GameManager.RunAimbot();

    // Render ESP (should be before ImGui::Render())
    g_Renderer->RenderESP();

    // Toggle menu with INSERT key
    static bool showMenu = true;
    if (GetAsyncKeyState(VK_INSERT) & 1) {
        showMenu = !showMenu;
    }

    if (showMenu) {
        g_Renderer->RenderMenu();
    }}
*/
