# Sequential Interaction Plugin

## Overview
An Unreal Engine plugin adding a component to facilitate sequences of interactions between a player and an actor.
It is designed for straightforward programmer and scripter setup for selections of interaction archetypes to be used across a game. Interactions are separated to dedicated objects that can be subclassed to add custom interaction logic, and can be made generic. 

This system is designed for single-player situations like the following:

The player interacts with a door.
- If the player is in combat:
  - Display UI dialogue "player in combat"
  - Exit the interaction
- If the player has a key:
  - Unlock the door
  - Play unlock sound
  - Show UI dialogue "door unlocked', wait for player confirmation
  - Open the door
  - Exit the interaction
- If the player does not have the required key:
  - Play handle jiggle sound
  - Show UI dialogue "I do not have the key for this door", wait for player confirmation
  - Exit the interaction

Each step in this process is considered an interaction, and the process is considered a sequence. Each interaction can have multiple conditions associated with it, dictating which interactions can occur.

In this example, the player will interact with an object. If the player has a red book in their inventory, they will trigger the 'has item' dialogue. If they do not, they will trigger the 'no item' dialogue.

![UnrealEditor_ybqhj35OMH](https://github.com/EvelynSchwab/Sequential-Interaction-Plugin/assets/33647307/9e72db49-0d34-4804-a33c-bf514ab22584)


## Features
- A sequential interaction component, to manage the available interactions and the sequential process.
- An interaction object, which acts as an individual interaction in the sequence.
- An interaction condition object, that acts as a check for the player's ability to enter an interaction.

## Usage
Adding a sequential interaction component to an actor marks it as interactive.
Adding interactions to the array of sequential interactions dictates the interactions and order of interactions.

There are no included complete interactions as the interaction object is intended to be subclassed.

### Interactions
To set up a custom interaction, subclass the _Interaction_ class in Blueprint, and override the events _InteractionActivated_ and _InteractionCommitted_.

There are a total of 4 events available in the interaction:
- Interaction Activated
  - Runs automatically when the interaction starts. This is where you can do any additional checks or setup for the interaction. To make the interaction continue, the _CommitInteration_ function must be run, which will check any interaction conditions.
- Interaction Committed
  - Runs when an interaction is successfully committed. This is where the interaction itself should be handled. Once the interaction is complete, the function EndInteraction must be run.
- Interaction Cancelled
  - Runs when an interaction was cancelled, either automatically through a condition fail or error, or when the function _CancelIntearction_ is used. Returns an enum with the cancel reason.
- Interaction Ended
  - Runs when an interaction is ended through the function _EndInteraction_.
 
In the below example, the interaction creates a UI dialogue prompt for the specified data table row. If the dialogue is created, the _EndInteraction_ function is bound to external player input. If there is a failure at any point, _EndInteraction_ is called manually.

![BP_Interaction_Dialogue-EventGraph](https://github.com/EvelynSchwab/Sequential-Interaction-Plugin/assets/33647307/38e906f9-40f1-4349-8879-908636ec8134)

The _Interaction_ class also includes the following properties:
- _bool_ Can Repeat Interaction
  - If this is false, the interaction is marked as non-repeatable. This does not get saved externally so should not be relied on.
- _bool_ Start Next Interaction Automatically
  - If this is true, the next interaction will start once InteractionEnded is called. If it is false, external input is required to start the next interaction.
- Conditions array
  - Conditions for the interaction to be successfully run.

### Interaction Conditions
To set up a custom interaction condition, subclass _InteractionCondition_ and override the function _CheckInteractionConditions_.
If _CheckInteractionConditions_ returns false, the interaction associated with the condition will cancel, with the reason given _ConditionCheckFailed_. If it returns true, the interaction will be committed (provided any other conditions are also passed).

In the below example, the interacting actor's inventory is checked for specific items. If the player has the required items, the condition check passes. If the items are not found, or the inventory component is not found, the condition fails and the interaction it is associated with will be cancelled.

![BP_InteractionCondition_HasItem-CheckInteractionConditions](https://github.com/EvelynSchwab/Sequential-Interaction-Plugin/assets/33647307/51f29e26-4f4c-46d3-b046-d2298fb2aef0)

The _InteractionCondition_ class also includes the following properties:
- _bool_ InvertCondition
  - If this is true, the result of _CheckInteractionCondition_ is inverted; e.g. does the player _not_ have this item.

### Sequential Interaction Component

The Sequential Interaction Component itself has a set of properties that change its behaviour:
- Array of interaction structures:
  - Interaction Debug Name
    - The name of the interaction that will show in the logs
  - Sequential Interaction
    - The interaction itself
  - _bool_ Reset Interactions on Condition Fail
    - If this is true, the interaction sequence will be reset. This means the next time an interaction is triggered, it will start from index 0.
- _bool_ Show Debug Information
  - If this is true, the actor the component is attached to will have debug text displayed above it in-game, showing the state of the sequential interactions and the names of any active interactions.

The plugin also has a custom log category, LogSequentialInteraction, which can be used to debug the state of any interactive objects.

## License

This repo is under MIT license.
