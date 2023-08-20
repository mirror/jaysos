/* $Id: WabaAbout.java,v 1.3 2001/08/18 20:37:52 bornet Exp $

PalmOsPref.java

Copyright (c) 2000 Amy High Craft, Ltd.
All rights are reserved.

****************************************

    01/18/2001 Isao F. Yamashita    : Inital coding.
*/

package waba.ui;

import waba.fx.*;
import waba.sys.*;



public class WabaAbout extends Container
{
    Font    boldFont;
    Font    plainFont;

    String  version = "Version ";
    String  title = "Waba Virtual Machine";
    String  status = "WabaVM installed and ready";
    String  url = "http://waba.sourceforge.net";

    public WabaAbout()
	{
	    int v = Vm.getVersion();

	    version += (v / 100) + "." + (v % 100) + " for " + Vm.getPlatform();

	    boldFont = new Font("Helvetica", Font.BOLD, 12);
	    plainFont = new Font("Helvetica", Font.PLAIN, 12);
	}//End of constructor



    public void onEvent(Event evnt)
	{
	    if(evnt.type == ControlEvent.PRESSED)
		{
		    //
		}
	}//End of onEvent()



    /** Called by the system to draw the application. */
    public void onPaint(Graphics g)
	{
	    int x = 0;
	    int y = 0;

	    FontMetrics boldFontMetrics = getFontMetrics(boldFont);
	    FontMetrics plainFontMetrics = getFontMetrics(plainFont);

	    // draw title
	    x = (this.width - boldFontMetrics.getTextWidth(title)) / 2;
	    y = this.height / 2 - 60;

	    g.setColor(0, 0, 0);
	    g.setFont(boldFont);
	    g.drawText(title, x, y);

	    y += boldFontMetrics.getHeight();

	    // draw verion
	    x = (this.width - plainFontMetrics.getTextWidth(version)) / 2;

	    g.setColor(0, 0, 0);
	    g.setFont(plainFont);
	    g.drawText(version, x, y);

	    // draw status
	    y += 40;
	    x = (this.width - plainFontMetrics.getTextWidth(status)) / 2;

	    g.drawText(status, x, y);

	    // draw url
	    y += 50;

	    int sw = plainFontMetrics.getTextWidth(url);

	    x = (this.width - sw) / 2;
	    g.setColor(0, 0, 0);

	    int sh = plainFontMetrics.getHeight();

	    g.fillRect(x - 2, y, sw + 4, sh);
	    g.setColor(255, 255, 255);
	    g.setFont(plainFont);
	    g.drawText(url, x, y);
	}//End of onPaint()


}//End of class PalmOsPref


