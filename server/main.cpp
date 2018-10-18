#include <iostream>
#include <regex>
#include <winsock2.h>

using namespace std;

bool IsServerActive;

DWORD WINAPI ServerThread(LPVOID lparam)
{
    SOCKET client = (SOCKET) lparam;
    SOCKADDR_IN clientAddr;
    SOCKADDR *w = (sockaddr*) &clientAddr;

    char msg[1024];
    int q = sizeof(SOCKADDR_IN), x = sizeof(SOCKADDR);
    getsockname(client,w,&x);
    int protocol = (int)w->sa_data[5];//1 - tcp, 0 - udp

    cout << "A new client connected!" << endl;
    while(true)
    {
        memset(msg, 0, sizeof(msg));
        int result = recvfrom(client, msg, sizeof(msg), 0, (sockaddr*) &clientAddr, &q);
        if (result == 0 || strcmp(msg,"exit") == 0){
            cout << "Connection closed." << endl;
            if(protocol) break;
        }
        else if (result == SOCKET_ERROR && protocol == 1){
            cout << WSAGetLastError() << endl;
            break;
        }
        else if (strcmp(msg,"close") == 0 ){
            IsServerActive = false;
            break;
        }
        else{
            cout << "Client says: " << msg << endl;

            string answer(msg);
            regex filter(R"(\D+)"); // replace all except digits
            answer=regex_replace(answer,filter," ");

            vector <int> mas;
            int x = 0;
            stringstream is(answer);

            while(is >> x) mas.push_back(x);
            sort(mas.begin(),mas.end());

            is.str(""); is.clear(); x=0;// reset variables

            for(int i=0;i<mas.size();i++){
                x += mas[i];
                is << mas[i] << " ";
            }
            is << "\n" << x; // add new line and summ
            answer = is.str();

            sendto(client, answer.c_str(), answer.size(), 0, (sockaddr *)&clientAddr, sizeof(clientAddr));
        }
    }
    cout << "Client disconnected." << endl;

    shutdown(client, SD_BOTH);
    closesocket(client);
    return 0;
}

int main()
{
    int N = 0;
    do{
        cout << "Choose the protocol:\n1 - TCP \t 2 - UDP" << endl;
        cin >> N;
        cin.clear(); cin.ignore();
    }while(N!=1 && N!=2);

    WSADATA WSAData;

    SOCKET server, client;
    SOCKADDR_IN serverAddr;

    WSAStartup(MAKEWORD(2,0), &WSAData);

    server = socket(AF_INET, N, (N-1)*11+6); // 1-1=0; 0*11=0; 0+6=6=TCP // 2-1=1; 1*11=11; 11+6=17=UDP

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);

    bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
    listen(server, SOMAXCONN);

    cout << "Listening for incoming connections over protocol #" << N << "..." << endl;
    IsServerActive = true;

    HANDLE recvThread;
    DWORD dwThread;
    while(IsServerActive){
        if(N == 2){
            ServerThread((LPVOID)server);
        }
        if(N == 1 && (client = accept(server, NULL, NULL)) != INVALID_SOCKET){
            if(IsServerActive == false) break;
            recvThread = CreateThread(NULL, 0, ServerThread, (LPVOID)client, 0, &dwThread);
            CloseHandle(recvThread);
        }
    }
    cout << "Server closed." << endl;
    TerminateThread(recvThread, 0);
    WSACleanup();
    return 0;
}
