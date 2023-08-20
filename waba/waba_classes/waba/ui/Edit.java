/* $Id: Edit.java,v 1.7 2002/01/08 11:10:04 cgrigis Exp $

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
 * Edit is a text entry control.
 * <p>
 * Here is an example showing an edit control being used:
 *
 * <pre>
 * public class MyProgram extends MainWindow
 * {
 * Edit edit;
 *
 * public void onStart()
 *  {
 *  edit = new Edit();
 *  edit.setRect(10, 10, 80, 30);
 *  add(edit);
 *  }
 * </pre>
 */

public class Edit extends Control
{
    Font font;
    FontMetrics fm;
    Graphics drawg; // only valid while the edit has focus

    Timer blinkTimer; // only valid while the edit has focus

    //NOTE: later these  should be bitflags in a state int in the Control class
    boolean hasFocus = false;
    boolean cursorShowing = false;
    boolean isPalmOS = false;

    /* Make it protected so as to be accessible by a subclass */
    protected char chars[] = new char[4];
    byte charWidths[] = new byte[4];
    Rect clipRect; // allocated when used and reused in many cases (see code)
    int totalCharWidth = 0;
    int len = 0;

    int insertPosition, lastInsertPosition;
    int startSelectPos;
    int xOffset;

    public Edit()
    {
	String platformStr = "PalmOS";
    
	if( platformStr.equals(Vm.getPlatform()) )
	    isPalmOS = true;
            
	this.font = MainWindow.defaultFont;
	fm = getFontMetrics (this.font);
    
	clearPosState();
    }

    /**
       Return the display representation of an internal character at a 
       given position.
       @param i position
       @return the display representation
    */
    protected char getDisplayChar (int i)
    {
	return chars [i];
    }

    /**
       Return the display representation of a given character.
       @param c the character
       @return the display representation
    */
    protected char getDisplayChar (char c)
    {
	return c;
    }

    /**
       Return the display representation of all the internal characters.
       @return the display representation
    */
    protected char [] getDisplayChars ()
    {
	return chars;
    }

    private void clearPosState()
    {
	insertPosition      = 0;
	lastInsertPosition  = 0;
    
	startSelectPos      = -1;
    
	if(Vm.isColor())
	    xOffset = 4;
	else
	    xOffset = 1;
    }

    private int XToCharPos(int x)
    {
	int cx = xOffset;
	for (int i = 0; i < len; i++)
	    {
		int cw = charWidths[i];
		if (x <= cx + (cw / 2))
		    return i;
		cx += cw;
	    }
	return len;
    }

    private int CharPosToX(int n)
    {
	int cx = xOffset;
	if (n > len)
	    n = len;
	for (int i = 0; i < n; i++)
	    cx += charWidths[i];
	return cx;
    }

    /**
     * Returns the text displayed in the edit control.
     */
    public String getText()
    {
	return new String(chars, 0, len);
    }

    /**
     * Sets the text displayed in the edit control.
     */
    public void setText(String s)
    {
	chars = s.toCharArray();
	len = chars.length;
	charWidths = null;
	totalCharWidth = 0;
	clearPosState();
	repaint();
    }

