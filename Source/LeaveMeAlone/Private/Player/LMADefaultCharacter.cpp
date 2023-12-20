// LeaveMeAlone Game by Netologiya. All RightsReserved.

#include "Player/LMADefaultCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/LMAHealthComponent.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/LMAWeaponComponent.h"

// Sets default values
ALMADefaultCharacter::ALMADefaultCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(GetRootComponent());

	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->TargetArmLength = ArmLength;
	SpringArmComponent->SetRelativeRotation(FRotator(YRotation, 0.0f, 0.0f));
	SpringArmComponent->bDoCollisionTest = false;
	SpringArmComponent->bEnableCameraLag = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetFieldOfView(FOV);
	CameraComponent->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<ULMAHealthComponent>("HealthComponent");

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	WeaponComponent = CreateDefaultSubobject<ULMAWeaponComponent>("Weapon");
}

// Called when the game starts or when spawned
void ALMADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnDeath.AddUObject(this, &ALMADefaultCharacter::OnDeath);
	if (CursorMaterial)
	{
		CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
	}
	
	
}

// Called every frame
void ALMADefaultCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!(HealthComponent->IsDead()))
	{
		RotationPlayerOnCursor();
	}
	
}

// Called to bind functionality to input
void ALMADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ALMADefaultCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ALMADefaultCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Scroll", this, &ALMADefaultCharacter::Scroll);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ALMADefaultCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ALMADefaultCharacter::StopSprint);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, WeaponComponent, &ULMAWeaponComponent::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, WeaponComponent, &ULMAWeaponComponent::Stop_Fire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, WeaponComponent, &ULMAWeaponComponent::Reload);

}

void ALMADefaultCharacter::MoveForward(float Value)
{
	if (Value > 0)
	{
		PermitSprint = true;
	}
	
	AddMovementInput(GetActorForwardVector(), Value);
}

void ALMADefaultCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void ALMADefaultCharacter::Scroll(float Value)
{
	if ((Value < 0 && SpringArmComponent->TargetArmLength < MaxLeng) 
		|| (Value > 0 &&SpringArmComponent->TargetArmLength > MinLeng ))
		SpringArmComponent->TargetArmLength = SpringArmComponent->TargetArmLength - (DeltaLeng * Value);
}

void ALMADefaultCharacter::OnDeath()
{
	CurrentCursor->DestroyRenderState_Concurrent();
	PlayAnimMontage(DeathMontage);
	GetCharacterMovement()->DisableMovement();
	SetLifeSpan(5.0f);
	if (Controller)
	{
		Controller->ChangeState(NAME_Spectating);
	}
}
void ALMADefaultCharacter::RotationPlayerOnCursor()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		FHitResult ResultHit;
		PC->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);
		float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
		SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));
		if (CurrentCursor)
		{
			CurrentCursor->SetWorldLocation(ResultHit.Location);
		}
	}
}



bool ULMAHealthComponent::IsHealthFull() const
{
	return FMath::IsNearlyEqual(Health, MaxHealth);
}

void ALMADefaultCharacter::StartSprint()
{
	
	if (!bIsSprinting && CurrentStamina > 0&&PermitSprint==true)
	{
		bIsSprinting = true;

		
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}

	
	if (GetWorldTimerManager().TimerExists(StaminaRechargeHandle))
	{
		GetWorldTimerManager().ClearTimer(StaminaRechargeHandle);
	}

	
	if (!GetWorldTimerManager().TimerExists(StaminaDrainHandle))
	{
		GetWorldTimerManager().SetTimer(StaminaDrainHandle, this, &ALMADefaultCharacter::DrainStamina, StaminaDrainFrequency, true);
	}

	
	
}

void ALMADefaultCharacter::StopSprint() 
{
	if (bIsSprinting)
	{
		bIsSprinting = false;
		PermitSprint = false;
		
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	
	if (GetWorldTimerManager().TimerExists(StaminaDrainHandle))
	{
		GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
	}

	
	if (!GetWorldTimerManager().TimerExists(StaminaRechargeHandle))
	{
		GetWorldTimerManager().SetTimer(StaminaRechargeHandle, this, &ALMADefaultCharacter::RechargeStamina, StaminaRechargeFrequency, true);
	}

}
void ALMADefaultCharacter::DrainStamina()
{
	
	if (bIsSprinting && CurrentStamina > 0)
	{
		
		CurrentStamina = FMath::Max(CurrentStamina + StaminaDrainAmount, 0);
	}

	
	if (CurrentStamina <= 0 && bIsSprinting)
	{
		GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
		StopSprint();
	}
	
}


void ALMADefaultCharacter::RechargeStamina()
{
	
	if (!bIsSprinting && CurrentStamina < MaxStamina)
	{
		
		CurrentStamina = FMath::Min(CurrentStamina + StaminaRechargeAmount, MaxStamina);
	}

	
	if (CurrentStamina >= MaxStamina)
	{
		GetWorldTimerManager().ClearTimer(StaminaRechargeHandle);
	}
}