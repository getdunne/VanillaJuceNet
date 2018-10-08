/* Copyright (c) 2017-2018 Shane D. Dunne

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "DSPServer.h"

DSP_Server::DSP_Server(StreamingSocket* socket, const String &threadName)
: Thread(threadName)
, clientSocket(socket)
, bEnabled(true)
, nParamMsgs(0)
{
    synthParams.setDefaultValues();
    pSynthSound = new SynthSound(synth);
    pSynthSound->pParams = &synthParams;
    synth.addSound(pSynthSound);

    for (int i = 0; i < 16; ++i)
        synth.addVoice(new SynthVoice());

    synth.setCurrentPlaybackSampleRate(44100);
}

DSP_Server::~DSP_Server()
{
    delete clientSocket;
}

void DSP_Server::run()
{
    bool runOK = ClientLoop();
    clientSocket->close();
    DBG("Closing client: " + getThreadName());
}

bool DSP_Server::ClientLoop()
{
    // Receive until the peer shuts down the connection
    while (!currentThreadShouldExit())
    {
        // Get header
        SendDataHeader hdr;
        int byteCount = clientSocket->read((char*)&hdr, sizeof(SendDataHeader), true);
        if (byteCount < 0) return false;
        if (byteCount == 0)
        {
            DBG("Connection closing...");
            break;
        }
        bool sampleDataPresent = (hdr.frameCount & UINT16_BIT15_MASK) != 0;
        hdr.frameCount &= UINT16_LOW15BITS_MASK;

        //DBG("Header (" + String(sizeof(hdr)) + " bytes) " + String(hdr.frameCount) + " " + String(hdr.midiCount) + " " + String(hdr.timeStamp));

        if (hdr.midiCount > 0)
        {
            // get midi data
            byteCount = ReceiveAndProcessMIDI(hdr.midiCount);
            if (byteCount < 0) return false;
            if (byteCount == 0)
            {
                DBG("Connection closing...");
                break;
            }
        }

        if (hdr.paramCount > 0)
        {
            // midi data follow header
            byteCount = ReceiveAndProcessParamChanges(hdr.paramCount);
            if (byteCount < 0) return false;
            if (byteCount == 0)
            {
                DBG("Connection closing...");
                break;
            }
        }

        if (sampleDataPresent)
        {
            // filter input sample data follow header
            byteCount = ReceiveSamples(hdr.frameCount);
            if (byteCount < 0) return false;
            if (byteCount == 0)
            {
                DBG("Connection closing...");
                break;
            }
        }
        else
        {
            // zero receive buffer again, so we don't interpret MIDI etc. as samples,
            // should the user load a filter VST (or no VST at all) instead of a synth VST
            memset(recvbuf, 0, sizeof(recvbuf));
        }

        // Process the data and send back result
        ProcessSamples(hdr.frameCount, hdr.timeStamp);
        if (!SendSamples(hdr.frameCount)) return false;
    }
    return true;
}

int DSP_Server::ReceiveAndProcessMIDI(int nMessageCount)
{
    //DBG("ReceiveAndProcessMIDI: " + String(nMessageCount));
    int byteCount = clientSocket->read(recvbuf, nMessageCount * sizeof(MIDIMessageInfoStruct), false);
    if (byteCount > 0)
    {
        MIDIMessageInfoStruct* midiMsgInfo = reinterpret_cast<MIDIMessageInfoStruct*>(recvbuf);
        for (int i = 0; i < nMessageCount; i++, midiMsgInfo++)
        {
            unsigned char rawMidi[3];
            rawMidi[0] = midiMsgInfo->status | midiMsgInfo->channel;
            rawMidi[1] = midiMsgInfo->data1;
            rawMidi[2] = midiMsgInfo->data2;
            midiBuffer.addEvent(rawMidi, 3, midiMsgInfo->startFrame);
        }
        //DBG("midiBuffer contains " + String(midiBuffer.getNumEvents()) + " events");
    }
    return byteCount;
}

int DSP_Server::ReceiveAndProcessParamChanges(int nMessageCount)
{
    //DBG("ReceiveAndProcessParamChanges " + String(nMessageCount));
    int byteCount = clientSocket->read(recvbuf, nMessageCount * sizeof(ParamMessageStruct), false);
    if (byteCount > 0)
    {
        memcpy(paramData, recvbuf, nMessageCount * sizeof(ParamMessageStruct));
        nParamMsgs = nMessageCount;
        pPm = paramData;
    }
    return byteCount;
}

int DSP_Server::ReceiveSamples(int nFrameCount)
{
    float* pInLeft = (float*)recvbuf;
    float* pInRight = pInLeft + nFrameCount;
    int byteCount = 0;  // value to return

    // get header for processed sample data
    SampleDataHeader shdr;
    byteCount = clientSocket->read(&shdr, sizeof(shdr), false);
    if (byteCount != sizeof(shdr))
    {
        DBG("1 Expected " + String(sizeof(shdr)) + " bytes, received " + String(byteCount));
        return byteCount;
    }

    int blockBytes = nFrameCount * sizeof(float);
    byteCount = clientSocket->read(pInLeft, blockBytes, false);
    if (byteCount != blockBytes)
    {
        DBG("2 Expected " + String(blockBytes) + " bytes, received " + String(byteCount));
        return byteCount;
    }
    byteCount = clientSocket->read(pInRight, blockBytes, false);
    if (byteCount != blockBytes)
    {
        DBG("2 Expected " + String(blockBytes) + " bytes, received " + String(byteCount));
        return byteCount;
    }

    return 1;   // return any value >0 to indicate success
}

void DSP_Server::ProcessSamples(int nFrameCount, uint32_t timeStamp)
{
    //DBG("ProcessSamples: " + String(timeStamp));
    float* pInLeft = (float*)recvbuf;
    float* pInRight = pInLeft + nFrameCount;
    float* pOutLeft = (float*)sendbuf;
    float* pOutRight = pOutLeft + nFrameCount;

    if (bEnabled)
    {
        float* buffers[4] = { pInLeft, pInRight, pOutLeft, pOutRight };
        render(buffers, nFrameCount);
    }
    else
    {
        memcpy(pOutLeft, pInLeft, nFrameCount * sizeof(float));
        memcpy(pOutRight, pInRight, nFrameCount * sizeof(float));
    }
}

bool DSP_Server::SendSamples(int nFrameCount)
{
    float* pOutLeft = (float*)sendbuf;
    float* pOutRight = pOutLeft + nFrameCount;
    char* pData; int byteCount, bytesToSend;

    SampleDataHeader shdr;
    int blockBytes = nFrameCount * sizeof(float);
    shdr.mainByteCount = (uint16_t)blockBytes;
    shdr.corrByteCount = 0xFFFF;

    bytesToSend = 0;
    pData = sendData;
    byteCount = sizeof(SampleDataHeader);
    memcpy(pData, &shdr, byteCount); pData += byteCount; bytesToSend += byteCount;
    byteCount = blockBytes;
    memcpy(pData, pOutLeft, byteCount); pData += byteCount; bytesToSend += byteCount;
    memcpy(pData, pOutRight, byteCount); pData += byteCount; bytesToSend += byteCount;

    return clientSocket->write(sendData, bytesToSend);
}

void DSP_Server::render(float** buffers, int nFrames)
{
    float* outBuffers[2] = { buffers[2], buffers[3] };
    memset(outBuffers[0], 0, nFrames * sizeof(float));
    memset(outBuffers[1], 0, nFrames * sizeof(float));

    // process parameter changes
    // TODO: allow finer timing and option for ramping
    for (; nParamMsgs > 0; nParamMsgs--, pPm++)
    {
        SynthParameters::ParameterIndex pi = (SynthParameters::ParameterIndex)(pPm->paramIndex);
        if (synthParams.updateParam(pi, pPm->paramValue))
            pSynthSound->parameterChanged();
    }

    AudioBuffer<float> audioBuffer(outBuffers, 2, nFrames);
    synth.renderNextBlock(audioBuffer, midiBuffer, 0, nFrames);
    midiBuffer.clear();
}