    private void draw(Graphics g, boolean cursorOnly)
    {
	/* to prevent Null exception pointer as suggested in bug #450570 */
	if( g == null ) return;

	g.setFont(font);
    
	if(charWidths == null)
	    {
		charWidths = new byte[len];
        
		totalCharWidth = 0;
        
		for(int i = 0; i < len; i++)
		    {
			int charWidth = fm.getCharWidth(getDisplayChar (i));
			charWidths[i] = (byte)charWidth;
			totalCharWidth += charWidth;
		    }
	    }
    
	int height = fm.getHeight();
	int xMin;
	int xMax;
	int y;
    
	if(Vm.isColor())
	    {
		xMin = 4;
		xMax = this.width - 4 - 1;
		height -= 1;
		y = this.height - height - 1;
	    }
	else
	    {
		xMin = 1;
		xMax = this.width - 1 - 1;
		y = this.height - height - 1;
	    }
    
	if(clipRect == null)
	    clipRect = new Rect(0, 0, 0, 0);

	//Get current clip rect and intersect with edit rect to set a new clip to draw in
	int cx1 = xMin;
	int cy1 = y;
	int cx2 = xMax;
	int cy2 = y + height - 1;
    
	Rect clip = g.getClip(clipRect);
    
	if(clip != null)
	    {
		//Intersect current clip rect and edit rect
		if (cx1 < clip.x)
		    cx1 = clip.x;
		if (cy1 < clip.y)
		    cy1 = clip.y;
            
		int clipX2 = clip.x + clip.width - 1;
        
		if (cx2 > clipX2)
		    cx2 = clipX2;
            
		int clipY2 = clip.y + clip.height - 1;
        
		if (cy2 > clipY2)
		    cy2 = clipY2;
	    }
    
	g.setClip(cx1, cy1, cx2 - cx1 + 1, cy2 - cy1 + 1);

	int x = xOffset;
    
	if(cursorOnly)
	    ;
	else if(startSelectPos == -1)
	    {
		//Draw unselected chars
		g.setColor(255, 255, 255); //Color after the text.
        
		if(isPalmOS)
		    g.setBackColor(255, 255, 255); //Edit field background
        
		g.fillRect(xMin, y, xMax - xMin + 1, height);
        
		g.setColor(0, 0, 0);
		g.setTextColor(0, 0, 0);
 		g.drawText(getDisplayChars (), 0, len, x, y);
	    }
	else
	    {
		//Character regions are:
		//0 to selection start-1 (sel1-1) to selection start (sel1) to
		//selection end-1 (sel2-1) to selection end (sel2) to last character
		int sel1 = startSelectPos;
		int sel2 = insertPosition;
        
		if(sel1 > sel2)
		    {
			int temp = sel1;
            
			sel1 = sel2;
			sel2 = temp;
		    }
        
		int sel1X = CharPosToX(sel1);
		int sel2X = CharPosToX(sel2);

		//0 to selection start-1 (sel1-1) to selection end (sel2) to last_char
		if(isPalmOS)
		    g.setBackColor(255, 255, 255);
        
		g.fillRect(xMin, y, sel1X - xMin, height);
		g.fillRect(sel2X, y, xMax - sel2X + 1, height); //<--- Here's the glitch 05/06/2000 last cursor doesn't go away.

		g.setColor(0, 0, 0);
        
		if(isPalmOS)
		    g.setTextColor(0, 0, 0);
            
 		g.drawText(getDisplayChars (), 0, sel1, x, y);
 		g.drawText(getDisplayChars (), sel2, len - sel2, sel2X, y);

		//Draw selection start (sel1) to selection end-1 (sel2-1)
		if(isPalmOS)
		    g.setColor(255, 255, 0);
		else
		    g.setColor(0, 0, 120);
            
		g.fillRect(sel1X, y, sel2X - sel1X, height);
        
		if(isPalmOS)
		    g.setBackColor(255, 255, 0);
		else
		    g.setColor(255, 255, 255);
            
 		g.drawText(getDisplayChars (), sel1, sel2 - sel1, sel1X, y);
	    }
    
	// restore clip rect
	if (clip == null)
	    g.clearClip();
	else
	    g.setClip(clip.x, clip.y, clip.width, clip.height);
    
	if(!cursorOnly)
	    {
		//Erase the space for the cursor at (xMin - 1)
		g.setColor(255, 255, 255);
		g.drawLine(xMin-1, y, xMin-1, y+height-1);
        
		if(isPalmOS)
		    g.drawCursor((CharPosToX(lastInsertPosition)-1), y, 1, height);
	    }
    
	if(hasFocus)
	    {
		//Draw cursor
		if(isPalmOS)
		    {
			if(cursorShowing)
			    g.setColor(0, 0, 0);
            
			g.drawCursor((CharPosToX(insertPosition)-1), y, 1, height);

			//Immediately set back to white.
			//Otherwise you'll see ugly black band behind the last character.
			g.setColor(255, 255, 255);
		    }
		else
		    {
			int cx = CharPosToX(insertPosition)-1;
			g.drawCursor(cx, y, 1, height);
		    }
        
		if(cursorOnly)
		    cursorShowing = !cursorShowing;
		else
		    cursorShowing = true;
	    }
	else
	    cursorShowing = false;
    }

