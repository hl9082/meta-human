/**
 * MetaHumanStreamingGameMode.h
 * 
 * This header file defines the AMetaHumanStreamingGameMode class, which sets up
 * the MetaHuman streaming environment in Unreal Engine.
 * 
 * Libraries/Modules used:
 * - CoreMinimal.h: Core Unreal Engine functionality
 * - GameFramework/GameModeBase.h: Base class for game modes in Unreal Engine
 * 
 * The class handles:
 * - Initializing the MetaHuman character
 * - Setting up the Pixel Streaming environment
 * - Connecting the various components of the system
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MetaHumanStreamingGameMode.generated.h"

// Forward declarations
class UMetaHumanStreamingReceiver;
class UPixelStreamingCustomHandler;
class AMetaHumanCharacter;

/**
 * Game mode class for MetaHuman streaming
 * 
 * This class is responsible for setting up the MetaHuman streaming environment
 * in Unreal Engine. It initializes the MetaHuman character, sets up the Pixel
 * Streaming environment, and connects the various components of the system.
 * 
 * UCLASS: Unreal Engine macro for defining a class that can be used in Blueprint
 * GENERATED_BODY: Unreal Engine macro for generating boilerplate code
 */
UCLASS()
class METAHUMANSTREAMING_API AMetaHumanStreamingGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    /**
     * Constructor
     * 
     * Sets default values for this game mode's properties.
     */
    AMetaHumanStreamingGameMode();

    /**
     * BeginPlay
     * 
     * Called when the game starts.
     * Initializes the MetaHuman character, sets up the Pixel Streaming environment,
     * and connects the various components of the system.
     */
    virtual void BeginPlay() override;

    /**
     * EndPlay
     * 
     * Called when the game ends.
     * Cleans up resources if needed.
     * 
     * @param EndPlayReason - Reason for ending the play
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /**
     * Get the MetaHuman streaming receiver
     * 
     * This function returns the MetaHuman streaming receiver.
     * 
     * @return UMetaHumanStreamingReceiver* - The MetaHuman streaming receiver
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Streaming")
    UMetaHumanStreamingReceiver* GetMetaHumanReceiver() const { return MetaHumanReceiver; }

    /**
     * Get the Pixel Streaming custom handler
     * 
     * This function returns the Pixel Streaming custom handler.
     * 
     * @return UPixelStreamingCustomHandler* - The Pixel Streaming custom handler
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Streaming")
    UPixelStreamingCustomHandler* GetPixelStreamingHandler() const { return PixelStreamingHandler; }

    /**
     * Get the MetaHuman character
     * 
     * This function returns the MetaHuman character.
     * 
     * @return AMetaHumanCharacter* - The MetaHuman character
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Streaming")
    AMetaHumanCharacter* GetMetaHumanCharacter() const { return MetaHumanCharacter; }

private:
    // The MetaHuman streaming receiver
    UPROPERTY()
    UMetaHumanStreamingReceiver* MetaHumanReceiver;

    // The Pixel Streaming custom handler
    UPROPERTY()
    UPixelStreamingCustomHandler* PixelStreamingHandler;

    // The MetaHuman character
    UPROPERTY()
    AMetaHumanCharacter* MetaHumanCharacter;

    /**
     * Initialize the MetaHuman character
     * 
     * This function initializes the MetaHuman character.
     * It finds the MetaHuman character in the world and stores a reference to it.
     */
    void InitializeMetaHumanCharacter();

    /**
     * Initialize the Pixel Streaming environment
     * 
     * This function initializes the Pixel Streaming environment.
     * It creates the MetaHuman streaming receiver and the Pixel Streaming custom handler.
     */
    void InitializePixelStreaming();

    /**
     * Connect the various components of the system
     * 
     * This function connects the various components of the system.
     * It sets the MetaHuman mesh for the receiver, sets the MetaHuman receiver for the
     * Pixel Streaming handler, and initializes the WebSocket connection for real-time communication.
     */
    void ConnectComponents();
};
