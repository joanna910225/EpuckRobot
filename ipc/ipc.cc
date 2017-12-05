/*
 *      Created on: 01/2012
 *          Author: Wenguo Liu (wenguo.liu@brl.ac.uk)
 *
 */
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <netdb.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include "ipc.hh"

namespace IPC{

#define Close(fd) {\
    if(fd>=0){\
    printf("\tclose socket %d (line %d)\n",fd, __LINE__);\
    close(fd);\
    fd=-1;}\
}

#define Shutdown(fd, val){\
    if(fd>=0){\
    printf("\tshutdown socket %d (line %d)\n", fd, __LINE__);\
    int ret=shutdown(fd,SHUT_RDWR);\
    if(ret<0){\
    perror("error");\
    Close(fd);}\
    }\
}


Connection::Connection()
{
    ipc = NULL;
    callback = NULL;
    callback_raw = NULL;
    connected = true;
    BQInit(&txq, txbuffer, IPCTXBUFFERSIZE);
    pthread_mutex_init(&mutex_txq, NULL);
    user_data = NULL;
    transmiting_thread_running = false;
    receiving_thread_running = false;
}
Connection::~Connection()
{
    //clean up
    connected = false;
    Close(sockfds);
}

bool Connection::Start()
{
    pthread_create(&receiving_thread, 0, Receiving, this);
    pthread_create(&transmiting_thread, 0, Transmiting, this);
    return true;
}

void Connection::Disconnect()
{
    connected = false;
    Shutdown(sockfds, 2); //this will stops each connections, so their transmit and receiver thread will quit
}


IPC::IPC()
{
    name = strdup("default");
    sockfd = -1;
    port = 10000;
    host = NULL;
    server = true;
    callback = NULL;
    callback_raw = NULL;
    user_data = NULL;
    monitoring_thread_running = false;
}

IPC::~IPC()
{
    free(name);
    //cleanup
}

bool IPC::Start(uint32_t ip, int p, bool s)
{
    struct in_addr addr;
    addr.s_addr = ip;
    return Start(inet_ntoa(addr), p, s);
}

bool IPC::Start(const char* h, int p, bool s)
{
    if(Running())
    {
        printf("IPC %s is running, can not create another one: connection %d (%d)\n", name, connections.size(), BrokenConnections());
        return false;
    }

    printf("Restart IPC %s to %s:%d\n", name, h, p);
    port = p;
    server = s;
    if(h)
        host=strdup(h);
    else
        host=NULL;

    int broken_connections = RemoveBrokenConnections();
    if(broken_connections >0)
        printf("%s IPC removed %d broken connections\n", name, broken_connections);

    if(connections.size()>0)
    {
        printf("Warning! %s IPC restarted with the following existing connection\n", name);
        for(int i=0;i<connections.size();i++)
            printf("\t connection from %s:%d\n", inet_ntoa(connections[i]->addr.sin_addr), ntohs(connections[i]->addr.sin_port));
    }
    //create monitoring thread
    pthread_create(&monitor_thread, 0, Monitoring, this);
    return true;
}

void * IPC::Monitoring(void * ptr)
{
    IPC* ipc = (IPC*)ptr;

    ipc->monitoring_thread_running = true;

    bool ret=false;
    if(ipc->server)
        ret = ipc->StartServer(ipc->port);
    else
        ret = ipc->ConnectToServer(ipc->host, ipc->port);

    ipc->monitoring_thread_running = false;

    printf("------ exit monitoring thread %s-----\n",ipc->Name());
    return NULL;
}

