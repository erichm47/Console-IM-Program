===================Simple IM Console Application ===============

Written by: Eric Makino 


OVERVIEW: ======================================================

Simple IM Console Application consists of a client side and a server side. Here is an outline of its functionality:

1) IM clients are able to join and leave the IM network. Every client has a user name and at any one particular time
no two clients connected to the IM network can have the same user name.

2) When IM clients join the network other connected clients are made aware of this change within the network. Similarly,
the reverse is true; that is, when clients leave the network other connected clients are informed. Clients can view a list
of all the other clients as well.

3) Connected IM clients are able to send messages to any other connected IM client. Messages are addressed to particular 
IM clients using their user name. IM clients are also able to receive messages from any other connected IM client.


ASSUMPTIONS: ====================================================

1) All clients are aware of the single IM server. The server sits on a known host and has a known port. Clients simply connect 
using sockets.

2) When an IMclient is in the process of typing a message to another client they will appear "offline" to other clients and the
server during that state. Once they are finished typing a message they will appear "online" again.  Thus, clients in the talking
state cannot receive messages from other clients or any broadcasts (broadcasting of available users) from the server.

3) Client "username" has no spaces.

4) Assume no malicious activity by clients.  

COMPILING AND RUNNING: ==========================================

Use the makefile provided to compile. (Type "make" into the console)

To run the server, type in: ./IMserver [port]
To run the client, type in: ./IMclient [hostname] [port]

The server needs to be running in order for clients to connect.
