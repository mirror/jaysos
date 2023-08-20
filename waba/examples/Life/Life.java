/* 
Life

Simple implementation of Conway's Life in Java for the WABA vm and 
interface. 

Based on an example program in Greg Hewgill's Jump development
kit, which is a Java to assembler system for the Pilot. User interface
elements taken from Scribble example in WABA SDK.

The code has been adapted and streamlined a bit, and the option
of filling the board with random bits has been added.

Continuous running is not yet possible because of the lack of
an "idle" event or a timer.

David T. Linker May 12, 1999
dtlinker@u.washington.edu

*/

package Life;

import waba.ui.*;
import waba.fx.*;
import waba.sys.*;

class Random {
/*
 Class implementing pseudorandom numbers, based loosely on "Numerical Methods" by
 Dahlquist, Bjork and Anderson
*/

	static final int A = 0x4000;
	static final int B = 5;
	static final int lambda = A + B;
	static final int P = 0x8000;
	static final int mu = 1;
	int lastRandom;
	
public Random () { // Default constructor uses time as seed number
Time t = new Time();
lastRandom = 	(((((((((t.year * 12)
				 	+ t.month) * 30) 
				    + t.day) * 24)
				    + t.hour) * 60) 
				    + t.minute) * 60)
				    + t.second;
}

public Random (int seed) { // Use fixed seed for repeating series
	lastRandom = seed;
}

public void setSeed (int seed) { // Use fixed seed to start again
	lastRandom = seed;
}

public int getRandom() {
	lastRandom = ((lambda * lastRandom) + B) % P;
	return lastRandom;
}

}

class Title extends Control
{
String name;
Font font;

public Title(String name)
	{
	this.name = name;
	this.font = new Font("Helvetica", Font.BOLD, 12);
	}

public void onPaint(Graphics g)
	{
	// draw line across
	g.setColor(0, 0, 0);
	int y = height - 1;
	g.drawLine(0, y, width, y);
	y--;
	g.drawLine(0, y, width, y);

	// draw title
	FontMetrics fm = getFontMetrics(font);
	int boxWidth = fm.getTextWidth(name) + 8;
	g.fillRect(0, 0, boxWidth, y);
	g.setColor(255, 255, 255);
	g.setFont(font);
	g.drawText(name, 4, 2);
	}
}

public class Life  extends MainWindow{

  static final int CELLSIZE = 4;	// Try 4 for more cells
  static final int XSIZE = 160 / CELLSIZE;
  static final int YSIZE = 128 / CELLSIZE;
  static final int TITLEHEIGHT = 15;
  static final int BUTTONHEIGHT = 15;
  static final int BUTTONWIDTH  = 39;
  static final int ON = 1;
  static final int OFF = 0;
  static final int ratio = 2;
  

  byte Boards[][][] = new byte[2][YSIZE+2][XSIZE+2];
  int dispboard;
  Random rand = new Random();
  
  Graphics g;

  Button closeButton, clearButton, stepButton, randomButton;
  Title title;
  Font plainFont = new Font("Helvetica", Font.PLAIN, 12);


  public Life()
  {
  	g = new Graphics(this);
	g.setColor(255,255,255);
	Rect rect = getRect();
	g.fillRect(0,0,width,height);
	g.setColor(0,0,0);

	closeButton = new Button("Close");
	closeButton.setRect(0, height - BUTTONHEIGHT, BUTTONWIDTH, BUTTONHEIGHT);
	add(closeButton);
	
	clearButton = new Button("Clear");
	clearButton.setRect(BUTTONWIDTH + 1 , height - BUTTONHEIGHT, BUTTONWIDTH, BUTTONHEIGHT);
	add(clearButton);
	
	randomButton = new Button("Random");
	randomButton.setRect(2 * (BUTTONWIDTH + 1), height - BUTTONHEIGHT, BUTTONWIDTH, BUTTONHEIGHT);
	add(randomButton);
	
	stepButton = new Button("Step");
	stepButton.setRect(3 * (BUTTONWIDTH + 1), height - BUTTONHEIGHT, BUTTONWIDTH, BUTTONHEIGHT);
	add(stepButton);

	title = new Title("Life");
	title.setRect(0, 0, width, TITLEHEIGHT);
	add(title);
    dispboard = 0;
   }

