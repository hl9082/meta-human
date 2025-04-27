"use client"

import { useEffect, useRef, useState } from "react"

interface PixelStreamingPlayerProps {
  onConnectionStatus: (status: boolean) => void
}



export default function PixelStreamingPlayer({ onConnectionStatus }: PixelStreamingPlayerProps) {
  const [hasMounted, setHasMounted] = useState(false);
useEffect(() => { setHasMounted(true); }, []);
if (!hasMounted) return null;
  const containerRef = useRef<HTMLDivElement>(null)
  const [connectionStatus, setConnectionStatus] = useState<
    "idle" | "connecting" | "connected" | "disconnected" | "demo"
  >("idle")
  const [loadError, setLoadError] = useState<string | null>(null)
  const isInitialized = useRef(false)

  // New state for mic/ASR
  const [recording, setRecording] = useState(false)
  const [transcript, setTranscript] = useState("")
  const [audioLoading, setAudioLoading] = useState(false)
  const mediaRecorderRef = useRef<MediaRecorder | null>(null)
  const audioChunks = useRef<Blob[]>([])

  useEffect(() => {
    if (isInitialized.current) return
    isInitialized.current = true

    setConnectionStatus("connecting")

    const pixelStreamingUrl = process.env.NEXT_PUBLIC_PIXEL_STREAMING_URL;
    if (!pixelStreamingUrl) {
      setLoadError("NEXT_PUBLIC_PIXEL_STREAMING_URL environment variable is not set")
    }

    const timer = setTimeout(() => {
      setConnectionStatus("demo")
      onConnectionStatus(true)
    }, 1500)

    return () => {
      clearTimeout(timer)
      setConnectionStatus("disconnected")
      onConnectionStatus(false)
    }
  }, [onConnectionStatus])

  // --- MICROPHONE/SPEECH TO TEXT LOGIC ---
  const startRecording = async () => {
    setTranscript("")
    const stream = await navigator.mediaDevices.getUserMedia({ audio: true })
    const recorder = new window.MediaRecorder(stream)
    audioChunks.current = []
    recorder.ondataavailable = (e) => {
      audioChunks.current.push(e.data)
    }
    recorder.onstop = async () => {
      const audioBlob = new Blob(audioChunks.current, { type: "audio/webm" })
      const formData = new FormData()
      formData.append("audio", audioBlob, "audio.webm")
      setAudioLoading(true)
      try {
        const backend = process.env.NEXT_PUBLIC_BACKEND_URL // <--- SET IN .env.local
        const res = await fetch(`${backend}/api/asr`, {
          method: "POST",
          body: formData,
        })
        const data = await res.json()
        setTranscript(data.text || data.detail || "Unable to transcribe")
      } catch (e: any) {
        setTranscript("API error: " + e.message)
      }
      setAudioLoading(false)
    }
    recorder.start()
    mediaRecorderRef.current = recorder
    setRecording(true)
  }

  const stopRecording = () => {
    mediaRecorderRef.current?.stop()
    setRecording(false)
  }

  // --- END MIC/ASR LOGIC ---

  return (
    <div ref={containerRef} className="w-full h-full bg-black">
      <div className="w-full h-full flex flex-col items-center justify-center text-white">
        {/* ... original header ... */}
        <p className="font-bold text-lg">
          MetaHuman Stream &middot;{" "}
          <span
            className={
              connectionStatus === "connected"
                ? "text-green-400"
                : connectionStatus === "demo"
                ? "text-yellow-300"
                : connectionStatus === "connecting"
                ? "text-blue-200"
                : "text-red-400"
            }
          >
            {connectionStatus === "connected"
              ? "Connected"
              : connectionStatus === "demo"
              ? "Demo Mode"
              : connectionStatus === "connecting"
              ? "Connecting..."
              : "Disconnected"}
          </span>
        </p>

        {/* Demo avatar */}
        {(connectionStatus === "demo" || connectionStatus === "connected") && (
          <div className="mt-8 flex flex-col items-center">
            <div className="w-64 h-64 bg-gray-800 rounded-full overflow-hidden relative">
              <div className="absolute inset-0 flex items-center justify-center">
                <svg
                  className="w-32 h-32 text-gray-600"
                  fill="currentColor"
                  viewBox="0 0 20 20"
                  xmlns="http://www.w3.org/2000/svg"
                >
                  <path
                    fillRule="evenodd"
                    d="M10 9a3 3 0 100-6 3 3 0 000 6zm-7 9a7 7 0 1114 0H3z"
                    clipRule="evenodd"
                  />
                </svg>
              </div>
            </div>
            <p className="mt-4 text-center">MetaHuman Avatar (Demo)</p>

            {/* === Microphone/Speech-to-Text UI === */}
            <div className="mt-4 w-full flex flex-col items-center">
              <button
                className={`px-4 py-2 rounded ${
                  recording ? "bg-red-600" : "bg-blue-600"
                }`}
                onClick={recording ? stopRecording : startRecording}
                disabled={audioLoading}
              >
                {recording ? "Stop & Transcribe" : "Start Recording"}
              </button>
              <div className="mt-3 min-h-[2em] text-lg">
                {audioLoading
                  ? "Transcribing..."
                  : transcript && <span>Transcript: <span className="font-mono">{transcript}</span></span>}
              </div>
            </div>
            {/* === END MICROPHONE UI === */}
          </div>
        )}

        {/* Loading indicator */}
        {connectionStatus === "connecting" && (
          <div className="mt-8 flex flex-col items-center">
            <div className="w-8 h-8 border-4 border-blue-500 border-t-transparent rounded-full animate-spin"></div>
            <p className="mt-4">Connecting to Pixel Streaming server...</p>
          </div>
        )}

        {loadError && (
          <div className="mt-4 p-4 bg-red-800/50 rounded-md max-w-md text-center">
            <p className="text-sm font-mono break-all">Error: {loadError}</p>
            <p className="mt-2 text-xs">Running in demo mode</p>
          </div>
        )}

        <div className="mt-5 text-xs opacity-60">
          <p>
            <b>Note:</b> This is a demo mode. In production, this would display the actual MetaHuman stream.
          </p>
          <p>
            <b>Env:</b> <span className="font-mono">NEXT_PUBLIC_PIXEL_STREAMING_URL</span>
            {process.env.NEXT_PUBLIC_PIXEL_STREAMING_URL && (
              <span className="font-mono"> = {process.env.NEXT_PUBLIC_PIXEL_STREAMING_URL}</span>
            )}
          </p>
        </div>
      </div>
    </div>
  )
}