#include "scapesock.hpp"

/**********************************/
/*   BEGIN SCAPECONNECTION CODE   */
/**********************************/

sosc::ScapeConnection::ScapeConnection() {
    this->client_open = false;
    this->handshaked = false;
}

void sosc::ScapeConnection::Open(const TcpClient& client) {
    this->client = client;
    this->client_open = true;
}

int sosc::ScapeConnection::Handshake() {    
    if(this->handshaked)
        return WS_SHAKE_DONE;
    if(!this->client_open)
        return WS_SHAKE_ERR;
    
    if(!this->client.IsDataReady())
        return WS_SHAKE_CONT;
    
    if(this->client.Receive(&this->buffer, SOSC_TCP_APPEND) <= 0) {
        this->Close();
        return WS_SHAKE_ERR;
    }
    
    if(!str::starts(this->buffer, "GET")) {
        this->Close();
        return WS_SHAKE_ERR;
    }
    
    if(!str::contains(this->buffer, "\r\n\r\n"))
        return WS_SHAKE_CONT;
    
    std::string websocket_key = "";
    auto lines = str::split(this->buffer, "\r\n");
    for(const auto& line_r : lines) {
        std::string line = str::trim(line_r);
        
        if(str::starts(line, "Sec-WebSocket-Key")) {
            auto parts = str::split(line, ':');
            if(parts.size() < 2)
                break;
            
            websocket_key = str::trim(parts[1]);
            break;
        }
    }
    
    if(websocket_key.empty()) {
        this->Close();
        return WS_SHAKE_ERR;
    }
    
    websocket_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    websocket_key = cgc::base64_encode(cgc::sha1(websocket_key, true));
    
    std::stringstream stream;
    stream << "HTTP/1.1 101 Switching Protocols\r\n"
           << "Upgrade: websocket\r\n"
           << "Connection: Upgrade\r\n"
           << "Sec-WebSocket-Accept: " << websocket_key << "\r\n\r\n";
    this->client.Send(stream.str());

    this->buffer = std::string(this->buffer.c_str() + this->buffer.find("\r\n\r\n") + 4);
    this->handshaked = true;
    return WS_SHAKE_DONE;
}

int sosc::ScapeConnection::Receive(Packet* packet, bool block) {
    if(!this->client_open)
        return PCK_ERR;
    if(!this->handshaked)
        return PCK_ERR;
    
    int status;
    ws::Frame frame;
    bool first_recv = true;
    while((status = frame.Parse(this->buffer, &this->buffer)) != FRAME_OK) {
        if(status == FRAME_ERR)
            return PCK_ERR;
        if(!block && !first_recv)
            return PCK_MORE;

        auto bufferSize = this->buffer.length();
        status = this->client.Receive
            (&this->buffer, SOSC_TCP_APPEND | (block ? SOSC_TCP_BLOCK : 0));
        if(status == -1)
            return PCK_ERR;

        first_recv = false;
    }
    
    // TODO optimize
    // TODO determine if packet->Parse should write back to pck_frames on o/f
    this->pck_frames += frame.GetBody();
    if(frame.IsFinal()) {
        if(packet->Parse(this->pck_frames, &this->pck_frames) == PCK_OK)
            return PCK_OK;
        else
            return PCK_ERR;
    } else
        return PCK_MORE;
}

bool sosc::ScapeConnection::Send(const Packet& packet) {
    if(!this->client_open)
        return PCK_ERR;
    if(!this->handshaked)
        return PCK_ERR;
    
    std::string packet_raw;
    packet.ToString(&packet_raw);
    return this->client.Send(ws::Frame(packet_raw));
}

/******************************/
/*  END SCAPECONNECTION CODE  */
/******************************/
/*   BEGIN SCAPESERVER CODE   */
/******************************/

sosc::ScapeServer::ScapeServer() {
    this->server_open = false;
}

bool sosc::ScapeServer::Listen(uint16_t port, bool secure) {
    if(this->server_open)
        return false;
    
    this->server = TcpServer();
    this->server_open = this->server.Listen(port, secure);
    return this->server_open;
}

bool sosc::ScapeServer::Accept(ScapeConnection* client) {
    TcpClient raw_client;
    if(!this->server.Accept(&raw_client))
        return false;
    
    client->Open(raw_client);
    return true;
}

/****************************/
/*   END SCAPESERVER CODE   */
/****************************/
