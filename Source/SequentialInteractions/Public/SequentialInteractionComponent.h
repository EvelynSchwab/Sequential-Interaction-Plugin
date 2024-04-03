// Copyright 2023 Evelyn Schwab under MIT license

#pragma once

#include "CoreMinimal.h"
#include "Interaction.h"
#include "SequentialInteractionComponent.generated.h"

// Possible states for a sequential interaction to be in
UENUM(BlueprintType, Category = "Interaction")
enum EInteractionState
{
	SequentialState_Idle	UMETA(DisplayName = "Idle", Tooltip = "No interaction is currently occuring"),
	SequentialState_InProgress	UMETA(DisplayName = "In Progress", Tooltip = "An interaction is currently occuring"),
	SequentialState_Waiting		UMETA(DisplayName = "Waiting", Tooltip = "Waiting for a new interaction to start"),
	SequentialState_Failed		UMETA(DisplayName = "Failed", Tooltip = "A condition check failed, or something went wrong during the interaction sequence")
};

/*
 * Data for available interactions
 */

USTRUCT(BlueprintType, Category = "Interaction")
struct FSequentialInteraction
{
	GENERATED_BODY()

	FSequentialInteraction()
	{
		SequentialInteraction = nullptr;
		bInteractionComplete = false;
		InteractionDebugName = "Unnamed Interaction";
	}
	
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category = "Interaction", AdvancedDisplay, meta = (DisplayPriority = 0))
    FString InteractionDebugName;
	
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ShowOnlyInnerProperties), Instanced, meta = (DisplayPriority = 1))
	UInteraction* SequentialInteraction;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (DisplayPriority = 2))
	bool bResetInteractionsOnConditionsFail;
	
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	bool bInteractionComplete;

};

/*
 * Component to manage sequential interactions on actors
 * Having this component marks an actor as interactive
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SEQUENTIALINTERACTIONS_API USequentialInteractionComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	

	USequentialInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interaction", meta = (ShowOnlyInnerProperties, TitleProperty = "{InteractionDebugName}"))
	TArray<FSequentialInteraction> SequentialInteractions;

	// Start the sequential interactions
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StartSequentialInteractions(AActor* InteractingActor);

	// Attempt to cancel the current interaction
	// Will require specific implementation on the currently active interaction to work
	// TODO::Implement this
	/*UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool CancelSequentialInteractions();*/

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	int32 CurrentSequentialInteractionIndex;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasInteractionBeenCompleted(int32 InteractionIndex);

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	AActor* CurrentlyInteractingActor;
	
	// Bound function for an individual sequential interaction ending
    UFUNCTION()
    void OnInteractionEnded(bool bCompletedSuccessfully);
    // Bound function for an individual sequential interaction cancelling
    UFUNCTION()
    void OnInteractionCancelled(TEnumAsByte<EInteractionCancelReason> CancelReason);

	// Pointer to the active interaction
	// This will be a duplicate of of the interaction at the current index of the interaction array
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	UInteraction* ActiveInteractionInstance;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	TEnumAsByte<EInteractionState> GetCurrentInteractionState() const { return CurrentInteractionState; };

protected:
	
	/* Debug */
	// Show debug information in runtime
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction",
		meta = (DisplayName = "Show Debug Information"), AdvancedDisplay)
	bool bShowDebugInformation;

	// Colour for debug text
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction",
		meta = (DisplayName = "Debug Text Colour", EditCondition = "bShowDebugInformation"), AdvancedDisplay)
	FColor DebugTextColour;

	// Size for debug text
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction",
		meta = (DisplayName = "Debug Text Size", UIMin = 1.0f, UIMax = 10.0f,
			EditCondition = "bShowDebugInformation"), AdvancedDisplay)
	float DebugTextSize;
	
private:
	// Start the next sequential interaction
	UFUNCTION(Category = "Interaction")
	void StartNextSequentialInteraction();
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
    void EndSequentialInteractions();
	
	EInteractionState CurrentInteractionState;

	// Gets the location at the top of the owner's bounds for debug text
	FVector GetDebugTextBaseDrawLocation() const;
};
