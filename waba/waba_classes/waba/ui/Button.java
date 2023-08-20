/* $Id: Button.java,v 1.5 2001/12/28 02:10:41 isachan Exp $

Copyright (c) 1998, 1999 Wabasoft  All rights reserved.

This software is furnished under a license and may be used only in accordance
with the terms of that license. This software and documentation, and its
copyrights are owned by Wabasoft and are protected by copyright law.

THIS SOFTWARE AND REFERENCE MATERIALS ARE PROVIDED "AS IS" WITHOUT WARRANTY
AS TO THEIR PERFORMANCE, MERCHANTABILITY, FITNESS FOR ANY PARTICULAR PURPOSE,
OR AGAINST INFRINGEMENT. WABASOFT ASSUMES NO RESPONSIBILITY FOR THE USE OR
INABILITY TO USE THIS SOFTWARE. WABASOFT SHALL NOT BE LIABLE FOR INDIRECT,
SPECIAL OR CONSEQUENTIAL DAMAGES RESULTING FROM THE USE OF THIS PRODUCT.

WABASOFT SHALL HAVE NO LIABILITY OR RESPONSIBILITY FOR SOFTWARE ALTERED,
MODIFIED, OR CONVERTED BY YOU OR A THIRD PARTY, DAMAGES RESULTING FROM
ACCIDENT, ABUSE OR MISAPPLICATION, OR FOR PROBLEMS DUE TO THE MALFUNCTION OF
YOUR EQUIPMENT OR SOFTWARE NOT SUPPLIED BY WABASOFT.
*/

package waba.ui;

import waba.fx.*;
import waba.sys.Vm;

/**
 * Button is a push button control.
 * <p>
 * Here is an example showing a push button being used:
 *
 * <pre>
 * public class MyProgram extends MainWindow
 * {
 * Button pushB;
 *
 * public void onStart()
 *  {
 *  pushB = new Button("Push me");
 *  pushB.setRect(10, 10, 80, 30);
 *  add(pushB);
 *  }
 *
 * public void onEvent(Event event)
 *  {
 *  if (event.type == ControlEvent.PRESSED &&
 *      event.target == pushB)
 *   {
 *   ... handle pushB being pressed
 * </pre>
 */

public class Button extends Control
{
    //static  boolean isToggledDownStaticCopy;
    
    Image   buttonImage;
    String  text;
    Font    font;

    boolean armed;
    boolean isTogglable;
    boolean isToggledDown;

    int     imgWidth, imgHeight;



    /** Creates a button displaying the given text. */
    public Button(String text)
    {
    	this.init(text, null, false);
    }

    public Button(String text, boolean togglable)
    {
    	this.init(text, null, togglable);
    }

    public Button(Image passedButtonImage)
    {
    	this.init(null, passedButtonImage, false);
    }

    public Button(Image passedButtonImage, boolean togglable)
    {
    	this.init(null, passedButtonImage, togglable);
    }

    public Button(String text, Image passedButtonImage)
    {
    	this.init(text, passedButtonImage, false);
    }

    public Button(String text, Image passedButtonImage, boolean togglable)
    {
    	this.init(text, passedButtonImage, togglable);
    }

    private void init(String text, Image passedButtonImage, boolean togglable)
    {
    	this.font           = MainWindow.defaultFont;

        this.isTogglable    = togglable;
        this.isToggledDown  = false;
        
        if(text != null)
            this.text       = text;
            
    	if(passedButtonImage != null)
	    {
		buttonImage    = passedButtonImage;
		imgWidth       = buttonImage.getWidth();
		imgHeight      = buttonImage.getHeight();
	    }
    }

    public void press()
    {
        armed = true;
        isToggledDown = true;

        repaint();
    }

    public void release()
    {
        armed = false;
        isToggledDown = false;

        repaint();
    }

    /** Sets the text that is displayed in the button. */
    public void setText(String text)
    {
    	this.text = text;
    	repaint();
    }

    /** Gets the text displayed in the button. */
    public String getText()
    {
    	return text;
    }

    /** Sets the image that is displayed in the button. */
    public void setImage(Image passedButtonImage)
    {
	this.buttonImage = passedButtonImage;

	imgWidth    = buttonImage.getWidth();
    	imgHeight   = buttonImage.getHeight();

	repaint();
    }

    /** Gets the image that is displayed in the button. */
    public Image getImage()
    {
	return this.buttonImage;
    }

    /** Gets the image that is displayed in the button. */
    public int getImageWidth()
    {
	return imgWidth;
    }

    /** Gets the image that is displayed in the button. */
    public int getImageHeight()
    {
	return imgHeight;
    }

