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
