/**
 * MetaHumanStreamingReceiver.h
 * 
 * This header file defines the UMetaHumanStreamingReceiver class, which is responsible for
 * receiving audio and blendshape data from the backend server and applying them to a MetaHuman
 * character in Unreal Engine.
 * 
 * Libraries/Modules used:
 * - CoreMinimal.h: Core Unreal Engine functionality
 * - GameFramework/Actor.h: Base class for actors in Unreal Engine
 * - Sound/SoundWave.h: Audio playback functionality
 * - Components/AudioComponent.h: Component for playing audio
 * - Interfaces/IHttpRequest.h: HTTP request functionality
 * - WebSocketsModule.h: WebSocket functionality
 * - IWebSocket.h: WebSocket interface
 * 
 * The class handles:
 * - Receiving data via HTTP or WebSocket
 * - Decoding audio data
 * - Applying blendshapes to the MetaHuman
 * - Synchronizing audio playback with facial animation
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "MetaHumanStreamingReceiver.generated.h"

// Forward declarations
class USkeletalMeshComponent;

/**
 * Structure to hold blendshape data for a single frame
 * 
 * This struct represents the facial animation data for a single frame of animation.
 * It contains the frame number and a map of blendshape names to values.
 * 
 * USTRUCT: Unreal Engine macro for defining a struct that can be used in Blueprint
 * GENERATED_BODY: Unreal Engine macro for generating boilerplate code
 */
USTRUCT(BlueprintType)
struct FBlendshapeFrame
{
    GENERATED_BODY()

    // Frame number
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blendshape")
    int32 FrameNumber;

    // Map of blendshape names to values (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blendshape")
    TMap<FString, float> BlendshapeValues;
};

/**
 * Structure to hold a complete animation sequence with audio and blendshapes
 * 
 * This struct represents a complete animation sequence with audio and blendshapes.
 * It contains the audio data, blendshape frames, and duration of the animation.
 * 
 * USTRUCT: Unreal Engine macro for defining a struct that can be used in Blueprint
 * GENERATED_BODY: Unreal Engine macro for generating boilerplate code
 */
USTRUCT(BlueprintType)
struct FMetaHumanAnimationData
{
    GENERATED_BODY()

    // Audio data as a sound wave
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    USoundWave* AudioData;

    // Blendshape frames for the animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<FBlendshapeFrame> BlendshapeFrames;

    // Duration of the animation in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Duration;
};

/**
 * Actor class that receives and processes streaming data for MetaHuman animation
 * 
 * This class is responsible for receiving audio and blendshape data from the backend server
 * and applying them to a MetaHuman character in Unreal Engine.
 * 
 * UCLASS: Unreal Engine macro for defining a class that can be used in Blueprint
 * GENERATED_BODY: Unreal Engine macro for generating boilerplate code
 */
UCLASS()
class METAHUMANSTREAMING_API UMetaHumanStreamingReceiver : public AActor
{
    GENERATED_BODY()

public:
    /**
     * Constructor
     * 
     * Sets default values for this actor's properties.
     */
    UMetaHumanStreamingReceiver();

    /**
     * BeginPlay
     * 
     * Called when the game starts or when spawned.
     * Initializes the WebSockets module.
     */
    virtual void BeginPlay() override;

    /**
     * Tick
     * 
     * Called every frame.
     * Updates the animation based on elapsed time.
     * 
     * @param DeltaTime - Time elapsed since the last frame
     */
    virtual void Tick(float DeltaTime) override;

