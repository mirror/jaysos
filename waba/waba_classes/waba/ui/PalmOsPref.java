/* $Id: PalmOsPref.java,v 1.2 2001/08/18 20:37:52 bornet Exp $

PalmOsPref.java

Copyright (c) 2000 Amy High Craft, Ltd.
All rights are reserved.

****************************************

    01/18/2001 Isao F. Yamashita    : Inital coding.
*/

package waba.ui;

import waba.fx.*;
import waba.sys.*;



public class PalmOsPref extends Container
{
    Button  lBtn, rBtn;

    Font    boldFont;
    Font    plainFont;

    Label   screenDepthLabel, tmpLbl;
    String  screenDepthChoice[] = {"Automatic", "Black & White", "4 Level Gray", "16 Level Gray", "256 Color", "65536 Color"};

    //Notice, that I can't use bit shift operation to easily set the value here,
    //since '-1' and '24' is included in the selection.
    byte    screenDepthValue[] = {-1, 1, 2, 4, 8, 16 /*, 24, 32 ... in the future */};
    int     screenDepthChoiceIndex = 0;
    
    byte    palmOsPrefData[] = new byte[1];
    byte    tmpData;


    public PalmOsPref()
	{
	    boldFont = new Font("Helvetica", Font.BOLD, 12);
	    plainFont = new Font("Helvetica", Font.PLAIN, 12);

        if(getPalmOsPref(palmOsPrefData, 0, 1) != -1)
        {
            for( screenDepthChoiceIndex = 0;
		 (screenDepthValue[screenDepthChoiceIndex] 
		  != palmOsPrefData[0])&&(screenDepthChoiceIndex < 5);
		 screenDepthChoiceIndex++);
        }
        
	    screenDepthLabel = new Label(screenDepthChoice[screenDepthChoiceIndex], Label.CENTER);
	    //screenDepthLabel = new Label(String.valueOf(palmOsPrefData[0]), Label.CENTER);
        screenDepthLabel.setRect(38, 40, 80, 20);
        add(screenDepthLabel);
        
        //tmpLbl = new Label(String.valueOf(screenDepthChoiceIndex), Label.CENTER);
        //tmpLbl.setRect(38, 95, 80, 20);
        //add(tmpLbl);
        
	    lBtn = new Button("<");
	    rBtn = new Button(">");
	    lBtn.setRect(38, 70, 40, 20);
	    rBtn.setRect(78, 70, 40, 20);
	    add(lBtn);
	    add(rBtn);
	}//End of constructor



    public void onEvent(Event evnt)
	{
	    if(evnt.type == ControlEvent.PRESSED)
		{
		    if(evnt.target == lBtn)
			{
			    if(screenDepthChoiceIndex > 0)
			        screenDepthChoiceIndex--;
			}//End of if(evnt.target == lBtn)
			else if(evnt.target == rBtn)
			{
			    if(screenDepthChoiceIndex < 5)
			        screenDepthChoiceIndex++;
			}//End of else if(evnt.target == rBtn)

			palmOsPrefData[0] = screenDepthValue[screenDepthChoiceIndex];

			setPalmOsPref(palmOsPrefData);

            screenDepthLabel.setText(screenDepthChoice[screenDepthChoiceIndex]);
            //screenDepthLabel.setText(String.valueOf(palmOsPrefData[0]));
            //tmpLbl.setText(String.valueOf(screenDepthChoiceIndex));
            
			repaint();
		}//End of if(evnt.type == ControlEvent.PRESSED)
	}//End of onEvent()



	public void onPaint(Graphics scrn)
	{
	    scrn.setColor(0, 0, 0);
	    scrn.setFont(boldFont);
	    scrn.drawText("Force Color Setting of", 28, 10);
	    scrn.drawText("Waba for This PalmOS to:", 18, 20);
    }//End of onPaint()



    //private final native byte[] getPalmOsPref();
    private final native int getPalmOsPref(byte buf[], int start, int count);
    
    private final native void setPalmOsPref(byte[] prefData);

}//End of class PalmOsPref


