////////////////////////////////////////////////////////////////////////////
//                          
//  FILE NAME: mchControl.java   
//                      
//  DESCRIPTION: This file contains mchControl applet class
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
import java.awt.*;
import java.awt.event.*;
import java.net.*;
import java.applet.*;
import DialogLayout;
import ResponseTextArea;
import MchCommThread;

public class mchControl extends Applet implements ActionListener
{
	Container    m_Parent       = null;
	boolean      m_fInitialized = false;
	DialogLayout m_Layout;

	// Control definitions
	//--------------------------------------------------------------------------
	Button        sendButton;
	TextField     cmdTextField;
	Label         Lbl1;
	Label         Lbl2;
	ResponseTextArea  rspTextArea;
	Button Clear;
	MchCommThread commObj;

	public final static short TCP_SERVER_PORT = 10977;

	// Constructor
	//--------------------------------------------------------------------------
	public void init() 
	{
		Class[] param = { java.lang.String.class };
		m_Parent = this;
		
		System.out.println("init method has been called");
		CreateControls();	
    	commObj = new MchCommThread(getCodeBase().getHost(), TCP_SERVER_PORT);
		commObj.startReadThread(rspTextArea);
		
	}
	
	public void destroy()
	{
		commObj.finishReadThread();
		System.out.println("Destroy method has been called");
	}
	
	public void start()
	{
		System.out.println("The applet has started");
	}
		
	public void stop()
	{
		System.out.println("The applet has stopped");
	}
		

	// Initialization.
	//--------------------------------------------------------------------------
	public boolean CreateControls()
	{
		// CreateControls should be called only once
		//----------------------------------------------------------------------
		if (m_fInitialized || m_Parent == null)
			return false;

		// m_Parent must be extended from the Container class
		//----------------------------------------------------------------------
		if (!(m_Parent instanceof Container))
			return false;

		// Since a given font may not be supported across all platforms, it
		// is safe to modify only the size of the font, not the typeface.
		//----------------------------------------------------------------------
		Font OldFnt = m_Parent.getFont();
		if (OldFnt != null)
		{
			Font NewFnt = new Font(OldFnt.getName(), OldFnt.getStyle(), 8);

			m_Parent.setFont(NewFnt);
		}

		// All position and sizes are in dialog logical units, so we use a
		// DialogLayout as our layout manager.
		//----------------------------------------------------------------------
		m_Layout = new DialogLayout(m_Parent, 400, 295);
		m_Parent.setLayout(m_Layout);
		m_Parent.addNotify();

		Dimension size   = m_Layout.getDialogSize();
		Insets    insets = m_Parent.getInsets();
		
		m_Parent.setSize(insets.left + size.width  + insets.right,
						 insets.top  + size.height + insets.bottom);

		// Control creation
		//----------------------------------------------------------------------
		Lbl1 = new Label ("Command:", Label.LEFT);
		m_Parent.add(Lbl1);
		m_Layout.setShape(Lbl1, 10, 10, 56, 16);

		cmdTextField = new TextField ("Type your command and press send");
		m_Parent.add(cmdTextField);
		m_Layout.setShape(cmdTextField, 15, 26, 340, 16);		
		cmdTextField.setFont(new Font("Courier", Font.BOLD, 12));
		cmdTextField.addActionListener(this);
		
		sendButton = new Button ("Send");
		m_Parent.add(sendButton);
		m_Layout.setShape(sendButton, 15+340+10, 26, 40, 16);
		sendButton.addActionListener(this);
		
		
		Lbl2 = new Label ("Response:", Label.LEFT);
		m_Parent.add(Lbl2);
		m_Layout.setShape(Lbl2, 15, 50, 56, 16);

		rspTextArea = new ResponseTextArea();
		m_Parent.add(rspTextArea);
		m_Layout.setShape(rspTextArea, 15, 66, 400, 200);
		
		Clear = new Button("Clear");
		m_Parent.add(Clear);
		m_Layout.setShape(Clear, 370, 275, 40, 16);
		Clear.addActionListener(this);

		
		m_fInitialized = true;
		return true;
	}
	
    // Handling the WFC events
	//----------------------------------------------------------------------
	public void actionPerformed(ActionEvent evt) {
		
		
		if (evt.getSource().equals(sendButton))
		{
			String buttonName = ((Button)evt.getSource()).getLabel();
			//System.out.println("You clicked button " + buttonName);
			commObj.send(cmdTextField.getText());
		} else if (evt.getSource().equals(Clear)) {
			String buttonName = ((Button)evt.getSource()).getLabel();   
			// System.out.println("You clicked button " + buttonName);
			rspTextArea.setText("");
			
		} else {
			// System.out.println("You pressed Enter key");
			commObj.send(cmdTextField.getText());
		}
			
	
	}
	
}