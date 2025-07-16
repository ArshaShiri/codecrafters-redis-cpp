#pragma once

class TCPServer {
  public:
    explicit TCPServer(int listening_port);
    void run();
    ~TCPServer();

  private:
    int listening_socket_;
};
