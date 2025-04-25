"""
MetaHuman Backend Server

This FastAPI application serves as the backend for the MetaHuman Interactive Streaming application.
It processes user input, generates responses using an LLM, converts text to speech,
generates facial blendshapes, and sends the data to Unreal Engine.

Libraries used:
- fastapi: Web framework for building APIs with Python
- pydantic: Data validation and settings management
- httpx: Asynchronous HTTP client
- base64: Encoding and decoding binary data
- json: JSON parsing and serialization
- os: Operating system interface for environment variables
- dotenv: Loading environment variables from .env files
- logging: Logging facility for Python

The server provides the following endpoints:
- POST /api/chat: Process user messages and generate responses
- GET /health: Health check endpoint

Environment variables:
- OPENAI_API_KEY: API key for OpenAI
- TTS_API_KEY: API key for the text-to-speech service
- NEUROSYNC_API_KEY: API key for the Neurosync API
- UNREAL_API_ENDPOINT: Endpoint for the Unreal Engine API
"""

from fastapi import FastAPI, HTTPException, Request
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import List, Dict, Any, Optional
import httpx
import base64
import json
import os
from dotenv import load_dotenv
import logging

# Load environment variables from .env file
# This allows configuration via environment variables without hardcoding sensitive data
load_dotenv()

# Configure logging
# This sets up the logging system to output messages at INFO level and above
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Create FastAPI application instance
app = FastAPI(title="MetaHuman Backend")

# Add CORS middleware
# This allows the frontend to make requests to the backend from a different origin
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # In production, replace with specific origins
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Models
class Message(BaseModel):
    """
    Represents a message in a conversation.
    
    Pydantic model for data validation and serialization.
    
    Attributes:
        role (str): The role of the message sender (e.g., "user" or "assistant")
        content (str): The content of the message
    """
    role: str
    content: str

class ChatRequest(BaseModel):
    """
    Request model for the chat endpoint.
    
    Pydantic model for data validation and serialization.
    
    Attributes:
        message (str): The current message from the user
        messages (List[Message]): The conversation history
    """
    message: str
    messages: List[Message]

class AudioToBlendshapeRequest(BaseModel):
    """
    Request model for the audio-to-blendshape endpoint.
    
    Pydantic model for data validation and serialization.
    
    Attributes:
        audio_base64 (str): Base64-encoded audio data
    """
    audio_base64: str

class UnrealEngineRequest(BaseModel):
    """
    Request model for sending data to Unreal Engine.
    
    Pydantic model for data validation and serialization.
    
    Attributes:
        audio_base64 (str): Base64-encoded audio data
        blendshapes (List[Dict[str, Any]]): Blendshape data for facial animation
    """
    audio_base64: str
    blendshapes: List[Dict[str, Any]]

