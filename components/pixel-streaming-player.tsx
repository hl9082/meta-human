/**
 * PixelStreamingPlayer Component
 *
 * This component provides a placeholder for the Pixel Streaming functionality.
 * In a production environment, this would connect to an Unreal Engine Pixel Streaming server.
 * For now, it displays a demo interface to allow testing of other application features.
 */
"use client"

import { useEffect, useRef, useState } from "react"

interface PixelStreamingPlayerProps {
  onConnectionStatus: (status: boolean) => void
}

export default function PixelStreamingPlayer({ onConnectionStatus }: PixelStreamingPlayerProps) {
  const containerRef = useRef<HTMLDivElement>(null)
  const [connectionStatus, setConnectionStatus] = useState<
    "idle" | "connecting" | "connected" | "disconnected" | "demo"
  >("idle")
  const [loadError, setLoadError] = useState<string | null>(null)
  const isInitialized = useRef(false)

  useEffect(() => {
    // Only initialize once
    if (isInitialized.current) return
    isInitialized.current = true

    console.log("Initializing PixelStreamingPlayer in demo mode")

    // Simulate connecting to Pixel Streaming server
    setConnectionStatus("connecting")

    // Check if the environment variable is set
    const pixelStreamingUrl = "http://localhost:8888"
    if (!pixelStreamingUrl) {
      setLoadError("NEXT_PUBLIC_PIXEL_STREAMING_URL environment variable is not set")
    }

    // Simulate connection delay
    const timer = setTimeout(() => {
      console.log("Connected to Pixel Streaming server (demo mode)")
      setConnectionStatus("demo")
      onConnectionStatus(true)
    }, 1500)

    // Cleanup function
    return () => {
      clearTimeout(timer)
      setConnectionStatus("disconnected")
      onConnectionStatus(false)
    }
  }, [onConnectionStatus])

  return (
    <div ref={containerRef} className="w-full h-full bg-black">
      <div className="w-full h-full flex flex-col items-center justify-center text-white">
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

        {/* Demo content - replace with actual Pixel Streaming content in production */}
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
                  <path fillRule="evenodd" d="M10 9a3 3 0 100-6 3 3 0 000 6zm-7 9a7 7 0 1114 0H3z" clipRule="evenodd" />
                </svg>
              </div>
            </div>
            <p className="mt-4 text-center">MetaHuman Avatar (Demo)</p>
          </div>
        )}

        {/* Loading indicator */}
        {connectionStatus === "connecting" && (
          <div className="mt-8 flex flex-col items-center">
            <div className="w-8 h-8 border-4 border-blue-500 border-t-transparent rounded-full animate-spin"></div>
            <p className="mt-4">Connecting to Pixel Streaming server...</p>
          </div>
        )}

        {/* Error message */}
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
