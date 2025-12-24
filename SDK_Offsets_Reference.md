# SDK Offsets Reference - 透视自瞄开发参考

## 类继承关系

```
ABP_Hero_C (0x0BF0)
    └── ABP_CharacterDefault_C (0x0BC0)
        └── AP2HeroCharacter (0x0BC0)
            └── AP2BaseCharacter (0x09F0)
                └── ACharacter (0x0658)
                    └── APawn (0x0338)
                        └── AActor (0x02B0)
                            └── UObject (0x0028)
```

## 核心偏移量

### AActor (0x02B0)
| 偏移量 | 类型 | 名称 | 说明 |
|--------|------|------|------|
| 0x01B8 | USceneComponent* | RootComponent | 根组件，包含位置信息 |
| 0x01A0 | APawn* | Instigator | - |

### APawn (0x0338)
| 偏移量 | 类型 | 名称 | 说明 |
|--------|------|------|------|
| 0x02BC | float | BaseEyeHeight | 眼睛高度 |
| 0x02C2 | uint8 | RemoteViewPitch | 远程视角俯仰 |
| 0x02D0 | APlayerState* | PlayerState | 玩家状态 |
| 0x02E8 | AController* | Controller | 控制器 |

### USceneComponent (0x02E0)
| 偏移量 | 类型 | 名称 | 说明 |
|--------|------|------|------|
| 0x0148 | FVector | RelativeLocation | 相对位置 (双精度XYZ) |
| 0x0160 | FRotator | RelativeRotation | 相对旋转 |
| 0x0178 | FVector | RelativeScale3D | 相对缩放 |
| 0x0190 | FVector | ComponentVelocity | 组件速度 |

### AP2BaseCharacter (0x09F0)
| 偏移量 | 类型 | 名称 | 说明 |
|--------|------|------|------|
| 0x07C8 | UP2HealthSet* | AttributeSetHealth | 血量属性 |
| 0x07E0 | UP2HealthComponent* | HealthComponent | 血量组件 |
| 0x07F0 | UP2TracePointComponent* | TracePointComponent | 追踪点组件 |
| 0x0814 | FName | UpperBodyBoneName | 上身骨骼名 |

### AP2HeroCharacter (0x0BC0)
| 偏移量 | 类型 | 名称 | 说明 |
|--------|------|------|------|
| 0x0A64 | float | StandEyeHeight | 站立眼睛高度 |
| 0x0A70 | float | RemoteAimViewPitch | 远程瞄准俯仰角 |
| 0x0A74 | float | RemoteAimViewYaw | 远程瞄准偏航角 |
| 0x0A90 | UP2CameraComponent* | CameraComponent | 相机组件 |
| 0x0A98 | UP2EquipmentManagerComponent* | EquipmentManagerComponent | 装备管理 |
| 0x0AB0 | UP2HeroComponent* | HeroComponent | 英雄组件 |
| 0x0AE8 | USkeletalMeshComponent* | Mesh1P | 第一人称骨骼网格 |
| 0x0AF0 | USkeletalMeshComponent* | MeshFakeBody | 假身体网格 |

### ABP_Hero_C (0x0BF0)
| 偏移量 | 类型 | 名称 | 说明 |
|--------|------|------|------|
| 0x0BC0 | UP2WeaponObstructionComponent* | P2WeaponObstruction | 武器阻挡组件 |
| 0x0BE8 | UWeaponSimulatorComponent* | WeaponSimulator | 武器模拟器 |

## 关键函数地址

| 函数 | 地址 | 说明 |
|------|------|------|
| USceneComponent::GetSocketLocation | 0x68d4764 | 获取Socket/骨骼位置 |
| USkinnedMeshComponent::GetBoneName | 0x68db044 | 获取骨骼名称 |
| APlayerController::ProjectWorldLocationToScreen | 0x6d16558 | 世界坐标转屏幕坐标 |
| UGameplayStatics::GetAllActorsOfClass | 0x166d748 | 获取所有指定类的Actor |
| UGameplayStatics::GetPlayerController | 0x17b678c | 获取玩家控制器 |
| AActor::K2_GetActorLocation | 0x164476c | 获取Actor位置 |
| AActor::K2_GetActorRotation | 0x163a084 | 获取Actor旋转 |
| APawn::GetControlRotation | 0x6cbefd4 | 获取控制旋转 |
| APawn::GetBaseAimRotation | 0x16666e8 | 获取基础瞄准旋转 |
| AP2BaseCharacter::GetMesh3P | 0x16099bc | 获取第三人称网格 |
| AP2BaseCharacter::GetMesh1P | 0x173f274 | 获取第一人称网格 |

## 透视实现步骤

1. **获取GWorld指针**
   - 通过模式扫描或静态地址获取UWorld*