    /** Called by the system to pass events to the edit control. */
    public void onEvent(Event event)
    {
	if(charWidths == null)
	    return; // widths have not been initialized - not added to hierarchy
        
	boolean redraw          = false;
	boolean extendSelect    = false;
	boolean clearSelect     = false;
    
	int newinsertPosition = insertPosition;
    
	switch(event.type)
	    {
	    case ControlEvent.TIMER:
		{
		    draw(drawg, true);
		    return;
		}
	    case ControlEvent.FOCUS_IN:
		{
		    drawg = createGraphics();
		    hasFocus = true;
		    redraw = true;
		    blinkTimer = addTimer(350);
		    break;
		}
	    case ControlEvent.FOCUS_OUT:
		{
		    hasFocus = false;
		    clearPosState();
		    newinsertPosition = lastInsertPosition = 0;
		    redraw = true;
		    removeTimer(blinkTimer);
		    break;
		}
	    case KeyEvent.KEY_PRESS:
		{
		    KeyEvent ke = (KeyEvent)event;
            
		    boolean isPrintable;
            
		    if (ke.key > 0 && ke.key < 65536 && (ke.modifiers & IKeys.ALT) == 0 &&
			(ke.modifiers & IKeys.CONTROL) == 0)
			isPrintable = true;
		    else
			isPrintable = false;

		    boolean isDelete = (ke.key == IKeys.DELETE);
		    boolean isBackspace = (ke.key == IKeys.BACKSPACE);
            
		    int del1 = -1;
		    int del2 = -1;
		    int sel1 = startSelectPos;
		    int sel2 = insertPosition;
            
		    if(sel1 > sel2)
			{
			    int temp = sel1;
			    sel1 = sel2;
			    sel2 = temp;
			}
		    if(sel1 != -1 && (isPrintable || isDelete || isBackspace))
			{
			    del1 = sel1;
			    del2 = sel2 - 1;
			}
		    else if(isDelete)
			{
			    del1 = insertPosition;
			    del2 = insertPosition;
			}
		    else if(isBackspace)
			{
			    del1 = insertPosition - 1;
			    del2 = insertPosition - 1;
			}
		    if(del1 >= 0 && del2 < len)
			{
			    int deleteCount = del2 - del1 + 1;
			    int numOnRight = len - del2 - 1;
                
			    for(int i = del1; i <= del2; i++)
				totalCharWidth -= charWidths[i];
                    
			    if(numOnRight > 0)
				{
				    Vm.copyArray(chars, del2 + 1, chars, del1, numOnRight);
				    Vm.copyArray(charWidths, del2 + 1, charWidths, del1, numOnRight);
				}
                
			    len -= deleteCount;
			    newinsertPosition = del1;
			    redraw = true;
			    clearSelect = true;
			}
            
		    if(isPrintable)
			{
			    // grow the array if required (grows by 8)
			    if(len == chars.length)
				{
				    char newChars[] = new char[len + 8];
				    Vm.copyArray(chars, 0, newChars, 0, len);
				    chars = newChars;
				    byte newCharWidths[] = new byte[len + 8];
				    Vm.copyArray(charWidths, 0, newCharWidths, 0, len);
				    charWidths = newCharWidths;
				}
                
			    char c = (char)ke.key;
                
			    int charWidth = fm.getCharWidth(getDisplayChar (c));
                
			    if(newinsertPosition != len)
				{
				    int i = newinsertPosition;
				    int l = len - newinsertPosition;
				    Vm.copyArray(chars, i, chars, i + 1, l);
				    Vm.copyArray(charWidths, i, charWidths, i + 1, l);
				}
                
			    chars[newinsertPosition] = c;
                
			    charWidths[newinsertPosition] = (byte)charWidth;
                
			    len++;
                
			    lastInsertPosition = newinsertPosition; //Isao
                
			    newinsertPosition++;
                
			    totalCharWidth += charWidth;
                
			    redraw = true;
                
			    clearSelect = true;
			}
            
		    boolean isMove = false;
            
		    switch (ke.key)
			{
			case IKeys.HOME:
			case IKeys.END:
			case IKeys.LEFT:
			case IKeys.RIGHT:
			case IKeys.UP:
			case IKeys.DOWN:
			    {
				isMove = true;
                    
				break;
			    }
			}
		    if(isMove)
			{
			    if (ke.key == IKeys.HOME)
				newinsertPosition = 0;
			    else if (ke.key == IKeys.END)
				newinsertPosition = len;
			    else if (ke.key == IKeys.LEFT || ke.key == IKeys.UP)
				newinsertPosition--;
			    else if (ke.key == IKeys.RIGHT || ke.key == IKeys.DOWN)
				newinsertPosition++;
                    
			    if(newinsertPosition != insertPosition)
				{
				    lastInsertPosition = insertPosition;
                    
				    if((ke.modifiers & IKeys.SHIFT) > 0)
					extendSelect = true;
				    else    
					clearSelect = true;
				}
			}
            
		    break;
		}
        
	    case PenEvent.PEN_DOWN:
		{
		    PenEvent pe = (PenEvent)event;
            
		    newinsertPosition = XToCharPos(pe.x);
            
		    if((pe.modifiers & IKeys.SHIFT) > 0) // shift
			extendSelect = true;
		    else
			clearSelect = true;
                
		    break;
		}
	    case PenEvent.PEN_DRAG:
		{
		    PenEvent pe = (PenEvent)event;
            
		    newinsertPosition = XToCharPos(pe.x);
            
		    if (newinsertPosition != insertPosition)
			{
			    lastInsertPosition = insertPosition;             
                
			    extendSelect = true;
			}
            
		    break;
		}
        
	    default:
		return;
	    }
        
	if(extendSelect)
	    {
		if(startSelectPos == -1)
		    {
			lastInsertPosition = insertPosition;
            
			startSelectPos = insertPosition;
		    }
		else if (newinsertPosition == startSelectPos)
		    {
			lastInsertPosition = -1;
			startSelectPos = -1;
		    }
            
		redraw = true;
	    }
    
	if(clearSelect && startSelectPos != -1)
	    {
		lastInsertPosition = -1;
		startSelectPos = -1;
        
		redraw = true;
	    }
    
	if (newinsertPosition > len)
	    newinsertPosition = len;
        
	if (newinsertPosition < 0)
	    newinsertPosition = 0;
        
	Graphics g = drawg;
    
	boolean insertChanged = (newinsertPosition != insertPosition);
    
	if(insertChanged && !redraw && cursorShowing)
	    {
		lastInsertPosition = insertPosition;
        
		draw(g, true); // erase cursor at old insert position
	    }
    
	if(insertChanged)
	    {
		int xMin;
		int xMax;
        
		if(Vm.isColor())
		    {
			xMin = 4;
			xMax = this.width - 4 - 1;
		    }
		else
		    {
			//xMin = 1;
			//xMax = this.width - 1 - 1;
            
			xMin = 1;
			xMax = this.width - 1;
		    }
        
		int x = CharPosToX(newinsertPosition);
        
		if(x - 5 < xMin)
		    {
			// characters hidden on left - jump
			xOffset += (xMin - x) + 20;
            
			if(xOffset > xMin)
			    xOffset = xMin;
                
			redraw = true;
		    }
		if(x + 5 > xMax)
		    {
			// characters hidden on right - jump
			xOffset -= (x - xMax) + 20;
			if (xOffset < xMax - totalCharWidth)
			    xOffset = xMax - totalCharWidth;
			redraw = true;
		    }
		if(totalCharWidth < xMax - xMin && xOffset != xMin)
		    {
			xOffset = xMin;
			redraw = true;
		    }
	    }
        
	lastInsertPosition = insertPosition;
    
	insertPosition = newinsertPosition;
    
	if(redraw)
	    draw(g, false);
	else if(insertChanged)
	    {
		//Draw cursor at new insert position
		//changed by Isao F. Yamashita 01/29/2001 
		if(isPalmOS)
		    draw(g, false);
		else
		    draw(g, true); 
	    }
    
	if(event.type == ControlEvent.FOCUS_OUT)
	    {
		drawg = null;
        
		clipRect = null;
	    }
    }

    /** Called by the system to draw the edit control. */
    public void onPaint(Graphics g)
    {
        int width = this.width;
        int height = this.height;
        int x2 = width - 1;
        int y2 = height - 1;

        if(Vm.isColor())
	    {
		// top, left
		g.setColor(127, 127, 127);
		g.drawLine(0, 0, x2 - 1, 0);
		g.drawLine(0, 0, 0, y2 - 1);

		// top, left shadow
		g.setColor(0, 0, 0);
		g.drawLine(1, 1, x2 - 2, 1);
		g.drawLine(1, 1, 1, y2 - 2);

		// bottom, right
		g.setColor(208, 208, 208);
		g.drawLine(0, y2, x2, y2);
		g.drawLine(x2, y2, x2, 0);

		g.setColor(255, 255, 255);
		g.fillRect(2, 2, width - 4, height - 4);
	    }
        else
	    {
		g.setColor(0, 0, 0);
		g.drawDots(0, y2, x2, y2);
	    }
    
        draw(g, false);
    
        clipRect = null;
    }//End of public void onPaint(Graphics g)
}
