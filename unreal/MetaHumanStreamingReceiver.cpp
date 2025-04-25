/**
 * MetaHumanStreamingReceiver.cpp
 * 
 * Implementation of the UMetaHumanStreamingReceiver class, which handles receiving
 * audio and blendshape data from the backend server and applying them to a MetaHuman
 * character in Unreal Engine.
 */

#include "MetaHumanStreamingReceiver.h"
#include "Components/SkeletalMeshComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sound/SoundWave.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Misc/Base64.h"

// Sets default values
UMetaHumanStreamingReceiver::UMetaHumanStreamingReceiver()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize animation variables
    bIsAnimating = false;
    CurrentFrame = 0;
    AnimationTime = 0.0f;
    FrameRate = 60.0f; // Default to 60 FPS
}

// Called when the game starts or when spawned
void UMetaHumanStreamingReceiver::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize WebSockets module
    FWebSocketsModule& WebSocketsModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");
}

// Called when the game ends
void UMetaHumanStreamingReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // Close WebSocket connection if it exists
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        WebSocket->Close();
        WebSocket = nullptr;
    }
}

// Called every frame
void UMetaHumanStreamingReceiver::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update animation if currently playing
    if (bIsAnimating)
    {
        UpdateAnimation(DeltaTime);
    }
}

bool UMetaHumanStreamingReceiver::InitializeWebSocketConnection(const FString& ServerURL)
{
    // Create WebSocket
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerURL);

    // Bind event handlers
    WebSocket->OnConnected().AddUObject(this, &UMetaHumanStreamingReceiver::OnWebSocketConnected);
    WebSocket->OnConnectionError().AddUObject(this, &UMetaHumanStreamingReceiver::OnWebSocketConnectionError);
    WebSocket->OnClosed().AddUObject(this, &UMetaHumanStreamingReceiver::OnWebSocketClosed);
    WebSocket->OnMessage().AddUObject(this, &UMetaHumanStreamingReceiver::OnWebSocketMessage);

    // Connect to server
    WebSocket->Connect();

    return true;
}

bool UMetaHumanStreamingReceiver::InitializeHTTPEndpoint(const FString& EndpointURL)
{
    // Nothing to initialize for HTTP endpoint, just store the URL
    // In a real implementation, you might set up a polling mechanism or server-sent events
    
    UE_LOG(LogTemp, Log, TEXT("HTTP endpoint initialized: %s"), *EndpointURL);
    return true;
}

void UMetaHumanStreamingReceiver::SetMetaHumanMesh(USkeletalMeshComponent* InSkeletalMeshComponent)
{
    MetaHumanMeshComponent = InSkeletalMeshComponent;
}

void UMetaHumanStreamingReceiver::ProcessReceivedData(const FString& AudioBase64, const FString& BlendshapeData)
{
    // Stop any current animation
    StopAnimation();

    // Decode audio data
    USoundWave* SoundWave = DecodeAudioData(AudioBase64);
    if (!SoundWave)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to decode audio data"));
        return;
    }

    // Parse blendshape data
    TArray<FBlendshapeFrame> BlendshapeFrames = ParseBlendshapeData(BlendshapeData);
    if (BlendshapeFrames.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse blendshape data"));
        return;
    }

    // Set up current animation data
    CurrentAnimationData.AudioData = SoundWave;
    CurrentAnimationData.BlendshapeFrames = BlendshapeFrames;
    CurrentAnimationData.Duration = SoundWave->Duration;

    // Start the animation
    StartAnimation();
}

USoundWave* UMetaHumanStreamingReceiver::DecodeAudioData(const FString& AudioBase64)
{
    // Decode base64 string to binary data
    TArray<uint8> DecodedAudio;
    if (!FBase64::Decode(AudioBase64, DecodedAudio))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to decode base64 audio data"));
        return nullptr;
    }

    // Create a new sound wave
    USoundWave* SoundWave = NewObject<USoundWave>(this);
    
    // Set sound wave properties
    // Note: This is a simplified implementation. In a real application,
    // you would need to parse the audio format (e.g., WAV, MP3) and set
    // properties accordingly.
    SoundWave->SetSampleRate(44100); // Assuming 44.1kHz sample rate
    SoundWave->NumChannels = 1;      // Assuming mono audio
    
    // Set the audio data
    // Note: In a real implementation, you would need to handle different audio formats
    // and possibly convert the data to the format expected by Unreal Engine.
    SoundWave->RawData.Lock(LOCK_READ_WRITE);
    void* LockedData = SoundWave->RawData.Realloc(DecodedAudio.Num());
    FMemory::Memcpy(LockedData, DecodedAudio.GetData(), DecodedAudio.Num());
    SoundWave->RawData.Unlock();
    
    // Set duration based on audio data
    // Note: In a real implementation, you would calculate this based on the audio format
    SoundWave->Duration = DecodedAudio.Num() / (44100.0f * 1 * 2); // Assuming 16-bit samples
    
    return SoundWave;
}