void * Connection::Receiving(void * p)
{

    Connection * ptr = (Connection*)p;
    printf(" (%d) %s create receiving thread for %s:%d\n",ptr->sockfds, ((IPC*)ptr->ipc)->Name(),inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));

    //main loop, keep reading
    unsigned char rx_buffer[IPCBLOCKSIZE];

    ElolmsgParseInit(&ptr->parseContext, new uint8_t[IPCLOLBUFFERSIZE], IPCLOLBUFFERSIZE);
    ptr->receiving_thread_running = true;

    while(ptr->connected)
    {
        //reading
        memset(rx_buffer, 0, IPCBLOCKSIZE);
        int received = read(ptr->sockfds,rx_buffer, IPCBLOCKSIZE);
        if (received <= 0) 
        {
            printf("ERROR read to socket %d : %d -- it seems connection lost\n",ptr->sockfds, received);
            ptr->connected = false;
            break;
        }
        else
        { 
            if(ptr->callback_raw)
            {
                ptr->callback_raw(rx_buffer, received, ptr, ptr->user_data);
            }
            else
            {
                int parsed = 0;
                while (parsed < received)
                {
                    parsed += ElolmsgParse(&ptr->parseContext, rx_buffer + parsed, received - parsed);
                    ELolMessage* msg = ElolmsgParseDone(&ptr->parseContext);
                    if(msg!=NULL && ptr->callback)
                    {
                   //     printf("received data from %s : %d\n",inet_ntoa(ptr->addr.sin_addr),ntohs(ptr->addr.sin_port));
                        ptr->callback(msg, ptr, ptr->user_data);
                    }
                }
            }
        }
    }

    printf(" (%d) %s exit receiving thread for %s:%d\n",ptr->sockfds, ((IPC*)ptr->ipc)->Name(),  inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));
    ptr->receiving_thread_running = false;
    if(!ptr->transmiting_thread_running)
        Close(ptr->sockfds);


    pthread_exit(NULL);
}

void * Connection::Transmiting(void *p)
{
    Connection * ptr = (Connection*)p;
    printf(" (%d) %s create transmiting thread for  %s:%d\n", ptr->sockfds, ((IPC*)ptr->ipc)->Name(), inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));
    uint8_t txBuf[IPCBLOCKSIZE];

    ptr->transmiting_thread_running = true;

    while(ptr->connected)
    {
        pthread_mutex_lock(&ptr->mutex_txq);
        if(BQCount(&ptr->txq) > 0 )
        {
            register int byteCount = BQPopBytes(&ptr->txq, txBuf, IPCBLOCKSIZE);

            int n = write(ptr->sockfds, txBuf, byteCount);
            if(n<0)
            {
                ptr->connected = false;
                printf("write error %d\n", n);
                break;
            }
        }
        pthread_mutex_unlock(&ptr->mutex_txq);

        usleep(100000);
    }

    printf(" (%d) %s exit transmiting thread for %s:%d\n",ptr->sockfds, ((IPC*)ptr->ipc)->Name(),  inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));
    ptr->transmiting_thread_running = false;

    if(!ptr->receiving_thread_running)
        Close(ptr->sockfds);

    pthread_exit(NULL);
}

bool IPC::StartServer(int port)
{
    printf("Start Server @ port: %d\n", port);
    struct sockaddr_in serv_addr;

    bool binded = false;
    //start server
    while(!binded)
    {
        sockfd = socket(AF_INET, SOCK_STREAM,0);
        if(sockfd <0)
        {
            binded =false;
            continue;
        }

        //set sockfd options
        int flag = 1;
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(int)) == -1)
        {
            perror("Set sock option failed:");
            exit(1);
        }

        memset((char*) &serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        {
            perror("Server socket bind failed:");
            printf("failed on port: %d");
            binded = false;
            Close(sockfd);
            usleep(1000000);
        }
        else
            binded  = true;

    }

    if(sockfd<0)
        return false;

    printf("%d Listening on port %d\n",sockfd, port);

    socklen_t clilen;
    //listening for connection
    listen(sockfd,10);

    while(monitoring_thread_running)
    {
        struct sockaddr_in client;
        clilen = sizeof(client);
        int clientsockfd = accept(sockfd, (struct sockaddr *) &client, &clilen);

        if (clientsockfd < 0) 
        {
            printf("ERROR on accept %d\n", clientsockfd);
        }
        else
        {
            printf("accept connection from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
            Connection *conn = new Connection;
            conn->sockfds = clientsockfd;
            conn->addr = client;
            conn->ipc = this;
            conn->connected = true;
            conn->SetCallback(callback, user_data);
            conn->SetCallbackRaw(callback_raw, user_data);
            connections.push_back(conn);
            conn->Start();
        }

    }

    Close(sockfd);

    return true;
}

