#pragma once
#include <stdint.h>
#define JUCE_DONT_DECLARE_PROJECTINFO 1
#include "../JuceLibraryCode/JuceHeader.h"

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

class DSP_Server
{
protected:
    StreamingSocket* clientSocket;
    bool volatile bCancel;                      // cancel (stop) flag
    bool volatile bRunning;                     // true while running
    bool bEnabled;                              // if false, thread should essentially do nothing

public:
    DSP_Server(StreamingSocket* socket);
    ~DSP_Server();
    void Enable() { bEnabled = true; }
    void Disable() { bEnabled = false; }
    bool ClientLoop();
    void ForceShutdown() { bCancel = true; while (bRunning) {} }

protected:
    char recvbuf[BUFLEN], sendbuf[BUFLEN];
    char sendData[sizeof(SampleDataHeader) + BUFLEN];
    MidiBuffer midiBuffer;
    //MIDIMessageInfoStruct midiData[1000], *pMm;
    //int nMidiMsgs;
    ParamMessageStruct paramData[500], *pPm;
    int nParamMsgs;

    int ReceiveAndProcessMIDI(int nMessageCount);  // return bytecount, or 0 if client disconnected, <0 for error
    int ReceiveAndProcessParamChanges(int nMessageCount);  // return bytecount, or 0 on disconnect, <0 on errr
    int ReceiveSamples(int nFrameCount);           // return bytecount, or 0 if client disconnected, <0 for error
    void ProcessSamples(int nFrameCount, uint32_t timeStamp);
    bool SendSamples(int nFrameCount);   // return false only if socket send failed
    void render(float** buffers, int nFrames);
};
