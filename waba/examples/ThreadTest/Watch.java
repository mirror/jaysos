/*

Watch.java

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

import waba.ui.*;
import waba.fx.*;
import waba.sys.*;
import waba.sys.Runnable;


public class Watch implements waba.sys.Runnable
{
    int heure   = 0;
    int minute  = 0;
    int seconde = 0;
    int delayValue = 0;
    int x, y;

    boolean isQuit = false;

    private waba.sys.Thread clockThread;

    //public Watch(){}

    /***By Isao F. Yamashita 11/11/2001 ***/
    public void setDelay(int delay)
    {
        delayValue = delay;
    }
    
    int getX(){return this.x;}
    int getY(){return this.y;}
    
    void setX(int x){this.x = x;}
    void setY(int y){this.y = y;}
    
    int getHour()
    {
        return heure;
    }

    int getMinute()
    {
        return minute;
    }

    int getSecond()
    {
        return seconde;
    }

    public void start()
    {
        if(clockThread == null)
        {
            clockThread = new waba.sys.Thread(this);
            clockThread.start();
        }
    }

    public void stop()
    {
        isQuit = true;
    }

    public void run()
    {
        while(!isQuit)
        {
            if(heure >= 23)
                heure = 0;
            else
                heure++;

            if(minute >= 59)
                minute = 0;
            else
                minute++;

            if(seconde >= 59)
                seconde = 0;
            else
                seconde++;

            clockThread.sleep(delayValue);
        }
    }
}


