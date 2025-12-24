#pragma once

/*
 * Bone/Skeleton Helper for ESP
 * Based on USkeletalMeshComponent analysis
 */

#include "GameHack.hpp"

namespace GameHack
{
    //=============================================================================
    // Common Bone IDs for humanoid characters
    //=============================================================================
    enum class EBoneID : int
    {
        Root = 0,
        Pelvis = 1,
        Spine_01 = 2,
        Spine_02 = 3,
        Spine_03 = 4,
        Neck = 5,
        Head = 6,

        // Left Arm
        Clavicle_L = 7,
        UpperArm_L = 8,
        LowerArm_L = 9,
        Hand_L = 10,

        // Right Arm
        Clavicle_R = 11,
        UpperArm_R = 12,
        LowerArm_R = 13,
        Hand_R = 14,

        // Left Leg
        Thigh_L = 15,
        Calf_L = 16,
        Foot_L = 17,

        // Right Leg
        Thigh_R = 18,
        Calf_R = 19,
        Foot_R = 20,

        Max = 21
    };

    // Bone connection pairs for drawing skeleton
    struct BoneConnection
    {
        EBoneID from;
        EBoneID to;
    };

    // Standard humanoid skeleton connections
    constexpr BoneConnection SkeletonConnections[] = {
        // Spine
        { EBoneID::Pelvis, EBoneID::Spine_01 },
        { EBoneID::Spine_01, EBoneID::Spine_02 },
        { EBoneID::Spine_02, EBoneID::Spine_03 },
        { EBoneID::Spine_03, EBoneID::Neck },
        { EBoneID::Neck, EBoneID::Head },

        // Left Arm
        { EBoneID::Spine_03, EBoneID::Clavicle_L },
        { EBoneID::Clavicle_L, EBoneID::UpperArm_L },
        { EBoneID::UpperArm_L, EBoneID::LowerArm_L },
        { EBoneID::LowerArm_L, EBoneID::Hand_L },

        // Right Arm
        { EBoneID::Spine_03, EBoneID::Clavicle_R },
        { EBoneID::Clavicle_R, EBoneID::UpperArm_R },
        { EBoneID::UpperArm_R, EBoneID::LowerArm_R },
        { EBoneID::LowerArm_R, EBoneID::Hand_R },

        // Left Leg
        { EBoneID::Pelvis, EBoneID::Thigh_L },
        { EBoneID::Thigh_L, EBoneID::Calf_L },
        { EBoneID::Calf_L, EBoneID::Foot_L },

        // Right Leg
        { EBoneID::Pelvis, EBoneID::Thigh_R },
        { EBoneID::Thigh_R, EBoneID::Calf_R },
        { EBoneID::Calf_R, EBoneID::Foot_R },
    };

    constexpr int SkeletonConnectionCount = sizeof(SkeletonConnections) / sizeof(BoneConnection);

    //=============================================================================
    // USkeletalMeshComponent Wrapper
    //=============================================================================
    class USkeletalMeshComponent
    {
    private:
        uintptr_t address;

        // Offsets within USkeletalMeshComponent
        // These may vary - need to verify from SDK
        static constexpr uintptr_t BoneSpaceTransforms_Offset = 0x0570;
        static constexpr uintptr_t ComponentToWorld_Offset = 0x01D0;

    public:
        USkeletalMeshComponent(uintptr_t addr) : address(addr) {}

        bool IsValid() const { return address != 0; }

        // Get component transform matrix
        FMatrix GetComponentToWorld() const {
            if (!IsValid()) return FMatrix();
            return Memory::Read<FMatrix>(address + ComponentToWorld_Offset);
        }

        // Get bone count
        int GetBoneCount() const {
            if (!IsValid()) return 0;
            // TArray<FTransform> structure: Data pointer at offset, Count at offset+8
            return Memory::Read<int>(address + BoneSpaceTransforms_Offset + 0x08);
        }

        // Get bone transform in component space
        // FTransform is typically 0x60 bytes (with scale, rotation, translation)
        struct FTransform {
            float Rotation[4];    // Quaternion (X, Y, Z, W)
            FVector Translation;  // Position
            float Pad1;
            FVector Scale3D;      // Scale
            float Pad2;
        };

        FTransform GetBoneTransform(int boneIndex) const {
            FTransform result = {};
            if (!IsValid()) return result;

            int boneCount = GetBoneCount();
            if (boneIndex < 0 || boneIndex >= boneCount) return result;

            uintptr_t transformArray = Memory::Read<uintptr_t>(address + BoneSpaceTransforms_Offset);
            if (!transformArray) return result;

            // FTransform is 0x60 bytes (96 bytes)
            return Memory::Read<FTransform>(transformArray + boneIndex * 0x60);
        }

