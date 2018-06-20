/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef  __RM_HEAD_H__
#define  __RM_HEAD_H__

#include "vxWorks.h"
#include "stdio.h"
#include "string.h"
#include "msgQlib.h"
#include "wdLib.h"
#include "taskLib.h"
#include "rm_extdef.h"
#include "MnetModuleId.h"
#include "logging\vclogging.h"
#include "lapdm\lapdm_common.h"
#include "lapdm\lapdm_config.h"
#include "lapdm\lapdm_l3intf.h"
#include "ril3\ril3irt.h"
#include "ril3\ril3md.h"
#include "ril3\ril3msgtype.h"
#include "ril3\ril3_common.h"
#include "ril3\ie_ia_rest_octets.h"
#include "ril3\ie_neighbour_cells_description.h"
#include "ril3\ie_rach_control_param.h"
#include "ril3\ie_control_channel_description.h"
#include "ril3\ie_cell_options.h"
#include "ril3\ie_cell_selection.h"
#include "ril3\ie_bearer_capability.h"
#include "ril3\com_freqlistbitmap.h"
#include "jcc\JCCHOMsg.h"
#include "jcc\JCCL3Msg.h"
#include "jcc\JCCbcCb.h"
#include "csunew\csu_head.h"
#include "defs.h"
#include "oam_api.h"
#include "pm\pm_class.h"
#include "grr\grr_intf.h"     //G2
#include "bssgp\bssgp_api.h"  //G2
#include "sys_init.h"
#include "rm_import.h"
#include "rm_prim.h"
#include "rml1_intf.h"
#include "rm_const.h"
#include "rm_types.h"
#include "rm_marco.h"
#include "rm_struct.h"
#include "rm_itcmsg.h"
#include "rm_proto.h"
#include "rm_oamif.h"
#include "rm_data.h"
#include "debug.h"

#endif // __RM_HEAD_H__
