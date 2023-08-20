/*
Copyright (c) 2000 - 2001 Amy High Craft, Ltd. All rights reserved.

This software is furnished under a license and may be used only in accordance
with the terms of that license. This software and documentation, and its
copyrights are owned by Wabasoft and are protected by copyright law.

THIS SOFTWARE AND REFERENCE MATERIALS ARE PROVIDED "AS IS" WITHOUT WARRANTY
AS TO THEIR PERFORMANCE, MERCHANTABILITY, FITNESS FOR ANY PARTICULAR PURPOSE,
OR AGAINST INFRINGEMENT. WABASOFT ASSUMES NO RESPONSIBILITY FOR THE USE OR
INABILITY TO USE THIS SOFTWARE. WABASOFT SHALL NOT BE LIABLE FOR INDIRECT,
SPECIAL OR CONSEQUENTIAL DAMAGES RESULTING FROM THE USE OF THIS PRODUCT.

AMY HIGH CRAFT, LTD. AND ITS LICENSORS
SHALL HAVE NO LIABILITY OR RESPONSIBILITY FOR SOFTWARE ALTERED,
MODIFIED, OR CONVERTED BY YOU OR A THIRD PARTY, DAMAGES RESULTING FROM
ACCIDENT, ABUSE OR MISAPPLICATION, OR FOR PROBLEMS DUE TO THE MALFUNCTION OF
YOUR EQUIPMENT OR SOFTWARE NOT SUPPLIED BY WABASOFT.
*/

package waba.ui;

import waba.fx.*;
import waba.sys.Vm;

/**
 * ScrollPane is a scrollable container that can be added to a parent container,
 * or mostly to the MainWindow.
 *
 * <p>
 * On WinCE & Win32:
 * Currently (11/20/2001) this class is usable only on these two platforms.
 * This class has became a base class for ToolBar class.
 * <p>
 * In the future, Menu, PopMenu, TablePanel, ListBox classes are to be based on this class too.
 * <p>
 * The basic function of the controls in the Scroll Pane is as follows:
 * Insert image here.
 *
 * <p>
 * <b>Points to be mentioned:</b>
 * <p>
 * This class is heavily dependent on the double-buffering screen refresh,
 * so this class only works with the Waba VM which has this feature.
 * <p>
 * All the Graphics & GUI enhancements / modifications are still based on the original Waba VM 1.0,
 * and they are planned to be replaced with a new architecture eventually.
 * <p>
 * Default scroll increment value is 8 pixels.
 *
 * <p>
 * If bigger than VGA size (640 X 480):
 * - the size of the free scroll buttons are set to 16 X 16 pixels.
 * - the thickness of the scroll buttons are set to 16 pixels.
 * <p>
 * If smaller than VGA size:
 * - the size of the free scroll buttons are set to 8 X 8 pixels.
 * - the thickness of the scroll buttons are set to 8 pixels.
 *
 * <p>
 * Here is an example showing a scroll pane being used:
 *
 * <pre>
 * public class MyProgram extends MainWindow
 * {
 *  Button btnOne;
 *  Button btnTwo;
 *
 *  public void MyProgram()
 *  {
 *      ScrollPane scrlPane = new ScrollPane(0, 20, 160, 160);
 *      btnOne = new Button("One");
 *      scrlPane.addControl(btnOne);
 *      btnTwo = new Button("Two");
 *      scrlPane.addControl(btnTwo);
 *      add(ScrollPane);
 *  }
 *
 *  public void onEvent(Event event)
 *  {
 *      if (event.type == ControlEvent.PRESSED && event.target == btnOne)
 *      {
 *          ... handle tab one being pressed
 *      }
 *  }
 * </pre>
 */



public class ScrollPanel extends Container
{
    Button      freeScrlBtns[];
    Button      uBtn, dBtn, lBtn, rBtn;
    Container   cntntPane, dragPane;
    Graphics    lBtnGfx, rBtnGfx, uBtnGfx, dBtnGfx;

    //*** ScrollPanel default settings ***
    /*boolean isTopMost               = true;
    boolean isBottomMost            = false;
    boolean isLeftMost              = true;
    boolean isRightMost             = false;*/

