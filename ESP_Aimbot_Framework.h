#pragma once

/**
 * ESP & Aimbot Framework - Based on SDK Analysis
 * For game development/testing purposes only
 *
 * Key Classes Inheritance Chain:
 * ABP_Hero_C -> ABP_CharacterDefault_C -> AP2HeroCharacter -> AP2BaseCharacter -> ACharacter -> APawn -> AActor
 */

#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>

namespace SDK
{
    // ==================== Core Offsets ====================

    // AActor (Size: 0x02B0)
    constexpr uintptr_t OFFSET_ACTOR_RootComponent = 0x01B8;          // USceneComponent*

    // APawn (Size: 0x0338)
    constexpr uintptr_t OFFSET_PAWN_PlayerState = 0x02D0;             // APlayerState*
    constexpr uintptr_t OFFSET_PAWN_Controller = 0x02E8;              // AController*

    // USceneComponent
    constexpr uintptr_t OFFSET_SCENE_RelativeLocation = 0x0148;       // FVector (0x18 bytes - double x3)
    constexpr uintptr_t OFFSET_SCENE_RelativeRotation = 0x0160;       // FRotator (0x18 bytes - double x3)
    constexpr uintptr_t OFFSET_SCENE_ComponentVelocity = 0x0190;      // FVector

    // AP2BaseCharacter (Size: 0x09F0)
    constexpr uintptr_t OFFSET_P2BASE_HealthComponent = 0x07E0;       // UP2HealthComponent*
    constexpr uintptr_t OFFSET_P2BASE_UpperBodyBoneName = 0x0814;     // FName

    // AP2HeroCharacter (Size: 0x0BC0)
    constexpr uintptr_t OFFSET_P2HERO_CameraComponent = 0x0A90;       // UP2CameraComponent*
    constexpr uintptr_t OFFSET_P2HERO_RemoteAimViewPitch = 0x0A70;    // float
    constexpr uintptr_t OFFSET_P2HERO_RemoteAimViewYaw = 0x0A74;      // float
    constexpr uintptr_t OFFSET_P2HERO_Mesh1P = 0x0AE8;                // USkeletalMeshComponent* (First Person Mesh)
    constexpr uintptr_t OFFSET_P2HERO_MeshFakeBody = 0x0AF0;          // USkeletalMeshComponent* (Fake Body for 3P)
    constexpr uintptr_t OFFSET_P2HERO_HeroComponent = 0x0AB0;         // UP2HeroComponent*
    constexpr uintptr_t OFFSET_P2HERO_EquipmentManager = 0x0A98;      // UP2EquipmentManagerComponent*

    // ABP_Hero_C (Size: 0x0BF0) - Final player character class
    constexpr uintptr_t OFFSET_BPHERO_WeaponSimulator = 0x0BE8;       // UWeaponSimulatorComponent*

    // APlayerController
    // Use ProjectWorldLocationToScreen function at 0x6d16558

    // ==================== Structures ====================

    struct FVector
    {
        double X;
        double Y;
        double Z;

        FVector() : X(0), Y(0), Z(0) {}
        FVector(double x, double y, double z) : X(x), Y(y), Z(z) {}

        FVector operator-(const FVector& other) const {
            return FVector(X - other.X, Y - other.Y, Z - other.Z);
        }

        FVector operator+(const FVector& other) const {
            return FVector(X + other.X, Y + other.Y, Z + other.Z);
        }

        double Length() const {
            return sqrt(X * X + Y * Y + Z * Z);
        }

        double Distance(const FVector& other) const {
            return (*this - other).Length();
        }
    };

    struct FVector2D
    {
        double X;
        double Y;
    };

    struct FRotator
    {
        double Pitch;
        double Yaw;
        double Roll;
    };

    // ==================== Bone Names ====================
    // Common UE5 skeleton bone names for aiming
    namespace Bones
    {
        constexpr const char* Head = "head";
        constexpr const char* Neck = "neck_01";
        constexpr const char* Spine = "spine_03";
        constexpr const char* Pelvis = "pelvis";
        constexpr const char* LeftHand = "hand_l";
        constexpr const char* RightHand = "hand_r";
        constexpr const char* LeftFoot = "foot_l";
        constexpr const char* RightFoot = "foot_r";
    }

    // ==================== Function Pointers ====================

    // GetSocketLocation - USceneComponent virtual function
    // Address: 0x68d4764
    using fn_GetSocketLocation = FVector(*)(void* SceneComponent, const char* SocketName);

    // GetBoneName - USkinnedMeshComponent
    // Address: 0x68db044
    using fn_GetBoneName = const char*(*)(void* SkinnedMeshComponent, int BoneIndex);

    // ProjectWorldLocationToScreen - APlayerController
    // Address: 0x6d16558
    using fn_ProjectWorldLocationToScreen = bool(*)(void* PlayerController, const FVector& WorldLocation, FVector2D* ScreenLocation, bool bPlayerViewportRelative);

