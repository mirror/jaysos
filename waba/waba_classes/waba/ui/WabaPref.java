/* $Id: WabaPref.java,v 1.2 2001/08/18 20:37:52 bornet Exp $

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
import waba.sys.*;

/**
 * Welcome is the welcome application.
 * <p>
 * This is the default program run when none is specified or when the VM needs
 * a program to run to show that the VM is functioning on a device.
 */

public class WabaPref extends MainWindow
{
    Button  closeBtn;

    Tab     aboutPageTab, palmOsPrefPageTab;
    TabBar  tabBar;

    WabaAbout   wabaAboutPanel;
    PalmOsPref  palmOsPrefPanel;



    /*** Constructs the welcome application. ***/
    public WabaPref()
	{
	    tabBar = new TabBar();
	    tabBar.setRect(0, 0, this.width, 20);
	    this.add(tabBar);

	    //*** Setting up tabs and containers according to the platform the VM is installed on.
	    aboutPageTab = new Tab("About");
	    tabBar.add(aboutPageTab);
	    wabaAboutPanel = new WabaAbout();
	    wabaAboutPanel.setRect(0, 20, this.width, this.height);

	    add(wabaAboutPanel);

	    if((Vm.getPlatform()).equals("PalmOS"))
	    {
	        palmOsPrefPageTab = new Tab("PalmOS");
            tabBar.add(palmOsPrefPageTab);
            palmOsPrefPanel = new PalmOsPref();

            palmOsPrefPanel.setRect(0, 20, this.width, this.height);
        }
	}

    /** Called by the system to pass events to the application. */
    public void onEvent(Event evnt)
	{
	    if(evnt.type == ControlEvent.PRESSED)
		{
		    if(evnt.target == aboutPageTab)
			{
                remove(palmOsPrefPanel);

			    add(wabaAboutPanel);
			}//End of if(evnt.target == aboutPageTab)
			else if(evnt.target == palmOsPrefPageTab)
			{
                remove(wabaAboutPanel);

			    add(palmOsPrefPanel);
			}//End of else if(evnt.target == palmOsPrefPageTab)
		}//End of else if(evnt.type == ControlEvent.PRESSED)
	}//End of public void onEvent(Event evnt)
}//End of class WabaPref


