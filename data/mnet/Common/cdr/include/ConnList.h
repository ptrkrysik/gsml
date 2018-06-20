#ifndef _CONNLIST_H_
#define _CONNLIST_H_

// *******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ConnList.h
// Author(s)   : Igal Gutkin
// Create Date : 6/22/00
// Description :  
//
// *******************************************************************

#include "socket\GenSocket.h"

// Forward declarations
class ClientConnectionList;
class ClientConnListIterator;


// *******************************************************************
// Class ClientConnection
// *******************************************************************
class ClientConnection {
public:
    friend class ClientConnectionList;
    friend class ClientConnListIterator;

             ClientConnection (GenSocket&);

    virtual ~ClientConnection ();

    OS_SPECIFIC_SOCKET_HANDLE  getSocket() {return (connSock_.getSocket());}
    GenSocket              *getClient() {return (&connSock_)           ;}

    int serveClient (const fd_set *set);


private:
    GenSocket       & connSock_;
    ClientConnection *next_    ;

    // No default constructor
    ClientConnection();
};


// *******************************************************************
// Class ClientConnectionList
// *******************************************************************
class ClientConnectionList {
public:
    friend class ClientConnListIterator;

    ClientConnectionList ();
    ~ClientConnectionList();

    SOC_STATUS InsertClientConnection (ClientConnection *pConn);
    SOC_STATUS DeleteClientConnection (ClientConnection *pConn);

private:
    ClientConnection *pConnListHead_;
};


// *******************************************************************
// Class ClientConnListIterator
// *******************************************************************
class ClientConnListIterator {
public:
    ClientConnListIterator (ClientConnectionList& List) : theList_(List)
                              { pClientConn_ = theList_.pConnListHead_; }
    ~ClientConnListIterator() {}

    void              Reset() { pClientConn_ = theList_.pConnListHead_; }
    ClientConnection *Next () 
    { 
        ClientConnection *pCurrConn = pClientConn_;
        return (pCurrConn ? (pClientConn_ = pCurrConn->next_) : pCurrConn);
    }

    ClientConnection *Current() { return pClientConn_;}

private:
    ClientConnection     *pClientConn_;
    ClientConnectionList& theList_    ;
};


#endif /* _CONNLIST_H_ */
