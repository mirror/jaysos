/* $Id: DataStream.java,v 1.2 2001/08/18 20:37:52 bornet Exp $ */

/*****************************************************************************
 *                                Waba Extras
 *
 * Version History
 * Date                Version  Programmer
 * ----------  -------  -------  ------------------------------------------
 * 17/10/1999  New      1.0.0    Rob Nielsen
 * Class created
 * 22/07/2000           1.0.1    Guich
 * Added support for Strings.
 *
 ****************************************************************************/

package waba.io;

import waba.io.*;
import waba.util.*;
import waba.sys.*;

/**
 * DataStream is a wrapper you can place around any Stream such as a
 * SerialPort, Catalog, or BufferStream which lets you read and write
 * standard Waba data types like ints, floats, and Strings in a simple
 * manner.  Here's an example
 * <pre>
 *   SerialPort port=new SerialPort(9600,0);
 *   DataStream ds=new DataStream(port);
 *   ds.writeString("Hello");
 *   int status=ds.readUnsignedByte();
 *   if (status==1)
 *   {
 *     ds.writeString("Pi");
 *     ds.writeFloat(3.14);
 *   }
 *   port.close();
 * </pre>
 * <br>ps: this class was created in the Waba Extras package, but i added a few methods 
 * for storing and retrieving strings. (guich@email.com)
 */
public class DataStream extends Stream
{
  /** the underlying stream */
  protected Stream stream;

  /** a four byte array for reading and writing numbers */
  protected byte[] b=new byte[4];

  /**
   * Constructs a new DataStream which sits upon the given stream using
   * big endian notation for multibyte values.
   * @param stream the base stream
   */
  public DataStream(Stream stream)
  {
    this.stream=stream;
  }

   /** closes the stream */
   public boolean close()
   {
      if (stream != null)
         return stream.close();
      return true;
   }
   
  /**
   * Reads an integer from the stream as four bytes.  The returned value
   * will range from -2147483648 to 2147483647.
   * @return the integer
   */
  public int readInt()
  {
    stream.readBytes(b,0,4);

    return (((b[0]&0xFF) << 24) | ((b[1]&0xFF) << 16) | ((b[2]&0xFF) << 8) | (b[3]&0xFF));
  }
  
  /**
   * Writes an integer to the stream as four bytes.
   * @param i the integer to write
   * @return the number of bytes written
   */
  public int writeInt(int i)
  {
    for(int j=0;j<4;j++)
    {
      b[3-j]=(byte)(i&0xFF);
      i>>=8;
    }
    stream.writeBytes(b,0,4);
    return 4;
  }  

  /**
   * Reads a short from the stream as two bytes.  The returned value will
   * range from -32768 to 32767.
   * @return the short
   */
  public short readShort()
  {
    return (short)readUnsignedShort();
  }

  /**
   * Writes an short to the stream as two bytes.  As there is no
   * short type in Waba but we often only want to only use two bytes in 
   * storage. 
   * an int is used but the upper two bytes are ignored.
   * @param i the short to write
   * @return the number of bytes written
   */
  public int writeShort(int i)
  {
    for(int j=0;j<2;j++)
    {
      b[1-j]=(byte)(i&0xFF);
      i>>=8;
    }
    stream.writeBytes(b,0,2);
    return 2;
  }  

  /**
   * Reads an unsigned short from the stream as two bytes.  The returned
   * value will range from 0 to 65535.
   * @return the short
   */
  public int readUnsignedShort()
  {
    stream.readBytes(b,0,2);
    return (((b[0]&0xFF) << 8) | (b[1]&0xFF));
  }

  /**
   * Reads a float value from the stream as four bytes in IEEE 754 format.
   * @return the float value
   */
  public float readFloat()
  {
    return Convert.toFloatBitwise(readInt());
  }

  /**
   * Writes a float value to the stream as four bytes in IEEE 754 format
   * @param f the float to write
   * @return the number of bytes written
   */
  public int writeFloat(float f)
  {
    writeInt(Convert.toIntBitwise(f));
    return 4;
  }

  /**
   * Reads a boolean from the stream as a byte.  True is returned if
   * the byte is not zero, false if it is.
   * @returns the boolean value read.
   */
  public boolean readBoolean()
  {
    stream.readBytes(b,0,1);
    return b[0]!=0;
  }

