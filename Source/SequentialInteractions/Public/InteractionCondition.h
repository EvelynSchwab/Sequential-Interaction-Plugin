// Copyright 2023 Evelyn Schwab under MIT license
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InteractionCondition.generated.h"

/**
 * Conditions for interactions
 * Used to check if an interaction can be completed
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew)
class SEQUENTIALINTERACTIONS_API UInteractionCondition : public UObject
{
	GENERATED_BODY()

public:

	UInteractionCondition();
	
	// Main function for checking interaction conditions
	// Returns false by default, should be fully overridden in all child classes
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Condition")
	bool CheckInteractionConditions(AActor* InteractingActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Condition")
	bool InvertCondition;
	
};
