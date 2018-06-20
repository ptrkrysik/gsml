#include "parser_if.h"

#include "oam_api.h"
#include "mibtags.h"
#include "MnetModuleId.h"

/*
 * Parse chains for adjacent cell configuration
 */
#define ALTERNATE       NONE
#include "../include/cfg_adjcell.h"
LINK_POINT(cfg_adj_cell, ALTERNATE);
#undef  ALTERNATE


static parser_extension_request cfg_bts_adjcell_init_table[] = {
  { PARSE_ADD_CFG_GP10_ADJCELL, &pname(cfg_adj_cell) },
  { PARSE_LIST_END, NULL }
};

/*
 * Register the parser chain
 */
void cfg_bts_adjcell_parser_init (void)
{
  
  parser_add_command_list(cfg_bts_adjcell_init_table,"config");
}


void adjcell_nvgen (void)
{
  int cellnum;
  AdjCell_ReselectionEntry res_entry;
  AdjCell_HandoverEntry hand_entry;

  for (cellnum = 0;cellnum < 16;cellnum ++) {
    printf("\n\n config-adjcell %d",cellnum+1);
    oam_getMibTblEntry(MIBT_adjCell_reselectionEntry,cellnum,&res_entry,
		       sizeof(AdjCell_ReselectionEntry));
    oam_getMibTblEntry(MIBT_adjCell_handoverEntry,cellnum,&hand_entry,
		       sizeof(AdjCell_HandoverEntry));
    
    if (hand_entry.adjCell_handoverCellID)
      printf("\n\t handover selected");
    else 
      printf("\n\t no handover selected");
    
    if (hand_entry.adjCell_synchronized)
      printf("\n\t handover synchronised");
    else 
      printf("\n\t no handover synchronised");


    printf("\n\t handover bcch-frequency %d handover-margin %d max-tx-power %d min-rx-level %d",
	   hand_entry.adjCell_bCCHFrequency,hand_entry.adjCell_hoMargin,
	   hand_entry.adjCell_msTxPwrMaxCell,hand_entry.adjCell_rxLevMinCell);
    
    printf("\n\t handover cell-global-identity mcc %d",hand_entry.adjCell_mcc);
    printf("\n\t handover cell-global-identity mnc %d",hand_entry.adjCell_mnc);
    printf("\n\t handover cell-global-identity lac %d",hand_entry.adjCell_lac);
    printf("\n\t handover cell-global-identity ci %d",hand_entry.adjCell_ci);

    printf("\n\t handover bsic ncc %d",hand_entry.adjCell_ncc);
    printf("\n\t handover bsic bcc %d",hand_entry.adjCell_cid);

    printf("\n\t reselection bcch-frequency %d ",res_entry.adjCell_reselectionBCCHFrequency);
    if (res_entry.adjCell_reselectionCellID)
      printf("\n\t reselection selected");
    else 
      printf("\n\t no reselection selected");
  }
}


/*
 * Configuration handler routine for adjacent cell parameters
 */

void adj_cell_parameters (parseinfo *csb)
{
  int cellnum;
  

  cellnum = (int) GETOBJ(udb,current1);

  
  switch (csb->which) {
  case ADJ_CELL_RES_BCCH :
    if (csb->sense) {
      oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_reselectionBCCHFrequency,
			   cellnum - 1,GETOBJ(int,1));
    } else {
      oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_reselectionBCCHFrequency,
			   cellnum - 1,0);
    }
    break;
    
  case ADJ_CELL_RES :
    if (csb->sense) {
      oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_reselectionCellID,
			   cellnum - 1,1);
    } else {
      oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_reselectionCellID,
			   cellnum - 1,0);
    }
    break;

  case ADJ_CELL_CC_VALUE :
    switch (GETOBJ(int,1)) {
    case BSIC_NCC :
      if (csb->sense) {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_ncc,
			     cellnum - 1,GETOBJ(int,2));
      } else {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_ncc,
			     cellnum - 1,0);
      }
      break;
    case BSIC_BCC :
      if (csb->sense) {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_cid,
			     cellnum - 1,GETOBJ(int,2));
      } else {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_cid,
			     cellnum - 1,0);
      }
      break;
    }
    break;
  case ADJ_CELL_GI_VALUE :
    if (csb->nvgen) {
      return;
    }
    switch (GETOBJ(int,1)) {
    case GI_MCC :
      if (csb->sense) {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_mcc,
			     cellnum - 1,GETOBJ(int,2));
      } else {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_mcc,
			     cellnum - 1,0);
      }
      break;
    case GI_MNC :
      if (csb->sense) {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_mnc,
			     cellnum - 1,GETOBJ(int,2));
      } else {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_mnc,
			     cellnum - 1,0);
      }
      break;
    case GI_LAC :
      if (csb->sense) {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_lac,
			     cellnum - 1,GETOBJ(int,2));
      } else {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_lac,
			     cellnum - 1,0);
      }
      break;
    case GI_CI :
      if (csb->sense) {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_ci,
			     cellnum - 1,GETOBJ(int,2));
      } else {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_ci,
			     cellnum - 1,0);
      }
      break;
    }
    break;

  case ADJ_CELL_BCCH :
    if (csb->nvgen) {
      return;
    }
    oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_bCCHFrequency,
			     cellnum - 1,GETOBJ(int,1));
    oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_hoMargin,
			     cellnum - 1,GETOBJ(int,2));
    oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_msTxPwrMaxCell,
			     cellnum - 1,GETOBJ(int,3));
    oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_rxLevMinCell,
			     cellnum - 1,GETOBJ(int,4));
    break;

  case ADJ_CELL_SELECTION :
    if (csb->nvgen) {
      return;
    }
    switch (GETOBJ(int,1)) {
    case CELL_SELECTED :
      if (csb->sense) {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_handoverCellID,
			     cellnum - 1,GETOBJ(int,1));
      } else {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_handoverCellID,
			     cellnum - 1,0);
      }
      break;
    case CELL_SYNCHRONISED :
      if (csb->sense) {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_synchronized,
			     cellnum - 1,GETOBJ(int,1));
      } else {
	oam_setTblEntryField(CLI_MODULE_ID,MIBT_adjCell_synchronized,
			     cellnum - 1,0);
      }
      break;
    }
    break;
  }
}
