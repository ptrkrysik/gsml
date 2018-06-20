#ifndef __SMQUEUE_H__
#define __SMQUEUE_H__

// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************


// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : smqueue.h
// Author(s)   : Igal Gutkin
// Create Date : 26-05-00
// Description : Session Manager queue for SAPI=3 messages
//
// *******************************************************************

#include "lapdm\lapdm_l3intf.h"

//Forward definition
class CCSessionHandler;
class CCHandover      ;

// Queue message capacity
#define MAX_SM_QUEUE_DEPTH 5


class SmQueueBasic
{

public:
          SmQueueBasic () : queueCounter_(0), queueLock_(false){;}

         ~SmQueueBasic () {;}

    bool  isEmpty () {return (queueCounter_ == 0);}
    bool  isFull  () {return (queueCounter_ >= MAX_SM_QUEUE_DEPTH);}

    void lock     () {queueLock_ = true ; }
    void unlock   () {queueLock_ = false; }
    bool isLocked () {return (queueLock_);}

    // Remove the data from the queue
    void  reset   () {initData ();}

    // Send all the queued messages
    virtual 
    void  flash   () = 0; 


protected:

    virtual 
    void initData () {queueCounter_ = 0; queueLock_ = false;}

// Data members    
    unsigned                  queueCounter_;
    bool                      queueLock_   ;

private:

};


class SmQueue : public SmQueueBasic
{

public:
           SmQueue (CCSessionHandler *smHandler) : SmQueueBasic(), parent_(*smHandler) {;}
          ~SmQueue () {;}
    
    
    // Send all the queued messages
    virtual 
    void  flash   (); 
    
    // Add new message to the queue. Returns false in the queue is full
    bool  add     (const T_CNI_LAPDM_L3MessageUnit *);

protected:

// Data members
    T_CNI_LAPDM_L3MessageUnit storage_ [MAX_SM_QUEUE_DEPTH];
    CCSessionHandler&         parent_                      ;

private:

};


class SmQueueMm : public SmQueueBasic
{

public:
           SmQueueMm (CCHandover *hoHandler) : SmQueueBasic(), parent_(*hoHandler) {;}
          ~SmQueueMm () {;}
    
    
    // Send all the queued messages
    virtual 
    void  flash   (); 
    
    // Add new message to the queue. Returns false in the queue is full
    bool  add     (const IntraL3Msg_t *);

protected:

// Data members
    IntraL3Msg_t storage_ [MAX_SM_QUEUE_DEPTH];
    CCHandover&  parent_                      ;

private:

};


#endif //__SMQUEUE_H__
