/**
 * MetaHumanStreamingGameMode.cpp
 * 
 * Implementation of the AMetaHumanStreamingGameMode class, which sets up
 * the MetaHuman streaming environment in Unreal Engine.
 */

#include "MetaHumanStreamingGameMode.h"
#include "MetaHumanStreamingReceiver.h"
#include "PixelStreamingCustomHandler.h"
#include "MetaHumanCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AMetaHumanStreamingGameMode::AMetaHumanStreamingGameMode()
{
    // Set default values
    MetaHumanReceiver = nullptr;
    PixelStreamingHandler = nullptr;
    MetaHumanCharacter = nullptr;
}

void AMetaHumanStreamingGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the MetaHuman character
    InitializeMetaHumanCharacter();
    
    // Initialize the Pixel Streaming environment
    InitializePixelStreaming();
    
    // Connect the various components
    ConnectComponents();
    
    UE_LOG(LogTemp, Log, TEXT("MetaHuman Streaming Game Mode initialized"));
}

void AMetaHumanStreamingGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // Clean up resources if needed
}

void AMetaHumanStreamingGameMode::InitializeMetaHumanCharacter()
{
    // Find the MetaHuman character in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMetaHumanCharacter::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        MetaHumanCharacter = Cast<AMetaHumanCharacter>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("Found MetaHuman character: %s"), *MetaHumanCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No MetaHuman character found in the world"));
    }
}

void AMetaHumanStreamingGameMode::InitializePixelStreaming()
{
    // Create the MetaHuman streaming receiver
    MetaHumanReceiver = NewObject<UMetaHumanStreamingReceiver>(this);
    
    // Create the Pixel Streaming custom handler
    PixelStreamingHandler = NewObject<UPixelStreamingCustomHandler>(this);
    
    UE_LOG(LogTemp, Log, TEXT("Pixel Streaming environment initialized"));
}

void AMetaHumanStreamingGameMode::ConnectComponents()
{
    if (MetaHumanReceiver && MetaHumanCharacter)
    {
        // Set the MetaHuman mesh for the receiver
        MetaHumanReceiver->SetMetaHumanMesh(MetaHumanCharacter->GetMesh());
    }
    
    if (PixelStreamingHandler && MetaHumanReceiver)
    {
        // Set the MetaHuman receiver for the Pixel Streaming handler
        PixelStreamingHandler->SetMetaHumanReceiver(MetaHumanReceiver);
    }
    
    // Initialize WebSocket connection for real-time communication
    if (MetaHumanReceiver)
    {
        // Get the WebSocket URL from environment variables or configuration
        FString WebSocketURL = TEXT("ws://localhost:8000/ws");
        MetaHumanReceiver->InitializeWebSocketConnection(WebSocketURL);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Components connected"));
}
