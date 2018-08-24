#include "../JuceLibraryCode/JuceHeader.h"
#include "DSPServer.h"
#include <signal.h>

std::unique_ptr<StreamingSocket> hostSocket;
DSP_Server* server = 0;
bool breakForced = false;

void SigBreak_Handler(int n_signal)
{
    breakForced = true;
    if (server) server->ForceShutdown();
}

int main (int argc, char* argv[])
{
    signal(SIGINT, &SigBreak_Handler);
    signal(SIGTERM, &SigBreak_Handler);
    signal(SIGBREAK, &SigBreak_Handler);

    hostSocket.reset(new StreamingSocket());
    while (!breakForced && hostSocket->createListener(DEFAULT_PORT))
    {
        StreamingSocket* clientSocket = hostSocket->waitForNextConnection();
        if (clientSocket == nullptr)
        {
            DBG("Forced shutdown");
            break;
        }
        DBG("Client connected!");
        server = new DSP_Server(clientSocket);
        if (!(server->ClientLoop()))
        {
            DBG("ClientLoop returned false");
            //return 1;
        }
        clientSocket->close();
        delete server;
        server = 0;
    }

    if (breakForced) return 0;

    DBG("Unable to perform createListener()");
    return 1;
}
