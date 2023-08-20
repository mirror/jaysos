/* $Id: FileTest.java,v 1.3 2002/01/25 17:24:17 cgrigis Exp $ */

package Applications;

// library inclusions
import waba.io.*;
import java.lang.*;
import waba.sys.*;
import waba.sys.VmShell;

/*
 * Todo: Test createDir, delete, exists, rename, seek
 */ 

class FileTest
{
  public static void main (String[] args) {
    VmShell.println("FileTest" + " "+args.length);
    FileTest f = new FileTest();

    if(args.length<1 || args[0]==null)
      usage();
    else {
      if(args[0].equals("a"))
	testLD();
      else if(args[0].equals("b"))
	testFRd();
      else if(args[0].equals("c"))
	testFCr();
      else
	usage();
    }
  }  // method main


  private static void usage()
  {
      VmShell.println("usage: FileTest a|b|c|d\n");
  }

  private static void testLD()
  {
    File file;
    VmShell.println("ListDir:");
    file = new File(".", File.READ_ONLY);
    if(!file.isOpen()) {
      VmShell.println("*** could not open dir");
    }
    else {
      String list[];
      list = file.listDir();
      if(list==null) {
	VmShell.println("*** could not listdir");
      }
      else {
	int l = list.length;
	VmShell.println("Found "+l+" object"+(l>1?"s":""));
	for(int i=0; i<l; ++i) {
	  File f = new File(list[i], File.READ_ONLY);

	  VmShell.print(f.isDir() ? "d ":"f ");
	  VmShell.print("[");
	  VmShell.print(Convert.toString(i));
	  VmShell.print("] = ");
	  VmShell.print(list[i]);

	  VmShell.print("\r\t\t\t\t");
	  VmShell.print(Convert.toString(f.getLength()));
	  VmShell.print("\t[");
	  VmShell.print(f.getPath());
	  VmShell.print("]");


	  VmShell.println("");
	  f.close();
	}
      }
    }
    
  }  // method

  static final String fname = "test.txt";

  private static void testFRd()
  {
    VmShell.println("Read file:");

    File file = new File(fname, File.READ_ONLY);
    if (!file.isOpen()) {
      VmShell.println("*** could not open file "+'"'+fname+'"'+" for reading");
      return;
    }
    byte b[] = new byte[25];
    int nr = file.readBytes(b, 0, b.length);
    VmShell.println("read "+nr+" byte"+(nr>1?"s":"")+":");
    VmShell.print("<");
    for(int i=0; i<nr; ++i)
      VmShell.print(""+(char)b[i]);
    VmShell.println(">");
    file.close();
  }  // method

  private static void testFCr()
  {
    VmShell.println("Create file:");

    File file = new File(fname, File.CREATE);
    if (!file.isOpen()) {
      VmShell.println("*** could not open file "+'"'+fname+'"'+" for create");
      return;
    }
    byte b[] = new byte[25];
    for(int i=0; i<b.length; ++i) b[i]=(byte)(255-i);
    int nr = file.writeBytes(b, 0, b.length);
    VmShell.println("wrote "+nr+" byte"+(nr>1?"s":"")+":");
    file.close();
  }  // method

  
}  // class HelloWorld


/*
 * File is a file or directory.
 * <p>
 * The File class will not work under the PalmPilot since it does not
 * contain a filesystem.
 * <p>
 * Here is an example showing data being read from a file:
 *
 * <pre>
 * File file = new File("/temp/tempfile", File.READ_ONLY);
 * if (!file.isOpen())
 *   return;
 * byte b[] = new byte[10];
 * file.readBytes(b, 0, 10);
 * file.close();
 * file = new File("/temp/tempfile", File.DONT_OPEN);
 * file.delete();
 * </pre>
 */
