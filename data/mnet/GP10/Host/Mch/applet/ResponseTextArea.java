////////////////////////////////////////////////////////////////////////////
//                          
//  FILE NAME: ResponseTextArea.java   
//                      
//  DESCRIPTION: This file contains routine for the response area class
//           
//      COMPONENTS:                      
//                              
//                                 
//  NOTES:                              
//                                  
//  (C) Copyright 1999 JetCell Inc., CA, USA http://www.jetcell.com/
//                                 
//  REVISION HISTORY                            
//__________________________________________________________________
//----------+--------+----------------------------------------------
// Name     |  Date  |  Reason                
//----------+--------+----------------------------------------------
// Bhawani  |2/08/00 | Initial Draft
//----------+--------+----------------------------------------------
//////////////////////////////////////////////////////////////////////////
//

import java.awt.TextArea;
import java.awt.Font;

public class ResponseTextArea extends TextArea
{
    public ResponseTextArea()
    {
        super("", 0, 0, SCROLLBARS_BOTH);
		super.setEditable(false);
		super.setFont(new Font("Courier", Font.PLAIN, 12));
    }
    
    public void addItem(String s)
    {
		append(s);
    }
}