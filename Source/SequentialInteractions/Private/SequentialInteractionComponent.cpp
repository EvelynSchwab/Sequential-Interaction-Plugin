// Copyright 2023 Evelyn Schwab under MIT license


#include "SequentialInteractionComponent.h"
#include "SequentialInteractions.h"
#include "Logging/StructuredLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SequentialInteractionComponent)

USequentialInteractionComponent::USequentialInteractionComponent()
{
	// Only tick this component if we are showing debug information
	//TODO::Set up a timer for this instead of relying on tick
	PrimaryComponentTick.bCanEverTick = true;
	CurrentSequentialInteractionIndex = -1;
	CurrentlyInteractingActor = nullptr;
	ActiveInteractionInstance = nullptr;
	CurrentInteractionState = EInteractionState::SequentialState_Idle;
	bShowDebugInformation = false;
	DebugTextColour = FColor::Cyan;
	DebugTextSize = 3.0f;
}

void USequentialInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Draw debug information if we are showing it
	if (!bShowDebugInformation) return;
	const FString CurrentState = "Current State: " + UEnum::GetValueAsString(CurrentInteractionState);
	DrawDebugString(GetWorld(), GetDebugTextBaseDrawLocation() + FVector(0, 0, 30), CurrentState, nullptr,
	                DebugTextColour, 0.0f, false, DebugTextSize);
	
	if (ActiveInteractionInstance != nullptr)
	{
		const FString CurrentInteraction = "Current Interaction: " + ActiveInteractionInstance->GetName();
		const FString CurrentInteractionName = SequentialInteractions[CurrentSequentialInteractionIndex].
			InteractionDebugName + " : Index " + FString::FromInt(CurrentSequentialInteractionIndex);
		// Draw a debug string for the current active interaction
		DrawDebugString(GetWorld(), GetDebugTextBaseDrawLocation() + FVector(0, 0, 20), CurrentInteraction, nullptr,
		                DebugTextColour, 0.0f, false, DebugTextSize);
		DrawDebugString(GetWorld(), GetDebugTextBaseDrawLocation() + FVector(0, 0, 10), CurrentInteractionName, nullptr,
						DebugTextColour, 0.0f, false, DebugTextSize);
	}
}

void USequentialInteractionComponent::StartSequentialInteractions(AActor* InteractingActor)
{
	if (ActiveInteractionInstance != nullptr)
	{
		UE_LOGFMT(LogSequentialInteractions, Log, "Component on {Actor} tried to start interactions while an interaction was already active",
			GetOwner()->GetName());
		return;
	}

	CurrentInteractionState = EInteractionState::SequentialState_Waiting;
	
	// Early return if the interacting actor is not valid
	if (!IsValid(InteractingActor)) return;
	// Save the interacting actor for this interaction sequence
	CurrentlyInteractingActor = InteractingActor;
	// Start the interactions
	StartNextSequentialInteraction();
	
	UE_LOGFMT(LogSequentialInteractions, Log, "Component starting interactions on actor {Actor} (instigator {instigator})",
			GetOwner()->GetName(), InteractingActor->GetName());
}

void USequentialInteractionComponent::StartNextSequentialInteraction()
{
	UE_LOGFMT(LogSequentialInteractions, Log, "Component on {Actor} looking for new potential interaction in sequence", GetOwner()->GetName());
	
	const int32 MaxPotentialIndex = SequentialInteractions.Num() - 1;
	
	// Loop through the potential interactions to find the next valid interaction
	for (int32 PotentialInteractionIndex = CurrentSequentialInteractionIndex + 1; PotentialInteractionIndex <= MaxPotentialIndex; ++PotentialInteractionIndex)
	{
		if (!SequentialInteractions[PotentialInteractionIndex].bInteractionComplete)
		{
			CurrentSequentialInteractionIndex = PotentialInteractionIndex;

			UE_LOGFMT(LogSequentialInteractions, Log, "Component starting next interaction on actor {Actor} at index {index} (instigator {instigator})",
            	GetOwner()->GetName(), FString::FromInt(CurrentSequentialInteractionIndex), CurrentlyInteractingActor->GetName());

			// Create a duplicate of the interaction to perform the actual interaction
			// This is done so that the original object is not modified, and the interaction can be repeated with the same default values
			
			// Get a pointer to the current interaction at the current index
			const FSequentialInteraction* CurrentSequentialInteraction = &SequentialInteractions[CurrentSequentialInteractionIndex];
			ActiveInteractionInstance = DuplicateObject(CurrentSequentialInteraction->SequentialInteraction, this->GetOwner());

			UE_LOGFMT(LogSequentialInteractions, Log,
			          "Component of {Actor} instanciated interaction {Interaction} at index {Index} to: {DuplicateInteraction}",
			          GetOwner()->GetName(), CurrentSequentialInteraction->SequentialInteraction->GetName(),
			          FString::FromInt(CurrentSequentialInteractionIndex), ActiveInteractionInstance->GetName());
			
			// Try to start the interaction, then bind the OnInteractionEnded and OnInteractionCancelled functions to the interaction's delegates
            ActiveInteractionInstance->OnInteractionEnded.AddDynamic(this, &USequentialInteractionComponent::OnInteractionEnded);
            ActiveInteractionInstance->OnInteractionCancelled.AddDynamic(this, &USequentialInteractionComponent::OnInteractionCancelled);
            ActiveInteractionInstance->TryActivateInteraction(CurrentlyInteractingActor);
			// Only update the state if the interaction is valid. Ideally this should only happen if the interaction
			// is instantly cancelled due to a condition change, which would set the ActiveInteractionInstance to
			// nullptr in the OnInteractionEnded function (via OnInteractionCancelled)
			if (IsValid(ActiveInteractionInstance)) CurrentInteractionState = EInteractionState::SequentialState_InProgress;
			return;
		}
	}

	// If we did not find a valid a valid interaction, end the interaction sequence
	UE_LOGFMT(LogSequentialInteractions, Log, "Component of {Actor} did not find a new valid interaction, ending interactions", GetOwner()->GetName());
	EndSequentialInteractions();
}