    boolean isUpPressed             = false;
    boolean isDownPressed           = false;
    boolean isLeftPressed           = false;
    boolean isRightPressed          = false;
    boolean isFreeScrlBtnPressed    = false;

    boolean isDragPaneAlreadyAdded  = false;
    boolean hasFreeScrlBtn, hasVerticalBtn, hasHorizontalBtn;

    int     maxBoundX, maxBoundY; //The sum of all the rectangle size of added controls.
    int     minBoundX, minBoundY;
    int     cntntX, cntntY, cntntWidth, cntntHeight; //"cntnt" stands for content.
    int     hBtnHeight, hBtnWidth, vBtnHeight, vBtnWidth, freeScrlBtnWidth, freeScrlBtnHeight;
    int     oldPenX, oldPenY, dX, dY, hScrlIncrmnt, vScrlIncrmnt;

    int     lBtnGfxX[], lBtnGfxY[], rBtnGfxX[], rBtnGfxY[], uBtnGfxX[], uBtnGfxY[], dBtnGfxX[], dBtnGfxY[];

    /*** Constructs a scroll pane. ***/
    public ScrollPanel(){}

    public ScrollPanel(int x, int y, int width, int height)
    {
        this.resize(x, y, width, height, true, true, true);
    }

    public ScrollPanel(int x, int y, int width, int height, boolean hBtns, boolean vBtns)
    {
        this.resize(x, y, width, height, false, hBtns, vBtns);
    }

    public ScrollPanel(int x, int y, int width, int height, boolean freeScrlBtns, boolean hBtns, boolean vBtns)
    {
        this.resize(x, y, width, height, freeScrlBtns, hBtns, vBtns);
    }

