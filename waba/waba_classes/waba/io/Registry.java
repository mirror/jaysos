/*
  $Id: Registry.java,v 1.2 2001/10/25 09:20:35 cgrigis Exp $

  Copyright (c) 2001 SmartData

  This software is furnished under a license and may be used only in accordance
  with the terms of that license. This software and documentation, and its
  copyrights are owned by SmartData and are protected by copyright law.
  
  THIS SOFTWARE AND REFERENCE MATERIALS ARE PROVIDED "AS IS" WITHOUT WARRANTY
  AS TO THEIR PERFORMANCE, MERCHANTABILITY, FITNESS FOR ANY PARTICULAR PURPOSE,
  OR AGAINST INFRINGEMENT. SMARTDATA ASSUMES NO RESPONSIBILITY FOR THE USE OR
  INABILITY TO USE THIS SOFTWARE. SMARTDATA SHALL NOT BE LIABLE FOR INDIRECT,
  SPECIAL OR CONSEQUENTIAL DAMAGES RESULTING FROM THE USE OF THIS PRODUCT.
  
  SMARTDATA SHALL HAVE NO LIABILITY OR RESPONSIBILITY FOR SOFTWARE ALTERED,
  MODIFIED, OR CONVERTED BY YOU OR A THIRD PARTY, DAMAGES RESULTING FROM
  ACCIDENT, ABUSE OR MISAPPLICATION, OR FOR PROBLEMS DUE TO THE MALFUNCTION OF
  YOUR EQUIPMENT OR SOFTWARE NOT SUPPLIED BY SMARTDATA.
*/  

package waba.io;

/**
   Registry
   <P>
   Allows to save and read application-specific data in non-volatile storage.
   <P>
   A Registry is a stream that, once obtained, can be read from or written to. It is also possible
   to skip bytes, using a positive argument to skip forward and a negative argument to skip back to a previous position.
   <P>
   It is also important to close the Registry when data has been written to it, as its contents is not
   guaranteed to be saved in non-volatile storage until the Registry is closed.
   <P>
   Deleting the Registry eliminates all data currently contained in it. Reading
   from or writing to the Registry after it has been deleted has undefined
   results. After deleting the Registry, it should be re-obtained using
   <TT>getRegistry ()</TT>.
   <P>
   Registry data is associated with the Waba main class name, so as to be application-specific. Accessing the Registry simultaneously by multiple instances of the
   same application can yield unexpected results.
   <P>
   Here is an example of Registry usage:

   <PRE>
   (...)
   Registry r = Registry.getRegistry ();
   byte [] data = {0x1F, 0x07, 'A', 'B'};
   r.writeBytes (data, 0, 4);
   r.close ();
   (...)
   r = Registry.getRegistry ();
   r.readBytes (data, 0, 4);
   (...)
   </PRE>
*/
   
public class Registry extends Stream
{
  private static Registry registry = null;

  private Registry ()
  {
    _nativeCreate ();
  }

  private native boolean _nativeCreate ();

  /**
     Obtain the Registry object associated with this application.
   */
  public static Registry getRegistry ()
  {
    if (registry == null)
      {
	registry = new Registry ();
      }

    return registry;
  }

  /**
     Return the space available in the Registry object associated with this
     application.
     <P>
     Depending on the specific underlying implementation of Registry objects,
     the total size returned by this method might not be available at the next 
     write, as it may have been claimed by another application in the mean time.
     @return the space available in bytes
  */
  public static synchronized native int spaceAvailable ();

  /**
     Read bytes from the Registry at the current position.
     @param buf buffer into which the data should be stored
     @param start position in the buffer to start storing data
     @param count number of bytes to read
     @return the number of bytes actually read, -1 indicating an error
   */
  public native int readBytes (byte [] buf, int start, int count);

  /**
     Write bytes into the Registry at the current position.
     @param buf buffer from which the data should be taken
     @param start position in the buffer to start taking data
     @param count number of bytes to write
     @return the number of bytes actually written, -1 indicating an error
  */
  public synchronized native int writeBytes (byte [] buf, int start, int count);

  /**
     Skip bytes.
     @param count the number of bytes to skip; the result of skipping past data that has been previously written is implementation-dependent
     @return the new position in the Registry, or -1 if an error occurred
  */
  public native int skipBytes (int count);

  /**
     Close the Registry and save its contents into non-volatile storage.
     @return <TT>false</TT> if an error occurred, <TT>true</TT> otherwise
  */
  public synchronized boolean close ()
  {
    boolean retValue = _nativeClose ();
    registry = null;

    return retValue;
  }

  private native boolean _nativeClose ();

  /**
     Delete the Registry and all its contents from non-volatile storage.
     @return <TT>false</TT> if an error occurred, <TT>true</TT> otherwise
  */
  public boolean delete ()
  {
    boolean retValue = _nativeDelete ();
    registry = null;

    return retValue;
  }

  private native boolean _nativeDelete ();
}