    /**
     * EndPlay
     * 
     * Called when the game ends.
     * Closes the WebSocket connection if it exists.
     * 
     * @param EndPlayReason - Reason for ending the play
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /**
     * Initialize the WebSocket connection to the backend server
     * 
     * This function creates a WebSocket connection to the backend server
     * and sets up event handlers for the connection.
     * 
     * @param ServerURL - The URL of the WebSocket server
     * @return bool - True if the connection was successful
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Streaming")
    bool InitializeWebSocketConnection(const FString& ServerURL);

    /**
     * Initialize the HTTP endpoint for receiving data
     * 
     * This function initializes the HTTP endpoint for receiving data.
     * In a real implementation, you might set up a polling mechanism or server-sent events.
     * 
     * @param EndpointURL - The URL of the HTTP endpoint
     * @return bool - True if the initialization was successful
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Streaming")
    bool InitializeHTTPEndpoint(const FString& EndpointURL);

    /**
     * Set the MetaHuman skeletal mesh component to animate
     * 
     * This function sets the skeletal mesh component to animate.
     * The skeletal mesh component should be the MetaHuman character's mesh.
     * 
     * @param InSkeletalMeshComponent - The skeletal mesh component to animate
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Streaming")
    void SetMetaHumanMesh(USkeletalMeshComponent* InSkeletalMeshComponent);

    /**
     * Process received data from the backend
     * 
     * This function processes the received audio and blendshape data.
     * It decodes the audio data, parses the blendshape data, and starts the animation.
     * 
     * @param AudioBase64 - Base64-encoded audio data
     * @param BlendshapeData - JSON string containing blendshape data
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Streaming")
    void ProcessReceivedData(const FString& AudioBase64, const FString& BlendshapeData);

private:
    // The skeletal mesh component of the MetaHuman to animate
    UPROPERTY()
    USkeletalMeshComponent* MetaHumanMeshComponent;

    // Audio component for playing received audio
    UPROPERTY()
    UAudioComponent* AudioComponent;

    // WebSocket instance for real-time communication
    TSharedPtr<IWebSocket> WebSocket;

    // Current animation data being processed
    FMetaHumanAnimationData CurrentAnimationData;

    // Flag indicating whether animation is currently playing
    bool bIsAnimating;

    // Current frame being processed during animation
    int32 CurrentFrame;

    // Time elapsed since animation started
    float AnimationTime;

    // Frame rate for blendshape animation (frames per second)
    float FrameRate;

    /**
     * Decode base64-encoded audio data to a USoundWave
     * 
     * This function decodes base64-encoded audio data to a USoundWave object
     * that can be played by the audio component.
     * 
     * @param AudioBase64 - Base64-encoded audio data
     * @return USoundWave* - The decoded sound wave
     */
    USoundWave* DecodeAudioData(const FString& AudioBase64);

    /**
     * Parse blendshape data from JSON
     * 
     * This function parses blendshape data from a JSON string.
     * The JSON string should contain an array of blendshape frames.
     * 
     * @param BlendshapeJSON - JSON string containing blendshape data
     * @return TArray<FBlendshapeFrame> - Array of blendshape frames
     */
    TArray<FBlendshapeFrame> ParseBlendshapeData(const FString& BlendshapeJSON);

    /**
     * Apply blendshapes to the MetaHuman mesh
     * 
     * This function applies blendshape values to the MetaHuman mesh.
     * It sets the morph target values on the skeletal mesh component.
     * 
     * @param BlendshapeValues - Map of blendshape names to values
     */
    void ApplyBlendshapesToMesh(const TMap<FString, float>& BlendshapeValues);

    /**
     * Start playing the animation
     * 
     * This function starts playing the animation.
     * It sets up the audio component, resets the animation state, and starts audio playback.
     */
    void StartAnimation();

    /**
     * Stop the current animation
     * 
     * This function stops the current animation.
     * It stops audio playback, resets the animation state, and resets all blendshapes to zero.
     */
    void StopAnimation();

    /**
     * Update the animation based on elapsed time
     * 
     * This function updates the animation based on elapsed time.
     * It calculates the current frame based on time and frame rate,
     * and applies blendshapes for the current frame.
     * 
     * @param DeltaTime - Time elapsed since the last frame
     */
    void UpdateAnimation(float DeltaTime);

    /**
     * Handle WebSocket connection established
     * 
     * This function is called when the WebSocket connection is established.
     * It logs a message indicating that the connection was successful.
     */
    void OnWebSocketConnected();

    /**
     * Handle WebSocket connection error
     * 
     * This function is called when there is an error with the WebSocket connection.
     * It logs an error message with the error details.
     * 
     * @param Error - Error message
     */
    void OnWebSocketConnectionError(const FString& Error);

    /**
     * Handle WebSocket connection closed
     * 
     * This function is called when the WebSocket connection is closed.
     * It logs a message with the status code, reason, and whether the closure was clean.
     * 
     * @param StatusCode - Status code for the closure
     * @param Reason - Reason for the closure
     * @param bWasClean - Whether the closure was clean
     */
    void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);

    /**
     * Handle message received via WebSocket
     * 
     * This function is called when a message is received via WebSocket.
     * It parses the message as JSON, extracts the audio and blendshape data,
     * and processes the received data.
     * 
     * @param Message - The received message
     */
    void OnWebSocketMessage(const FString& Message);

    /**
     * Handle HTTP response received
     * 
     * This function is called when an HTTP response is received.
     * It parses the response as JSON, extracts the audio and blendshape data,
     * and processes the received data.
     * 
     * @param Request - The HTTP request
     * @param Response - The HTTP response
     * @param bSucceeded - Whether the request succeeded
     */
    void OnHTTPResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
};