    /**
     * This method allows an application program to change
     * the configuration / size of the scroll pane dynamically at run-time.
     *
     * @param   x the left coordinate of the panel's bounding box
     * @param   y the top coordinate of the panel's bounding box
     * @param   hasFreeScrlBtn is set to true,
     *          if the free scroll feature is necessary.
     * @param   hasHorizontalBtn is set to true,
     *          if horizontal scroll is necessary.
     * @param   hasVerticalBtn is set to true,
     *          if vertical scroll is necessary.
     *
     * <p>
     * <b>Points to be mentioned:</b>
     * <p>
     * If bigger than VGA size (640 X 480):
     * - the size of the free scroll buttons are set to 16 X 16 pixels.
     * - the thickness of the scroll buttons are set to 16 pixels.
     *
     * If smaller than VGA size:
     * - the size of the free scroll buttons are set to 8 X 8 pixels.
     * - the thickness of the scroll buttons are set to 8 pixels.
     */
    public void resize(     int x,
                            int y,
                            int width,
                            int height,
                            boolean hasFreeScrlBtn,
                            boolean hasHorizontalBtn,
                            boolean hasVerticalBtn)
    {
        this.hasFreeScrlBtn     = hasFreeScrlBtn;
        this.hasHorizontalBtn   = hasHorizontalBtn;
        this.hasVerticalBtn     = hasVerticalBtn;

        hScrlIncrmnt = vScrlIncrmnt = 8; //Default scroll increment value.

        this.setRect(x, y, width, height);

        //"hBtn" is for horizontal scroll buttons, the ones on the left and the right of the scroll pane.
        //"vBtn" is for vertical scroll buttons, the ones on the top and the bottom of the scroll pane.

        //Determine the appropriate size for scroll buttons from the size of the MainWindow.

        //If bigger than VGA size:
        //- the size of the free scroll buttons are set to 16 X 16 pixels.
        //- the thickness of the scroll buttons are set to 16 pixels.
        if( (this.getRect().width >= 640)&&(this.getRect().width >= 480) )
            freeScrlBtnWidth = freeScrlBtnHeight = hBtnWidth = vBtnHeight = 16;
        //If smaller than VGA size:
        //- the size of the free scroll buttons are set to 8 X 8 pixels.
        //- the thickness of the scroll buttons are set to 8 pixels.
        else
            freeScrlBtnWidth = freeScrlBtnHeight = hBtnWidth = vBtnHeight = 8;

        if(hasHorizontalBtn)
            hBtnHeight = this.getRect().height;
        else
            hBtnHeight = hBtnWidth = 0;

        if(hasVerticalBtn)
            vBtnWidth = this.getRect().width;
        else
            vBtnHeight = vBtnWidth = 0;

        if(hasFreeScrlBtn)
        {
            //Times two, since 2 free scroll buttons on the both sides.
            hBtnHeight  -= (freeScrlBtnHeight*2);
            vBtnWidth   -= (freeScrlBtnWidth*2);
        }
        else
            freeScrlBtnWidth = freeScrlBtnHeight = 0;

        if(hasFreeScrlBtn)
        {
            freeScrlBtns = new Button[4];

            for(int index=0; index < 4; index++)
                freeScrlBtns[index] = new Button("", true);

            //upper left corner
            freeScrlBtns[0].setRect(0, 0, freeScrlBtnWidth, freeScrlBtnHeight);
            //upper right corner
            freeScrlBtns[1].setRect(width-freeScrlBtnWidth, 0, freeScrlBtnWidth, freeScrlBtnHeight);
            //lower left corner
            freeScrlBtns[2].setRect(0, height-freeScrlBtnHeight, freeScrlBtnWidth, freeScrlBtnHeight);
            //lower right corner
            freeScrlBtns[3].setRect(width-freeScrlBtnWidth, height-freeScrlBtnHeight, freeScrlBtnWidth, freeScrlBtnHeight);

            for(int index=0; index < 4; index++)
                this.add(freeScrlBtns[index]);
        }

        if( (hasVerticalBtn)||(vBtnWidth > 0)&&(vBtnHeight > 0) )
        {
            uBtn = new Button("", false);
            dBtn = new Button("", false);

            uBtn.setRect(hBtnWidth, 0, vBtnWidth, vBtnHeight);
            dBtn.setRect(hBtnWidth, height-vBtnHeight, vBtnWidth, vBtnHeight);

            this.add(uBtn);
            this.add(dBtn);

            //These cumbersome stuff is for drawing the up/down facing triangles.
            uBtnGfxX = new int[3]; uBtnGfxY = new int[3];
            dBtnGfxX = new int[3]; dBtnGfxY = new int[3];

            uBtnGfxX[0] = (vBtnWidth/2)-4; uBtnGfxY[0] = freeScrlBtnHeight-1;
            uBtnGfxX[1] = (vBtnWidth/2)+4; uBtnGfxY[1] = freeScrlBtnHeight-1;
            uBtnGfxX[2] = (vBtnWidth/2); uBtnGfxY[2] = 1;

            dBtnGfxX[0] = (vBtnWidth/2)-4; dBtnGfxY[0] = vBtnHeight-freeScrlBtnHeight+1;
            dBtnGfxX[1] = (vBtnWidth/2)+4; dBtnGfxY[1] = vBtnHeight-freeScrlBtnHeight+1;
            dBtnGfxX[2] = (vBtnWidth/2); dBtnGfxY[2] = vBtnHeight-1;/**/
        }

        if( (hasHorizontalBtn)||(hBtnWidth > 0)&&(hBtnHeight > 0) )
        {
            lBtn = new Button("", false);
            rBtn = new Button("", false);

            lBtn.setRect(0, vBtnHeight, hBtnWidth, hBtnHeight);
            rBtn.setRect(width-hBtnWidth, vBtnHeight, hBtnWidth, hBtnHeight);

            this.add(lBtn);
            this.add(rBtn);

            //These cumbersome stuff is for drawing the left/right facing triangles.
            lBtnGfxX = new int[3]; lBtnGfxY = new int[3];
            rBtnGfxX = new int[3]; rBtnGfxY = new int[3];

            lBtnGfxX[0] = hBtnWidth-1; lBtnGfxY[0] = (hBtnHeight/2)+4;
            lBtnGfxX[1] = hBtnWidth-1; lBtnGfxY[1] = (hBtnHeight/2)-4;
            lBtnGfxX[2] = 1; lBtnGfxY[2] = hBtnHeight/2;

            rBtnGfxX[0] = 1; rBtnGfxY[0] = (hBtnHeight/2)+4;
            rBtnGfxX[1] = 1; rBtnGfxY[1] = (hBtnHeight/2)-4;
            rBtnGfxX[2] = hBtnWidth-1; rBtnGfxY[2] = hBtnHeight/2;
        }

        cntntX      = hBtnWidth;
        cntntY      = vBtnHeight;
        cntntWidth  = width-(hBtnWidth*2);
        cntntHeight = height-(vBtnHeight*2);

        cntntPane  = new Container();
        cntntPane.setRect(cntntX, cntntY, cntntWidth, cntntHeight);

        if(cntntPane != null)
            this.add(cntntPane);
        /*if( (hasVerticalBtn)||(vBtnWidth > 0)&&(vBtnHeight > 0) )
        {
            this.add(uBtn);
            this.add(dBtn);
        }
        if( (hasHorizontalBtn)||(hBtnWidth > 0)&&(hBtnHeight > 0) )
        {
            this.add(lBtn);
            this.add(rBtn);
        }*/

        if(hasFreeScrlBtn)
        {
            dragPane = new Container();
            dragPane.setRect(cntntX, cntntY, cntntWidth, cntntHeight);
        }

        oldPenX = cntntX;
        oldPenY = cntntY;

        minBoundX = cntntX;
        minBoundY = cntntY;
        maxBoundX = cntntWidth;
        maxBoundY = cntntHeight;
    }

