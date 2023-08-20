
package Lines;

import waba.ui.*;
import waba.fx.*;
import waba.sys.*;


public class Lines extends MainWindow {

    int count = 0;
    int x = 20;
    int y = 40;
    
    Timer timer;
    
    public Lines() {


    }

    public void onPaint(Graphics g)
	    {
             
/*	    
            g.setColor(255,0,100);  
	    g.drawLine(100, 200, 3, 300);
	    g.drawLine(400, 100, 20, 30);
	    g.drawLine(400, 200, 420, 300);
	    g.drawLine(100, 200, 3, 300);
	    g.drawLine(400, 100, 20, 30);
	    g.drawLine(400, 200, 420, 300);
*/
            g.setColor(255,0,0);  
            g.drawLine(x, y, 0, height);
            g.drawLine(x, y, width, 0);
            g.drawLine(x, y, width, height);
            g.drawLine(x, y, 0, 0);
            g.setColor(0,255,0);  
            g.drawLine(x, 200, x, y);
            g.drawLine(x, y, 200, 200);
            g.setColor(0,0,255); 
            g.drawLine(x, y, 100, 30);
            //g.drawDots(10, y, 80, y);            
	    }

    public void onEvent(Event event) {

            if (event.type == KeyEvent.KEY_PRESS) {
                KeyEvent ke = (KeyEvent)event;
                if (ke.key == IKeys.LEFT)
                    x -= 5;
                else if (ke.key == IKeys.RIGHT)
                    x += 5;
                else if (ke.key == IKeys.UP)
                    y -= 5;
                else if (ke.key == IKeys.DOWN)
                    y += 5;

            }

	    repaint();

    }

}

