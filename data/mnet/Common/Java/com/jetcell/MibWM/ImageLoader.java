//////////////////////////////////////////////////////////////////////////////
//
//  Class  : ImageLoader
//  Desc   : Use to load images from the Images directory.
//  Author : George Zhao
//  History: April 22, 1999 Intial Creation.
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import java.awt.*;
import java.net.*;
import java.util.*;
import javax.swing.*;
import com.jetcell.MibWM.Images.ImageStub;

public class ImageLoader 
{
    private static Hashtable imageCache = new Hashtable();


	/**
	 *	load image from com/jetcell/MibWM/Images
	 */
    public synchronized static Image loadImage(String filename)
    {
        Image image = null;
        
        /*
        if (ImageLoader.imageInCache(filename))
            image = ImageLoader.getImageFromCache(filename);
        else*/
        {
            if ( Util.applet != null ) 
			{

                /**
                 *  load using applet method
                 */
                
             	try {
                    URL url = new URL(Util.applet.getCodeBase()+ "com/jetcell/MibWM/Images/" + filename);
                    image = Util.applet.getImage(url);
                }
                catch(Exception e) {}
                //image = Util.applet.getImage(Util.applet.getCodeBase(), "com/jetcell/MibWM/Images/" + filename);
                
                
                if (image == null) System.out.println("Image is NULL: " +filename);
            } 
			else 
			{
                /**
                 * load using application method
                 */

    		    URL url = new ImageStub().getClass().getResource(filename);
    		    
    		    Toolkit tk = java.awt.Toolkit.getDefaultToolkit();

    		    try 
				{
    		        image = tk.createImage((java.awt.image.ImageProducer) url.getContent());
    		    } 
				catch (java.io.IOException e) 
				{
    		        System.out.println("[ImageLoader] " + e.toString());
    		    }
            }

			/**
    		 *  block while waiting for image to load
			 */
			
			if (image != null)
			{
                MediaTracker tracker = new MediaTracker(new Component() {} );
                tracker.addImage(image, 0);
                try 
			    {
                    tracker.waitForID(0);
                } 
			    catch (InterruptedException e) 
			    {
                    System.out.println("[ImageLoader] " + e.toString());
                }

                /**
			    *  add to cache
			    */
                ImageLoader.putImageInCache(filename, image);
            }
        }

        return image;
    }


	/**
	 *	load icon from com/mci/vdds/Images
	 */
    public synchronized static ImageIcon loadIcon(String filename)
	{
		return new ImageIcon(loadImage(filename));
	}

    private static boolean imageInCache(String filename)
    {
        if (imageCache.containsKey(filename))
            return true;
        else
            return false;
    }

    private static Image getImageFromCache(String filename)
    {
        return (Image)imageCache.get(filename);
    }

    private static void putImageInCache(String filename, Image image)
    {
        imageCache.put(filename, image);
    }
    
    public static void main(String argv[])
    {
        JFrame f = new JFrame();
        ImageIcon icon = ImageLoader.loadIcon("closed.gif");
        JButton b = new JButton(icon);
        System.out.println("after new messagepanel");
        f.setBounds(0,0,100,100);
        f.getContentPane().add(b);
        f.setVisible(true);
    }
}
//$History:$