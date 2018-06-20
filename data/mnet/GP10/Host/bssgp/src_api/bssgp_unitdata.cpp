/****************************************************************************************
 *                                                                                      *
 *  Copyright Cisco Systems, Inc 2000 All rights reserved                               *
 *                                                                                      *
 *  File                : bssgp_unitdata.cpp                                            *
 *                                                                                      *
 *  Description         : Routines for manipulation of the unitdata buffers             *
 *                                                                                      *
 *  Author              : Dinesh Nambisan                                               *
 *                                                                                      *
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description                                               *
 *--------------------------------------------------------------------------------------*
 * 00:00 |10/10/00 | DSN    | File created                                              *
 *       |         |        |                                                           *
 ****************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include "bssgp/bssgp_prot.h"
#include "bssgp/bssgp_util.h"
#include "bssgp/bssgp_unitdata.h"
#include "rlc_mac/rlcmacintf.h"


/*
 * TO DO : TO DO : TO DO : TO DO : TO DO : IMPLEMENT LOCKS FOR THE QUEUES !!!!!
 *
 *
 *
 *
 *
 */

/*
 * Here is how this Queues data structure looks :
 *
 *
 *
 *
 *
 *                                  (head)                                    (tail)
 *                              |------------|   |------------|            |------------|
 *               |--- QOS 1 ----| UDT BUFF 1 |---| UDT BUFF 2 |.........---| UDT BUFF N |
 *               |              |------------|   |------------|            |------------|
 *               |
 *               |              |------------|
 *   |--------|  |--- QOS 2 ----| UDT BUFF 1 |  
 *   |        |  |              |------------|  
 *   | TLLI 1 |--|  
 *   | (head) |  |              |------------|  
 *   |--------|  |--- QOS 3 ----| UDT BUFF 1 |    
 *       .       |              |------------|
 *       .       |
 *       .       |              |------------|  
 *       .       |--- QOS 4 ----| UDT BUFF 1 |  
 *       .                      |------------|
 *       .       
 *       .                          (head)                                    (tail)
 *       .                      |------------|   |------------|            |------------|
 *       .       |--- QOS 1 ----| UDT BUFF 1 |---| UDT BUFF 2 |.........---| UDT BUFF N |
 *       .       |              |------------|   |------------|            |------------|
 *       .       |
 *       .       |              |------------|
 *   |--------|  |--- QOS 2 ----| UDT BUFF 1 |  
 *   |        |  |              |------------|  
 *   | TLLI N |--|  
 *   | (tail) |  |              |------------|  
 *   |--------|  |--- QOS 3 ----| UDT BUFF 1 |    
 *               |              |------------|
 *               |
 *               |              |------------|  
 *               |--- QOS 4 ----| UDT BUFF 1 |  
 *                              |------------|
 *           
 *           
 *
 */

static BSSGP_UNITDATA_CONFIG            *bssgp_udt_config=NULL;




/*
 * bssgp_udt_initialize
 */
