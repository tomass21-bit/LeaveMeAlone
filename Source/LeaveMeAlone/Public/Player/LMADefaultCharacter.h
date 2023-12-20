// LeaveMeAlone Game by Netologiya. All RightsReserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LMADefaultCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class ULMAHealthComponent;
class ULMAWeaponComponent; 
	
	UCLASS()
class LEAVEMEALONE_API ALMADefaultCharacter : public ACharacter
{
	GENERATED_BODY()


public:
	// Sets default values for this character's properties
	ALMADefaultCharacter();
	
	UFUNCTION()
	ULMAHealthComponent* GetHealthComponent() const { return HealthComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* CameraComponent;
	
	UPROPERTY()
	UDecalComponent* CurrentCursor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* CursorMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FVector CursorSize = FVector(20.0f, 40.0f, 40.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringArm")
	float DeltaLeng = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringArm")
	float MinLeng = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringArm")
	float MaxLeng = 1000.0f;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void Scroll(float Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components|Health")
	ULMAHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* DeathMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	ULMAWeaponComponent* WeaponComponent;
	
	void StartSprint();
	void StopSprint();
	void DrainStamina();
	void RechargeStamina();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	float YRotation = -75.0f;

	float ArmLength = 1400.0f;

	float FOV = 55.0f;
	bool bIsSprinting = false;
	bool PermitSprint = false;
	float MaxStamina = 100.f;
	UPROPERTY(VisibleAnywhere)
	float CurrentStamina = MaxStamina;
	float StaminaDrainAmount = -10.f;
	float StaminaRechargeAmount = 10.f;
	float StaminaDrainFrequency = .3f;
	float StaminaRechargeFrequency = .3f;
	FTimerHandle StaminaDrainHandle;
	FTimerHandle StaminaRechargeHandle;
	float WalkSpeed = 300.0f;
	float SprintSpeed = 700.0f;
	void MoveForward(float Value);
	void MoveRight(float Value);
	
	
	
	void OnDeath();
	void RotationPlayerOnCursor();
	
};
