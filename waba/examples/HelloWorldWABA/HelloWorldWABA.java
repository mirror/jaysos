/* $Id: HelloWorldWABA.java,v 1.7 2002/01/25 17:24:17 cgrigis Exp $ */

package HelloWorldWABA;

import waba.ui.*;
import waba.fx.*;

public class HelloWorldWABA extends MainWindow {

    private Button closeButton;
    private Button aboutButton;
    private Image testImage;
    private Timer aTimer;
    private WabaAbout about;
    private boolean inAbout;

    private Font theFont = new Font("Helvetica", Font.PLAIN, 18 );
    private Font theFont2 = new Font("Helvetica", Font.PLAIN, 10 );

    //    public void onPaint(Graphics g) {
    public HelloWorldWABA() {

	aboutButton = new Button( "About" );
        aboutButton.setRect( 46, this.height - 50, 44, 15 );
        add( aboutButton );

	closeButton = new Button( "End" );
        closeButton.setRect( 100, this.height - 50, 44, 15 );
        add( closeButton );

	testImage = new Image( "test.bmp" );

	about = new WabaAbout();
	about.setRect( 0, 0, this.width, this.height );
	inAbout = false;

        if( aTimer == null ) {
            aTimer = addTimer( 30000 );
        }
    }

    public void onPaint( Graphics g ) {

	if( !inAbout ) {

	    FontMetrics fm = new FontMetrics( theFont, this );
	    int fontHeight = fm.getAscent() + fm.getDescent() + fm.getLeading();
	    FontMetrics fm2 = new FontMetrics( theFont2, this );
	    int fontHeight2 = fm2.getAscent() + fm2.getDescent() + fm2.getLeading();

	    g.setColor( 0, 0, 0 );

	    g.setFont(theFont);
	    g.drawText( "Hello World", 0, 0 );

	    g.setFont(theFont2);
	    g.drawText( "Bottom of screen", 0, this.height - fontHeight2 );

	    g.drawImage( testImage, 
			 ( this.width - testImage.getWidth() ) / 2, 
			 fontHeight + ( this.height - testImage.getHeight() - 50 - fontHeight ) / 2 );

	    /* draw a dotted-box 2 pixels around the image */
	    int x1, y1, x2, y2;
	    x1 = ( this.width - testImage.getWidth() ) / 2 - 2;
	    y1 = fontHeight + ( this.height - testImage.getHeight() - 50 - fontHeight ) / 2 - 2;
	    x2 = x1 + testImage.getWidth() + 4;
	    y2 = y1 + testImage.getHeight() + 4;
	    g.drawDots( x1, y1, x1, y2 );
	    g.drawDots( x1, y2, x2, y2 );
	    g.drawDots( x2, y2, x2, y1 );
	    g.drawDots( x2, y1, x1, y1 );

	}
    }

    public void onEvent( Event event ) {

        if( event.type == ControlEvent.PRESSED ){
            if( event.target == closeButton ) {

                exit(0);

            } else if( event.target == aboutButton ) {

		add( about );
		remove( aboutButton );
		remove( closeButton );
		inAbout = true;

	    }
	} else if(( event.target == about ) && ( event.type == PenEvent.PEN_DOWN )) {

	    remove( about );
	    add( aboutButton );
	    add( closeButton );
	    inAbout = false;

	} else if( event.type == ControlEvent.TIMER ) {

	    exit( 0 );

	}
    }
}



