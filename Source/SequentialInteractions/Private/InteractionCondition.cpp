// Copyright 2023 Evelyn Schwab under MIT license


#include "InteractionCondition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionCondition)

UInteractionCondition::UInteractionCondition()
{
	InvertCondition = false;
}

bool UInteractionCondition::CheckInteractionConditions_Implementation(AActor* InteractingActor)
{
	return false;
}
