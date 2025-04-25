/**
 * PixelStreamingCustomHandler.cpp
 * 
 * Implementation of the UPixelStreamingCustomHandler class, which extends
 * the Pixel Streaming functionality to handle custom messages from the frontend.
 */

#include "PixelStreamingCustomHandler.h"
#include "MetaHumanStreamingReceiver.h"
#include "PixelStreamingModule.h"
#include "IPixelStreamingModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// Sets default values
UPixelStreamingCustomHandler::UPixelStreamingCustomHandler()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void UPixelStreamingCustomHandler::BeginPlay()
{
    Super::BeginPlay();
    
    // Register custom message handlers
    RegisterCustomMessageHandlers();
}

// Called when the game ends
void UPixelStreamingCustomHandler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // Unregister custom message handlers
    UnregisterCustomMessageHandlers();
}

void UPixelStreamingCustomHandler::SetMetaHumanReceiver(UMetaHumanStreamingReceiver* InReceiver)
{
    MetaHumanReceiver = InReceiver;
}

void UPixelStreamingCustomHandler::RegisterCustomMessageHandlers()
{
    // Get the Pixel Streaming module
    IPixelStreamingModule* PixelStreamingModule = FModuleManager::GetModulePtr<IPixelStreamingModule>("PixelStreaming");
    if (!PixelStreamingModule)
    {
        UE_LOG(LogTemp, Error, TEXT("Pixel Streaming module not found"));
        return;
    }
    
    // Register custom message handler
    PixelStreamingModule->AddCommandHandler(
        TEXT("process_data"),
        [this](const FString& MessageContents) {
            HandleProcessDataMessage(MessageContents);
        }
    );
    
    UE_LOG(LogTemp, Log, TEXT("Registered custom Pixel Streaming message handlers"));
}

void UPixelStreamingCustomHandler::UnregisterCustomMessageHandlers()
{
    // Get the Pixel Streaming module
    IPixelStreamingModule* PixelStreamingModule = FModuleManager::GetModulePtr<IPixelStreamingModule>("PixelStreaming");
    if (!PixelStreamingModule)
    {
        UE_LOG(LogTemp, Error, TEXT("Pixel Streaming module not found"));
        return;
    }
    
    // Unregister custom message handler
    PixelStreamingModule->RemoveCommandHandler(TEXT("process_data"));
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered custom Pixel Streaming message handlers"));
}

void UPixelStreamingCustomHandler::HandleCustomMessage(const FString& MessageType, const FString& MessageContents)
{
    // Handle different message types
    if (MessageType == TEXT("process_data"))
    {
        HandleProcessDataMessage(MessageContents);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unknown custom message type: %s"), *MessageType);
    }
}

void UPixelStreamingCustomHandler::HandleProcessDataMessage(const FString& MessageContents)
{
    if (!MetaHumanReceiver)
    {
        UE_LOG(LogTemp, Error, TEXT("MetaHuman receiver not set"));
        return;
    }
    
    // Parse the message as JSON
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MessageContents);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse process_data message as JSON"));
        return;
    }
    
    // Extract audio and blendshape data
    FString AudioBase64 = JsonObject->GetStringField(TEXT("audio_base64"));
    FString BlendshapeData = JsonObject->GetObjectField(TEXT("blendshapes"))->ToJsonString();
    
    // Forward the data to the MetaHuman receiver
    MetaHumanReceiver->ProcessReceivedData(AudioBase64, BlendshapeData);
    
    UE_LOG(LogTemp, Log, TEXT("Processed data message from frontend"));
}
