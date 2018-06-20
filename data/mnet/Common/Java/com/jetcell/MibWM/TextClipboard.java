//////////////////////////////////////////////////////////////////////////////
//
//  Class  : TextClipboard
//  Desc   : Clipboard abstraction for working in both applet and application
//  Author : George Zhao
//  Created: 6/8/98
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import java.io.*;
import java.awt.*;
import java.awt.datatransfer.*;

public abstract class TextClipboard
{
	private static String clipboard;

    private static ClipboardOwner defaultClipboardOwner = new ClipboardObserver();

    static class ClipboardObserver implements ClipboardOwner 
	{
        public void lostOwnership(Clipboard clipboard, Transferable contents) 
		{
        }
    }


	/**
	 *	Set the value of the clipboard
	 *  @param	s	string value
	 */
	public static synchronized void setText(String s)
	{
		if (s == null)
			s = "";

		if (Util.applet != null)
			clipboard = s;	
		else
			Toolkit.getDefaultToolkit().getSystemClipboard().setContents(new StringSelection(s), defaultClipboardOwner);
	}


	/**
	 *	Get the string contents of the clipboard
	 */
	public static synchronized String getText()
	{
		if (Util.applet != null)
			return clipboard;
		else
		{
			String s = null;
			Transferable t = Toolkit.getDefaultToolkit().getSystemClipboard().getContents(defaultClipboardOwner);
			if (t == null)
				return "";
			else
				try
				{
					 s = (String)(t.getTransferData(DataFlavor.stringFlavor));
				}
				catch (UnsupportedFlavorException e)
				{
					System.out.println(e);
				}
				catch (IOException e)
				{
					System.out.println(e);
				}
				finally
				{
					return s;
				}
		}
	}
}

//$History:$