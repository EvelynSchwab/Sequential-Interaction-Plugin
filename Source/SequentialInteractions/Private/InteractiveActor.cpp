// Copyright 2023 Evelyn Schwab under MIT license


#include "InteractiveActor.h"
#include "SequentialInteractionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractiveActor)

AInteractiveActor::AInteractiveActor()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionComponent = CreateDefaultSubobject<USequentialInteractionComponent>(TEXT("Interaction Component"));
}
