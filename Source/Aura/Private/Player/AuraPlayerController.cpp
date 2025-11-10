// It 


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interfaces/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	check(AuraInputMappingContext);
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	
	if (EnhancedInputSubsystem)
	{
		EnhancedInputSubsystem->AddMappingContext(AuraInputMappingContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Type::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::MoveActionCallBack);
}

void AAuraPlayerController::MoveActionCallBack(const FInputActionValue& InputActionValue)
{
	const FVector2d InputAxisVector = InputActionValue.Get<FVector2d>();
	const FRotator ControlledRotation = GetControlRotation();
	const FRotator YawRotation(0.f, ControlledRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);
	if (!CursorHit.IsValidBlockingHit())
	{
		return;
	}

	LastCursorHitActor = CurrentCursorHitActor;
	CurrentCursorHitActor = Cast<IEnemyInterface>(CursorHit.GetActor());
	/**
	 * Line trace from cursor, There are Several scenarios:
	 * 1. LastCursorHitActor is nullptr && CurrentCursorHitActor is nullptr
	 *	  - Do nothing.
	 * 2. LastCursorHitActor is nullptr && CurrentCursorHitActor is valid
	 *    - Highlight CurrentCursorHitActor. 
	 * 3. LastCursorHitActor is valid && CurrentCursorHitActor is nullptr
	 *    - Unhighlight LastCursorHitActor.
	 * 4. Both actors are valid, but LastCursorHitActor != CurrentCursorHitActor
	 *    - Unhighlight LastCursorHitActor, and Highlight CurrentCursorHitActor.
	 * 5. Both actors are valid, and them are same actor
	 *    - Do nothing.
	 */
	if (LastCursorHitActor != CurrentCursorHitActor)
	{
		if (LastCursorHitActor != nullptr)
		{
			// need to be done in both Case 3. & 4. 
			LastCursorHitActor->UnHighlightActor();
		}
		if (CurrentCursorHitActor != nullptr)
		{
			// need to be done in both Case 2. & 4. 
			CurrentCursorHitActor->HighlightActor();
		}
	}
}