void USequentialInteractionComponent::EndSequentialInteractions()
{
	// End the interactions and clean up properties
	UE_LOGFMT(LogSequentialInteractions, Log, "Component ending interactions on actor {Actor} (instigator {instigator})",
		GetOwner()->GetName(), CurrentlyInteractingActor->GetName());
	CurrentSequentialInteractionIndex = -1;
	CurrentInteractionState = EInteractionState::SequentialState_Idle;
}

bool USequentialInteractionComponent::HasInteractionBeenCompleted(const int32 InteractionIndex)
{
	if (!SequentialInteractions.IsValidIndex(InteractionIndex)) return false;
	return SequentialInteractions[InteractionIndex].bInteractionComplete;
}

void USequentialInteractionComponent::OnInteractionEnded(bool bCompletedSuccessfully)
{
	// Clear the reference to the duplicate interaction index
	// This should open it up for garbage collection
	// TODO::Consider manual garbage collection here, or potentially when the next interaction starts

	const bool bStartNextInteractionAutomatically = ActiveInteractionInstance->bStartNextInteractionAutomatically;
	ActiveInteractionInstance = nullptr;
	
	// Check that the index is valid
	if (!SequentialInteractions.IsValidIndex(CurrentSequentialInteractionIndex))
	{
		UE_LOGFMT(LogSequentialInteractions, Error, "Component on {Actor} had an interaction end on an invalid index {Index}",
		          GetOwner()->GetName(), FString::FromInt(CurrentSequentialInteractionIndex));
		CurrentInteractionState = EInteractionState::SequentialState_Failed;
		return;
	}

	// If the interaction should not repeat, mark it as complete
	if (!SequentialInteractions[CurrentSequentialInteractionIndex].SequentialInteraction->bCanRepeatInteraction)
	{
		SequentialInteractions[CurrentSequentialInteractionIndex].bInteractionComplete = true;
	}

	CurrentInteractionState = EInteractionState::SequentialState_Waiting;
	
	// If this is the last interaction in the sequence, or the interaction failed and bResetInteractionsOnConditionFail
	// was true & the conditions failed, run EndSequentialInteractions to so that the next
	// interaction will go from the beginning of the interactions
	if (CurrentSequentialInteractionIndex == SequentialInteractions.Num() - 1 || (!bCompletedSuccessfully &&
			SequentialInteractions[CurrentSequentialInteractionIndex].bResetInteractionsOnConditionsFail))
	{
		EndSequentialInteractions();
	}
	
	// If this interaction should start the next interaction automatically, start it now
	if (bStartNextInteractionAutomatically) { StartNextSequentialInteraction(); }
}

void USequentialInteractionComponent::OnInteractionCancelled(TEnumAsByte<EInteractionCancelReason> CancelReason)
{
	if (bShowDebugInformation)
	{
		const FString InteractionFailMessage = "Interaction " + ActiveInteractionInstance->GetName() +
			" failed due to: " + UEnum::GetDisplayValueAsText(CancelReason).ToString();
		DrawDebugString(GetWorld(), GetDebugTextBaseDrawLocation() + FVector(0, 0, 40), InteractionFailMessage, nullptr,
		                FColor::Red, 3.0f, false, DebugTextSize);
	}
	
	OnInteractionEnded(false);
}


FVector USequentialInteractionComponent::GetDebugTextBaseDrawLocation() const
{
	FVector OwnerOrigin;
	FVector OwnerBounds;
	GetOwner()->GetActorBounds(false, OwnerOrigin, OwnerBounds);
	return OwnerOrigin + FVector(0, 0, OwnerBounds.Z);
}
