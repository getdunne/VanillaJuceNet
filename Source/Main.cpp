#include "../JuceLibraryCode/JuceHeader.h"
#include "DSPServer.h"
#include <signal.h>
#include <list>

std::unique_ptr<StreamingSocket> hostSocket;
std::list<DSP_Server*> servers;
bool breakForced = false;

void SigBreak_Handler(int n_signal)
{
    breakForced = true;
    while (!servers.empty())
    {
        DSP_Server* server = servers.front();
        server->stopThread(100);
        delete server;
        servers.pop_front();
    }
}

int main (int argc, char* argv[])
{
    signal(SIGINT, &SigBreak_Handler);
    signal(SIGTERM, &SigBreak_Handler);
    //signal(SIGBREAK, &SigBreak_Handler);

    hostSocket.reset(new StreamingSocket());
    while (!breakForced && hostSocket->createListener(DEFAULT_PORT))
    {
        StreamingSocket* clientSocket = hostSocket->waitForNextConnection();
        if (clientSocket == nullptr)
        {
            DBG("Forced shutdown");
            break;
        }
        DBG("Client connected: " + String((int64)clientSocket));
        DSP_Server* server = new DSP_Server(clientSocket, String((int64)clientSocket));
        server->startThread();
        servers.push_back(server);
    }

    if (breakForced) return 0;

    DBG("Unable to perform createListener()");
    return 1;
}