BOOL
bssgp_udt_initialize(int tlli_list_count,int bssgp_udt_buffer_count,BSSGP_UDT_WATERMARK_CB watermark_cb)
{
    DBG_FUNC("bssgp_udt_initialize",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    BOOL status = FALSE;

    if (bssgp_udt_config != NULL) 
    {   
        DBG_ERROR("UDT layer already initialized\n");
        DBG_LEAVE();
        return (status);
    }
    /* allocate the configuration structure */
    bssgp_udt_config = (BSSGP_UNITDATA_CONFIG*)malloc(sizeof(BSSGP_UNITDATA_CONFIG));
    if (bssgp_udt_config == NULL) 
    {
        DBG_ERROR("UDT layer already initialized\n");
        DBG_LEAVE();
        return (status);
    }
    memset(bssgp_udt_config,0,sizeof(BSSGP_UNITDATA_CONFIG));
    bssgp_udt_config->tlli_list_count   = tlli_list_count;
    bssgp_udt_config->udt_buff_count    = bssgp_udt_buffer_count;
    bssgp_udt_config->watermark_cb      = watermark_cb;
    /* FIXME : Got to retrieve these values from the MIB later on */
    bssgp_udt_config->tlli_low_watermark_bytes  = 8092;
    bssgp_udt_config->tlli_high_watermark_bytes = 10240;


    DBG_TRACE("%d UDT buffers specified, %d TLLI lists specified in pool\n",
            tlli_list_count,bssgp_udt_buffer_count);


    if (!bssgp_udt_allocate_buff_pool()) 
    {
        DBG_ERROR("Unable to allocate buffer pool\n");
        free(bssgp_udt_config);
        bssgp_udt_config = NULL;
        DBG_LEAVE();
        return (status);
    }
    DBG_TRACE("Allocated the UDT buffer pool\n");
    if (!bssgp_udt_allocate_tlli_pool()) 
    {
        DBG_ERROR("Unable to allocate TLLI list pool\n");
        bssgp_udt_free_buff_pool();
        free(bssgp_udt_config);
        bssgp_udt_config = NULL;
        DBG_LEAVE();
        return (status);
    }
    else 
    {
        status = TRUE;
    }

    DBG_TRACE("Allocated the TLLI lists pool\n");
    DBG_TRACE("Unitdata layer initialized successfully\n");
    DBG_LEAVE();
    return (status);
}




/*
 * bssgp_udt_shutdown
 */
void
bssgp_udt_shutdown()
{
    DBG_FUNC("bssgp_udt_shutdown",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    
    /*
     * Need to put in checks for verifying everything has been cleaned up
     */
    bssgp_udt_free_tlli_pool();

    bssgp_udt_free_buff_pool();

    free(bssgp_udt_config);
    bssgp_udt_config = NULL;

    DBG_LEAVE();
    return;
}





/*
 * bssgp_udt_put_buffer_in_q
 */
BOOL
bssgp_udt_put_buffer_in_q(TLLI tlli,BSSGP_NETWORK_QOS_LEVEL qos_level,UINT8 *buff, int buff_size,BSSGP_DL_UNITDATA_MSG dl_unitdata)
{
    DBG_FUNC("bssgp_udt_put_buffer_in_q",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    BOOL status= FALSE;
    PBSSGP_UDT_BUFFER udt_buff;
    PBSSGP_UDT_TLLI_Q tlli_q;


    udt_buff = (PBSSGP_UDT_BUFFER)bssgp_udt_get_buff_from_pool();
    if (udt_buff == NULL) 
    {
        DBG_ERROR("No more UDT buffers available in pool!\n");
        DBG_LEAVE();
        return (status);
    }
    DBG_TRACE("Using UDT buff %#x\n",udt_buff);

    udt_buff->data          = buff;
    udt_buff->data_size     = buff_size;
    memcpy(&udt_buff->dl_unitdata,&dl_unitdata,sizeof(dl_unitdata));
    udt_buff->dl_unitdata.pdu.data      = buff;
    udt_buff->dl_unitdata.pdu.data_len  = buff_size;


    /*
     * Initially, two possibilities : (1) either there are no TLLI Qs
     * or (2) there is atleast one TLLI Q
     */
    if (bssgp_udt_config->tlli_q_head == NULL) 
    {
        /*
         * No TLLI Qs present case:
         */
        DBG_TRACE("No TLLI Qs present, will add one\n");
        tlli_q = bssgp_udt_get_tlli_list_from_pool();
        if (tlli_q == NULL) 
        {
           DBG_ERROR("Cannot allocate TLLI List from pool!\n");
           bssgp_udt_return_buff_to_pool(udt_buff);
           DBG_LEAVE();
           return (FALSE);
        }
        DBG_TRACE("Using TLLI Q-List %#x\n",tlli_q);
        bssgp_udt_config->tlli_q_head  = tlli_q;
        bssgp_udt_config->tlli_q_tail  = tlli_q;
        tlli_q->prev                   = NULL;
        tlli_q->next                   = NULL;
        tlli_q->tlli                   = tlli;
        /* Stick it in the appropriate QoS level q */
        tlli_q->qos_queues[qos_level].qos_level    = qos_level;
        /* This fella would be the first */
        tlli_q->qos_queues[qos_level].buff_list_head = udt_buff;
        tlli_q->qos_queues[qos_level].buff_list_tail = udt_buff;
        udt_buff->prev  = NULL;
        udt_buff->next  = NULL;
        /* FIXME : Check return status here */
        RlcMacBssgpUnitDataRequest(&dl_unitdata);


    }
    else 
    {
        /*
         * One or more TLLI Qs present case; so now there are two
         * possibilities again; either (1) this TLLI is one which 
         * already has a Q or (2) we need to create a Q for this TLLI
         */
         if (bssgp_udt_config->tlli_q_head == NULL)
         {
             DBG_ERROR("BSSGP: ERROR-Queue head is null!\n");
             DBG_LEAVE();
             return (FALSE);
         }
         tlli_q = bssgp_udt_config->tlli_q_head;
         while ((tlli_q->next != NULL) && (tlli_q->tlli != tlli)) 
         {
            tlli_q = tlli_q->next;
         }
         if (tlli_q->tlli == tlli) 
         {
            /* 
             * Found the sucker, stick this guy into the right place 
             */

            DBG_TRACE("Found Q for TLLI %#x\n",tlli_q->tlli);
            /*
             * First check whether hi watermark has been indicated
             * and if so whether we have a callback function; otherwise
             * we need to drop it onto the floor
             */
            if ( (tlli_q->high_watermark_indicated) && (bssgp_udt_config->watermark_cb == NULL)) 
            {
                DBG_ERROR("High watermark reached for this TLLI %#x, RX Bytes %d, buff count %d, dropping buffer\n",
                tlli_q->tlli,tlli_q->rx_bytes,tlli_q->buff_count);
                bssgp_udt_return_buff_to_pool(udt_buff);
                DBG_LEAVE();
                return (FALSE);
            }

            /* Now again two cases, there might be no buffers in this
             * Q or there might be some
             */
             if (tlli_q->qos_queues[qos_level].buff_list_head == NULL) 
             {
                DBG_TRACE("No buffers in Q for TLLI %#x, will send ping to RLC\n",tlli_q->tlli);
                /* this guy becomes both the head n the tail */
                tlli_q->qos_queues[qos_level].buff_list_head = udt_buff;
                tlli_q->qos_queues[qos_level].buff_list_tail = udt_buff;
                udt_buff->prev = NULL;
                udt_buff->next = NULL;
                /* FIXME : Check return status here */
                RlcMacBssgpUnitDataRequest(&dl_unitdata);

             }
             else 
             {
                DBG_TRACE("%d buffers : %d bytes present in Q for TLLI %#x, will NOT send ping to RLC\n",
                    tlli_q->qos_queues[qos_level].buff_count,tlli_q->qos_queues[qos_level].rx_bytes,
                tlli_q->tlli);
                /* this guy becomes the new tail */
                udt_buff->prev =  tlli_q->qos_queues[qos_level].buff_list_tail;
                (tlli_q->qos_queues[qos_level].buff_list_tail)->next = udt_buff;
                udt_buff->next = NULL;
                tlli_q->qos_queues[qos_level].buff_list_tail = udt_buff;
             }
         }
         else if (tlli_q->next == NULL) 
         {
            DBG_TRACE("No Q for TLLI %#x, will allocate new one & ping RLC\n",tlli_q->tlli);
            /*
             * Duh, this tlli aint there, so grab a new one & stick it in
             */
            tlli_q = bssgp_udt_get_tlli_list_from_pool();
            if (tlli_q == NULL) 
            {
               DBG_ERROR("Cannot allocate TLLI List from pool!\n");
               bssgp_udt_return_buff_to_pool(udt_buff);
               DBG_LEAVE();
               return (FALSE);
            }
            DBG_TRACE("Using TLLI Q-List %#x\n",tlli_q);
            /* stick this TLLI list to the bottom */
            tlli_q->prev    = bssgp_udt_config->tlli_q_tail;
            (bssgp_udt_config->tlli_q_tail)->next = tlli_q;
            tlli_q->next    = NULL;
            bssgp_udt_config->tlli_q_tail = tlli_q;
            tlli_q->tlli                   = tlli;
            /* Stick it in the appropriate QoS level q */
            tlli_q->qos_queues[qos_level].qos_level    = qos_level;
            /* This fella would be the first in the QoS level list */
            tlli_q->qos_queues[qos_level].buff_list_head = udt_buff;
            tlli_q->qos_queues[qos_level].buff_list_tail = udt_buff;
            udt_buff->prev  = NULL;
            udt_buff->next  = NULL;
            /* FIXME : Check return status here */
            RlcMacBssgpUnitDataRequest(&dl_unitdata);

         }
    }
    

    status = TRUE;
    /* Update all the counters */
    tlli_q->qos_queues[qos_level].buff_count++;
    tlli_q->qos_queues[qos_level].rx_bytes += buff_size;

    tlli_q->buff_count++;
    tlli_q->rx_bytes += buff_size;
    if (tlli_q->rx_bytes >= bssgp_udt_config->tlli_high_watermark_bytes) 
    {
        tlli_q->high_watermark_indicated  = TRUE;
        /*
         * 
         */
        if (bssgp_udt_config->watermark_cb != NULL) 
        {
            (*bssgp_udt_config->watermark_cb)(tlli_q->tlli,tlli_q->rx_bytes,tlli_q->buff_count,TRUE);
        }
    }


    DBG_LEAVE();
    return (status);
}



/*
 * bssgp_udt_get_buff_by_tlli
 */
UINT8 *
bssgp_udt_get_buff_by_tlli(TLLI tlli, BSSGP_DL_UNITDATA_MSG *dl_unitdata)
{
    DBG_FUNC("bssgp_udt_get_buff_by_tlli",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    UINT8 *buff = NULL;
    int count;
    PBSSGP_UDT_TLLI_Q tlli_q;
    PBSSGP_UDT_BUFFER udt_buff;

    tlli_q = bssgp_udt_config->tlli_q_head;
    if (tlli_q == NULL) 
    {
        DBG_ERROR("No TLLI lists present\n");
        DBG_LEAVE();
        return NULL;
    }
    while( (tlli_q->next != NULL) && (tlli_q->tlli != tlli) ) 
    {
        tlli_q = tlli_q->next;
    }
    if (tlli_q->tlli == tlli) 
    {
        /* Found the tlli list */
        if (tlli_q->buff_count == 0) 
        {
            DBG_TRACE("No buffers in Q for TLLI %#x\n",tlli);
        }
        else 
        {
            /* Start by looking for the highest QoS level */
            for(count=0;count<BSSGP_MAX_NETWORK_QOS_LEVELS;count++) 
            {
                if (tlli_q->qos_queues[count].buff_count == 0) continue;
                else 
                {
                    udt_buff = tlli_q->qos_queues[count].buff_list_head;
                    if (udt_buff == NULL)
                    {
                         DBG_ERROR("BSSGP: ERROR-Queue head is null!\n");
                         DBG_LEAVE();
                         return (NULL);
                    }
                    if (udt_buff->next != NULL) 
                    {
                        /* move the head */
                        tlli_q->qos_queues[count].buff_list_head = udt_buff->next;
                        (udt_buff->next)->prev = NULL;
                    }
                    else 
                    {
                        /* nothing left now */
                        tlli_q->qos_queues[count].buff_list_head =
                        tlli_q->qos_queues[count].buff_list_tail = NULL;
                    }
                    tlli_q->qos_queues[count].buff_count--;
                    tlli_q->qos_queues[count].rx_bytes -= udt_buff->data_size;
                    tlli_q->buff_count--;
                    tlli_q->rx_bytes -= udt_buff->data_size;
                    if ( (tlli_q->rx_bytes <= bssgp_udt_config->tlli_low_watermark_bytes) &&
                           (tlli_q->high_watermark_indicated) ) 
                    {
                            if (bssgp_udt_config->watermark_cb != NULL) 
                            {
                                (*bssgp_udt_config->watermark_cb)(tlli_q->tlli,tlli_q->rx_bytes,tlli_q->buff_count,FALSE);
                            }
                            DBG_TRACE("Clearing out High Watermark flag for TLLI %#x\n",tlli_q->tlli);
                            tlli_q->high_watermark_indicated = FALSE;

                    }
                    buff = (UINT8*)udt_buff->data;
                    memcpy(dl_unitdata,&udt_buff->dl_unitdata,sizeof(BSSGP_DL_UNITDATA_MSG));
                    bssgp_udt_return_buff_to_pool(udt_buff);
                    if (tlli_q->buff_count == 0) 
                    {
                        /* 
                         * Nothing else in this TLLI's Q, so start a timer to wait for
                         * a while, and if still nothing is there remove the list 
                         * and stick it back into the available pool
                         */
                        /* TO DO : Timer for returning lists to pool */
                    }
                    break;
                }
            } /* End of for loop */
        } /* end of else if tlli_q->buff_count == 0 */
        
    }
    else 
    {
        /* Could not find a list for this TLLI */
        DBG_WARNING("No Q for TLLI %#x\n",tlli);
    }
    



    DBG_LEAVE();
    return(buff);
}


/*
 * bssgp_udt_get_buff_by_qos
 *
 * This function searches thru all the TLLI lists
 * to find any PDU at this QoS level; and if found
 * returns it. However, to make sure that this search,
 * if conducted repeatedly, has some round-robin method
 * in it w.r.t. TLLIs (the search shouldnt hit the same TLLI
 * constantly if other TLLIs have PDUs with the same QoS);
 * this TLLI list is extracted and stuck to the bottom of the
 * TLLI list linked list, so that next time this TLLI list
 * would be the last one searched
 */
UINT8 *
bssgp_udt_get_buff_by_qos(BSSGP_NETWORK_QOS_LEVEL qos_level,BSSGP_DL_UNITDATA_MSG *dl_unitdata)
{
    DBG_FUNC("bssgp_udt_get_buff_by_qos",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    UINT8 *buff = NULL;
    PBSSGP_UDT_TLLI_Q tlli_q;
    PBSSGP_UDT_BUFFER udt_buff;

    tlli_q = bssgp_udt_config->tlli_q_head;
    if (tlli_q == NULL) 
    {
        DBG_ERROR("No TLLI lists present\n");
        DBG_LEAVE();
        return NULL;
    }
    while (tlli_q->next != NULL) 
    {
        if (tlli_q->buff_count > 0) 
        {
            if (tlli_q->qos_queues[qos_level].buff_count > 0) 
            {
                /* 
                 * OK, we found a TLLI which has a buffered PDU with this QoS level 
                 * so extract it first
                 */
                 if (tlli_q->qos_queues[qos_level].buff_list_head == NULL)
                 {
                     DBG_ERROR("BSSGP: ERROR-Queue head is null!\n");
                     DBG_LEAVE();
                     return (NULL);
                 }
                 udt_buff = tlli_q->qos_queues[qos_level].buff_list_head;
                 if (udt_buff->next != NULL) 
                 {
                     tlli_q->qos_queues[qos_level].buff_list_head = udt_buff->next;
                     (udt_buff->next)->prev = NULL;
                 }
                 else 
                 {
                     tlli_q->qos_queues[qos_level].buff_list_head =
                     tlli_q->qos_queues[qos_level].buff_list_tail = NULL;
                 }
                 tlli_q->qos_queues[qos_level].buff_count--;
                 tlli_q->qos_queues[qos_level].rx_bytes -= udt_buff->data_size;
                 tlli_q->buff_count--;
                 tlli_q->rx_bytes -= udt_buff->data_size;
                 if ( (tlli_q->rx_bytes <= bssgp_udt_config->tlli_low_watermark_bytes) &&
                        (tlli_q->high_watermark_indicated) ) 
                 {
                    if (bssgp_udt_config->watermark_cb != NULL) 
                    {
                        (*bssgp_udt_config->watermark_cb)(tlli_q->tlli,tlli_q->rx_bytes,tlli_q->buff_count,FALSE);
                    }

                 }
                 buff = (UINT8*)udt_buff->data;
                 memcpy(dl_unitdata,&udt_buff->dl_unitdata,sizeof(BSSGP_DL_UNITDATA_MSG));
                 bssgp_udt_return_buff_to_pool(udt_buff);
                 /*
                  * Now manipulate the TLLI linked list table so that
                  * this TLLI list is at the end of the linked list
                  * Four cases here:
                  *  (1) it is at the head, but not the tail
                  *  (2) it is at the tail, but not at the head
                  *  (3) it is both the head & the tail
                  *  (4) it is neither the head nor the tail
                  *  For cases (1) & (4), we need to move it to the tail of the list;
                  * and for cases (2) & (3), we dont need to do anything for obvious reasons
                  */
                  /* case (3) above */
                 if ((tlli_q != bssgp_udt_config->tlli_q_head) && (tlli_q != bssgp_udt_config->tlli_q_tail)) 
                 {
                    (tlli_q->prev)->next = tlli_q->next;
                    (tlli_q->next)->prev  = tlli_q->prev;
                    /* move the tail */
                    tlli_q->prev = bssgp_udt_config->tlli_q_tail;
                    (bssgp_udt_config->tlli_q_tail)->next = tlli_q;
                    tlli_q->next = NULL;
                    bssgp_udt_config->tlli_q_tail = tlli_q;
                 }
                 /* case (1) above */
                 else if ((tlli_q == bssgp_udt_config->tlli_q_head) && (tlli_q != bssgp_udt_config->tlli_q_tail)) 
                 {
                    bssgp_udt_config->tlli_q_head = tlli_q->next;
                    (bssgp_udt_config->tlli_q_head)->prev = NULL;
                    tlli_q->prev = bssgp_udt_config->tlli_q_tail;
                    (bssgp_udt_config->tlli_q_tail)->next = tlli_q;
                    bssgp_udt_config->tlli_q_tail = tlli_q;
                    tlli_q->next = NULL;
                 }
                 DBG_LEAVE();
                 return (buff);

            }
        }
        /* code should fall through here and we should switch to the next TLLI list */
        tlli_q = tlli_q->next;
    }



    DBG_LEAVE();
    return(buff);
}


/*
 * bssgp_udt_get_buff_by_tlli_qos
 */
UINT8 *
bssgp_udt_get_buff_by_tlli_qos(TLLI tlli, BSSGP_NETWORK_QOS_LEVEL qos_level,BSSGP_DL_UNITDATA_MSG *dl_unitdata)
{
    DBG_FUNC("bssgp_udt_get_buff_by_tlli_qos",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    UINT8 *buff = NULL;
    PBSSGP_UDT_TLLI_Q tlli_q;
    PBSSGP_UDT_BUFFER udt_buff;

    tlli_q = bssgp_udt_config->tlli_q_head;
    if (tlli_q == NULL) 
    {
        DBG_ERROR("No TLLI lists present\n");
        DBG_LEAVE();
        return NULL;
    }
    while( (tlli_q->next != NULL) && (tlli_q->tlli != tlli) ) 
    {
        tlli_q = tlli_q->next;
    }
    if (tlli_q->tlli == tlli) 
    {
        /* Found the tlli list */
        if (tlli_q->buff_count == 0) 
        {
            DBG_TRACE("No buffers in Q for TLLI %#x\n",tlli);
        }
        else 
        {
            if (tlli_q->qos_queues[qos_level].buff_count == 0) 
            {
                DBG_TRACE("No buffers in Q for TLLI %#x at QoS level %d\n",tlli,qos_level);
            }
            else {
                udt_buff = tlli_q->qos_queues[qos_level].buff_list_head;
                if (udt_buff == NULL)
                {
                     DBG_ERROR("BSSGP: ERROR-Queue head is null!\n");
                     DBG_LEAVE();
                     return (NULL);
                }
                if (udt_buff->next != NULL) 
                {
                    /* Shift the head of the buff list */
                    tlli_q->qos_queues[qos_level].buff_list_head = udt_buff->next;
                    (udt_buff->next)->prev = NULL;
                }
                else 
                {
                    /* nothin left in the buff list */
                    tlli_q->qos_queues[qos_level].buff_list_head =
                    tlli_q->qos_queues[qos_level].buff_list_tail = NULL;
                }
                tlli_q->qos_queues[qos_level].buff_count--;
                tlli_q->qos_queues[qos_level].rx_bytes -= udt_buff->data_size;
                tlli_q->buff_count--;
                tlli_q->rx_bytes -= udt_buff->data_size;
                if ( (tlli_q->rx_bytes <= bssgp_udt_config->tlli_low_watermark_bytes) &&
                       (tlli_q->high_watermark_indicated) ) 
                {
                    if (bssgp_udt_config->watermark_cb != NULL) 
                    {
                        (*bssgp_udt_config->watermark_cb)(tlli_q->tlli,tlli_q->rx_bytes,tlli_q->buff_count,FALSE);
                    }

                }
                buff = (UINT8*)udt_buff->data;
                memcpy(dl_unitdata,&udt_buff->dl_unitdata,sizeof(BSSGP_DL_UNITDATA_MSG));
                bssgp_udt_return_buff_to_pool(udt_buff);
                if (tlli_q->buff_count == 0) 
                {
                    /* 
                     * Nothing else in this TLLI's Q, so start a timer to wait for
                     * a while, and if still nothing is there remove the list 
                     * and stick it back into the available pool
                     */
                    /* TO DO : Timer for returning lists to pool */
                }

                DBG_LEAVE();
                return (buff);
            }
        }
    }

    DBG_LEAVE();
    return(buff);
}








/*
 * bssgp_udt_check_buff_by_tlli
 *
 * This functions checks the lists/qs to see if there are ANY buffers
 * in the list/q for this particular TLLI; returns TRUE if there atleast
 * one, FALSE otherwise
 *
 */
BOOL
bssgp_udt_check_buff_by_tlli(TLLI tlli, BSSGP_DL_UNITDATA_MSG **dl_unitdata)
{
    DBG_FUNC("bssgp_udt_check_buff_by_tlli",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    UINT8 *buff = NULL;
    int count;
    PBSSGP_UDT_TLLI_Q tlli_q;
    PBSSGP_UDT_BUFFER udt_buff;

    tlli_q = bssgp_udt_config->tlli_q_head;
    if (tlli_q == NULL) 
    {
        DBG_LEAVE();
        return FALSE;
    }
    while( (tlli_q->next != NULL) && (tlli_q->tlli != tlli) ) 
    {
        tlli_q = tlli_q->next;
    }
    if (tlli_q->tlli == tlli) {
        /* Found the tlli list */
        if (tlli_q->buff_count == 0) 
        {
            DBG_TRACE("No buffers in Q for TLLI %#x\n",tlli);
            DBG_LEAVE();
            return(FALSE);
        }
        else {
            /* Start by looking for the highest QoS level */
            for(count=0;count<BSSGP_MAX_NETWORK_QOS_LEVELS;count++) 
            {
                if (tlli_q->qos_queues[count].buff_count == 0) continue;
                else 
                {
                    if (tlli_q->qos_queues[count].buff_list_head == NULL)
                    {
                         DBG_ERROR("BSSGP: ERROR-Queue head is null!\n");
                         DBG_LEAVE();
                         return (FALSE);
                    }
                    *dl_unitdata = &tlli_q->qos_queues[count].buff_list_head->dl_unitdata;
                    DBG_LEAVE();
                    return (TRUE);
                }
            } /* End of for loop */
        } /* end of else if tlli_q->buff_count == 0 */
        
    }
    else 
    {
        /* Could not find a list for this TLLI */
        DBG_TRACE("No Q for TLLI %#x\n",tlli);
    }
    



    DBG_LEAVE();
    return(FALSE);
}


/*
 * bssgp_udt_check_buff_by_qos
 *
 * This function checks the lists/qs to see if there is atleast one buffer
 * queued up at this QoS level; returns TRUE if there is atleast one,
 * FALSE otherwise
 */
BOOL
bssgp_udt_check_buff_by_qos(BSSGP_NETWORK_QOS_LEVEL qos_level, BSSGP_DL_UNITDATA_MSG **dl_unitdata)
{
    DBG_FUNC("bssgp_udt_check_buff_by_qos",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    UINT8 *buff = NULL;
    PBSSGP_UDT_TLLI_Q tlli_q;
    PBSSGP_UDT_BUFFER udt_buff;

    tlli_q = bssgp_udt_config->tlli_q_head;
    if (tlli_q == NULL) 
    {
        DBG_LEAVE();
        return FALSE;
    }
    while (tlli_q != NULL) 
    {
        if (tlli_q->buff_count > 0) 
        {
            if (tlli_q->qos_queues[qos_level].buff_count > 0) 
            {
                 DBG_TRACE("Found UDT buffer at QoS level %d\n",qos_level);
                 /*
                  * Now manipulate the TLLI linked list table so that
                  * this TLLI list is at the end of the linked list
                  * Four cases here:
                  *  (1) it is at the head, but not the tail
                  *  (2) it is at the tail, but not at the head
                  *  (3) it is both the head & the tail
                  *  (4) it is neither the head nor the tail
                  *  For cases (1) & (4), we need to move it to the tail of the list;
                  * and for cases (2) & (3), we dont need to do anything for obvious reasons
                  */
                  /* case (3) above */
                 if ((tlli_q != bssgp_udt_config->tlli_q_head) && (tlli_q != bssgp_udt_config->tlli_q_tail)) 
                 {
                    (tlli_q->prev)->next = tlli_q->next;
                    (tlli_q->next)->prev  = tlli_q->prev;
                    /* move the tail */
                    tlli_q->prev = bssgp_udt_config->tlli_q_tail;
                    (bssgp_udt_config->tlli_q_tail)->next = tlli_q;
                    tlli_q->next = NULL;
                    bssgp_udt_config->tlli_q_tail = tlli_q;
                 }
                 /* case (1) above */
                 else if ((tlli_q == bssgp_udt_config->tlli_q_head) && (tlli_q != bssgp_udt_config->tlli_q_tail)) 
                 {
                    bssgp_udt_config->tlli_q_head = tlli_q->next;
                    (bssgp_udt_config->tlli_q_head)->prev = NULL;
                    tlli_q->prev = bssgp_udt_config->tlli_q_tail;
                    (bssgp_udt_config->tlli_q_tail)->next = tlli_q;
                    bssgp_udt_config->tlli_q_tail = tlli_q;
                    tlli_q->next = NULL;
                 }
                 *dl_unitdata = &tlli_q->qos_queues[qos_level].buff_list_head->dl_unitdata;
                 DBG_LEAVE();
                 return (TRUE);

            }
        }
        else 
        {
            DBG_TRACE("TLLI %#x has no buffer at QoS level %d\n",tlli_q->tlli,qos_level);
        }
        /* code should fall through here and we should switch to the next TLLI list */
        tlli_q = tlli_q->next;
    }



    DBG_LEAVE();
    return(FALSE);
}













/*
 * bssgp_udt_flush_tlli
 *
 * Flushes (discards) all the buffers of a particular TLLI
 *
 */
void
bssgp_udt_flush_tlli(TLLI tlli)
{
    DBG_FUNC("bssgp_udt_flush_tlli",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    int count;
    PBSSGP_UDT_TLLI_Q tlli_q;
    UINT32  bytes_freed,buffs_freed;

    tlli_q = bssgp_udt_config->tlli_q_head;
    if (tlli_q == NULL) 
    {
        DBG_ERROR("No buffers in Q\n");
        DBG_LEAVE();
        return;
    }
    
    while( (tlli_q->next != NULL) && (tlli_q->tlli != tlli) ) 
    {
        tlli_q = tlli_q->next;
    }
    
    if (tlli_q->tlli == tlli) 
    {
        /* Found the tlli list */
        if (tlli_q->buff_count == 0) 
        {
            DBG_TRACE("No buffers in Q for TLLI %#x to be flushed\n",tlli);
        }
        else 
        {
            for(count=0;count<BSSGP_MAX_NETWORK_QOS_LEVELS;count++) 
            {
                if (tlli_q->qos_queues[count].buff_count > 0) 
                {
                    bytes_freed = buffs_freed = 0;
                    bssgp_udt_free_qos_list_buffs(&tlli_q->qos_queues[count],&bytes_freed,&buffs_freed);
                    tlli_q->buff_count -= buffs_freed;
                    tlli_q->rx_bytes -= bytes_freed;
                }
            }
            if ( (tlli_q->qos_queues[count].buff_count > 0) || (tlli_q->qos_queues[count].rx_bytes > 0) ) 
            {
                DBG_ERROR("Memory leak detected while trying to flush Q\n");
            }
        }
    }

    DBG_LEAVE();
    return;
}















/*
 * bssgp_udt_allocate_buff_pool
 *
 *  Initial allocation of the pool of buffers which are
 *  used to queue up the data in a FIFO style
 */
BOOL
bssgp_udt_allocate_buff_pool()
{
    DBG_FUNC("bssgp_udt_allocate_buff_pool",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    BOOL status = FALSE;
    UINT32 count,reverse_count;
    PBSSGP_UDT_BUFFER udt_buff;

    bssgp_udt_config->udt_buff_pool = (PBSSGP_UDT_BUFF_POOL_ENTRY)malloc(sizeof(BSSGP_UDT_BUFF_POOL_ENTRY) * bssgp_udt_config->udt_buff_count);
    if (bssgp_udt_config->udt_buff_pool == NULL)
    {
        DBG_ERROR("Could not allocate memory for UDT buffer pool entry array %d\n",count);
        free(bssgp_udt_config->udt_buff_pool);
        DBG_LEAVE();
        return (status);
    }
    DBG_TRACE("UDT buffer array allocated successfully\n");

    for(count=0;count<bssgp_udt_config->udt_buff_count;count++) 
    {
        bssgp_udt_config->udt_buff_pool[count].entry = (PBSSGP_UDT_BUFFER)malloc(sizeof(BSSGP_UDT_BUFFER));
        if (bssgp_udt_config->udt_buff_pool[count].entry == NULL) 
        {
            DBG_ERROR("Could not allocate memory for UDT buffer %d\n",count);
            for(reverse_count=count-1;reverse_count>=0;reverse_count--) 
            {
                free(bssgp_udt_config->udt_buff_pool[reverse_count].entry);
                free(bssgp_udt_config->udt_buff_pool);
                DBG_LEAVE();
                return (status);
            }
        }
        else 
        {
            udt_buff = bssgp_udt_config->udt_buff_pool[count].entry;
            memset(udt_buff,0,sizeof(BSSGP_UDT_BUFFER));
            bssgp_udt_config->udt_buff_pool[count].used  = FALSE;
            udt_buff->magic = BSSGP_UNITDATA_MAGIC;
            udt_buff->index = count;
        }
    }
    DBG_TRACE("All UDT buffers allocated successfully\n");
    DBG_LEAVE();
    return (TRUE);
}






/*
 * bssgp_udt_free_buff_pool
 */
BOOL
bssgp_udt_free_buff_pool()
{
    DBG_FUNC("bssgp_udt_free_buff_pool",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    UINT32 count;
    DBG_TRACE("Freeing alll UDT buffers\n");
    for(count=0;count<bssgp_udt_config->udt_buff_count;count++) 
    {
        free(bssgp_udt_config->udt_buff_pool[count].entry);
    }
    free(bssgp_udt_config->udt_buff_pool);
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_udt_get_buff_from_pool
 */
PBSSGP_UDT_BUFFER
bssgp_udt_get_buff_from_pool()
{
    DBG_FUNC("bssgp_udt_get_buff_from_pool",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    UINT32 count;


    for(count=0;count<bssgp_udt_config->udt_buff_count;count++) 
    {
        if (bssgp_udt_config->udt_buff_pool[count].used == FALSE) 
        {
            bssgp_udt_config->udt_buff_pool[count].used = TRUE;
            DBG_LEAVE();
            return(bssgp_udt_config->udt_buff_pool[count].entry);
        }
    }

    DBG_ERROR("Out of UDT buffers!!\n");

    DBG_LEAVE();
    return (NULL);
}




/*
 * bssgp_udt_return_buff_to_pool
 */
void
bssgp_udt_return_buff_to_pool(PBSSGP_UDT_BUFFER udt_buff)
{
    DBG_FUNC("bssgp_udt_return_buff_to_pool",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();

    
    if (udt_buff == NULL) 
    {
        DBG_ERROR("Invalid buffer : NULL returned into UDT buffer pool !\n");
    }
    else if (udt_buff->magic != BSSGP_UNITDATA_MAGIC) 
    {
        DBG_ERROR("Invalid buffer %#x returned into UDT buffer pool !\n",udt_buff);
    }
    else 
    {
        if (udt_buff->index >= bssgp_udt_config->udt_buff_count) 
        {
            DBG_ERROR("Invalid buffer %#x:%d returned into UDT buffer pool !\n",udt_buff,udt_buff->index);
        }
        else if (udt_buff != bssgp_udt_config->udt_buff_pool[udt_buff->index].entry) 
        {
            DBG_ERROR("Invalid buffer %#x:%d != %#x returned into UDT buffer pool !\n",
                udt_buff,udt_buff->index,
                bssgp_udt_config->udt_buff_pool[udt_buff->index].entry);
        }
        else 
        {
            udt_buff->magic                                         = BSSGP_UNITDATA_MAGIC;
            bssgp_udt_config->udt_buff_pool[udt_buff->index].used  = FALSE;
        }
    }
    DBG_LEAVE();
    return;
}


/*
 * bssgp_udt_allocate_tlli_pool
 */
BOOL
bssgp_udt_allocate_tlli_pool()
{
    DBG_FUNC("bssgp_udt_allocate_tlli_pool",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    BOOL status = FALSE;
    UINT32 count,reverse_count;
    PBSSGP_UDT_TLLI_Q tlli_q;

    bssgp_udt_config->tlli_pool = (PBSSGP_UDT_TLLI_POOL_ENTRY)malloc(sizeof(BSSGP_UDT_TLLI_POOL_ENTRY)*bssgp_udt_config->tlli_list_count);
    if (bssgp_udt_config->tlli_pool == NULL)
    {
        DBG_ERROR("Could not allocate memory for TLLI Pool List Array %d\n",count);
        free(bssgp_udt_config->tlli_pool);
        DBG_LEAVE();
        return (status);
    }
    DBG_TRACE("TLLI list array allocated successfully\n");
        
    for(count=0;count<bssgp_udt_config->tlli_list_count;count++) 
    {
        bssgp_udt_config->tlli_pool[count].entry = (PBSSGP_UDT_TLLI_Q)malloc(sizeof(BSSGP_UDT_TLLI_Q));
        if (bssgp_udt_config->tlli_pool[count].entry == NULL) 
        {
            DBG_ERROR("Could not allocate memory for TLLI List %d\n",count);
            for(reverse_count=count-1;reverse_count>=0;reverse_count--) 
            {
                free(bssgp_udt_config->tlli_pool[reverse_count].entry);
                free(bssgp_udt_config->tlli_pool);
                DBG_LEAVE();
                return (status);
            }
        }
        else 
        {
            tlli_q = bssgp_udt_config->tlli_pool[count].entry;
            memset(tlli_q,0,sizeof(BSSGP_UDT_TLLI_Q));
            bssgp_udt_config->tlli_pool[count].used = FALSE;
            tlli_q->magic           = BSSGP_UNITDATA_MAGIC;
    
            for(int index=0;index<BSSGP_MAX_NETWORK_QOS_LEVELS;index++) 
            {
                tlli_q->qos_queues[index].buff_list_head    = NULL;
                tlli_q->qos_queues[index].buff_list_tail    = NULL;
                tlli_q->qos_queues[index].qos_level         = (BSSGP_NETWORK_QOS_LEVEL)index;
            }
            tlli_q->index           = count;
        }
    }
    DBG_TRACE("All TLLI lists allocated successfully\n");

    DBG_LEAVE();
    return (TRUE);
}






/*
 * bssgp_udt_free_tlli_pool
 */
BOOL
bssgp_udt_free_tlli_pool()
{
    DBG_FUNC("bssgp_udt_free_tlli_pool",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    UINT32 count;

    DBG_TRACE("Freeing all TLLI lists\n");
    for(count=0;count<bssgp_udt_config->tlli_list_count;count++) 
    {
        free(bssgp_udt_config->tlli_pool[count].entry);
    }
    free(bssgp_udt_config->tlli_pool);
    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_udt_get_tlli_list_from_pool
 */
PBSSGP_UDT_TLLI_Q
bssgp_udt_get_tlli_list_from_pool()
{
    DBG_FUNC("bssgp_udt_get_tlli_list_from_pool",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    UINT32 count;

    for(count=0;count<bssgp_udt_config->tlli_list_count;count++) {
        if (bssgp_udt_config->tlli_pool[count].used == FALSE) {
            bssgp_udt_config->tlli_pool[count].used = TRUE;
            DBG_LEAVE();
            return(bssgp_udt_config->tlli_pool[count].entry);
        }
    }

    DBG_ERROR("Out of TLLI lists!!\n");


    DBG_LEAVE();
    return (NULL);
}


/*
 * bssgp_udt_return_tlli_list_to_pool
 */
void
bssgp_udt_return_tlli_list_to_pool(PBSSGP_UDT_TLLI_Q tlli_q)
{
    DBG_FUNC("bssgp_udt_return_tlli_list_to_pool",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();

    if (tlli_q == NULL) {
        DBG_ERROR("Invalid TLLI list : NULL returned into TLLI list pool !\n");
    }
    else if (tlli_q->magic != BSSGP_UNITDATA_MAGIC) {
        DBG_ERROR("Invalid TLLI list %#x returned into TLLI list pool !\n",tlli_q);
    }
    else {
        if (tlli_q->index >= bssgp_udt_config->tlli_list_count) {
            DBG_ERROR("Invalid TLLI list %#x:%d returned into TLLI list pool !\n",tlli_q,tlli_q->index);
        }
        else if (tlli_q != bssgp_udt_config->tlli_pool[tlli_q->index].entry) {
            DBG_ERROR("Invalid TLLI list %#x:%d != %#x returned into TLLI list pool !\n",tlli_q,tlli_q->index,
                bssgp_udt_config->tlli_pool[tlli_q->index].entry);
        }
        else {
            int index = tlli_q->index;
            if (tlli_q->qos_queues[index].buff_list_head != NULL) {
                DBG_ERROR("Memory leak detected; trying to free TLLI Q %d without freeing %d buffers\n",
                    index,tlli_q->qos_queues[index].buff_count);
                UINT32 bytes_freed=0,buffs_freed=0;
                bssgp_udt_free_qos_list_buffs(&tlli_q->qos_queues[index],&bytes_freed,&buffs_freed);
                tlli_q->buff_count -= buffs_freed;
                tlli_q->rx_bytes -= bytes_freed;
            }

            memset(tlli_q,0,sizeof(BSSGP_UDT_TLLI_Q));
            bssgp_udt_config->tlli_pool[index].used = FALSE;
            tlli_q->magic           = BSSGP_UNITDATA_MAGIC;
            for(index=0;index<BSSGP_MAX_NETWORK_QOS_LEVELS;index++) {
                tlli_q->qos_queues[index].buff_list_head    = NULL;
                tlli_q->qos_queues[index].buff_list_tail    = NULL;
                tlli_q->qos_queues[index].qos_level         = (BSSGP_NETWORK_QOS_LEVEL)index;
            }
            tlli_q->index   = index;
        }
    }
    DBG_LEAVE();
    return;
}


/*
 * bssgp_udt_free_qos_buffs
 */
void
bssgp_udt_free_qos_list_buffs(PBSSGP_UDT_QOS_Q qos_q, UINT32 *bytes_freed, UINT32 *buffs_freed)
{

    DBG_FUNC("bssgp_udt_free_qos_buffs",BSSGP_UNITDATA_LAYER);
    DBG_ENTER();
    PBSSGP_UDT_BUFFER udt_buff;
    udt_buff = qos_q->buff_list_head;
    if (udt_buff == NULL) {
        *bytes_freed = 0;
        *buffs_freed = 0;
        qos_q->buff_list_head = 
        qos_q->buff_list_tail = NULL;
        DBG_LEAVE();

    }
    while(udt_buff->next != NULL) {
        udt_buff = udt_buff->next;
        qos_q->rx_bytes -= (udt_buff->prev)->data_size;
        *bytes_freed += (udt_buff->prev)->data_size;
        bssgp_udt_return_buff_to_pool(udt_buff->prev);
        qos_q->buff_count--;
        *buffs_freed += 1;
    }
    bssgp_udt_return_buff_to_pool(udt_buff);

    qos_q->rx_bytes -= (udt_buff->prev)->data_size;
    *bytes_freed += (udt_buff->prev)->data_size;
    qos_q->buff_count--;
    *buffs_freed += 1;

    qos_q->buff_list_head =
    qos_q->buff_list_tail = NULL;

    if ((qos_q->buff_count > 0) || (qos_q->rx_bytes > 0)) {
        DBG_ERROR("Memory leak!!\n");
    }

    DBG_LEAVE();
    return;
}