    public void setHorizontalScrolllIncrement(int increment)
    {
        hScrlIncrmnt = increment;
    }

    public void setVerticalScrolllIncrement(int increment)
    {
        vScrlIncrmnt = increment;
    }

    public boolean signalScrollUp()
    {
        boolean tmpFlag = isUpPressed;

        if(tmpFlag)
            isUpPressed = false; //reset the flag

        return tmpFlag; //Returning the true status. (Confused? Me too. ;-))
    }

    public boolean signalScrollDown()
    {
        boolean tmpFlag = isDownPressed;

        if(tmpFlag)
            isDownPressed = false; //reset the flag

        return tmpFlag; //Returning the true status. (Confused? Me too. ;-))
    }

    public boolean signalScrollLeft()
    {
        boolean tmpFlag = isLeftPressed;

        if(tmpFlag)
            isLeftPressed = false; //reset the flag

        return tmpFlag; //Returning the true status. (Confused? Me too. ;-))
    }

    public boolean signalScrollRight()
    {
        boolean tmpFlag = isRightPressed;

        if(tmpFlag)
            isRightPressed = false; //reset the flag

        return tmpFlag; //Returning the true status. (Confused? Me too. ;-))
    }
/**/
    /**
     * An application program can swap / exchange / replace
     * the content container with the given Container.
     * <p>
     * Possible uses are for context-sensitive menus, etc.
     */
    public void changeContentPane(Container passedCntntPane)
    {
        this.remove(cntntPane);
        cntntPane = passedCntntPane;
        this.addControl(cntntPane);
    }

    /**
     * Adds a control to the content container of the scroll pane.
     */
    public void addControl(Control control)
    {
    	Rect    cntrlRect       = control.getRect();

    	int     newMaxBoundX    = cntrlRect.x+cntrlRect.width;
    	int     newMaxBoundY    = cntrlRect.y+cntrlRect.height;

    	//*** I'll add some more restriction to
    	//*** this type of infinite boundary expansion in the future.
    	if(newMaxBoundX > maxBoundX)
    	    maxBoundX = newMaxBoundX;

    	if(newMaxBoundY > maxBoundY)
    	    maxBoundY = newMaxBoundY;

        cntntPane.add(control);
    }

    /**
     * Removes a child control from the content container of the scroll pane.
     */
    public void removeControl(Control control)
	{
        cntntPane.remove(control);
	}