        // Get bone position in world space
        FVector GetBoneWorldPosition(int boneIndex) const {
            if (!IsValid()) return FVector();

            FTransform boneTransform = GetBoneTransform(boneIndex);
            FMatrix compToWorld = GetComponentToWorld();

            // Transform bone position to world space
            // Simplified - actual implementation needs full matrix multiplication
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

        // Alternative: Use GetBoneMatrix which returns FMatrix directly
        // USkeletalMeshComponent has GetBoneMatrix(int32 BoneIndex) function
        FMatrix GetBoneMatrix(int boneIndex) const {
            if (!IsValid()) return FMatrix();

            // This requires calling the actual game function via ProcessEvent
            // For now, we construct it from transform data
            FTransform boneTransform = GetBoneTransform(boneIndex);
            FMatrix compToWorld = GetComponentToWorld();

            // Construct bone matrix (simplified)
            FMatrix result;
            // ... matrix construction from quaternion and translation
            // This is a simplified version

            return result;
        }
    };

    //=============================================================================
    // Skeleton ESP Helper
    //=============================================================================
    struct SkeletonScreenData
    {
        FVector bonePositions[(int)EBoneID::Max];
        FVector screenPositions[(int)EBoneID::Max];
        bool boneVisible[(int)EBoneID::Max];
        bool hasValidBones;
    };

    class SkeletonESP
    {
    private:
        FMatrix viewMatrix;
        int screenWidth;
        int screenHeight;

    public:
        SkeletonESP() : screenWidth(1920), screenHeight(1080) {}

        void SetViewMatrix(const FMatrix& matrix) { viewMatrix = matrix; }
        void SetScreenSize(int w, int h) { screenWidth = w; screenHeight = h; }

        // Get all bone positions for a character
        SkeletonScreenData GetSkeletonData(USkeletalMeshComponent& mesh, const int* boneMapping) {
            SkeletonScreenData data = {};
            data.hasValidBones = false;

            if (!mesh.IsValid()) return data;

            bool anyValid = false;
            for (int i = 0; i < (int)EBoneID::Max; i++) {
                int actualBoneIndex = boneMapping ? boneMapping[i] : i;

                data.bonePositions[i] = mesh.GetBoneWorldPosition(actualBoneIndex);
                data.boneVisible[i] = WorldToScreen(
                    data.bonePositions[i],
                    viewMatrix,
                    screenWidth,
                    screenHeight,
                    data.screenPositions[i]
                );

                if (data.boneVisible[i]) anyValid = true;
            }

            data.hasValidBones = anyValid;
            return data;
        }

        // Draw skeleton lines (pseudo-code for your rendering system)
        void DrawSkeleton(const SkeletonScreenData& data, void* renderContext) {
            if (!data.hasValidBones) return;

            for (int i = 0; i < SkeletonConnectionCount; i++) {
                int from = (int)SkeletonConnections[i].from;
                int to = (int)SkeletonConnections[i].to;

                if (!data.boneVisible[from] || !data.boneVisible[to]) continue;

                // Draw line from data.screenPositions[from] to data.screenPositions[to]
                // Implementation depends on your rendering library (DirectX, OpenGL, ImGui, etc.)
                // Example:
                // DrawLine(renderContext,
                //          data.screenPositions[from].X, data.screenPositions[from].Y,
                //          data.screenPositions[to].X, data.screenPositions[to].Y,
                //          COLOR_WHITE);
            }
        }
    };

    //=============================================================================
    // Bone Name to Index Mapping
    // You need to find actual bone indices for this specific game
    //=============================================================================
    namespace BoneNames
    {
        // Common UE4/UE5 bone names
        constexpr const char* Head = "head";
        constexpr const char* Neck = "neck_01";
        constexpr const char* Spine03 = "spine_03";
        constexpr const char* Spine02 = "spine_02";
        constexpr const char* Spine01 = "spine_01";
        constexpr const char* Pelvis = "pelvis";

        constexpr const char* ClavicleL = "clavicle_l";
        constexpr const char* UpperArmL = "upperarm_l";
        constexpr const char* LowerArmL = "lowerarm_l";
        constexpr const char* HandL = "hand_l";

        constexpr const char* ClavicleR = "clavicle_r";
        constexpr const char* UpperArmR = "upperarm_r";
        constexpr const char* LowerArmR = "lowerarm_r";
        constexpr const char* HandR = "hand_r";

        constexpr const char* ThighL = "thigh_l";
        constexpr const char* CalfL = "calf_l";
        constexpr const char* FootL = "foot_l";

        constexpr const char* ThighR = "thigh_r";
        constexpr const char* CalfR = "calf_r";
        constexpr const char* FootR = "foot_r";
    }

    // Function to find bone index by name
    // This requires iterating through the skeleton's bone array
    int FindBoneIndexByName(USkeletalMeshComponent& mesh, const char* boneName) {
        // Implementation would need to:
        // 1. Get the SkeletalMesh from the component
        // 2. Access the RefSkeleton
        // 3. Iterate through bone names to find matching index

        // Placeholder - you need to implement based on actual memory layout
        return -1;
    }

} // namespace GameHack
