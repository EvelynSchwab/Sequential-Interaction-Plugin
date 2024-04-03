// Copyright 2023 Evelyn Schwab under MIT license


#include "InteractionFunctionLibrary.h"

#include "SequentialInteractionComponent.h"

bool UInteractionFunctionLibrary::TryStartInteraction(AActor* InteractiveActor, AActor* InteractingActor)
{
	USequentialInteractionComponent* InteractionComponent = InteractiveActor->FindComponentByClass<USequentialInteractionComponent>();
	if (!InteractionComponent) { return false; }
	InteractionComponent->StartSequentialInteractions(InteractingActor);
	return true;
}
