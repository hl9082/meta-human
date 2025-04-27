"""
MetaHuman Backend Server

FastAPI backend for the MetaHuman Interactive Streaming application.
Supports chat, TTS, blendshapes, and Unreal Engine interactions.

Endpoints:
- GET /                  : Health/info check for main backend
- GET /health            : Health specifically for monitoring
- GET /api/chat          : Info/help for chat API endpoint (browser-friendly!)
- POST /api/chat         : Full chat pipeline (LLM, TTS, blendshapes, Unreal)
- GET /api/tts           : Demo TTS endpoint (text passed via ?text= param)
"""

from fastapi import FastAPI, HTTPException, Request, Query
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import List, Dict, Any, Optional
import httpx
import base64
import os
from dotenv import load_dotenv
import logging

load_dotenv()

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(title="MetaHuman Backend")

# Enable CORS for demo
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], allow_credentials=True,
    allow_methods=["*"], allow_headers=["*"],
)

# ====== Models ======
class Message(BaseModel):
    role: str
    content: str

class ChatRequest(BaseModel):
    message: str
    messages: List[Message]

# ====== Chat endpoints ======
@app.post("/api/chat")
async def chat(request: ChatRequest):
    """
    Full processing pipeline: LLM → TTS → blendshapes → Unreal Engine
    """
    try:
        llm_response = await generate_llm_response(request.messages)
        audio_data = await text_to_speech(llm_response)
        blendshapes = await audio_to_blendshapes(audio_data)
        await send_to_unreal_engine(audio_data, blendshapes)
        return {"response": llm_response}
    except Exception as e:
        logger.error(f"Error in chat endpoint: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/chat")
async def chat_info():
    """Browser-friendly GET for /api/chat."""
    return {
        "info": "POST to this endpoint with {'message': str, 'messages': [role/content]} to start a chat pipeline."
    }

# ====== TTS demo endpoint ======
@app.get("/api/tts")
async def tts_demo(text: str = Query(..., description="Text to synthesize")):
    """
    GET /api/tts?text=Hello
    Returns base64-encoded audio bytes for quick testing.
    """
    if not text:
        return {"error": "Missing 'text' query parameter."}
    data = await text_to_speech(text)
    data_b64 = base64.b64encode(data).decode("utf-8")
    return {"audio_base64": data_b64}

# ====== Health endpoints ======
@app.get("/")
async def root():
    return {
        "message": "MetaHuman backend is running!",
        "endpoints": ["/api/chat (POST)", "/api/tts (GET)", "/health (GET)"]
    }

@app.get("/health")
async def health_check():
    return {"status": "healthy"}

@app.get("/api")
async def api_root():
    return {"message": "Welcome to the MetaHuman API! Available: /api/chat (POST), /api/chat (GET for info), /api/tts (GET)", "status": "ok"}


# ====== Helper Functions ======
async def generate_llm_response(messages: List[Message]) -> str:
    try:
        api_key = os.getenv("OPENAI_API_KEY")
        if not api_key:
            raise ValueError("OPENAI_API_KEY environment variable not set")
        formatted_messages = [{"role": msg.role, "content": msg.content} for msg in messages]
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
        logger.error(f"Error generating LLM response: {e}")
        raise

async def text_to_speech(text: str) -> bytes:
    try:
        api_key = os.getenv("TTS_API_KEY")
        if not api_key:
            logger.warning("TTS_API_KEY not set, returning dummy audio data")
            return b"DUMMY_AUDIO_DATA"
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
        logger.error(f"Error in text-to-speech: {e}")
        raise

async def audio_to_blendshapes(audio_data: bytes) -> list:
    try:
        api_key = os.getenv("NEUROSYNC_API_KEY")
        if not api_key:
            logger.warning("NEUROSYNC_API_KEY not set, returning dummy blendshape data")
            return [{"frame": 0, "blendshapes": {"mouthOpen": 0.5}}]
        audio_base64 = base64.b64encode(audio_data).decode("utf-8")
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
        logger.error(f"Error generating blendshapes: {e}")
        raise

async def send_to_unreal_engine(audio_data: bytes, blendshapes: list) -> None:
    try:
        unreal_api_endpoint = os.getenv("UNREAL_API_ENDPOINT")
        if not unreal_api_endpoint:
            logger.warning("UNREAL_API_ENDPOINT not set; would send data to Unreal Engine")
            return
        audio_base64 = base64.b64encode(audio_data).decode("utf-8")
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
        logger.error(f"Error sending to Unreal Engine: {e}")
        raise

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)