# MetaHuman Interactive Streaming Application

This project implements a full pipeline for capturing a user's spoken input, processing it through an LLM → TTS → Blendshape generation pipeline, and streaming a synchronized talking MetaHuman back to the user via Pixel Streaming.

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Prerequisites](#prerequisites)
- [Setup Instructions](#setup-instructions)
  - [Environment Variables](#environment-variables)
  - [Frontend Setup](#frontend-setup)
  - [Backend Setup](#backend-setup)
  - [Unreal Engine Setup](#unreal-engine-setup)
- [Running the Full Demo](#running-the-full-demo)
- [Deployment](#deployment)
  - [Frontend Deployment](#frontend-deployment-vercel)
  - [Backend Deployment](#backend-deployment-serverless)
  - [Unreal Engine Deployment](#unreal-engine-deployment)
- [Technical Details](#technical-details)
  - [Frontend Architecture](#frontend-architecture)
  - [Backend Architecture](#backend-architecture)
  - [Unreal Engine Integration](#unreal-engine-integration)
- [Troubleshooting](#troubleshooting)
- [Known Limitations](#known-limitations)
- [License](#license)

## Architecture Overview

The application consists of three main components:

1. **Frontend (Next.js + TypeScript)**
   - Displays the Pixel Streaming video feed
   - Captures microphone input and converts speech to text
   - Communicates with the backend

2. **Backend (Python + FastAPI)**
   - Processes user input through an LLM to generate responses
   - Converts text to speech using a TTS service
   - Generates facial blendshapes using the Neurosync API
   - Sends audio and blendshape data to Unreal Engine

3. **Unreal Engine (C++ + Pixel Streaming)**
   - Receives audio and blendshape data
   - Applies blendshapes to the MetaHuman model
   - Streams the result back to the frontend

## Prerequisites

- **Node.js 18+** and npm/yarn
- **Python 3.9+**
- **Unreal Engine 5.1+** with Pixel Streaming support
- **MetaHuman Creator** account for creating and exporting MetaHuman characters
- **API keys** for:
  - OpenAI (or other LLM provider)
  - Text-to-Speech service (e.g., ElevenLabs)
  - Neurosync API

## Setup Instructions

### Environment Variables

The application requires several environment variables to function properly. Create the following files:

#### Frontend (.env.local)

Create a `.env.local` file in the root directory of the frontend project with the following variables:

\`\`\`
NEXT_PUBLIC_BACKEND_URL=http://localhost:8000
NEXT_PUBLIC_PIXEL_STREAMING_URL=wss://localhost:8888
OPENAI_API_KEY=your_openai_api_key
\`\`\`

- `NEXT_PUBLIC_BACKEND_URL`: URL of the backend server
- `NEXT_PUBLIC_PIXEL_STREAMING_URL`: URL of the Pixel Streaming server
- `OPENAI_API_KEY`: Your OpenAI API key for the AI SDK

#### Backend (.env)

Create a `.env` file in the backend directory with the following variables:

\`\`\`
OPENAI_API_KEY=your_openai_api_key
TTS_API_KEY=your_tts_api_key
NEUROSYNC_API_KEY=your_neurosync_api_key
UNREAL_API_ENDPOINT=http://localhost:8888/api/receive-data
\`\`\`

- `OPENAI_API_KEY`: Your OpenAI API key
- `TTS_API_KEY`: Your Text-to-Speech API key (e.g., ElevenLabs)
- `NEUROSYNC_API_KEY`: Your Neurosync API key
- `UNREAL_API_ENDPOINT`: Endpoint for sending data to Unreal Engine

### Frontend Setup

1. **Clone the repository**:
   \`\`\`bash
   git clone https://github.com/yourusername/metahuman-streaming-app.git
   cd metahuman-streaming-app
   \`\`\`

2. **Install dependencies**:
   \`\`\`bash
   npm install
   # or
   yarn install
   \`\`\`

3. **Create environment variables**:
   Create a `.env.local` file as described in the [Environment Variables](#environment-variables) section.

4. **Start the development server**:
   \`\`\`bash
   npm run dev
   # or
   yarn dev
   \`\`\`

5. **Access the application**:
   Open your browser and navigate to `http://localhost:3000`

### Backend Setup

1. **Navigate to the backend directory**:
   \`\`\`bash
   cd backend
   \`\`\`

2. **Create a virtual environment**:
   \`\`\`bash
   # On Windows
   python -m venv venv
   venv\Scripts\activate

   # On macOS/Linux
   python3 -m venv venv
   source venv/bin/activate
   \`\`\`

3. **Install dependencies**:
   \`\`\`bash
   pip install -r requirements.txt
   \`\`\`

4. **Create environment variables**:
   Create a `.env` file as described in the [Environment Variables](#environment-variables) section.

5. **Start the FastAPI server**:
   \`\`\`bash
   uvicorn main:app --reload
   \`\`\`

6. **Verify the server is running**:
   Open your browser and navigate to `http://localhost:8000/health`
   You should see a response: `{"status": "healthy"}`

### Unreal Engine Setup

1. **Install Unreal Engine 5.1+**:
   Download and install Unreal Engine 5.1 or later from the Epic Games Launcher.

2. **Create a new Unreal Engine project**:
   - Create a new project using the "Games" template
   - Enable the Pixel Streaming plugin
   - Enable the MetaHuman plugin

3. **Import a MetaHuman character**:
   - Create a MetaHuman character using MetaHuman Creator
   - Export the character to your Unreal Engine project
   - Place the character in your scene

4. **Add the C++ code**:
   - Create a new C++ class in your project
   - Copy the provided C++ files to your project:
     - `MetaHumanStreamingReceiver.h` and `.cpp`
     - `PixelStreamingCustomHandler.h` and `.cpp`
     - `MetaHumanStreamingGameMode.h` and `.cpp`
   - Build the project

5. **Configure the project**:
   - Set the Game Mode to `MetaHumanStreamingGameMode`
   - Configure Pixel Streaming settings in the project settings

6. **Start the Pixel Streaming server**:
   \`\`\`bash
   # Navigate to your Unreal Engine installation directory
   cd Engine/Binaries/ThirdParty/PixelStreaming/WebServers/SignallingWebServer

   # Start the server
   node server.js
   \`\`\`

7. **Launch the Unreal Engine project with Pixel Streaming**:
   \`\`\`bash
   # From the Unreal Engine project directory
   ./YourProject.uproject -PixelStreamingIP=127.0.0.1 -PixelStreamingPort=8888
   \`\`\`

## Running the Full Demo

To run the complete demo, follow these steps in order:

1. **Start the backend server**:
   \`\`\`bash
   cd backend
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   uvicorn main:app --reload
   \`\`\`

2. **Start the Pixel Streaming server**:
   \`\`\`bash
   cd /path/to/UnrealEngine/Engine/Binaries/ThirdParty/PixelStreaming/WebServers/SignallingWebServer
   node server.js
   \`\`\`

3. **Launch the Unreal Engine project**:
   \`\`\`bash
   cd /path/to/your/unreal/project
   ./YourProject.uproject -PixelStreamingIP=127.0.0.1 -PixelStreamingPort=8888
   \`\`\`

4. **Start the frontend application**:
   \`\`\`bash
   cd /path/to/frontend
   npm run dev
   \`\`\`

5. **Access the application**:
   Open your browser and navigate to `http://localhost:3000`

6. **Interact with the MetaHuman**:
   - Allow microphone access when prompted
   - Click the microphone button to start speaking
   - Your speech will be processed and the MetaHuman will respond

## Deployment

### Frontend Deployment (Vercel)

1. **Push your code to a GitHub repository**.

2. **Create a new project on Vercel**:
   - Go to [Vercel](https://vercel.com) and sign in
   - Click "New Project"
   - Import your GitHub repository
   - Configure the project settings

3. **Set environment variables**:
   - Add the environment variables from your `.env.local` file to the Vercel project settings
   - Make sure to set `NEXT_PUBLIC_PIXEL_STREAMING_URL` to your deployed Pixel Streaming server URL

4. **Deploy the project**:
   - Click "Deploy"
   - Wait for the deployment to complete
   - Access your deployed application at the provided URL

### Backend Deployment (Serverless)

#### Option 1: Vercel Serverless Functions

1. **Create a `vercel.json` file** in the root directory:
   \`\`\`json
   {
     "functions": {
       "api/**/*.py": {
         "runtime": "python3.12.6"
       }
     }
   }
   \`\`\`

2. **Adapt the FastAPI code** to work with Vercel serverless functions.

3. **Deploy to Vercel**:
   \`\`\`bash
   vercel
   \`\`\`

#### Option 2: AWS Lambda with API Gateway

1. **Install the AWS CLI** and configure it with your credentials.

2. **Install the Serverless Framework**:
   \`\`\`bash
   npm install -g serverless
   \`\`\`

3. **Create a `serverless.yml` file** in the backend directory:
   \`\`\`yaml
   service: metahuman-backend

   provider:
     name: aws
     runtime: python3.9
     region: us-east-1

   functions:
     app:
       handler: main.handler
       events:
         - http:
             path: /{proxy+}
             method: any
   \`\`\`

4. **Adapt the FastAPI code** to work with AWS Lambda.

5. **Deploy to AWS**:
   \`\`\`bash
   serverless deploy
   \`\`\`

#### Option 3: Google Cloud Run

1. **Build and push the Docker image**:
   \`\`\`bash
   cd backend
   docker build -t gcr.io/your-project/metahuman-backend .
   docker push gcr.io/your-project/metahuman-backend
   \`\`\`

2. **Deploy to Google Cloud Run**:
   \`\`\`bash
   gcloud run deploy metahuman-backend \
     --image gcr.io/your-project/metahuman-backend \
     --platform managed \
     --allow-unauthenticated
   \`\`\`

### Unreal Engine Deployment

1. **Set up a dedicated server** with the following specifications:
   - Windows Server or Linux with GPU support
   - NVIDIA GPU with CUDA support
   - At least 16GB RAM
   - Fast internet connection

2. **Install Unreal Engine** on the server.

3. **Package your Unreal Engine project** for the target platform.

4. **Configure Pixel Streaming** for the server environment.

5. **Start the Pixel Streaming server** and the Unreal Engine application.

6. **Update the environment variables** in your frontend and backend deployments to point to the Unreal Engine server.

## Technical Details

### Frontend Architecture

The frontend is built with Next.js and TypeScript, featuring:

- **Components**:
  - `PixelStreamingPlayer`: Handles the connection to the Pixel Streaming server
  - `SpeechRecognition`: Manages speech recognition using the Web Speech API
  - `Home`: The main page component that integrates all other components

- **API Routes**:
  - `/api/chat`: Processes user messages and generates responses
  - `/api/unreal`: Mock endpoint for communication with Unreal Engine

### Backend Architecture

The Python FastAPI backend handles:

- **API Endpoints**:
  - `POST /api/chat`: Processes chat requests and generates responses
  - `GET /health`: Health check endpoint

- **Services**:
  - `generate_llm_response`: Generates responses using an LLM
  - `text_to_speech`: Converts text to speech
  - `audio_to_blendshapes`: Generates blendshapes from audio
  - `send_to_unreal_engine`: Sends data to Unreal Engine

### Unreal Engine Integration

The Unreal Engine integration consists of:

- **MetaHumanStreamingReceiver**: Receives audio and blendshape data and applies them to the MetaHuman
- **PixelStreamingCustomHandler**: Handles custom messages from the frontend
- **MetaHumanStreamingGameMode**: Sets up the MetaHuman streaming environment

## Troubleshooting

### Frontend Issues

- **"Missing Environment Variables" error**:
  - Make sure you've created a `.env.local` file with the required environment variables
  - Verify that `NEXT_PUBLIC_PIXEL_STREAMING_URL` is correctly set
  - Restart the development server after adding environment variables

- **Microphone not working**:
  - Ensure you've granted microphone permissions in your browser
  - Check that your microphone is working in other applications
  - Try using a different browser (Chrome provides the best support for the Web Speech API)

- **Cannot connect to Pixel Streaming**:
  - Verify that the Pixel Streaming server is running
  - Check that `NEXT_PUBLIC_PIXEL_STREAMING_URL` is correctly set
  - Ensure there are no network issues or firewalls blocking the connection

= **Cannot deploy to Render :(**:
  - Error of loading ASGI app. Could not import module "main".

### Backend Issues

- **Server won't start**:
  - Verify that all dependencies are installed
  - Check that the required environment variables are set
  - Look for error messages in the console

- **API calls failing**:
  - Check that the API keys are valid
  - Verify that the external services (OpenAI, TTS, Neurosync) are available
  - Look for error messages in the server logs

### Unreal Engine Issues

- **Pixel Streaming not working**:
  - Verify that the Pixel Streaming plugin is enabled
  - Check that the Pixel Streaming server is running
  - Ensure the correct command-line arguments are used when launching Unreal Engine

- **MetaHuman not animating**:
  - Verify that the MetaHuman character is properly set up
  - Check that the blendshape names match the morph target names in the MetaHuman
  - Look for error messages in the Unreal Engine logs

## Known Limitations

1. **Latency**: The full pipeline may introduce latency, especially in serverless environments. For production use, consider optimizing each step of the pipeline.

2. **Browser Compatibility**: The Web Speech API used for speech recognition has varying levels of support across browsers. Chrome provides the best support.

3. **API Rate Limits**: Be aware of rate limits for the various APIs used (OpenAI, TTS, Neurosync).

4. **Cost Considerations**: Running this pipeline at scale can incur significant costs due to the multiple API calls and compute resources required.

5. **Unreal Engine Requirements**: The Unreal Engine component requires significant computational resources, especially for high-quality MetaHuman rendering.

## License

[MIT License](LICENSE)
