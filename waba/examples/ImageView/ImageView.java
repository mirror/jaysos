
package ImageView;

import waba.ui.*;
import waba.fx.*;
import waba.sys.*;


public class ImageView extends MainWindow {


    int x;
    int y;

    int maxX;
    int minX;
    int maxY;
    int minY;

    int imageNum = 0;
    
    Image image;
    
    public ImageView() {

        loadImage();
    }


    void loadImage() {
        image = new Image("images/" + imageNum + ".bmp");
        x = (width/2) - (image.getWidth()/2);
        y = (height/2) - (image.getHeight()/2);

        maxX = 0;
        minX = width - image.getWidth();
        maxY = 0;
        minY = height - image.getHeight();

    }

    public void onPaint(Graphics g) {

        g.drawImage(image, x, y);
        g.setColor(255, 0, 255);
        g.drawText(Convert.toString(imageNum), 0, 0);
    }

    public void onEvent(Event event) {
            
        if (event.type == KeyEvent.KEY_PRESS) {
            KeyEvent ke = (KeyEvent)event;
            if (ke.key == IKeys.LEFT) {
                if (x < maxX)
                    x += 4;
            }
            else if (ke.key == IKeys.RIGHT) {
                if (x > minX)
                    x -= 4;
            }
            else if (ke.key == IKeys.UP) {
                if (y < maxY)
                    y += 4;
           }
            else if (ke.key == IKeys.DOWN) {
                if (y > minY)
                    y -= 4;
            }

        }
        else if (event.type == PenEvent.PEN_DOWN) {
            if (imageNum > 5)   imageNum = 0;
            else imageNum++;
            image = null;
            loadImage();
        
        
        }

        repaint();

    }

}