# API endpoints
@app.post("/api/chat")
async def chat(request: ChatRequest):
    """
    Process a chat request and generate a response.
    
    This endpoint:
    1. Generates a response using an LLM
    2. Converts the response to speech
    3. Generates blendshapes from the audio
    4. Sends the audio and blendshapes to Unreal Engine
    
    Args:
        request (ChatRequest): The chat request containing the message and conversation history
        
    Returns:
        dict: A dictionary containing the generated response
        
    Raises:
        HTTPException: If an error occurs during processing
    """
    try:
        # 1. Generate response using LLM
        llm_response = await generate_llm_response(request.messages)
        
        # 2. Convert text to speech
        audio_data = await text_to_speech(llm_response)
        
        # 3. Generate blendshapes from audio
        blendshapes = await audio_to_blendshapes(audio_data)
        
        # 4. Send audio and blendshapes to Unreal Engine
        await send_to_unreal_engine(audio_data, blendshapes)
        
        return {"response": llm_response}
    
    except Exception as e:
        logger.error(f"Error in chat endpoint: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))

# Helper functions
async def generate_llm_response(messages: List[Message]) -> str:
    """
    Generate a response using an LLM API.
    
    This function:
    1. Gets the OpenAI API key from environment variables
    2. Formats the messages for the OpenAI API
    3. Makes an API call to OpenAI
    4. Extracts and returns the generated response
    
    Args:
        messages (List[Message]): The conversation history
        
    Returns:
        str: The generated response
        
    Raises:
        ValueError: If the API key is not set
        HTTPException: If an error occurs during the API call
    """
    try:
        # Get API key from environment
        api_key = os.getenv("OPENAI_API_KEY")
        if not api_key:
            raise ValueError("OPENAI_API_KEY environment variable not set")
        
        # Prepare messages for OpenAI API
        formatted_messages = [{"role": msg.role, "content": msg.content} for msg in messages]
        
        # Call OpenAI API using httpx for async HTTP requests
        async with httpx.AsyncClient() as client:
            response = await client.post(
                "https://api.openai.com/v1/chat/completions",
                headers={
                    "Authorization": f"Bearer {api_key}",
                    "Content-Type": "application/json"
                },
                json={
                    "model": "gpt-4",
                    "messages": formatted_messages,
                    "temperature": 0.7,
                    "max_tokens": 150
                },
                timeout=30.0
            )
            
            if response.status_code != 200:
                logger.error(f"OpenAI API error: {response.text}")
                raise HTTPException(status_code=response.status_code, detail="LLM API error")
            
            result = response.json()
            return result["choices"][0]["message"]["content"]
    
    except Exception as e:
        logger.error(f"Error generating LLM response: {str(e)}")
        raise

async def text_to_speech(text: str) -> bytes:
    """
    Convert text to speech using a TTS API.
    
    This function:
    1. Gets the TTS API key from environment variables
    2. Makes an API call to a TTS service (e.g., ElevenLabs)
    3. Returns the audio data
    
    Args:
        text (str): The text to convert to speech
        
    Returns:
        bytes: The audio data
        
    Raises:
        HTTPException: If an error occurs during the API call
    """
    try:
        # Get API key from environment
        api_key = os.getenv("TTS_API_KEY")
        if not api_key:
            # For demo, return dummy audio data
            logger.warning("TTS_API_KEY not set, returning dummy audio data")
            return b"DUMMY_AUDIO_DATA"
        
        # Call TTS API (example using ElevenLabs)
        async with httpx.AsyncClient() as client:
            response = await client.post(
                "https://api.elevenlabs.io/v1/text-to-speech/voice_id",
                headers={
                    "xi-api-key": api_key,
                    "Content-Type": "application/json"
                },
                json={
                    "text": text,
                    "model_id": "eleven_monolingual_v1",
                    "voice_settings": {
                        "stability": 0.5,
                        "similarity_boost": 0.5
                    }
                },
                timeout=30.0
            )
            
            if response.status_code != 200:
                logger.error(f"TTS API error: {response.text}")
                raise HTTPException(status_code=response.status_code, detail="TTS API error")
            
            return response.content
    
    except Exception as e:
        logger.error(f"Error in text-to-speech: {str(e)}")
        raise

async def audio_to_blendshapes(audio_data: bytes) -> List[Dict[str, Any]]:
    """
    Generate blendshapes from audio using Neurosync API.
    
    This function:
    1. Gets the Neurosync API key from environment variables
    2. Encodes the audio data to base64
    3. Makes an API call to the Neurosync API
    4. Returns the generated blendshapes
    
    Args:
        audio_data (bytes): The audio data
        
    Returns:
        List[Dict[str, Any]]: The generated blendshapes
        
    Raises:
        HTTPException: If an error occurs during the API call
    """
    try:
        # Get API key from environment
        api_key = os.getenv("NEUROSYNC_API_KEY")
        if not api_key:
            # For demo, return dummy blendshape data
            logger.warning("NEUROSYNC_API_KEY not set, returning dummy blendshape data")
            return [{"frame": 0, "blendshapes": {"mouthOpen": 0.5}}]
        
        # Encode audio data to base64
        audio_base64 = base64.b64encode(audio_data).decode("utf-8")
        
        # Call Neurosync API
        async with httpx.AsyncClient() as client:
            response = await client.post(
                "https://api.neurosync.ai/audio-to-face",
                headers={
                    "Authorization": f"Bearer {api_key}",
                    "Content-Type": "application/json"
                },
                json={
                    "audio_base64": audio_base64,
                    "model": "NEUROSYNC_Audio_To_Face_Blendshape"
                },
                timeout=30.0
            )
            
            if response.status_code != 200:
                logger.error(f"Neurosync API error: {response.text}")
                raise HTTPException(status_code=response.status_code, detail="Neurosync API error")
            
            result = response.json()
            return result["blendshapes"]
    
    except Exception as e:
        logger.error(f"Error generating blendshapes: {str(e)}")
        raise

async def send_to_unreal_engine(audio_data: bytes, blendshapes: List[Dict[str, Any]]) -> None:
    """
    Send audio and blendshapes to Unreal Engine.
    
    This function:
    1. Gets the Unreal Engine API endpoint from environment variables
    2. Encodes the audio data to base64
    3. Makes an API call to the Unreal Engine endpoint
    
    Args:
        audio_data (bytes): The audio data
        blendshapes (List[Dict[str, Any]]): The blendshape data
        
    Raises:
        HTTPException: If an error occurs during the API call
    """
    try:
        # Get Unreal Engine API endpoint from environment
        unreal_api_endpoint = os.getenv("UNREAL_API_ENDPOINT")
        if not unreal_api_endpoint:
            # For demo, just log that we would send data
            logger.warning("UNREAL_API_ENDPOINT not set, would send data to Unreal Engine")
            return
        
        # Encode audio data to base64
        audio_base64 = base64.b64encode(audio_data).decode("utf-8")
        
        # Send data to Unreal Engine
        async with httpx.AsyncClient() as client:
            response = await client.post(
                unreal_api_endpoint,
                json={
                    "audio_base64": audio_base64,
                    "blendshapes": blendshapes
                },
                timeout=30.0
            )
            
            if response.status_code != 200:
                logger.error(f"Unreal Engine API error: {response.text}")
                raise HTTPException(status_code=response.status_code, detail="Unreal Engine API error")
    
    except Exception as e:
        logger.error(f"Error sending data to Unreal Engine: {str(e)}")
        raise

# Health check endpoint
@app.get("/health")
async def health_check():
    """
    Health check endpoint to verify the server is running.
    
    This endpoint simply returns a status indicating that the server is healthy.
    It can be used by monitoring tools to check if the server is up and running.
    
    Returns:
        dict: A dictionary with the status
    """
    return {"status": "healthy"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