bool IPC::ConnectToServer(const char * host, int port)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
    server = gethostbyname(host);

    printf("Trying to connect to Server [%s:%d]\n", host, port);

    int clientsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientsockfd < 0) 
    {
        printf("ERROR opening socket, exit monitor thread\n");
        return false;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(clientsockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        printf("ERROR connecting, exit monitor thread\n");
        return false;
    }
    printf("Success to connect to Server [%s:%d] @ %d\n", host, port, clientsockfd);

    Connection *conn = new Connection;
    conn->ipc = this;
    conn->sockfds = clientsockfd;
    conn->addr = serv_addr;
    conn->connected = true;
    conn->SetCallback(callback, user_data);
    conn->SetCallbackRaw(callback_raw, user_data);
    connections.push_back(conn);
    conn->Start();
    conn->transmiting_thread_running = true;
    conn->receiving_thread_running = true;

    //wait until user quit
    while(monitoring_thread_running)
        usleep(20000);

    Shutdown(conn->sockfds, 2);

    //wait until all thread quit
    while(conn->transmiting_thread_running || conn->receiving_thread_running)
        usleep(30000);

    //  delete conn;
    Close(conn->sockfds);

    return true;
}

bool Connection::SendData(const uint8_t type, uint8_t *data, int data_size)
{
    int len;
    int write;
    if(type == 0)
    {
        printf("Send Raw data (%d bytes) to %s:%d\n",data_size, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        len = data_size;

        pthread_mutex_lock(&mutex_txq);
        write = BQSize(&txq) - BQCount(&txq);
        if (len < write)
            write = len;
        BQPushBytes(&txq, data, write);       

        pthread_mutex_unlock(&mutex_txq);
    }
    else
    {
        ELolMessage msg;
        ElolmsgInit(&msg, type, data, data_size);
        len = ElolmsgSerializedSize(&msg);
        uint8_t buf[len];
        ElolmsgSerialize(&msg, buf);

        pthread_mutex_lock(&mutex_txq);
        write = BQSize(&txq) - BQCount(&txq);
        if (len < write)
            write = len;
        BQPushBytes(&txq, buf, write);       

        pthread_mutex_unlock(&mutex_txq);
    }
    return write == len;

}

bool IPC::SendData(const uint8_t type, uint8_t *data, int data_size)
{
    for(unsigned int i=0; i< connections.size(); i++)
    {
        if(connections[i] && connections[i]->connected)
            connections[i]->SendData(type, data, data_size);
    }
    return true;
}

bool IPC::SendData(const uint32_t dest, const uint8_t type, uint8_t * data, int data_size)
{
    bool ret = false;
    for(unsigned int i=0; i< connections.size(); i++)
    {
        if(connections[i] && connections[i]->addr.sin_addr.s_addr == dest && connections[i]->connected)
        {
            ret = connections[i]->SendData(type, data, data_size);
            break; // assumed only one connection from one address
        }
    }
    return ret;
}

int IPC::RemoveBrokenConnections()
{
    int count = 0;
    std::vector<Connection*>::iterator it = connections.begin();
    while(it != connections.end())
    {
        //remove broken connection
        if((*it) == NULL)
            it = connections.erase(it);
        else if(!(*it)->connected)
        { 
            printf("\tremove broken connection from %s:%d\n", inet_ntoa((*it)->addr.sin_addr), ntohs((*it)->addr.sin_port));
            delete *it;
            it = connections.erase(it);
            count++;
        }
        else
            it++;
    }
    return count;
}

int IPC::BrokenConnections()
{
    int count = 0;
    std::vector<Connection*>::iterator it = connections.begin();
    while(it != connections.end())
    {
        if(!(*it) || !(*it)->connected)
            count++;
        it++;
    }

    return count;
}

void IPC::Stop()
{
    monitoring_thread_running = false;
    if(sockfd >=0)
    {
        Shutdown(sockfd, 2); //this will stops all connections, so monitor thread will quit

        std::vector<Connection*>::iterator it = connections.begin();
        while(it != connections.end())
        {
            if((*it) !=NULL)
                Shutdown((*it)->sockfds, 2); //this will stops each connections, so their transmit and receiver thread will quit
            it++;
        }
    }

}

}//end of namespace
