#!/usr/bin/env python

import SocketServer

class MyTCPHandler(SocketServer.BaseRequestHandler):
    """
    The request handler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """

    def handle(self):
        # self.request is the TCP socket connected to the client
        self.data = self.request.recv(1024).strip()
        print "{} wrote:".format(self.client_address[0])
        print self.data
        if 'wX' in self.data:
            print 'Start sending real time data'
            self.sendRealTimeData()
    def sendRealTimeData(self):
        fd = open('realtime.log')
        for line in fd:
            if 'Q,X,' in line:
                self.request.sendall(line)
        self.request.sendall('!ENDMSG!')

if __name__ == "__main__":
    HOST, PORT = "192.168.2.100", 7003

    # Create the server, binding to localhost on port 9999
    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
