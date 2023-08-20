/*

TurnBaby.java

Copyright (c) 2000 - 2001 Amy High Craft, Ltd.

Amy High Craft, Ltd. grants you a non-exclusive license to use, modify and re-distribute
this program provided that this copyright notice and license appear on all
copies of the software.

Software is provided "AS IS," without a warranty of any kind. ALL EXPRESS OR
IMPLIED REPRESENTATIONS AND WARRANTIES, INCLUDING ANY IMPLIED WARRANTY OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE
HEREBY EXCLUDED. THE ENTIRE RISK ARISING OUT OF USING THE SOFTWARE IS ASSUMED
BY THE LICENSEE.

AMY HIGH CRAFT, LTD. AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY
LICENSEE OR ANY THIRD PARTY AS A RESULT OF USING OR DISTRIBUTING SOFTWARE.
IN NO EVENT WILL WABASOFT OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE,
PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL
OR PUNITIVE DAMAGES, HOWEVER CAUSED AN REGARDLESS OF THE THEORY OF LIABILITY,
ARISING OUT OF THE USE OF OR INABILITY TO USE SOFTWARE, EVEN IF WABASOFT HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/

package ThreadTest;

import waba.lang.*;
import waba.ui.*;
import waba.fx.*;



// Stands for "Turn Baby Turn!"
public class TurnBaby extends MainWindow implements waba.sys.Runnable
{
    Image watchImage;

    int watchIndex = 0;
    int watchWidth, watchHeight;
    int numOfRow, numOfColumn, numOfWatch;
    int rowIndex, columnIndex;

    boolean isQuit = false;

    Watch watch[];

    private waba.sys.Thread tickThread;

    static float sin[] =
    {
        0.0f, 0.17364818f, 0.34202015f, 0.5f,
        0.64278764f, 0.76604444f, 0.8660254f, 0.9396926f,
        0.9848077f, 1.0f, 0.9848077f, 0.9396926f,
        0.8660254f, 0.76604444f, 0.64278764f, 0.5f,
        0.34202015f, 0.17364818f, 0.0f, -0.17364818f,
        -0.34202015f, -0.5f, -0.64278764f, -0.76604444f,
        -0.8660254f, -0.9396926f, -0.9848077f, -1.0f,
        -0.9848077f, -0.9396926f, -0.8660254f, -0.76604444f,
        -0.64278764f, -0.5f, -0.34202015f, -0.17364818f,
        0.0f
    };

    public TurnBaby()
	{
	    // load watch image and create image buffer
        watchImage = new Image("smallClock.bmp");

        watchWidth = watchImage.getWidth();
        watchHeight = watchImage.getHeight();

        numOfRow = (this.getRect().width) / watchWidth;
        numOfColumn = (this.getRect().height) / watchHeight;
        numOfWatch = (numOfRow*numOfColumn)+1;

        watch = new Watch[numOfWatch+1];

        for(watchIndex=0; watchIndex < numOfWatch+1; watchIndex++)
            watch[watchIndex] = new Watch();

	    watchIndex = 0;

        for(rowIndex=0; rowIndex < numOfRow; rowIndex++)
        {
            for(columnIndex=0; columnIndex < numOfColumn; columnIndex++)
            {
                watch[watchIndex].setX(rowIndex*watchWidth);
                watch[watchIndex].setY(columnIndex*watchHeight);
                watchIndex++;
            }
        }

        for(watchIndex=0; watchIndex < numOfWatch; watchIndex++)
        {
            watch[watchIndex].setDelay((watchIndex*10)+1);
            watch[watchIndex].start();
        }

        //Start the main loop.
        this.start();
	}

    public void start()
    {
        if(tickThread == null)
        {
            tickThread = new waba.sys.Thread(this);
            tickThread.start();
        }
    }

	public void run()
	{
	    Graphics grfx = this.createGraphics();

	    //Or you can do this with timer, with lower time resolution.
	    while(!isQuit)
	    {
            for(watchIndex=0; watchIndex < numOfWatch; watchIndex++)
                updateClock(grfx, watchIndex);

	        MainWindow.paint();
	    }
	}//End of run()

    public void onEvent(Event mainEvent)
	{
	    if(mainEvent.type == PenEvent.PEN_DOWN)
        {
            for(watchIndex=0; watchIndex < numOfWatch; watchIndex++)
                watch[watchIndex].stop();

	        isQuit = true;

	        exit(0);
		}
	}//End of onEvent()

   /*
    * Calculates the sin of a value in degrees.
    * The value passed must be in the range of 0...360.
    * The degrees walk clockwise instead of
    * counterclockwise to match the clock.
    */
    static float qsin(int deg)
    {
        // get the nearest sin we have in the table
        int weight = deg % 10;
        int d10 = deg / 10;
        float low = sin[d10];

        if(weight == 0)
            return low;

        // we don't have an exact match so calculate a weighted
        // average of the low nearest sin and high nearest sin.
        float high = sin[d10 + 1];
        float wf = (float)weight;

        return (low * (10f - wf) + high * wf) / 10.0f;
    }

   /*
    * Calculates the cos of a value in degrees.
    * The degrees walk clockwise instead of
    * counterclockwise to match the clock.
    */
    static float qcos(int deg)
    {
        return qsin((deg + 90) % 360);
    }

    /** Draws a clock hand. */
    static void drawHand(Graphics g, int x, int y, int w, int h, int deg, boolean doTriangle)
    {
        // calculate cx, cy (center of box)
        int cx = x + w / 2;
        int cy = y + h / 2;

        // calculate width and height factors
        float hw = (float)((float)w / 2.0f);
        float hh = (float)((float)h / 2.0f);

        // calculate x2, y2
        int x2 = x + (int)(hw + qcos(deg) * hw + .5f);
        int y2 = y + (int)(hh + qsin(deg) * hh + .5f);

        if(!doTriangle)
        {
            g.drawLine(cx, cy, x2, y2);
            return;
        }
        // calc x1, y1 for triangle
        deg = (deg + 90) % 360;
        int x1 = x + (int)(hw + qcos(deg) * 1.75f + .5f);
        int y1 = y + (int)(hh + qsin(deg) * 1.75f + .5f);

        // calc x3, y3 for triangle
        deg = (deg + 180) % 360;
        int x3 = x + (int)(hw + qcos(deg) * 1.75f + .5f);
        int y3 = y + (int)(hh + qsin(deg) * 1.75f + .5f);

        // draw outer triangle
        g.drawLine(x1, y1, x2, y2);
        g.drawLine(x2, y2, x3, y3);
        g.drawLine(x3, y3, x1, y1);

        // fill triangle in a bit to make the hand darker
        g.drawLine((x1 + x3) / 2, (y1 + y3) / 2, x2, y2);
        g.drawLine((x1 * 3 + x3) / 4, (y1 * 3 + y3) / 4, x2, y2);
        g.drawLine((x1 + x3 * 3) / 4, (y1 + y3 * 3) / 4, x2, y2);
    }

    public void updateClock(Graphics grfx, int watchIndex)
    {
        grfx.drawImage(watchImage, watch[watchIndex].getX(), watch[watchIndex].getY());

        grfx.setColor(0, 0, 0);

        int hour = watch[watchIndex].getHour();
        int minute = watch[watchIndex].getMinute();
        int second = watch[watchIndex].getSecond();

        // draw hour hand
        int x = (watchWidth/2);
        int y = (watchHeight/2);
        int w = 50; //length of the hand.
        int h = 50; //length of the hand.
        int deg = (((hour + 9)% 12) * 30) + (minute / 2);
        drawHand(   grfx,
                    (x-(w/2))+watch[watchIndex].getX(),
                    (y-(h/2))+watch[watchIndex].getY(),
                    w,
                    h,
                    deg,
                    true);

        // draw minute hand
        w = 40; //length of the hand.
        h = 40; //length of the hand.
        deg = ((minute + 45) % 60) * 6 + (second / 10);
        drawHand(   grfx,
                    (x-(w/2))+watch[watchIndex].getX(),
                    (y-(h/2))+watch[watchIndex].getY(),
                    w,
                    h,
                    deg,
                    true);

        // draw second hand
        y += 10; //a bit off-center
        w = 10; //length of the hand.
        h = 10; //length of the hand.
        deg = ((second + 45) % 60) * 6;
        drawHand(   grfx,
                    (x-(w/2))+watch[watchIndex].getX(),
                    (y-(h/2))+watch[watchIndex].getY(),
                    w,
                    h,
                    deg,
                    false);
    }
}


