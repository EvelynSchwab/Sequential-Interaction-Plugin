// Copyright 2023 Evelyn Schwab under MIT license


#include "Interaction.h"

#include "InteractionCondition.h"
#include "SequentialInteractions.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Interaction)

UInteraction::UInteraction()
{
	bCanRepeatInteraction = true;
	bStartNextInteractionAutomatically = false;
	
	InteractingActor = nullptr;
	OwningActor = nullptr;
	
	bIsActive = false;
}

#pragma region Interaction Activation

void UInteraction::TryActivateInteraction(AActor* ActivatingActor)
{
	// Check that there is a valid instigator for this interaction
	if (!IsValid(ActivatingActor))
	{
		UE_LOGFMT(LogSequentialInteractions, Log,
		          "Interaction {Interaction} tried to activate on actor {Actor} without a valid instigator.", GetName(),
		          GetOuter()->GetName());
		InteractionCancelled(Cancel_Failed);
		return;
	}
	
	InteractingActor = ActivatingActor;

	// If the interaction can't be activated (due to condition failure), cancel the interaction
	if (!CanActivateInteraction())
	{
		CancelInteraction(Cancel_ConditionsNotMet);
		return;
	}
	
	// Activate the interaction if all conditions are met
	ActivateInteraction();
}

bool UInteraction::CanActivateInteraction()
{
	return AreInteractionConditionsMet();
}

bool UInteraction::AreInteractionConditionsMet()
{
	// Loop through interaction conditions, early returning false if any are not met
	for (UInteractionCondition* Condition : Conditions)
	{
		bool bConditionsMet = Condition->CheckInteractionConditions(InteractingActor);
		// If the condition is inverted, the result is negated
		if (Condition->InvertCondition) bConditionsMet = !bConditionsMet;
		if (!bConditionsMet) { return false; }
	}
	return true;
}

void UInteraction::ActivateInteraction()
{
	// Mark the interaction as active and run the interaction activated event
	bIsActive = true;
	InteractionActivated();
	UE_LOGFMT(LogSequentialInteractions, Log, "Interaction {Interaction} activating on actor {Actor} (instigator = {instigator})",
		GetName(), GetOuter()->GetName(), InteractingActor->GetName());
}

#pragma endregion

#pragma region Interaction Committing

void UInteraction::CommitInteraction(bool bBypassRequirements)
{
	// Check if the interaction can be committed
	if (!bBypassRequirements)
	{
		if (!CanCommitInteraction())
		{
			UE_LOGFMT(LogSequentialInteractions, Log, "Interaction {Interaction} failed to commit  on actor {Actor} (instigator = {instigator})",
				GetName(), GetOuter()->GetName(), InteractingActor->GetName());
		}
	}
	UE_LOGFMT(LogSequentialInteractions, Log, "Interaction {Interaction} committed on actor {Actor}{BypassRequirements}",
		GetName(), GetOuter()->GetName(), bBypassRequirements ? "" : "with requirements bypassed");
	InteractionCommitted();
}

bool UInteraction::CanCommitInteraction()
{
	// Loop through all conditions to check if we can activate the interaction
	for (UInteractionCondition* Condition : Conditions)
	{
		// Cancel the interaction if a condition is not met
		if (Condition->CheckInteractionConditions(InteractingActor))
		{
			CancelInteraction(Cancel_ConditionsNotMet);
			return false;
		}
	}
	return true;
}

#pragma endregion

#pragma region Interaction Ending

// Interaction Ending

void UInteraction::EndInteraction()
{
	// Only end the interaction if it is already active
	if (bIsActive == false) return;
	
	UE_LOGFMT(LogSequentialInteractions, Log, "Interaction {Interaction} ended on actor {Actor} (instigator = {instigator})",
	GetName(), GetOuter()->GetName(), InteractingActor->GetName());

	OnInteractionEnded.Broadcast(true);
	bIsActive = false;
	InteractionEnded();
}

// Interaction Cancellation



void UInteraction::CancelInteraction(TEnumAsByte<EInteractionCancelReason> CancelReason)
{
	UE_LOGFMT(LogSequentialInteractions, Log, "Interaction {Interaction} cancelled due to {Reason} on actor {Actor}",
		this->GetName(), UEnum::GetDisplayValueAsText(CancelReason).ToString(), GetOuter()->GetName());
	
	bIsActive = false;
	OnInteractionCancelled.Broadcast(CancelReason);
	InteractionCancelled(CancelReason);
}

#pragma endregion

#pragma region Helpers

UWorld* UInteraction::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return UObject::GetWorld();
}

/*bool UInteraction::AreInteractionConditionsMet_Implementation()
{
	// Loop through interaction conditions, early returning false if any are not met
	for (UInteractionCondition* Condition : Conditions)
	{
		bool bConditionsMet = Condition->CheckInteractionConditions(InteractingActor);
		// If the condition is inverted, the result is negated
		if (Condition->InvertCondition) bConditionsMet = !bConditionsMet;
		if (!bConditionsMet) { return false; }
	}
	return true;
}*/

AGameModeBase* UInteraction::GetCurrentGameMode(const AActor* ContextActor)
{
	return Cast<AGameModeBase>(UGameplayStatics::GetGameMode(ContextActor));
}



#pragma endregion
