/* $Id: ScrollPane.java,v 1.2 2001/08/18 20:37:52 bornet Exp $

Copyright (c) 2000 Wabasoft  All rights reserved.

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
 * ScrollPane is a scrollable container that is added to parent container.
 * <p>
 * Here is an example showing a scroll pane being used:
 *
 * <pre>
 * public class MyProgram extends MainWindow
 * {
 * Button btnOne;
 * Button btnTwo;
 *
 * public void onStart()
 *  {
 *  ScrollPane scrlPane = new ScrollPane(0, 20, 160, 160);
 *  btnOne = new Button("One");
 *  ScrollPane.add(btnOne);
 *  btnTwo = new Button("Two");
 *  ScrollPane.add(btnTwo);
 *  add(ScrollPane);
 *  }
 *
 * public void onEvent(Event event)
 *  {
 *  if (event.type == ControlEvent.PRESSED &&
 *      event.target == btnOne)
 *   {
 *   ... handle tab one being pressed
 * </pre>
 */



public class ScrollPane extends Container
{
    Button      freeScrlBtns[];
    Button      uBtn, dBtn, lBtn, rBtn;
    Container   cntntPane, dragPane;
    //Graphics    bffrGrphc;
    //Image       bffrImg;
    Image       freeScrlBtnImg, uBtnImg, dBtnImg, lBtnImg, rBtnImg;
    Rect        uBtnRect, dBtnRect, lBtnRect, rBtnRect; //These are here just to hold each size data.

    //*** ScrollPane default settings ***
    boolean isFreeScrlBtnPressed    = false;
    boolean isTopMost               = true;
    boolean isBottomMost            = false;
    boolean isLeftMost              = true;
    boolean isRightMost             = false;

    boolean isUpPressed             = false;
    boolean isDownPressed           = false;
    boolean isLeftPressed           = false;
    boolean isRightPressed          = false;

    int     maxBoundX, maxBoundY; //The sum of all the rectangle size of added controls.
    int     cntntX, cntntY, cntntWidth, cntntHeight; //"cntnt" stands for content.
    int     oldPenX, oldPenY;
    int     dX, dY;
    int     cntrlXOffset, cntrlYOffset;

    int     hScrlIncrmnt = 8;
    int     vScrlIncrmnt = 8;

    int     freeScrlBtnWidth    = 0;
    int     freeScrlBtnHeight   = 0;

    boolean isColor;
    boolean isFreeScrlBtnExist      = false;


    /*** Constructs a scroll pane. ***/
    public ScrollPane(int x, int y, int width, int height)
    {
        this.init(x, y, width, height, true, true, true);
    }

    public ScrollPane(int x, int y, int width, int height, boolean hBtns, boolean vBtns)
    {
        this.init(x, y, width, height, false, hBtns, vBtns);
    }



