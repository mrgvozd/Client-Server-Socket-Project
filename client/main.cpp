#include <iostream>
#include <winsock2.h>

using namespace std;

bool cnct(WSADATA &WSAData,SOCKET &server,SOCKADDR_IN &addr, int protocol){
    WSAStartup(MAKEWORD(2,0), &WSAData);

    server = socket(AF_INET, protocol, (protocol-1)*11+6); // 1-1=0; 0*11=0; 0+6=6=TCP // 2-1=1; 1*11=11; 11+6=17=UDP

    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);

    if(connect(server, (SOCKADDR *)&addr, sizeof(addr)) != SOCKET_ERROR) { return true; }
    else { return false; }
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
    SOCKET server;
    SOCKADDR_IN addr;

    if(cnct(WSAData,server,addr,N)){
        cout << "Connected to server via protocol #" << N << endl;
    }
    else {
        cout << "Error while connecting via protocol #" << N << "\nError #" << WSAGetLastError() << endl;
    }

    string msg;
    char answer[1024];

    while(true){
        memset(answer, 0, sizeof(answer));
        getline(cin,msg);

        if(send(server, msg.c_str(), msg.length(), 0) != SOCKET_ERROR) { cout << "Message sent!" << endl; }
//        sendto(server, msg.c_str(), msg.size(), 0, 0, 0);
        else { cout << "SOCKET_ERROR " << endl; }

        if(msg == "exit"){
            shutdown(server, SD_BOTH);
            break;
        }
        if(msg == "close"){
            shutdown(server, SD_BOTH);
            if(N==2) break;
            closesocket(server);
            WSACleanup();
            cnct(WSAData,server,addr,N);
            break;
        }
        if(recv(server, answer, sizeof(answer),0) > 0) { cout << "Server says: " << answer << endl; }
//        recvfrom(server, answer, sizeof(answer), 0, 0, 0);
    }

    cout << "Socket closed.";

    closesocket(server);
    WSACleanup();
    return 0;
}
