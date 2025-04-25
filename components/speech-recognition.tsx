/**
 * SpeechRecognition Component
 *
 * This component handles speech recognition functionality using the Web Speech API.
 * It captures user's spoken input and converts it to text, which is then passed
 * to the parent component for processing.
 *
 * Libraries used:
 * - React: Core library for building the UI component
 * - useEffect: React hook for handling side effects like initialization and cleanup
 * - useRef: React hook for maintaining references across renders
 * - Web Speech API: Browser API for speech recognition (window.SpeechRecognition or window.webkitSpeechRecognition)
 *
 * @param {boolean} isListening - Whether speech recognition should be active
 * @param {Function} onListen - Callback function that receives listening status updates
 * @param {Function} onTranscript - Callback function that receives the transcribed text
 */
"use client"

import { useEffect, useRef } from "react"

// Define the SpeechRecognition interface for TypeScript
// This extends the Window interface to include the SpeechRecognition API
declare global {
  interface Window {
    SpeechRecognition: any
    webkitSpeechRecognition: any
  }
}

/*
Properties of Speech Recognition
@param isListening check if the window is listening
@function onListen
@function onTranscript
*/
interface SpeechRecognitionProps {
  isListening: boolean
  onListen: (listening: boolean) => void
  onTranscript: (text: string) => void
}

export default function SpeechRecognition({ isListening, onListen, onTranscript }: SpeechRecognitionProps) {
  // recognitionRef: Reference to the SpeechRecognition instance
  const recognitionRef = useRef<any>(null)

  // isInitialized: Flag to ensure initialization only happens once
  const isInitialized = useRef(false)

  useEffect(() => {
    // Check if browser supports speech recognition
    // The Web Speech API is not fully standardized, so we need to check for both
    // the standard and webkit-prefixed versions
    if (!("webkitSpeechRecognition" in window) && !("SpeechRecognition" in window)) {
      console.error("Speech recognition not supported in this browser")
      return
    }

    // Initialize speech recognition if not already done
    if (!isInitialized.current) {
      // Get the appropriate SpeechRecognition constructor based on browser support
      const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition
      recognitionRef.current = new SpeechRecognition()

      // Configure speech recognition
      recognitionRef.current.continuous = false // Don't keep recognizing after results
      recognitionRef.current.interimResults = false // Only return final results
      recognitionRef.current.lang = "en-US" // Set language to English (US)

      // Event handlers for the SpeechRecognition instance

      // onstart: Fired when the speech recognition service has begun listening
      recognitionRef.current.onstart = () => {
        onListen(true)
      }

      // onend: Fired when the speech recognition service has disconnected
      recognitionRef.current.onend = () => {
        onListen(false)
      }

      // onresult: Fired when the speech recognition service returns a result
      // The event contains the transcribed text
      recognitionRef.current.onresult = (event: any) => {
        const transcript = event.results[0][0].transcript
        onTranscript(transcript)
      }

      // onerror: Fired when an error occurs in the speech recognition service
      recognitionRef.current.onerror = (event: any) => {
        console.error("Speech recognition error:", event.error)
        onListen(false)
      }

      isInitialized.current = true
    }

    // Start or stop recognition based on isListening prop
    if (isListening) {
      try {
        recognitionRef.current.start()
      } catch (error) {
        console.error("Error starting speech recognition:", error)
      }
    } else if (recognitionRef.current) {
      try {
        recognitionRef.current.stop()
      } catch (error) {
        console.error("Error stopping speech recognition:", error)
      }
    }

    // Cleanup function that runs when the component unmounts or when dependencies change
    return () => {
      if (recognitionRef.current) {
        try {
          recognitionRef.current.stop()
        } catch (error) {
          // Ignore errors on cleanup
        }
      }
    }
  }, [isListening, onListen, onTranscript])

  // This component doesn't render anything visible
  return null
}
