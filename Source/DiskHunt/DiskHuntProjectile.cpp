// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DiskHuntProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include <Target.h>

ADiskHuntProjectile::ADiskHuntProjectile() 
{
	// Replication
	bReplicates = true;
	bReplicateMovement = true;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ADiskHuntProjectile::OnHit);		// set up a notification for when this component hits something blocking
	CollisionComp->SetIsReplicated(true);

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void ADiskHuntProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		MulticastParticlesAndSound(OtherComp, NormalImpulse, Hit);
		// If this is game target, color it, mark as already scored, assign score and enable physic so it can be tripped by next if
		if (ATarget* target = Cast<ATarget>(OtherActor))
		{
			if (!Instigator)
			{
				UE_LOG(LogTemp, Error, TEXT("PROJECTILE HAS NO INSTIGATOR!"));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "PROJECTILE HAS NO INSTIGATOR!");
				return;
			}
			if (ADiskHuntCharacter* character = Cast<ADiskHuntCharacter>(Instigator))
			{
				UE_LOG(LogTemp, Log, TEXT("Calling Register Hit"));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Calling Register Hit");
				target->RegisterHit(character);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("INSTIGATOR IS NOT ADiskHuntCharacter!"));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "INSTIGATOR IS NOT ADiskHuntCharacter!");
			}
		}
		if (OtherComp->IsSimulatingPhysics())
		{
			OtherComp->AddImpulseAtLocation(GetVelocity() * 50.0f, GetActorLocation());
		}
		Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("!(OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "!(OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)");
	}

	
}

void ADiskHuntProjectile::MulticastParticlesAndSound_Implementation(UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UGameplayStatics::SpawnEmitterAttached(ParticleTemplate, OtherComp, NAME_None, Hit.Location, NormalImpulse.Rotation(), EAttachLocation::KeepWorldPosition, false);
	UGameplayStatics::PlaySoundAtLocation(this, SoundTemplate, Hit.Location, NormalImpulse.Rotation());
}
