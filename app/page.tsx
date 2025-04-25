/**
 * Home Page Component
 *
 * This is the main page of the MetaHuman Interactive Streaming application.
 * It integrates all the core components and manages the application state.
 *
 * Libraries used:
 * - React: Core library for building the UI component
 * - useState: React hook for managing component state
 * - @/components/ui/button: UI component for buttons from shadcn/ui
 * - @/components/ui/card: UI component for cards from shadcn/ui
 * - lucide-react: Icon library for UI elements
 * - PixelStreamingPlayer: Custom component for Pixel Streaming
 * - SpeechRecognition: Custom component for speech recognition
 */
"use client"

import { useState } from "react"
import { Button } from "@/components/ui/button"
import { Card } from "@/components/ui/card"
import { Mic, MicOff } from "lucide-react"
import PixelStreamingPlayer from "@/components/pixel-streaming-player"
import SpeechRecognition from "@/components/speech-recognition"

export default function Home() {
  // State management using React's useState hook

  // isListening: Whether speech recognition is currently active
  const [isListening, setIsListening] = useState(false)

  // transcript: The current transcribed text from speech recognition
  const [transcript, setTranscript] = useState("")

  // isConnected: Whether the connection to Pixel Streaming is active
  const [isConnected, setIsConnected] = useState(false)

  // isProcessing: Whether a request is currently being processed
  const [isProcessing, setIsProcessing] = useState(false)

  // messages: The conversation history between the user and the MetaHuman
  const [messages, setMessages] = useState<{ role: string; content: string }[]>([])

  /**
   * handleListen: Handle speech recognition listening state changes
   *
   * This function updates the isListening state based on the listening status
   * from the SpeechRecognition component.
   *
   * @param listening - Whether speech recognition is currently active
   */
  const handleListen = (listening: boolean) => {
    setIsListening(listening)
  }

  /**
   * handleTranscript: Process transcribed speech and send to backend
   *
   * This function:
   * 1. Validates the transcribed text
   * 2. Updates the transcript state
   * 3. Sets the processing state
   * 4. Adds the user message to the conversation history
   * 5. Sends the message to the backend for processing
   * 6. Adds the response to the conversation history
   *
   * @param text - The transcribed text from speech recognition
   */
  const handleTranscript = async (text: string) => {
    if (!text.trim()) return

    setTranscript(text)
    setIsProcessing(true)

    // Add user message to chat
    const updatedMessages = [...messages, { role: "user", content: text }]
    setMessages(updatedMessages)

    try {
      // Send the transcribed text to the backend for processing
      const response = await fetch("/api/chat", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          message: text,
          messages: updatedMessages,
        }),
      })

      if (!response.ok) {
        throw new Error("Failed to process speech")
      }

      const data = await response.json()

      // Add assistant response to chat
      setMessages([...updatedMessages, { role: "assistant", content: data.response }])

      // The backend has already sent the audio and blendshapes to Unreal Engine
      // No need to handle that here as it's done server-side
    } catch (error) {
      console.error("Error processing speech:", error)
    } finally {
      setIsProcessing(false)
    }
  }

  /**
   * handleConnectionStatus: Update connection status based on Pixel Streaming player state
   *
   * This function updates the isConnected state based on the connection status
   * from the PixelStreamingPlayer component.
   *
   * @param status - Whether the connection to Pixel Streaming is active
   */
  const handleConnectionStatus = (status: boolean) => {
    setIsConnected(status)
  }

  return (
    <main className="flex min-h-screen flex-col items-center justify-between p-4 md:p-24 bg-gray-100">
      <div className="w-full max-w-5xl flex flex-col items-center gap-8">
        <h1 className="text-3xl font-bold text-center">MetaHuman Interactive Streaming</h1>

        {/* Pixel Streaming video container */}
        <div className="w-full aspect-video bg-black rounded-lg overflow-hidden relative">
          <PixelStreamingPlayer onConnectionStatus={handleConnectionStatus} />

          {/* Connection status overlay */}
          {!isConnected && (
            <div className="absolute inset-0 flex items-center justify-center bg-black/70 text-white">
              Connecting to Pixel Streaming server...
            </div>
          )}
        </div>

        {/* User interaction area */}
        <div className="w-full flex flex-col gap-4">
          {/* Microphone button */}
          <div className="flex justify-center">
            <Button
              onClick={() => setIsListening(!isListening)}
              className={`rounded-full p-4 ${isListening ? "bg-red-500 hover:bg-red-600" : "bg-blue-500 hover:bg-blue-600"}`}
              disabled={isProcessing || !isConnected}
              aria-label={isListening ? "Stop listening" : "Start listening"}
            >
              {isListening ? <MicOff className="h-6 w-6" /> : <Mic className="h-6 w-6" />}
            </Button>
          </div>

          {/* Conversation history */}
          <Card className="p-4 min-h-[100px] max-h-[300px] overflow-y-auto">
            {messages.map((message, index) => (
              <div key={index} className={`mb-2 ${message.role === "user" ? "text-blue-600" : "text-green-600"}`}>
                <strong>{message.role === "user" ? "You: " : "MetaHuman: "}</strong> {message.content}
              </div>
            ))}
            {isProcessing && <div className="text-gray-500">Processing...</div>}
          </Card>
        </div>

        {/* Speech recognition component (invisible) */}
        <SpeechRecognition isListening={isListening} onListen={handleListen} onTranscript={handleTranscript} />
      </div>
    </main>
  )
}