  /**
   * Writes a boolean to the stream as a byte.  True values are written as
   * 1 and false values as 0.
   * @param b the boolean to write
   * @return the number of bytes written
   */
  public int writeBoolean(boolean bool)
  {
    b[0]=(bool?(byte)1:(byte)0);
    stream.writeBytes(b,0,1);
    return 1;
  }

  /**
   * Reads a single byte from the stream.  The returned value will range
   * from -128 to 127.
   * @returns the read byte
   */
  public byte readByte()
  {
    stream.readBytes(b,0,1);
    return b[0];
  }

  /**
   * Writes a single byte to the stream.
   * @param b the byte to write
   * @return the number of bytes written
   */
  public int writeByte(byte by)
  {
    b[0]=by;
    stream.writeBytes(b,0,1);
    return 1;
  }

  /**
   * Writes a single byte to the stream.
   * @param b the byte to write (only least significant byte is written)
   * @return the number of bytes written
   */
  public int writeByte(int by)
  {
    b[0]=(byte)(by&0xFF);
    stream.writeBytes(b,0,1);
    return 1;
  }

  /**
   * Reads a single unsigned byte from the stream.  The returned value will
   * range from 0 to 255.
   * @returns the read byte.
   */
  public int readUnsignedByte()
  {
    stream.readBytes(b,0,1);
    return b[0]&0xFF;
  }

  /**
   * Skips reading the next n bytes in the stream
   * @param n the number of bytes to skip
   */
  public void skip(int n)
  {
     byte[] b=new byte[n];
     stream.readBytes(b,0,n);
  }

  /**
   * Reads bytes from the stream. Returns the
   * number of bytes actually read or -1 if an error prevented the
   * read operation from occurring.
   * @param buf the byte array to read data into
   * @param start the start position in the array
   * @param count the number of bytes to read
   */
  public int readBytes(byte buf[], int start, int count)
  {
    return stream.readBytes(buf,start,count);
  }

  /**
   * Writes bytes to the the stream. Returns the
   * number of bytes actually written or -1 if an error prevented the
   * write operation from occurring.
   * @param buf the byte array to write data from
   * @param start the start position in the byte array
   * @param count the number of bytes to write
   */
  public int writeBytes(byte buf[], int start, int count)
  {
    return stream.writeBytes(buf,start,count);
  }

   
   /////////////////////////////// added by guich ////////////////////////////////////
   static private char chars[] = new char[128]; // buffer
   static private byte bytes[] = new byte[128]; // buffer
   
   /** pads the stream writting n bytes. all bytes will be 0. added by guich */
   public int pad(int n)
   {
      for (int i =0; i < n; i++)
         bytes[i] = 0;
      writeBytes(bytes,0,n);
      return n;
   }
   
   /** reads an string.
    @returns a zero or more length string. null is never returned.
   */
   public String readString()
   {
      int len = readShort();
      if (len == 0) return "";
      
      if (chars.length < len) chars = new char[len+16]; // grows the buffer if necessary

      readBytes(bytes,0,len);
      for (int i=0; i < len; i++)
         chars[i] = (char) bytes[i];
      return new String(chars,0,len);
   }

   /** reads an array of string. 
       @returns an zero or more length array. null is never returned.
   */
   public String[] readStringArray()
   {
      int size = (int)readShort();
      String []a = new String[size];
      for (int i =0; i < size; i++)
         a[i] = readString();
      return a;
   }
   /** writes the string array into the stream */
   public int writeStringArray(String []v)
   {
      int n = 0;
      if (v == null || v.length == 0)
         n += writeShort((short)0);
      else
      {
         n += writeShort((short)v.length);
         for (int i=0; i < v.length; i++)
            n += writeString(v[i]);
      }
      return n;
   }
   /** writes the string into the stream */
   public int writeString(String s)
   {
      int n = 0;
      if (s != null && s.length() > 0)
      {
         char[] c = s.toCharArray();
         if (bytes.length < c.length) bytes =new byte[c.length+16];
         
         for (int i=0; i < c.length; i++)
            bytes[i] = (byte) c[i];
         
         n += writeShort((short)c.length);
         n += writeBytes(bytes,0,c.length);
      }
      else n += writeShort(0);      
      return n;
   }
}
