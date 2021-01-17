// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DiskHuntCharacter.h"
#include "DiskHuntProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Interactable.h"
#include "DrawDebugHelpers.h"
#include <Runtime/Engine/Public/Net/UnrealNetwork.h>

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ADiskHuntCharacter


ADiskHuntCharacter::ADiskHuntCharacter()
{
	// Replication
	bReplicates = true;
	bReplicateMovement = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	// Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	ExternalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExternalMesh"));
	ExternalMesh->SetupAttachment(GetCapsuleComponent());
	ExternalMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ExternalMesh->SetOwnerNoSee(true);
	
	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetIsReplicated(true);
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void ADiskHuntCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ADiskHuntCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ADiskHuntCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ADiskHuntCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ADiskHuntCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADiskHuntCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ADiskHuntCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ADiskHuntCharacter::LookUpAtRate);

	// Bind fire event
	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &ADiskHuntCharacter::ServerOnUse);
}

void ADiskHuntCharacter::OnRep_PlayerMaterial()
{
	UE_LOG(LogTemp, Log, TEXT("OnRep_PlayerMaterial"));
	Mesh1P->SetMaterial(0, PlayerMaterial);
	ExternalMesh->SetMaterial(0, PlayerMaterial);
}

bool ADiskHuntCharacter::CanShoot()
{
	return Ammo > 0;
}

void ADiskHuntCharacter::OnFire()
{
	if (CanShoot())
	{
		ServerFire();	// real shoot on server
	}
	
}

void ADiskHuntCharacter::MulticastFire_Implementation()
{
	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "FireAnim_Implementation should have played sound");
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "FireAnim_Implementation should have played montage");
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "AnimInstance == NULL");
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "FireAnimation == NULL");
	}
}

void ADiskHuntCharacter::ServerFire_Implementation()
{
	if (!CanShoot())
	{
		return; // maybe diffrent anim and no sound? 
	}

	// try and fire a projectile
	if (ProjectileClass == NULL)
	{
		//print
		return;
	}

	UWorld* const World = GetWorld();
	if (World != NULL && HasAuthority())
	{
		if (bUsingMotionControllers)
		{
			const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
			const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
			ADiskHuntProjectile* projectile = World->SpawnActor<ADiskHuntProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			if (projectile)
			{
				projectile->Instigator = this;
				Ammo--;
				MulticastFire(); // multicast animation and sound
			}
		}
		else
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			ADiskHuntProjectile* projectile = World->SpawnActor<ADiskHuntProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			if (projectile)
			{
				projectile->Instigator = this;
				Ammo--;
				MulticastFire(); // multicast animation and sound
			}
		}
	}

}

void ADiskHuntCharacter::ServerOnUse_Implementation()
{
	if (GetInteractable() && GetInteractable()->CanBeUsed())
	{
		GetInteractable()->OnUse(this);
	}
}

void ADiskHuntCharacter::SetMaterial_Implementation(UMaterialInstance* material)
{
	PlayerMaterial = material;
	Mesh1P->SetMaterial(0, material);
	ExternalMesh->SetMaterial(0, material);
}

void ADiskHuntCharacter::IncreaseAmmo(int32 ammoCount)
{
	Ammo = FMath::Clamp(Ammo + ammoCount, 0, MaxAmmo);
}

void ADiskHuntCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ADiskHuntCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ADiskHuntCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

AInteractable* ADiskHuntCharacter::UpdateInteractable()
{
	FHitResult hitResult;
	const auto* camera = GetFirstPersonCameraComponent();
	FVector start = camera->GetComponentLocation();
	FVector end = start + camera->GetForwardVector() * 600.0f;
	
	// Debug line for raytracing range
	//DrawDebugLine(GetWorld(), start, end, FColor(255, 0, 0), false, .0f, 0, 1.f);

	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel2)); // channel made for Interactable

	if (GetWorld()->LineTraceSingleByObjectType(hitResult, start, end, traceObjectTypes))
	{
		AActor* actor = hitResult.GetActor();
		AInteractable* ret = Cast<AInteractable>(actor);
		return ret;
	}
	return nullptr;
}

AInteractable* ADiskHuntCharacter::GetInteractable() const
{
	return InteractableItem;
}

void ADiskHuntCharacter::Tick(float DeltaTime)
{
	InteractableItem = UpdateInteractable();
		if (HasAuthority())
	{
		ReplicateRotation(GetControlRotation());
	}
}

void ADiskHuntCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to every client, no special condition required
	DOREPLIFETIME(ADiskHuntCharacter, Ammo);
	DOREPLIFETIME(ADiskHuntCharacter, PlayerMaterial);
}

void ADiskHuntCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ADiskHuntCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ADiskHuntCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ADiskHuntCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void ADiskHuntCharacter::ReplicateRotation_Implementation(FRotator rotation)
{
	FirstPersonCameraComponent->SetWorldRotation(rotation);
}


bool ADiskHuntCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ADiskHuntCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ADiskHuntCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ADiskHuntCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

