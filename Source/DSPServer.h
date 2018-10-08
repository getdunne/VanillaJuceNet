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
#pragma once
#include <stdint.h>
#define JUCE_DONT_DECLARE_PROJECTINFO 1
#include "../JuceLibraryCode/JuceHeader.h"
#include "Synth.h"
#include "SynthVoice.h"

#define MAXFRAMES 2048
#define CHANNELS 2
#define DATASIZE sizeof(float)
#define BUFLEN (MAXFRAMES * CHANNELS * DATASIZE)
#define DEFAULT_PORT 27016
#define CHUNKSIZE 16

#pragma pack(push, 1)

#define UINT16_BIT15_MASK       0x8000
#define UINT16_LOW15BITS_MASK   0x7FFF

typedef struct
{
    uint16_t    frameCount;
    uint16_t    midiCount;
    uint16_t    paramCount;
    uint16_t    padding;
    uint32_t    timeStamp;
} SendDataHeader;

typedef struct
{
    uint8_t     status;
    uint8_t     channel;
    uint8_t     data1;
    uint8_t     data2;
    uint32_t    startFrame;
} MIDIMessageInfoStruct;

typedef struct
{
    uint16_t    effectIndex;
    uint16_t    paramIndex;
    float       paramValue;
} ParamMessageStruct;

typedef struct
{
    uint16_t    mainByteCount;      // or total bytecount if uncompressed
    uint16_t    corrByteCount;      // 0 = lossy compression, 0xFFFF = uncompressed
} SampleDataHeader;

#pragma pack(pop)

class DSP_Server : public Thread
{
protected:
    StreamingSocket* clientSocket;
    bool bEnabled;                              // if false, thread should essentially do nothing

public:
    DSP_Server(StreamingSocket* socket, const String &threadName);
    ~DSP_Server();
    void run() override;
    bool ClientLoop();

protected:
    char recvbuf[BUFLEN], sendbuf[BUFLEN];
    char sendData[sizeof(SampleDataHeader) + BUFLEN];
    MidiBuffer midiBuffer;
    //MIDIMessageInfoStruct midiData[1000], *pMm;
    //int nMidiMsgs;
    ParamMessageStruct paramData[500], *pPm;
    int nParamMsgs;

    SynthParameters synthParams;
    Synth synth;
    SynthSound* pSynthSound;

    int ReceiveAndProcessMIDI(int nMessageCount);  // return bytecount, or 0 if client disconnected, <0 for error
    int ReceiveAndProcessParamChanges(int nMessageCount);  // return bytecount, or 0 on disconnect, <0 on errr
    int ReceiveSamples(int nFrameCount);           // return bytecount, or 0 if client disconnected, <0 for error
    void ProcessSamples(int nFrameCount, uint32_t timeStamp);
    bool SendSamples(int nFrameCount);   // return false only if socket send failed
    void render(float** buffers, int nFrames);
};
