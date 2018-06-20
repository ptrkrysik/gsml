////////////////////////////////////////////////////////////////////////////
//                          
//  FILE NAME: MchCommThread.java   
//                      
//  DESCRIPTION: This file contains a class that allow the applet to
//               communicate with the Tcp Server at the ViperCell
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

import java.net.*;
import java.io.*;
import java.awt.TextArea;


class MchCommThread extends Thread {
	private Socket socket;
	private BufferedReader in;
	private PrintWriter out;
	private ResponseTextArea p_txtArea;
	private boolean running;

	public MchCommThread(String addr, short port) {
		System.out.println("Initializing TCP/Ip socket environment");
		try {
			socket = new Socket(addr, port);
		} catch(IOException e) {
			// If the creation of the socket fails, 
			// nothing needs to be cleaned up.
			System.out.println("Could not connect to the Server" + addr + "at port " + port + ": " + socket);
		}
		try {    
			in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
			// Enable auto-flush:
			out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
			// register client's message group			
			out.print("MchApplication");  
			out.flush();
		} catch(IOException e) {
			// The socket should be closed on any 
			// failures other than the socket 
			// constructor:
			System.out.println("IO error in MchCommThread: ");
			try {
				socket.close();
			} catch(IOException e2) {}
		}
		// Otherwise the socket will be closed by
		// the run() method of the thread.
	}
	
	public void send(String str)
	{
		out.print(str);
		out.flush();
	}

	public void startReadThread(ResponseTextArea txtArea)
	{
		
		p_txtArea = txtArea;
		start();
	}	
	public void run() {
		try {
			running = true;
			while (running) {
				if(in.ready())
				{
					String str = in.readLine();
					p_txtArea.addItem(str + "\n");
				} else {
					try {
						this.sleep(100);
					} catch (InterruptedException ie) {
						System.out.println(" Interrupted Excution = " + ie);
					}
				}
			}
		} catch(IOException e3) {
			System.out.println(" IO expection at read thread" + e3 );			
		} finally {
			// Always close it:
			try {
				System.out.println("Socket is closing");
				socket.close();
			} catch(IOException e) {}
			// Ending this thread
		}
	}

	public void finishReadThread()
	{
		System.out.println("The read thread is being shutdown");
		running = false;
	}
}