    protected void init(    int x,
                            int y,
                            int width,
                            int height,
                            boolean hasFreeScrlBtn,
                            boolean hasHorizontalBtn,
                            boolean hasVerticalBtn)
    {
        isColor = Vm.isColor();

        this.setRect(x, y, width, height);

        if(hasFreeScrlBtn)
        {
            freeScrlBtns = new Button[4];

            isFreeScrlBtnExist = true;

            freeScrlBtnWidth    = 8;
            freeScrlBtnHeight   = 8;

            for(int index=0; index < 4; index++)
            {
                if( (freeScrlBtnWidth >= 16)||(freeScrlBtnHeight >=16) )
                {
                    freeScrlBtnImg      = new Image("handIcon.bmp");
                    freeScrlBtns[index] = new Button(freeScrlBtnImg, true);
                }
                else
                {
                    freeScrlBtns[index] = new Button("x", true);
                }
            }

            freeScrlBtns[0].setRect(0, 0, freeScrlBtnWidth, freeScrlBtnHeight);
            freeScrlBtns[1].setRect(width-freeScrlBtnWidth, 0, freeScrlBtnWidth, freeScrlBtnHeight);
            freeScrlBtns[2].setRect(0, height-freeScrlBtnHeight, freeScrlBtnWidth, freeScrlBtnHeight);
            freeScrlBtns[3].setRect(width-freeScrlBtnWidth, height-freeScrlBtnHeight, freeScrlBtnWidth, freeScrlBtnHeight);

            for(int index=0; index < 4; index++)
                this.add(freeScrlBtns[index]);
        }

        if(hasVerticalBtn)
        {
            uBtnImg = new Image("sUIcon.bmp");
            uBtn = new Button(uBtnImg);

            dBtnImg = new Image("sDIcon.bmp");
            dBtn = new Button(dBtnImg);
        }

        if(hasVerticalBtn)
        {
            uBtnRect = new Rect(freeScrlBtnWidth, 0, width-(freeScrlBtnWidth*2), uBtnImg.getHeight());
            dBtnRect = new Rect(freeScrlBtnWidth, height-dBtnImg.getHeight(), width-(freeScrlBtnWidth*2), dBtnImg.getHeight());
        }
        else
        {
            uBtnRect = new Rect( 0, 0, 0, 0);
            dBtnRect = new Rect( 0, 0, 0, 0);
        }

        if(hasVerticalBtn)
        {
            uBtn.setRect(uBtnRect.x, uBtnRect.y, uBtnRect.width, uBtnRect.height);
            dBtn.setRect(dBtnRect.x, dBtnRect.y, dBtnRect.width, dBtnRect.height);

            this.add(uBtn);
            this.add(dBtn);
        }

        if(hasHorizontalBtn)
        {
            lBtnImg = new Image("sLIcon.bmp");
            lBtn = new Button(lBtnImg);

            rBtnImg = new Image("sRIcon.bmp");
            rBtn = new Button(rBtnImg);
        }

        if(hasHorizontalBtn)
        {
            lBtnRect = new Rect(0, freeScrlBtnWidth, lBtnImg.getWidth(), height-(freeScrlBtnWidth*2));
            rBtnRect = new Rect(width-rBtnImg.getWidth(), freeScrlBtnHeight, rBtnImg.getWidth(), height-(freeScrlBtnHeight*2));
        }
        else
        {
            lBtnRect = new Rect(0, 0, 0, 0);
            rBtnRect = new Rect(0, 0, 0, 0);
        }

        if(hasHorizontalBtn)
        {
            lBtn.setRect(lBtnRect.x, lBtnRect.y, lBtnRect.width, lBtnRect.height);
            rBtn.setRect(rBtnRect.x, rBtnRect.y, rBtnRect.width, rBtnRect.height);

            this.add(lBtn);
            this.add(rBtn);
        }

        cntntX      = lBtnRect.width;
        cntntY      = uBtnRect.height;
        cntntWidth  = this.width  - (lBtnRect.width  + rBtnRect.width);
        cntntHeight = this.height - (uBtnRect.height + dBtnRect.height);

        cntntPane  = new Container();
        cntntPane.setRect(cntntX, cntntY, cntntWidth, cntntHeight);
        cntntPane.setRepaint(false);
        this.add(cntntPane);

        dragPane   = new Container();
        dragPane.setRect(cntntX, cntntY, cntntWidth, cntntHeight);
        dragPane.setRepaint(false);
        
        //bffrImg = new Image(cntntWidth, cntntHeight);
	    //bffrGrphc = new Graphics(bffrImg);
        //bffrGrphc.setColor(255, 0, 0);
        //bffrGrphc.fillRect(0, 0, cntntWidth, cntntHeight);

        oldPenX = cntntX;
        oldPenY = cntntY;

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

    public boolean upScrollNotify()
    {
        boolean tmpFlag = isUpPressed;

        if(tmpFlag)
            isUpPressed = false; //reset the flag

        return tmpFlag; //Returning the true status. (Confused? Me too.)
    }

    public boolean downScrollNotify()
    {
        boolean tmpFlag = isDownPressed;

        if(tmpFlag)
            isDownPressed = false; //reset the flag

        return tmpFlag; //Returning the true status. (Confused? Me too.)
    }

    public boolean leftScrollNotify()
    {
        boolean tmpFlag = isLeftPressed;

        if(tmpFlag)
            isLeftPressed = false; //reset the flag

        return tmpFlag; //Returning the true status. (Confused? Me too.)
    }

    public boolean rightScrollNotify()
    {
        boolean tmpFlag = isRightPressed;

        if(tmpFlag)
            isRightPressed = false; //reset the flag

        return tmpFlag; //Returning the true status. (Confused? Me too.)
    }



    public void changeContentPane(Container passedCntntPane)
    {
        this.remove(cntntPane);
        cntntPane = passedCntntPane;
        this.add(cntntPane);
    }



    /*** Adds a control to the content pane of the scroll pane. ***/
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



    /*** Removes a child control from the container. ***/
    public void removeControl(Control control)
	{
        cntntPane.remove(control);
	}



    /** Called by the system to pass events to the scroll pane control. */
    public void onEvent(Event evnt)
    {
        Control     child           = this.children;
        Control     cntntPaneChild  = cntntPane.children;
        PenEvent    pe;
        Rect        cntrlRect;

        int         cX, cY;

        cX = cY = 0;

        while(cntntPaneChild != null)
        {
            cntrlRect = cntntPaneChild.getRect();
            //cX = cY = 0;

    	    if(evnt.type == PenEvent.PEN_DOWN)
    	    {
    	        if(isFreeScrlBtnExist)
    	        {
        	        if( (evnt.target == freeScrlBtns[0])||
        	            (evnt.target == freeScrlBtns[1])||
        	            (evnt.target == freeScrlBtns[2])||
        	            (evnt.target == freeScrlBtns[3]) )
        	        {
        	            isFreeScrlBtnPressed = !isFreeScrlBtnPressed;

        	            if(isFreeScrlBtnPressed)
        	                this.add(dragPane);
        	            else
        	            {
        	                while(child != null)
        	                {
        	                    if(child == dragPane) //If dragPane exists on the list, it can be removed.
        	                        this.remove(dragPane);

        	                    child = child.next;
        	                }//End of while(child != null)
        	            }//End of else(isFreeScrlBtnPressed)
        	        }//End of if( (evnt.target == freeScrlBtns[x]) )
        	    }//End of if(isFreeScrlBtnExist)

        	    if(uBtn != null)
        	    {
        	        if(evnt.target == uBtn)
        	        {/*
        	            if( (cntntY-vScrlIncrmnt) >= 0)
        	            {
        	                cntntY -= vScrlIncrmnt;
        	                isTopMost = false;
        	            }
        	            else
        	            {
        	                cntntY = 0;
        	                isTopMost = true;
        	            }
        	            */

        	            cY = vScrlIncrmnt;

        	            isUpPressed = true;
        	        }//End of if(evnt.target == uBtn)
        	    }//End of else if(uBtn != null)

        	    if(dBtn != null)
        	    {
        	        if(evnt.target == dBtn)
        	        {/*
        	            if( (cntntY+vScrlIncrmnt) <= maxBoundY)
        	            {
        	                cntntY += vScrlIncrmnt;
        	                isBottomMost = false;
        	            }
        	            else
        	            {
        	                cntntY = maxBoundY;
        	                isBottomMost = true;
        	            }*/

        	            cY = -vScrlIncrmnt;

        	            isDownPressed = true;
        	        }//End of if(evnt.target == dBtn)
        	    }//End of else if(dBtn != null)

        	    if(lBtn != null)
        	    {
        	        if(evnt.target == lBtn)
            	    {/*
            	        if( (cntntX-hScrlIncrmnt) >= 0)
            	        {
            	            cntntX -= hScrlIncrmnt;
            	            isLeftMost = false;
            	        }
            	        else
            	        {
            	            cntntX = 0;
            	            isLeftMost = true;
            	        }*/

                        cX = hScrlIncrmnt;

            	        isLeftPressed = true;
            	    }//End of if(evnt.target == lBtn)
            	}//End of else if(lBtn != null)

            	if(rBtn != null)
            	{
            	    if(evnt.target == rBtn)
            	    {/*
            	        if( (cntntX+hScrlIncrmnt) <= maxBoundY)
            	        {
            	            cntntX += hScrlIncrmnt;
            	            isRightMost = false;
            	        }
            	        else
            	        {
            	            cntntX = maxBoundY;
            	            isRightMost = true;
            	        }*/

                        cX = -hScrlIncrmnt;

            	        isRightPressed = true;
            	    }
                }//End of else if(rBtn != null)

                //Three lines below are needed to prevent jerking motion
                //when PEN_DOWN event occurs after PEN_DRAG event.
        	    pe = (PenEvent)evnt;
            	oldPenX = pe.x;
                oldPenY = pe.y;

                //If I take out the line below, the screen gets flashy. Why?
                cntntPaneChild.setRect(cX+cntrlRect.x, cY+cntrlRect.y, cntrlRect.width, cntrlRect.height);
        	}//End of if(evnt.type == PenEvent.PEN_DOWN)
        	else if(evnt.type == PenEvent.PEN_DRAG)
        	{
        	    if( (isFreeScrlBtnPressed)&&(evnt.target == dragPane) )
        	    {
            	    pe = (PenEvent)evnt;

                    dX = pe.x - oldPenX;
                    dY = pe.y - oldPenY;
                	oldPenX = pe.x;
                    oldPenY = pe.y;

                    //If I take out the line below, the screen gets flashy. Why?
                    cntntPaneChild.setRect(cntrlRect.x+dX, cntrlRect.y+dY, cntrlRect.width, cntrlRect.height);
                }
        	}//End of else if(evnt.type == PenEvent.PEN_DRAG)

            cntntPaneChild = cntntPaneChild.next;
        }//End of while()
    }//End of public void onEvent(Event evnt)

}//End of class ScrollPane