    // GetAllActorsOfClass - UGameplayStatics
    // Address: 0x166d748
    using fn_GetAllActorsOfClass = void(*)(void* WorldContextObject, void* ActorClass, void* OutActors);

    // K2_GetActorLocation - AActor
    // Returns actor world location
    using fn_GetActorLocation = FVector(*)(void* Actor);

    // ==================== ESP Framework ====================

    class ESP
    {
    public:
        struct PlayerData
        {
            void* Actor;
            FVector WorldLocation;
            FVector2D ScreenLocation;
            float Distance;
            float Health;
            bool IsVisible;
            bool IsEnemy;
        };

        // Get all enemy players
        static std::vector<PlayerData> GetAllPlayers(void* World, void* LocalPlayer)
        {
            std::vector<PlayerData> players;
            // Implementation would iterate through UWorld actors
            // Filter by AP2HeroCharacter class
            return players;
        }

        // World to screen projection
        static bool WorldToScreen(void* PlayerController, const FVector& WorldPos, FVector2D& ScreenPos)
        {
            // Call APlayerController::ProjectWorldLocationToScreen
            // Function address: 0x6d16558
            return false;
        }

        // Get bone world location
        static FVector GetBoneLocation(void* SkeletalMesh, const char* BoneName)
        {
            // Call USceneComponent::GetSocketLocation
            // Function address: 0x68d4764
            return FVector();
        }
    };

    // ==================== Aimbot Framework ====================

    class Aimbot
    {
    public:
        enum class AimBone
        {
            Head,
            Neck,
            Chest,
            Pelvis
        };

        struct AimbotConfig
        {
            float FOV = 90.0f;           // Field of view for target selection
            float Smoothness = 5.0f;      // Aim smoothness (higher = slower)
            AimBone TargetBone = AimBone::Head;
            bool PredictMovement = true;
            float MaxDistance = 500.0f;   // Max aim distance in meters
        };

        // Calculate angle to target
        static FRotator CalcAngle(const FVector& CameraPos, const FVector& TargetPos)
        {
            FVector delta = TargetPos - CameraPos;

            FRotator result;
            result.Pitch = -atan2(delta.Z, sqrt(delta.X * delta.X + delta.Y * delta.Y)) * (180.0 / 3.14159265358979323846);
            result.Yaw = atan2(delta.Y, delta.X) * (180.0 / 3.14159265358979323846);
            result.Roll = 0;

            return result;
        }

        // Get closest target to crosshair
        static void* GetBestTarget(const std::vector<ESP::PlayerData>& Players,
                                   const FVector& CameraPos,
                                   const FRotator& CameraRot,
                                   const AimbotConfig& Config)
        {
            void* bestTarget = nullptr;
            float bestFOV = Config.FOV;

            for (const auto& player : Players)
            {
                if (!player.IsEnemy || player.Distance > Config.MaxDistance)
                    continue;

                FRotator angleToTarget = CalcAngle(CameraPos, player.WorldLocation);

                // Calculate FOV difference
                float fovDiff = sqrt(
                    pow(angleToTarget.Pitch - CameraRot.Pitch, 2) +
                    pow(angleToTarget.Yaw - CameraRot.Yaw, 2)
                );

                if (fovDiff < bestFOV)
                {
                    bestFOV = fovDiff;
                    bestTarget = player.Actor;
                }
            }

            return bestTarget;
        }

        // Smooth aim
        static FRotator SmoothAim(const FRotator& CurrentRot, const FRotator& TargetRot, float Smoothness)
        {
            FRotator result;
            result.Pitch = CurrentRot.Pitch + (TargetRot.Pitch - CurrentRot.Pitch) / Smoothness;
            result.Yaw = CurrentRot.Yaw + (TargetRot.Yaw - CurrentRot.Yaw) / Smoothness;
            result.Roll = 0;
            return result;
        }
    };

    // ==================== Memory Reading Helpers ====================

    template<typename T>
    T Read(uintptr_t Address)
    {
        if (Address == 0) return T();
        return *reinterpret_cast<T*>(Address);
    }

    template<typename T>
    void Write(uintptr_t Address, T Value)
    {
        if (Address == 0) return;
        *reinterpret_cast<T*>(Address) = Value;
    }

    // ==================== Example Usage ====================
    /*
    void MainLoop()
    {
        // 1. Get GWorld (global UWorld pointer)
        // 2. Get LocalPlayerController
        // 3. Get LocalPlayer Pawn

        // ESP Loop:
        // - Get all AP2HeroCharacter actors
        // - For each enemy:
        //   - Get Mesh3P or Mesh1P
        //   - Get bone location (head, etc)
        //   - Project to screen using ProjectWorldLocationToScreen
        //   - Draw ESP box/name/health

        // Aimbot Loop:
        // - Find best target within FOV
        // - Get target bone location
        // - Calculate aim angle
        // - Apply smoothing
        // - Set camera rotation
    }
    */
}
