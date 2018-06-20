#ifndef POST_H
#define POST_H

// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : post.h
// Author(s)   : Tim Olson
// Create Date : 7/26/99
// Description : 
//
// *******************************************************************


//********************************************************************
// Power on tests will execute prior to any application code being
// loaded or exectued.  The following functions may be called to 
// determine the outcome of a particular power on test.
// 
// A return value of "1" means the test failed.
//********************************************************************
extern bool postAllTestsOkay();
extern bool postMpcSramOkay();
extern bool postDsp0IntDataRamOkay();
extern bool postDsp0IntProgRamOkay();
extern bool postDsp0ExtRamOkay();
extern bool postDsp1IntDataRamOkay();
extern bool postDsp1IntProgRamOkay();
extern bool postDsp1ExtRamOkay();
extern bool postFPGA0Okay();
extern bool postFPGA1Okay();
extern bool postI2COkay();


#endif                                       // POST_H
