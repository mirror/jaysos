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
 * ToolBar is a bar of controls.
 * <p>
 * Here is an example showing a tool bar being used:
 *
 *  <pre>
 *  public class MyProgram extends MainWindow
 *  {
 *      Button btnOne;
 *      Button btnTwo;
 *
 *      public void onStart()
 *      {
 *          ToolBar toolBar = new ToolBar();
 *          btnOne = new Button("One");
 *          toolBar.add(btnOne);
 *          btnTwo = new Button("Two");
 *          toolBar.add(btnTwo);
 *          add(toolBar);
 *      }
 *
 *      public void onEvent(Event event)
 *      {
 *          if(event.type == ControlEvent.PRESSED && event.target == btnOne)
 *          {
 *              ... handle the control being pressed
 *          }
 *      }
 *  }
 *  </pre>
 */

public class ToolBar extends ScrollPanel
{
    final int   TOOLBAR_ICON_SPACING = 4;

    Color       toolBarColor; //Not used for now
    int         toolBarWidth, toolBarHeight;
    int         cntrlX_Offset = 4;

    /** Constructs a tool bar control. */
    public ToolBar()
    {
    	super.resize(0, 0, MainWindow.getMainWindow().getRect().width, 16, false, true, false);
    	super.setHorizontalScrolllIncrement(16);
    }

    public ToolBar(int x, int y, int width, int height)
    {
        super(x, y, width, height, true, false);
        super.setHorizontalScrolllIncrement(16);
    }

    /** Adds a control to the tool bar.*/
    public void addControl(Control control)
    {
        control.setRect(cntrlX_Offset, 0, control.getRect().width, control.getRect().height);
        cntrlX_Offset += (control.getRect().width + TOOLBAR_ICON_SPACING);
        super.addControl(control);
    }

    /*** Removes a child control from the container. ***/
    public void removeControl(Control control)
	{
        super.removeControl(control);
	}
}


