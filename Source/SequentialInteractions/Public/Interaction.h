// Copyright 2023 Evelyn Schwab under MIT license

#pragma once

#include "CoreMinimal.h"
#include "Interaction.generated.h"

class UInteractionCondition;

// Enum representing the reason for failing an interaction
UENUM(BlueprintType, Category = "Interaction")
enum EInteractionCancelReason
{
	Cancel_Cancelled			UMETA(DisplayName = "Cancelled"),
	Cancel_ConditionsNotMet		UMETA(Displayname = "Conditions Not Met"),
	Cancel_Failed				UMETA(DisplayName = "Failed")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionEnded, bool, bCompletdSuccessfully);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionFailed, TEnumAsByte<EInteractionCancelReason>, CancelReason);

/**
 * Parent object for all interactions
 * Subclass to add a new interaction type
 *
 * - Try activate the interaction
 * - If the interaction can activate, event InteractionActivated() fires
 * - From BP, try commit the interaction
 * - If the interaction can be committed, event InteractionCommitted() fires
 * - The interaction should be ended manually via BP after it has been committed.
 *		EndInteraction() is not called automatically.
 *
 * - If any checks fail, event InteractionCancelled is triggered. Any additional checks should also trigger
 *   CancelInteraction(InteractionCancelReason).
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, CollapseCategories)
class SEQUENTIALINTERACTIONS_API UInteraction : public UObject
{
	GENERATED_BODY()

public:
	UInteraction();
	
	// Triggers when an interaction is successfully activated
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void InteractionActivated();

	// Triggers when an interaction is successfully committed
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void InteractionCommitted();

	/* Blueprint event for interaction cancellation. Triggers when the interaction is cancelled by CancelInteraction(),
	or when a condition check fails.*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void InteractionCancelled(const TEnumAsByte<EInteractionCancelReason>& CancelReason);

	// Blueprint event for interaction ending. Triggers when the interaction is ended by EndInteraction()
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void InteractionEnded();

	/* Blueprint bindable function for the end of an interaction
	 * Called by EndInteraction(). */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionEnded OnInteractionEnded;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionFailed OnInteractionCancelled;
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelInteraction(TEnumAsByte<EInteractionCancelReason> CancelReason);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CommitInteraction(bool bBypassRequirements = false);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool CanCommitInteraction();

	/* End an interaction
	 * This will trigger the blueprint event InteractionEnded, set the interaction to inactive
	 * and broadcast OnInteractionEnded to the outer interaction component */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void EndInteraction();
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInteractionActive() const { return bIsActive; }

	UPROPERTY(EditAnywhere, Category = "Interaction", Instanced,
		meta = (ShowOnlyInnerProperties))
	TArray<UInteractionCondition*> Conditions;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetInteractingActor() const { return InteractingActor; }
	
	void TryActivateInteraction(AActor* ActivatingActor);

	// Can this interaction be triggered more than once
	// This value can be changed during runtime
	// This does not get saved, so the interaction repeat is per-session.
	// If you want to only have this interaction occur once, use a condition that relies on a saved value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanRepeatInteraction;

	// Should the next interaction in the sequence start automatically
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bStartNextInteractionAutomatically;

	// Returns true if all the conditions for this interaction are met
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool AreInteractionConditionsMet();
	
protected:
	
	// Get the game mode with UGameplayStatics from the interaction UObject
	// ContextActor is designed to be the interacting actor
	UFUNCTION(BlueprintPure, Category = "Interaction")
	static AGameModeBase* GetCurrentGameMode(const AActor* ContextActor);

	// Get the world the interaction exists in
	// Will allow library functions with WorldContextObject to be called in BP
	UFUNCTION(BlueprintCallable, Category = "Interaction|World")
	virtual UWorld* GetWorld() const override;
	
private:

	// The instigator of the interaction
	UPROPERTY() AActor* InteractingActor;

	// The actor that owns the component that owns this interaction
	UPROPERTY() AActor* OwningActor;

	// Checks if the interactions conditions can all be met
	bool CanActivateInteraction();
	void ActivateInteraction();
	bool bIsActive;
};