2. **获取本地玩家控制器**
   - 使用 `UGameplayStatics::GetPlayerController(World, 0)`

3. **获取所有角色**
   - 使用 `UGameplayStatics::GetAllActorsOfClass` 过滤 AP2HeroCharacter

4. **遍历每个角色**
   ```cpp
   for (auto Actor : AllCharacters) {
       // 跳过自己
       if (Actor == LocalPawn) continue;

       // 获取骨骼网格
       auto Mesh3P = Actor->GetMesh3P();  // 0x16099bc

## 魔法子弹 (Magic Bullet) 相关

### UProjectileMovementComponent (Engine_classes.hpp)
| 偏移量 | 类型 | 名称 | 说明 |
|--------|------|------|------|
| 0x0130 | uint8 | bIsHomingProjectile | 是否为追踪弹 (Bit 7) |
| 0x0188 | float | HomingAccelerationMagnitude | 追踪加速度大小 |
| 0x018C | TWeakObjectPtr<USceneComponent> | HomingTargetComponent | 追踪目标组件 |

### AP2Projectile (Game_classes.hpp)
| 偏移量 | 类型 | 名称 | 说明 |
|--------|------|------|------|
| 0x02B0 | UProjectileMovementComponent* | ProjectileMovement | 投射物移动组件 |

### 实现思路
1. 遍历所有 `AP2Projectile` 类型的 Actor。
2. 获取其 `ProjectileMovement` 组件。
3. 设置 `bIsHomingProjectile` 为 true (Bit 7)。
4. 设置 `HomingAccelerationMagnitude` 为极大值 (如 99999.0f)。
5. 设置 `HomingTargetComponent` 为目标角色的 `RootComponent` 或 `Mesh`。

### 关键函数 (Internal)
| 函数 | 地址 | 说明 |
|------|------|------|
| UProjectileMovementComponent::SetInterpolatedComponent | 0x6a159a4 | 设置追踪目标组件 (自动处理 WeakPtr) |



       // 获取头部位置
       FVector HeadPos = Mesh3P->GetSocketLocation("head");  // 0x68d4764

       // 世界坐标转屏幕
       FVector2D ScreenPos;
       if (PlayerController->ProjectWorldLocationToScreen(HeadPos, &ScreenPos, true)) {
           // 绘制ESP框/名称/血条
       }
   }
   ```

## 自瞄实现步骤

1. **获取相机位置和旋转**
   - 从 `CameraComponent` (偏移 0x0A90) 获取

2. **找到最佳目标**
   - 遍历敌人，计算FOV内最近的目标

3. **计算瞄准角度**
   ```cpp
   FRotator CalcAngle(FVector CameraPos, FVector TargetPos) {
       FVector Delta = TargetPos - CameraPos;
       FRotator Angle;
       Angle.Pitch = -atan2(Delta.Z, sqrt(Delta.X*Delta.X + Delta.Y*Delta.Y)) * (180/PI);
       Angle.Yaw = atan2(Delta.Y, Delta.X) * (180/PI);
       return Angle;
   }
   ```

4. **平滑瞄准**
   ```cpp
   FRotator SmoothAim(FRotator Current, FRotator Target, float Smooth) {
       return {
           Current.Pitch + (Target.Pitch - Current.Pitch) / Smooth,
           Current.Yaw + (Target.Yaw - Current.Yaw) / Smooth,
           0
       };
   }
   ```

5. **设置视角旋转**
   - 通过 PlayerController 或直接修改相机旋转

## 常用骨骼名称

| 名称 | 说明 |
|------|------|
| head | 头部 |
| neck_01 | 颈部 |
| spine_03 | 上脊椎 |
| spine_02 | 中脊椎 |
| spine_01 | 下脊椎 |
| pelvis | 骨盆 |
| hand_l / hand_r | 左/右手 |
| foot_l / foot_r | 左/右脚 |
| upperarm_l / upperarm_r | 左/右上臂 |
| lowerarm_l / lowerarm_r | 左/右前臂 |

## 数据类型

### FVector (UE5 使用双精度)
```cpp
struct FVector {
    double X;  // 0x00
    double Y;  // 0x08
    double Z;  // 0x10
};  // Size: 0x18
```

### FRotator (UE5 使用双精度)
```cpp
struct FRotator {
    double Pitch;  // 0x00
    double Yaw;    // 0x08
    double Roll;   // 0x10
};  // Size: 0x18
```

### FVector2D
```cpp
struct FVector2D {
    double X;  // 0x00
    double Y;  // 0x08
};  // Size: 0x10
```

## 注意事项

1. UE5 中 FVector/FRotator 使用 **double** 而非 float
2. 偏移量可能随游戏版本更新而变化
3. 某些字段可能有服务端验证
4. 建议使用外部overlay进行绘制