    /**
     * Called by the system to pass events to the scroll panel control.
     */
    public void onEvent(Event evnt)
    {
        Control     child           = this.children;
        Control     cntntPaneChild  = cntntPane.children;
        PenEvent    pe;
        Rect        cntrlRect;

        int         cX, cY;

        cX = cY = 0;

    	if(evnt.type == PenEvent.PEN_DOWN)
    	{
    	    if(hasFreeScrlBtn)
    	    {
                if( (evnt.target == freeScrlBtns[0])||
                    (evnt.target == freeScrlBtns[1])||
                    (evnt.target == freeScrlBtns[2])||
                    (evnt.target == freeScrlBtns[3]) )
                {
                    isFreeScrlBtnPressed = !isFreeScrlBtnPressed;

                    if(isFreeScrlBtnPressed)
                    {
                        for(int index=0; index < 4; index++)
                            freeScrlBtns[index].press();
                    }
                    else
                    {
                        for(int index=0; index < 4; index++)
                            freeScrlBtns[index].release();
                    }

                    if( isFreeScrlBtnPressed && !isDragPaneAlreadyAdded )
                    {
                        this.add(dragPane);
                        isDragPaneAlreadyAdded = true;
                    }
                    else
                    {
                        while(child != null)
                        {
                            if(child == dragPane) //If dragPane exists on the list, it can be removed.
                            {
                                this.remove(dragPane);
                                isDragPaneAlreadyAdded = false;
                            }

                            child = child.next;
                        }//End of while(child != null)
                    }//End of else(isFreeScrlBtnPressed)
                }//End of if( (evnt.target == freeScrlBtns[x]) )
            }//End of if(hasFreeScrlBtn)

            if( (uBtn != null)&&(evnt.target == uBtn) )
            {
                if( (minBoundY+cY) < cntntY )
                {
                    if( (cntntY-(minBoundY+cY)) < vScrlIncrmnt )
                        cY = (cntntY-minBoundY)%vScrlIncrmnt;
                    else
                        cY = vScrlIncrmnt;
                }

                isUpPressed = true;
            }//End of else if(uBtn != null)

            if( (dBtn != null)&&(evnt.target == dBtn) )
            {
                if( (maxBoundY+cY) > cntntHeight)
        	    {
        	        if( ((maxBoundY+cY)-cntntHeight) < vScrlIncrmnt )
        	            cY = -(maxBoundY%vScrlIncrmnt);
        	        else
                        cY = -vScrlIncrmnt;
                }

                isDownPressed = true;
            }//End of else if(dBtn != null)

            if( (lBtn != null)&&(evnt.target == lBtn) )
            {
                if( (minBoundX+cX) < cntntX )
                {
                    if( (cntntX-(minBoundX+cX)) < hScrlIncrmnt )
                        cX = (cntntX-minBoundX)%hScrlIncrmnt;
                    else
                        cX = hScrlIncrmnt;
                }

        	    isLeftPressed = true;
        	}//End of else if(lBtn != null)

        	if( (rBtn != null)&&(evnt.target == rBtn) )
        	{
        	    if( (maxBoundX+cX) > cntntWidth )
        	    {
        	        if( ((maxBoundX+cX)-cntntWidth) < hScrlIncrmnt )
        	            cX = -(maxBoundX%hScrlIncrmnt);
        	        else
        	            cX = -hScrlIncrmnt;
                }

        	    isRightPressed = true;
            }//End of else if(rBtn != null)

            //Three lines below are REALLY needed to prevent jumping motion
            //when PEN_DOWN event occurs after PEN_DRAG event.
            pe = (PenEvent)evnt;
        	oldPenX = pe.x;
            oldPenY = pe.y;

            //Unfortunately, we can't just move the content panel.
            //If we did that, the content affixed inside of the panel
            //will be clipped when the panel is moved.
            //And the clipped part won't be redrawn.
            //If we do ".repaint()" of the panel, it becomes redundunt later.
            //---
            //So we have to move each controls added inside of the panel individually.
            //Any improvemnet here from anybody would be appreciated.
            while(cntntPaneChild != null)
            {
                if(cntntPaneChild != dragPane)
                {
                    cntrlRect = cntntPaneChild.getRect();
                    cntntPaneChild.setRect(cntrlRect.x+=cX, cntrlRect.y+=cY, cntrlRect.width, cntrlRect.height);
                }//End of if(cntntPaneChild != dragPane)

                cntntPaneChild = cntntPaneChild.next;
            }//End of while(cntntPaneChild != null)

            minBoundX+=cX; maxBoundX+=cX; minBoundY+=cY; maxBoundY+=cY;
        }//End of if(evnt.type == PenEvent.PEN_DOWN)
        else if(evnt.type == PenEvent.PEN_DRAG)
        {
            boolean doMoveX = false;
            boolean doMoveY = false;

            //*** Do not move these 5 lines below for your sanity.
            //*** A lot of trials & errors went into this section.

            //*** One of the numerous reasons is to prevent jumping motion
            //*** when PEN_DOWN event occurs after PEN_DRAG event.
            pe = (PenEvent)evnt;
            dX = pe.x - oldPenX;
            dY = pe.y - oldPenY;
            oldPenX = pe.x;
            oldPenY = pe.y;

            if( (isFreeScrlBtnPressed) && (evnt.target == dragPane) && (cntntPane.contains(pe.x, pe.y)) )
            {
                if( ((minBoundX+dX)<=cntntX) &&
                    ((maxBoundX+dX)>=cntntWidth) )
                {
                    doMoveX = true;
                    minBoundX+=dX; maxBoundX+=dX;
                }

                if( ((minBoundY+dY)<=cntntY) &&
                    ((maxBoundY+dY)>=cntntHeight) )
                {
                    doMoveY = true;
                    minBoundY+=dY; maxBoundY+=dY;
                }

                while(cntntPaneChild != null)
                {
                    if(cntntPaneChild != dragPane)
                    {
                        cntrlRect = cntntPaneChild.getRect();

                        //*** I'm not sure why, but moving control objects in X & Y axis separately
                        //*** works well with this dragging logic.
                        //*** Any improvement to efficiently handle this part of logic would be appreciated.
                        if(doMoveX)
                            cntntPaneChild.setRect(cntrlRect.x+=dX, cntrlRect.y, cntrlRect.width, cntrlRect.height);

                        if(doMoveY)
                            cntntPaneChild.setRect(cntrlRect.x, cntrlRect.y+=dY, cntrlRect.width, cntrlRect.height);
                        // Checking to see if the area of child controls is in the boundary.
                        // (Old version. This works only when there is one control in the panel.)
                        // This section of code is left here as a reference.
                        /*if( (cntrlRect.x+dX)<=0 &&
                            (cntrlRect.x+cntrlRect.width+dX+hBtnWidth)>=(cntntX+cntntWidth) )
                        {
                            cntntPaneChild.setRect(cntrlRect.x+=dX, cntrlRect.y, cntrlRect.width, cntrlRect.height);
                        }

                        if( (cntrlRect.y+dY)<=0 &&
                            (cntrlRect.y+cntrlRect.height+dY+vBtnHeight)>=(cntntY+cntntHeight) )
                        {
                            cntntPaneChild.setRect(cntrlRect.x, cntrlRect.y+=dY, cntrlRect.width, cntrlRect.height);
                        }*/
                    }//End of if(cntntPaneChild != dragPane)

                    cntntPaneChild = cntntPaneChild.next;
                }//End of while(cntntPaneChild != null)
            }//End of if( (isFreeScrlBtnPressed)&&(evnt.target == dragPane) )
        }//End of else if(evnt.type == PenEvent.PEN_DRAG)
    }//End of public void onEvent(Event evnt)

    public void onPaint(Graphics grfx)
    {
        if(hasVerticalBtn)
        {
            uBtnGfx = uBtn.createGraphics();
            dBtnGfx = dBtn.createGraphics();
            uBtnGfx.fillPolygon(uBtnGfxX, uBtnGfxY, 3);
            dBtnGfx.fillPolygon(dBtnGfxX, dBtnGfxY, 3);
        }

        if(hasHorizontalBtn)
        {
            lBtnGfx = lBtn.createGraphics();
            rBtnGfx = rBtn.createGraphics();
            lBtnGfx.fillPolygon(lBtnGfxX, lBtnGfxY, 3);
            rBtnGfx.fillPolygon(rBtnGfxX, rBtnGfxY, 3);
        }

        //Due to a bug in .createGraphics(),
        //I couldn't decorate the free scroll buttons.
        //11/07/2001 - Isao F. Yamashita
    }//End of onPaint()
}//End of class ScrollPane