TArray<FBlendshapeFrame> UMetaHumanStreamingReceiver::ParseBlendshapeData(const FString& BlendshapeJSON)
{
    TArray<FBlendshapeFrame> BlendshapeFrames;
    
    // Parse JSON string
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BlendshapeJSON);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse blendshape JSON data"));
        return BlendshapeFrames;
    }
    
    // Extract blendshape frames from JSON
    TArray<TSharedPtr<FJsonValue>> FramesArray = JsonObject->GetArrayField(TEXT("frames"));
    for (int32 i = 0; i < FramesArray.Num(); i++)
    {
        TSharedPtr<FJsonObject> FrameObject = FramesArray[i]->AsObject();
        
        FBlendshapeFrame Frame;
        Frame.FrameNumber = FrameObject->GetIntegerField(TEXT("frame"));
        
        // Extract blendshape values
        TSharedPtr<FJsonObject> BlendshapesObject = FrameObject->GetObjectField(TEXT("blendshapes"));
        for (auto& Pair : BlendshapesObject->Values)
        {
            Frame.BlendshapeValues.Add(Pair.Key, Pair.Value->AsNumber());
        }
        
        BlendshapeFrames.Add(Frame);
    }
    
    return BlendshapeFrames;
}

void UMetaHumanStreamingReceiver::ApplyBlendshapesToMesh(const TMap<FString, float>& BlendshapeValues)
{
    if (!MetaHumanMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("MetaHuman mesh component not set"));
        return;
    }
    
    // Apply each blendshape value to the mesh
    for (auto& Pair : BlendshapeValues)
    {
        // In a real implementation, you would map the blendshape names to the
        // corresponding morph target names in the MetaHuman mesh
        FString MorphTargetName = Pair.Key;
        float Value = Pair.Value;
        
        // Set morph target value
        MetaHumanMeshComponent->SetMorphTarget(*MorphTargetName, Value);
    }
}

void UMetaHumanStreamingReceiver::StartAnimation()
{
    if (!CurrentAnimationData.AudioData || CurrentAnimationData.BlendshapeFrames.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start animation: Invalid animation data"));
        return;
    }
    
    // Set up audio component
    AudioComponent->SetSound(CurrentAnimationData.AudioData);
    
    // Reset animation state
    CurrentFrame = 0;
    AnimationTime = 0.0f;
    bIsAnimating = true;
    
    // Start audio playback
    AudioComponent->Play();
    
    UE_LOG(LogTemp, Log, TEXT("Started animation with %d blendshape frames"), CurrentAnimationData.BlendshapeFrames.Num());
}

void UMetaHumanStreamingReceiver::StopAnimation()
{
    if (bIsAnimating)
    {
        // Stop audio playback
        AudioComponent->Stop();
        
        // Reset animation state
        bIsAnimating = false;
        CurrentFrame = 0;
        AnimationTime = 0.0f;
        
        // Reset all blendshapes to zero
        if (MetaHumanMeshComponent)
        {
            TArray<FName> MorphTargetNames;
            MetaHumanMeshComponent->GetAllMorphTargetNames(MorphTargetNames);
            
            for (const FName& MorphTargetName : MorphTargetNames)
            {
                MetaHumanMeshComponent->SetMorphTarget(MorphTargetName, 0.0f);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Stopped animation"));
    }
}

void UMetaHumanStreamingReceiver::UpdateAnimation(float DeltaTime)
{
    // Update animation time
    AnimationTime += DeltaTime;
    
    // Check if animation has finished
    if (AnimationTime >= CurrentAnimationData.Duration)
    {
        StopAnimation();
        return;
    }
    
    // Calculate current frame based on time and frame rate
    int32 TargetFrame = FMath::FloorToInt(AnimationTime * FrameRate);
    
    // Apply blendshapes for the current frame
    if (TargetFrame != CurrentFrame && TargetFrame < CurrentAnimationData.BlendshapeFrames.Num())
    {
        CurrentFrame = TargetFrame;
        ApplyBlendshapesToMesh(CurrentAnimationData.BlendshapeFrames[CurrentFrame].BlendshapeValues);
    }
}

void UMetaHumanStreamingReceiver::OnWebSocketConnected()
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket connected"));
}

void UMetaHumanStreamingReceiver::OnWebSocketConnectionError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("WebSocket connection error: %s"), *Error);
}

void UMetaHumanStreamingReceiver::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket closed: %d, %s, Clean: %s"), 
        StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false"));
}

void UMetaHumanStreamingReceiver::OnWebSocketMessage(const FString& Message)
{
    // Parse the message as JSON
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse WebSocket message as JSON"));
        return;
    }
    
    // Extract audio and blendshape data
    FString AudioBase64 = JsonObject->GetStringField(TEXT("audio_base64"));
    FString BlendshapeData = JsonObject->GetObjectField(TEXT("blendshapes"))->ToJsonString();
    
    // Process the received data
    ProcessReceivedData(AudioBase64, BlendshapeData);
}

void UMetaHumanStreamingReceiver::OnHTTPResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
    if (!bSucceeded || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP request failed"));
        return;
    }
    
    // Parse the response as JSON
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse HTTP response as JSON"));
        return;
    }
    
    // Extract audio and blendshape data
    FString AudioBase64 = JsonObject->GetStringField(TEXT("audio_base64"));
    FString BlendshapeData = JsonObject->GetObjectField(TEXT("blendshapes"))->ToJsonString();
    
    // Process the received data
    ProcessReceivedData(AudioBase64, BlendshapeData);
}