    /** Called by the system to pass events to the button. */
    public void onEvent(Event evnt)
    {
    	if(evnt.type == PenEvent.PEN_DOWN)
	    {
    		armed = true;

		if(isTogglable)
		    {
			isToggledDown = !isToggledDown;
                
			//isToggledDownStaticCopy = isToggledDown;
		    }

    		repaint();
	    }
    	else if(evnt.type == PenEvent.PEN_UP)
	    {
    		armed = false;

    		repaint();

    		PenEvent pe = (PenEvent)evnt;

    		if(pe.x >= 0 && pe.x < this.width && pe.y >= 0 && pe.y < this.height)
		    postEvent(new ControlEvent(ControlEvent.PRESSED, this));
	    }
    	else if(evnt.type == PenEvent.PEN_DRAG)
	    {
    		PenEvent pe = (PenEvent)evnt;

    		boolean lArmed = false;

    		if(pe.x >= 0 && pe.x < this.width && pe.y >= 0 && pe.y < this.height)
		    lArmed = true;

    		if(armed != lArmed)
		    {
    			armed = lArmed;
    			repaint();
		    }
	    }
    }

    //public static void drawButton(Graphics g, boolean armed, int width, int height)
    public void drawButton(Graphics g, boolean armed, int width, int height)
    {
    	boolean isColor = Vm.isColor();

    	int x2 = width - 1;
    	int y2 = height - 1;

    	if(!isColor)
	    {
    		// draw top, bottom, left and right lines
    		g.setColor(0, 0, 0);
    		g.drawLine(3, 0, x2 - 3, 0);
    		g.drawLine(3, y2, x2 - 3, y2);
    		g.drawLine(0, 3, 0, y2 - 3);
    		g.drawLine(x2, 3, x2, y2 - 3);

    		//if(armed)
    		if( (armed)||(isToggledDown) )
		    g.fillRect(1, 1, width - 2, height - 2);
    		else
		    {
    			// draw corners (tl, tr, bl, br)
    			g.drawLine(1, 1, 2, 1);
    			g.drawLine(x2 - 2, 1, x2 - 1, 1);
    			g.drawLine(1, y2 - 1, 2, y2 - 1);
    			g.drawLine(x2 - 2, y2 - 1, x2 - 1, y2 - 1);

    			// draw corner dots
    			g.drawLine(1, 2, 1, 2);
    			g.drawLine(x2 - 1, 2, x2 - 1, 2);
    			g.drawLine(1, y2 - 2, 1, y2 - 2);
    			g.drawLine(x2 - 1, y2 - 2, x2 - 1, y2 - 2);
		    }

	    }
    	else
	    {
    		// top, left
    		//if(armed)
    		if( (armed)||(isToggledDown) )
		    g.setColor(0, 0, 0);
    		else
		    g.setColor(255, 255, 255);

    		g.drawLine(0, 0, x2 - 1, 0);
    		g.drawLine(0, 0, 0, y2 - 1);

    		// top, left shadow
    		//if(armed)
    		if( (armed)||(isToggledDown) )
		    {
    			g.setColor(130, 130, 130);
    			g.drawLine(1, 1, x2 - 1, 1);
    			g.drawLine(1, 1, 1, y2 - 1);
		    }

    		// bottom, right
    		//if(armed)
    		if( (armed)||(isToggledDown) )
		    g.setColor(255, 255, 255);
    		else
		    g.setColor(0, 0, 0);

    		g.drawLine(0, y2, x2, y2);
    		g.drawLine(x2, y2, x2, 0);

    		// bottom, right shadow
    		//if(!armed)
    		if( (!armed)||(!isToggledDown) )
		    {
    			g.setColor(130, 130, 130);
    			g.drawLine(1, y2 - 1, x2 - 1, y2 - 1);
    			g.drawLine(x2 - 1, y2 - 1, x2 - 1, 1);
		    }
	    }
    }

    /** Called by the system to draw the button. */
    public void onPaint(Graphics g)
    {
    	drawButton(g, armed, this.width, this.height);

    	// draw label
    	if (armed && !Vm.isColor())
	    g.setColor(255, 255, 255);
    	else
	    g.setColor(0, 0, 0);

    	g.setFont(font);

    	FontMetrics fm = getFontMetrics(font);

    	int fntX = (this.width  - fm.getTextWidth(text)) / 2;
    	int fntY = (this.height - fm.getHeight()) / 2;

    	int imgX = (this.width  - imgWidth) / 2;
	int imgY = (this.height - imgHeight) / 2;

        int offset = 0;

    	if( (armed)||(isToggledDown) )
	    {
		offset = 1;
    	    
		/*
		  fntX++;
		  fntY++;

		  imgX++;
		  imgY++;
    		*/
	    }
	/*
	  if( (text != null)&&(buttonImage != null) )
	  {
	  g.drawImage(buttonImage, imgX, imgY);
	  g.drawText(text, fntX, fntY);
	  }
	*/
    	if(text != null)
    	    g.drawText(text, fntX+offset, fntY+offset);

    	if(buttonImage != null)
    	    g.drawImage(buttonImage, imgX+offset, imgY+offset);
    }
}
