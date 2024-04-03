// Copyright 2023 Evelyn Schwab under MIT license

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractiveActor.generated.h"

class USequentialInteractionComponent;

UCLASS(Abstract, Blueprintable)
class SEQUENTIALINTERACTIONS_API AInteractiveActor : public AActor
{
	GENERATED_BODY()

public:

	AInteractiveActor();

	// Interaction component for default interactive actor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	USequentialInteractionComponent* InteractionComponent;
};