 public void onEvent(Event event)
{
 	//if (event.type == ControlEvent.PRESSED)

	if (event.type == PenEvent.PEN_DOWN)
	{ 		if (event.target == closeButton){
		exit(0);} else
	if (event.target == clearButton)
		{
		ClearBoard();
		Rect rect = getRect();
		onPaint(0, TITLEHEIGHT, width, height - TITLEHEIGHT - BUTTONHEIGHT); // repaint
		} else
	if (event.target == randomButton) {
		ClearBoard(); // Note use of random threshold for variable "density"
		int threshold = rand.getRandom();
		for (int yloc = 1; yloc <= YSIZE; yloc++) 
      		for (int xloc = 1; xloc <= XSIZE; xloc++) 
        	if (rand.getRandom() > threshold)
        		Boards[dispboard][yloc][xloc] = ON; 
		Rect rect = getRect();
		onPaint(0, TITLEHEIGHT, width, width - TITLEHEIGHT - BUTTONHEIGHT); // repaint
	} else
	if (event.target == stepButton) {
		step();} // }
		else 
		{
 		PenEvent penEvent = (PenEvent)event;
    	int x = penEvent.x / CELLSIZE + 1;
    	int y = (penEvent.y  - TITLEHEIGHT) / CELLSIZE + 1;
      if (x > 0 && x <= XSIZE && y > 0 && y <= YSIZE) 
         drawCell(x,y, Boards[dispboard][y][x] = (byte)(Boards[dispboard][y][x] ^ 1));
       }
       }
}

 void step()
  {
    int newboard = dispboard ^ 1;
    for (int y = 1; y <= YSIZE; y++) {
      for (int x = 1; x <= XSIZE; x++) {
        int n = neighbors(Boards[dispboard], x, y);
        byte state = OFF;
        byte oldstate = Boards[dispboard][y][x];
        if (n == 3) {
        	state = ON;
        } else if ((n == 2) && (Boards[dispboard][y][x] != 0))
        	state = ON;
        if (state != oldstate ) drawCell(x, y, state);
        Boards[newboard][y][x] = state;
        }
      }
    dispboard = newboard;
  }

  int neighbors(byte[][] board, int x, int y)
  {
    return board[y-1][x-1] + board[y-1][x] + board[y-1][x+1]
         + board[y  ][x-1] +                 board[y  ][x+1]
         + board[y+1][x-1] + board[y+1][x] + board[y+1][x+1];
  }

  void drawCell(int x, int y, byte on)
  {
    if (on != 0) {
      g.setColor(0,0,0);
    } else {
      g.setColor(255,255,255);
    }
    g.fillRect( (short)((x-1)*CELLSIZE), 
    			(short)((y-1)*CELLSIZE + TITLEHEIGHT), 
    			(short)(CELLSIZE), 
    			(short)(CELLSIZE));
    g.setColor(0,0,0);
  }
  
void ClearBoard () {

for (int ycount = 1; ycount <= YSIZE; ycount++) {
     for (int xcount = 1; xcount <= XSIZE; xcount++) 
       Boards[dispboard][ycount][xcount] = OFF;
    } 
}

public void onPaint(int x, int y, int width, int height)
{
	// clear area
	g.setColor(255, 255, 255);
	g.fillRect(x, y, width, height);

	// draw contents
	if (y <TITLEHEIGHT) title.onPaint(g);
	Rect rect = getRect();
	if ((y + height) > (rect.height - BUTTONHEIGHT)) {
		closeButton.onPaint(g);
		clearButton.onPaint(g);
		randomButton.onPaint(g);
		stepButton.onPaint(g);
	}
	for (int ycount = 1; ycount <= YSIZE; ycount++) {
     for (int xcount = 1; xcount <= XSIZE; xcount++) 
       if (Boards[dispboard][ycount][xcount] != 0 ) drawCell(xcount, ycount, (byte)ON);
    } 
   }
}
