/* $Id: Image.java,v 1.3 2001/08/18 20:37:52 bornet Exp $

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

package waba.fx;


/**
 * Image is a rectangular image.
 * <p>
 * You can draw into an image and copy an image to a surface using a Graphics
 * object.
 * @see Graphics
 */
public class Image implements ISurface
{
    int width;
    int height;

    /**
     * Creates an image of the specified width and height. The image has
     * a color depth (number of bitplanes) and color map that matches the
     * default drawing surface.
     */
    public Image(int width, int height)
    {
        useImagePalette(true);

    	this.width = width;
    	this.height = height;

    	_nativeCreate();
    }

    private native void _nativeCreate();

    /**
     * Loads and constructs an image from a file. The path given is the path to the
     * image file. The file must be in 2, 16 or 256 color uncompressed BMP bitmap
     * format. If the image cannot be loaded, the image constructed will have
     * a width and height of 0.
     */
    public Image(String path)
    {
        useImagePalette(true);

    	_nativeLoad(path);
    }

    private native void _nativeLoad(String path);

    /**
     * Creates an image of the specified width and height. The image has
     * a color depth (number of bitplanes) and color map that matches the
     * default drawing surface.
     *<p>
     *Set enable to false, when system palette matches that of the image for
     *faster performance.
     */

    public Image(int width, int height, boolean enable)
    {
        useImagePalette(enable);

    	this.width  = width;
    	this.height = height;

    	_nativeCreate();
    }

    /**
     * Loads and constructs an image from a file. The path given is the path to the
     * image file. The file must be in 2, 16 or 256 color uncompressed BMP bitmap
     * format. If the image cannot be loaded, the image constructed will have
     * a width and height of 0.
     *<p>
     *Set enable to false, when system palette matches that of the image for
     *faster performance.
     */
    public Image(String path, boolean enable)
    {
        useImagePalette(enable);

    	_nativeLoad(path);
    }

    /**
     * Sets one or more row(s) of pixels in an image. This method sets the values of
     * a number of pixel rows in an image and is commonly used when writing code
     * to load an image from a stream such as a file. The source pixels byte array
     * must be in 1, 4 or 8 bit per pixel format with a matching color map size
     * of 2, 16 or 256 colors.
     * <p>
     * Each color in the color map of the source pixels is identified by a single
     * integer value. The integer is composed of 8 bits (value [0..255]) of red,
     * green and blue using the following calculation:
     * <pre>
     * int color = (red << 16) | (green << 8) | blue;
     * </pre>
     * As an example, to load a 16 color image, we would pass bitsPerPixel
     * as 4 and would create a int array of 16 values for the color map.
     * Then we would set each of the values in the color map to the colors
     * used using the equation above. We could then either read data line
     * by line from the source stream, calling this method for each row of
     * pixels or could read a number of rows at once and then call this
     * method to set the pixels.
     * <p>
     * The former approach uses less memory, the latter approach is faster.
     *
     * @param bitsPerPixel bits per pixel of the source pixels (1, 4 or 8)
     * @param colorMap the color map of the source pixels (must be 2, 16 or 256 in length)
     * @param bytesPerRow number of bytes per row of pixels in the source pixels array
     * @param numRows the number of rows of pixels in the source pixels array
     * @param y y coordinate in the image to start setting pixels
     * @param pixels array containing the source pixels
     */
    public native void setPixels(int bitsPerPixel, int colorMap[], int bytesPerRow, int numRows, int y, byte pixels[]);


    /**
     * Sets the image width and height to 0 and frees any systems resources
     * associated with the image.
     */
    public native void free();

    public native void useImagePalette(boolean enable);

    /** Returns the height of the image. */
    public int getHeight()
    {
    	return height;
    }

    /** Returns the width of the image. */
    public int getWidth()
    {
    	return width;
    }
}
