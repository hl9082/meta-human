/**
 * PixelStreamingCustomHandler.h
 * 
 * This header file defines the UPixelStreamingCustomHandler class, which extends
 * the Pixel Streaming functionality to handle custom messages from the frontend.
 * 
 * Libraries/Modules used:
 * - CoreMinimal.h: Core Unreal Engine functionality
 * - GameFramework/Actor.h: Base class for actors in Unreal Engine
 * 
 * The class handles:
 * - Registering custom message handlers with the Pixel Streaming subsystem
 * - Processing custom messages from the frontend
 * - Forwarding data to the MetaHumanStreamingReceiver
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PixelStreamingCustomHandler.generated.h"

// Forward declarations
class UMetaHumanStreamingReceiver;

/**
 * Actor class that handles custom Pixel Streaming messages
 * 
 * This class is responsible for handling custom messages from the frontend
 * via Pixel Streaming and forwarding them to the MetaHumanStreamingReceiver.
 * 
 * UCLASS: Unreal Engine macro for defining a class that can be used in Blueprint
 * GENERATED_BODY: Unreal Engine macro for generating boilerplate code
 */
UCLASS()
class METAHUMANSTREAMING_API UPixelStreamingCustomHandler : public AActor
{
    GENERATED_BODY()

public:
    /**
     * Constructor
     * 
     * Sets default values for this actor's properties.
     */
    UPixelStreamingCustomHandler();

    /**
     * BeginPlay
     * 
     * Called when the game starts or when spawned.
     * Registers custom message handlers with the Pixel Streaming subsystem.
     */
    virtual void BeginPlay() override;

    /**
     * EndPlay
     * 
     * Called when the game ends.
     * Unregisters custom message handlers from the Pixel Streaming subsystem.
     * 
     * @param EndPlayReason - Reason for ending the play
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /**
     * Set the MetaHuman streaming receiver to forward data to
     * 
     * This function sets the MetaHuman streaming receiver to forward data to.
     * The receiver should be an instance of UMetaHumanStreamingReceiver.
     * 
     * @param InReceiver - The MetaHuman streaming receiver
     */
    UFUNCTION(BlueprintCallable, Category = "PixelStreaming|CustomHandler")
    void SetMetaHumanReceiver(UMetaHumanStreamingReceiver* InReceiver);

private:
    // Reference to the MetaHuman streaming receiver
    UPROPERTY()
    UMetaHumanStreamingReceiver* MetaHumanReceiver;

    /**
     * Register custom message handlers with the Pixel Streaming subsystem
     * 
     * This function registers custom message handlers with the Pixel Streaming subsystem.
     * It sets up handlers for specific message types like "process_data".
     */
    void RegisterCustomMessageHandlers();

    /**
     * Unregister custom message handlers from the Pixel Streaming subsystem
     * 
     * This function unregisters custom message handlers from the Pixel Streaming subsystem.
     * It removes handlers for specific message types like "process_data".
     */
    void UnregisterCustomMessageHandlers();

    /**
     * Handle custom message from the frontend
     * 
     * This function handles custom messages from the frontend.
     * It dispatches the message to the appropriate handler based on the message type.
     * 
     * @param MessageType - The type of message
     * @param MessageContents - The contents of the message
     */
    void HandleCustomMessage(const FString& MessageType, const FString& MessageContents);

    /**
     * Handle process data message from the frontend
     * 
     * This function handles process_data messages from the frontend.
     * It parses the message as JSON, extracts the audio and blendshape data,
     * and forwards them to the MetaHuman receiver.
     * 
     * @param MessageContents - The contents of the message
     */
    void HandleProcessDataMessage(const FString& MessageContents);
};
