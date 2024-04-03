// Copyright 2023 Evelyn Schwab under MIT license

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InteractionFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SEQUENTIALINTERACTIONS_API UInteractionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static bool TryStartInteraction(AActor* InteractiveActor, AActor* InteractingActor);
};
