#include "scapesock.hpp"

/**********************************/
/*   BEGIN SCAPECONNECTION CODE   */
/**********************************/

sosc::ScapeConnection::ScapeConnection() {
    this->client_open = false;
    this->handshaked = false;
}

void sosc::ScapeConnection::Open(TcpClient client) {
    this->client = client;
    this->client_open = true;
}

int sosc::ScapeConnection::Handshake() {    
    if(this->handshaked)
        return SOSC_SHAKE_DONE;
    if(!this->client_open)
        return SOSC_SHAKE_ERR;
    
    if(!this->client.IsDataReady())
        return SOSC_SHAKE_CONT;
    
    this->client.Receive(&this->buffer, SOSC_TCP_APPEND);
    
    if(!str::starts(this->buffer, "GET")) {
        this->Close();
        return SOSC_SHAKE_ERR;
    }
    
    if(!str::contains(this->buffer, "\r\n\r\n"))
        return SOSC_SHAKE_CONT;
    
    std::string websocket_key = "";
    auto lines = str::split(this->buffer, "\r\n");
    for(auto line_r = lines.begin() + 1; line_r != lines.end(); ++line_r) {
        std::string line = str::trim(*line_r);
        
        if(str::starts(line, "Sec-WebSocket-Key")) {
            auto parts = str::split(line, ':');
            if(parts.size() < 2)
                break;
            
            websocket_key = str::trim(parts[1]);
            break;
        }
    }
    
    if(websocket_key == "") {
        this->Close();
        return SOSC_SHAKE_ERR;
    }
    
    std::stringstream stream;
    stream << "HTTP/1.1 101 Switching Protocols\r\n"
           << "Upgrade: websocket\r\n"
           << "Connection: Upgrade\r\n"
           << "Sec-WebSock";
    
    this->handshaked = true;
    return SOSC_SHAKE_DONE;
}

/******************************/
/*  END SCAPECONNECTION CODE  */
/******************************/
/*   BEGIN SCAPESERVER CODE   */
/******************************/

sosc::ScapeServer::ScapeServer() {
    this->server_open = false;
}

bool sosc::ScapeServer::Listen(uint16_t port) {
    if(this->server_open)
        return false;
    
    this->server = TcpServer();
    this->server.Listen(port);
    this->server_open = true;
    return true;
}

bool sosc::ScapeServer::Accept(ScapeConnection* client) {
    TcpClient raw_client;
    if(!this->server.Accept(&raw_client))
        return false;
    
    client->Open(raw_client);
    return true;
}

sosc::ScapeServer::~ScapeServer() {
    this->Close();
}
