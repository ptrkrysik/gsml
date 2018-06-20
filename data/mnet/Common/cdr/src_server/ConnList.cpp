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
// File        : ConnList.cpp
// Author(s)   : Igal Gutkin
// Create Date : 6/21/00
// Description :  
//
// *******************************************************************


#include "stdAfx.h"
#include "ConnList.h"

// *******************************************************************
// Class ClientConnection
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ClientConnection constructor
**
**    PURPOSE: Save the socket for this connection and initialize the next 
**        pointer to be null.
**
**    INPUT PARAMETERS: sock - socket for the new client connection
**
**    RETURN VALUE(S):    none
**
**----------------------------------------------------------------------------*/
ClientConnection::ClientConnection (GenSocket& Client)
                : connSock_ (Client), next_ (NULL)
{
    ;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ClientConnection destructor
**
**    PURPOSE: Delete ClientConnection data members.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S):    none
**
**----------------------------------------------------------------------------*/
ClientConnection::~ClientConnection ()
{
    delete &connSock_;
}


int ClientConnection::serveClient (const fd_set *set)
{
  GenSocket * notInUse = NULL;  // dummy value

  return (getClient()->serve(set, notInUse));
}



// *******************************************************************
// Class ClientConnectionList
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ClientConnectionList constructor
**
**    PURPOSE: Initialize list to be null.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S):    none
**
**----------------------------------------------------------------------------*/
ClientConnectionList::ClientConnectionList()
{
    pConnListHead_ = NULL;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ClientConnectionList destructor
**
**    PURPOSE: Delete ClientConnectionList data members.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S):    none
**
**----------------------------------------------------------------------------*/
ClientConnectionList::~ClientConnectionList()
{
    ClientConnection *pPrevConn, *pCurrConn = pConnListHead_;
    
    while (pCurrConn)
    {
        pPrevConn = pCurrConn;
        pCurrConn = pCurrConn->next_;
        delete pPrevConn;
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: InsertClientConnection
**
**    PURPOSE: Insert an client connection at the head of the list.
**
**    INPUT PARAMETERS: pConn - pointer to the connection object to be added.
**
**    RETURN VALUE(S):    SOC_STATUS_OK - for success
**                        SOC_STATUS_ERROR - for failure
**
**----------------------------------------------------------------------------*/
SOC_STATUS ClientConnectionList::InsertClientConnection (ClientConnection *pConn)
{
    pConn->next_   = pConnListHead_;
    pConnListHead_ = pConn;
    return (SOC_STATUS_OK);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DeleteClientConnection
**
**    PURPOSE: Remove an client connection from the list of client connections.
**        The socket is used as the identifier to locate the client connection.
**
**    INPUT PARAMETERS: sock - socket for the desired client connection
**
**    RETURN VALUE(S):    SOC_STATUS_OK - for success
**                        SOC_STATUS_ERROR - for failure
**
**----------------------------------------------------------------------------*/
SOC_STATUS ClientConnectionList::DeleteClientConnection (ClientConnection *pConn)
{
  ClientConnection          *pCurrConn, *pPrevConn;
  OS_SPECIFIC_SOCKET_HANDLE  Socket   = pConn->getSocket();

    pCurrConn = pPrevConn = pConnListHead_;
    
    while (pCurrConn)
    {
        if (pCurrConn->connSock_.getSocket() == Socket)
        {
            if (pCurrConn == pConnListHead_)
            {
                pConnListHead_ = pCurrConn->next_;
            }
            else
            {
                pPrevConn->next_ = pCurrConn->next_;
            }

            delete pCurrConn;
            break;
        }

        pPrevConn = pCurrConn;
        pCurrConn = pCurrConn->next_;
    }

    return (SOC_STATUS_OK);
}
