/* $Id: Label.java,v 1.4 2001/12/23 03:44:38 isachan Exp $

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

/**
 * Label is a text label control.
 * <p>
 * Here is an example showing a label being used:
 *
 * <pre>
 * public class MyProgram extends MainWindow
 * {
 * public void onStart()
 *  {
 *  Label label = new Label("Value:");
 *  label.setRect(10, 10, 80, 30);
 *  add(label);
 *  }
 * </pre>
 */

public class Label extends Control
{
    /** Constant for left alignment. */
    public static final int LEFT    = 0;
    /** Constant for center alignment. */
    public static final int CENTER  = 1;
    /** Constant for right alignment. */
    public static final int RIGHT   = 2;
    
    Image   labelImg;
    
    String  text;
    Font    font;
    int     align;
    
    int     imgWidth, imgHeight;
    
    
    
    /** 
     * Creates a label displaying the given text with left alignment.
     */
    public Label(String text)
    {
    	this(text, LEFT);
    }
    
    /**
     * Creates a label displaying the given text with the given alignment.
     * @param text the text displayed
     * @param align the alignment
     * @see #LEFT
     * @see #RIGHT
     * @see #CENTER
     */
    public Label(String text, int align)
    {
    	this.text = text;
    	this.align = align;
    	this.font = MainWindow.defaultFont;
    }
    
    public Label(Image labelImage)
    {
    	this.text = null;
    	this.align = LEFT;
    	this.labelImg = labelImage;
    	this.font = MainWindow.defaultFont;
    	
    	imgWidth    = labelImg.getWidth();
    	imgHeight   = labelImg.getHeight();
    }
    
    public Label(String text, Image passedlabelImg, int align)
    {
        this.text = null;
        this.align = align;
    	this.labelImg = passedlabelImg;
    	this.font = MainWindow.defaultFont;

    	imgWidth    = labelImg.getWidth();
    	imgHeight   = labelImg.getHeight();
    }
    
    /** Sets the image that is displayed in the label. */
    public void setImage(Image passedlabelImg)
    {
        this.labelImg = passedlabelImg;

        imgWidth    = labelImg.getWidth();
        imgHeight   = labelImg.getHeight();

        repaint();
    }
    
    public Image getImage()
    {
        return this.labelImg;
    }
    
    /** Sets the font that to be used in the label. */
    public void setFont(Font font)
    {
        this.font = font;
    }
    
    /** Sets the font that to be used in the label. */
    public Font getFont()
    {
        return this.font;
    }

    /** Sets the text that is displayed in the label. */
    public void setText(String text)
    {
    	this.text = text;
    	repaint();
    }
    
    /** Gets the text that is displayed in the label. */
    public String getText()
    {
    	return text;
    }
    
    
    
    /** Called by the system to draw the button. */
    public void onPaint(Graphics g)
    {
    	// draw label
    	g.setColor(0, 0, 0);    	
    	g.setFont(font);

    	FontMetrics fm = getFontMetrics(font);
    	
    	int fntX = 0;
    	int fntY = (this.height - fm.getHeight()) / 2;
    	
    	int imgX = (this.width  - imgWidth) / 2;
        int imgY = (this.height - imgHeight) / 2;
	    
    	if (align == CENTER)
	    fntX = (this.width - fm.getTextWidth(text)) / 2;
    	else if (align == RIGHT)
	    fntX = this.width - fm.getTextWidth(text);

        if(text != null)
    	    g.drawText(text, fntX, fntY);
    	    
    	if(labelImg != null)
    	    g.drawImage(labelImg, imgX, imgY);
    }
}
