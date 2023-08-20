package waba.io;

/* $Id: Catalog.java,v 1.4 2002/02/19 18:32:17 bauermeister Exp $

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

import waba.util.Vector;
import waba.sys.Vm;
import waba.sys.VmShell;

/**
 * Catalog is a collection of records commonly referred to as a database
 * on small devices.
 * <p>
 * Here is an example showing data being read from records in a catalog:
 *
 * <pre>
 * Catalog c = new Catalog("MyCatalog", Catalog.READ_ONLY);
 * if (!c.isOpen())
 *   return;
 * int count = c.getRecordCount();
 * byte b[] = new byte[10];
 * for (int i = 0; i < count; i++)
 *   {
 *   c.setRecord(i);
 *   c.readBytes(b, 0, 10);
 *   ...
 *   }
 * c.close();
 * </pre>
 * Notes added by Dave Slaughter @120
 * Catalog now reads from and writes to .pdb files, so you can do a full test of your
 * database application.
 * The path of the database is the <current directory>\db.
 */

public class Catalog extends Stream
{
/** Read-only open mode. */
public static final int READ_ONLY  = 1;
/** Write-only open mode. */
public static final int WRITE_ONLY = 2;
/** Read-write open mode. */
public static final int READ_WRITE = 3; // READ | WRITE
/** Create open mode. Used to create a database if one does not exist. */
public static final int CREATE = 4;

private boolean _isOpen;
private String _name;
private int _mode;
private Vector _records;
private int _recordPos;
private int _cursor;
private String _creator; // ds@120
private String _type; // ds@120
private boolean _modified; // ds@120
private static final String BASE_DIR = "db"; // ds@120
/**
 * Opens a catalog with the given name and mode. If mode is CREATE, the
 * catalog will be created if it does not exist.
 * <p>
 * For PalmOS: A PalmOS creator id and type can be specified by appending
 * a 4 character creator id and 4 character type to the name seperated
 * by periods. For example:
 * <pre>
 * Catalog c = new Catalog("MyCatalog.CRTR.TYPE", Catalog.CREATE);
 * </pre>
 * Will create a PalmOS database with the name "MyCatalog", creator id
 * of "CRTR" and type of "TYPE".
 * <p>
 * If no creator id and type is specified, the creator id will default
 * to the creator id of current waba program and the type will default
 * to "DATA".
 * <p>
 * You must close the catalog to write the data to pdb file on disk!
 * <br><b>Note</b>: since a bug in the original Waba vm, the <i>TYPE</i>
 * <b>must</b> be identical to <i>MyCatalog</i> and they must have 4 letters.
 * If you don't follow this rules and you write a conduit to talk with the waba
 * created database, you will never find it with the conduit! (guich@120)
 * <p> * Under PalmOS, the name of the catalog must be 31 characters or less,
 * not including the creator id and type. Windows CE supports a 32
 * character catalog name but to maintain compatibility with PalmOS,
 * you should use 31 characters maximum for the name of the catalog.
 * @param name catalog name
 * @param mode one of READ_ONLY, WRITE_ONLY, READ_WRITE or CREATE
 */
public Catalog(String name, int mode) {
	// parse name
	int i, j;
	i = name.indexOf('.');
	if (i == -1) {
		_name = name;
		_creator = "CRTR";
		_type = "DATA";
	} else {
		_name = name.substring(0, i);
		j = i + 1;
		if ((i = name.indexOf('.', j)) == -1) {
			_creator = name.substring(j);
			_type = "DATA";
		} else {
			_creator = name.substring(j, i);
			_type = name.substring(++i);
		}
	}
	_mode = mode;
	if (mode == CREATE) {
		_records = new Vector();
		if (!toPDB(File.CREATE)) { // make sure empty file is written so it gets included in the list
			_isOpen = false;
		} else {
			_isOpen = true;
		};
	} else {
		if (!fromPDB()) {
			_isOpen = false;
		} else {
			_isOpen = true;
		}
	}
}
private int _readWriteBytes(byte buf[], int start, int count, boolean isRead)
{
	if (_recordPos == -1 || (start < 0 || count < 0) || (start + count > buf.length))
	   return -1;
	if ((_mode == READ_ONLY && !isRead) || (_mode == WRITE_ONLY && isRead))
	{
	   return -1;
	}

	byte rec[] = (byte[])_records.get(_recordPos);
	if (_cursor + count > rec.length)
		return -1;
	if (isRead) // guich@120: System.arraycopy is faster than a for loop.
    {
	   Vm.copyArray(rec,_cursor,buf,start,count);
       VmShell.println("Copied bytes:");
       for (int i=0; i<count; ++i)
           VmShell.print(buf[start+i]+" ");
    }
	else
	   Vm.copyArray(buf,start,rec,_cursor,count);
	if (!isRead) _modified = true;
	_cursor += count;
	return count;
}
/**
 * Adds a record to the end of the catalog. If this operation is successful,
 * the position of the new record is returned and the current position is
 * set to the new record. If it is unsuccessful the current position is
 * unset and -1 is returned.
 * @param size the size in bytes of the record to add
 */

public int addRecord(int size) {
	if (!_isOpen)
		return -1;
	_recordPos = _records.getCount();
	_records.add(new byte[size]);
	_cursor = 0;
	return _recordPos;
}
/**
 * Adds a record to the <pos> position of the catalog. If this operation is successful,
 * the position of the new record is returned and the current position is
 * set to the new record. If it is unsuccessful the current position is
 * unset and -1 is returned.
 * implemented by guich (guich@email.com) in 06/30/2000.
 * @param size the size in bytes of the record to add
 */
public int addRecord(int size, int pos) {
	if (!_isOpen || size < 0)
		return -1;
	if (pos < 0 || pos > _records.getCount())
		return -1;
	_records.insert(pos, new byte[size]);
	_recordPos = pos;
	_cursor = 0;
	return _recordPos;
}
/**
 * Closes the catalog: writtes all information back to the pdb file.
 * Returns true if the operation is successful and false otherwise.
 */

public boolean close() {
	if (!_isOpen)
		return false;
	if (_modified) {
		// guich@120 - write only if modified
		toPDB(File.READ_WRITE);
	}
	_isOpen = false;
	_recordPos = -1;
	return true;
}
/**
 * Deletes all the records of the catalog. Returns true if the operation is successful and false
 * otherwise. Note that this behavior is diferent from palm os, because in applets we cannot erase files at the server.
 */

public boolean delete()
{
	if (!_isOpen)
		return false;
   _records.removeAll(); // erases the vector
	_isOpen = false;
	_recordPos = -1;

   File os = new File(getFileName(), File.WRITE_ONLY);
   if (os == null)
	  return false;
   return true;
}
/**
 * Deletes the current record and sets the current record position to -1.
 * The record is immediately removed from the catalog and all subsequent
 * records are moved up one position.
 */

public boolean deleteRecord() {
	if (_recordPos == -1)
		return false;
	_records.del(_recordPos);
	_recordPos = -1;
	return true;
}
/** reads a pdb file */
private boolean fromPDB() {
	File file = new File(getFileName(), File.READ_ONLY);
	if (!file.isOpen()) {
		return false;
	}
	DataStream is = new DataStream(file);

	// DatabaseHdrType
	byte name[] = new byte[32]; // ps: the writted string is in c++ format, so this routine doesnt loads the name correctly (comes trash with it)
	byte type[] = new byte[4];
	byte creator[] = new byte[4];
	is.readBytes(name,0,name.length);
	short attributes = is.readShort();
	short version = is.readShort();
	int creationDate = is.readInt();
	int modificationDate = is.readInt();
	int lastBackupDate = is.readInt();
	int modificationNumber = is.readInt();
	int appInfoID = is.readInt();
	int sortInfoID = is.readInt();
	is.readBytes(type,0,type.length);
	is.readBytes(creator,0,creator.length);
	int uniqueIDSeed = is.readInt();

	// verify if the creatorId is valid
	if (!_creator.equals(getString(creator)) || !_type.equals(getString(type))) {
		is.close();
		return false;
	}

	// RecordListType
	int nextRecordListID = is.readInt();
	short numRecords = is.readShort();

    VmShell.println("# records: "+numRecords);


	// reads the header (meaningless)
	int recOffsets[] = new int[numRecords + 1];
	byte recAttributes;
	byte recUniqueID[] = new byte[3];
	for (int i = 0; i < numRecords; i++) {
        int nOffset = is.readInt(); // offset
		recOffsets[i] = nOffset;
        VmShell.println("Offset "+i+": "+nOffset);
		recAttributes = is.readByte();
        VmShell.println("  attributes: "+recAttributes);

        VmShell.println("recUniqueID.length = "+recUniqueID.length);

		is.readBytes(recUniqueID,0,recUniqueID.length);
        for (int k=0; k<recUniqueID.length; ++k)
            VmShell.println("  "+recUniqueID[k]);
	}
	recOffsets[numRecords] = file.getLength(); // add the total size so we can compute the size of each record

	is.readShort(); // pad
	int offset = 80 + numRecords * 8;

	// the records were writted in sequence from here
	_records = new Vector(numRecords);
	int size = 0;
	for (int i = 0; i < numRecords; i++) {
		size = recOffsets[i + 1] - recOffsets[i];
		byte[] bytes = new byte[size];
        file.seek(recOffsets[i]);
		is.readBytes(bytes,0,size);
		_records.add(bytes);
	}
	is.close();
	return true;
}
private byte[] getBytes(String s) {
	char[] chars = s.toCharArray();
	byte[] bytes = new byte[chars.length];
	for (int i=chars.length -1; i>=0; i--) {
		bytes[i] = (byte) chars[i];
	}
	return bytes;
}
/** returns the file name of this catalog */
// guich@120
private String getFileName()
{
   return BASE_DIR+"/"+_name+".PDB";//+_creator+"."+_type;
}
/**
 * Returns the number of records in the catalog or -1 if the catalog is not open.
 */

public int getRecordCount() {
	if (!_isOpen)
		return -1;
	return _records.getCount();
}
/**
 * Returns the size of the current record in bytes or -1 if there is no
 * current record.
 */

public int getRecordSize() {
	if (_recordPos == -1)
		return -1;
	byte rec[] = (byte[]) _records.get(_recordPos);
	return rec.length;
}
private String getString(byte[] bytes) {
	char[] chars = new char[bytes.length];
	for (int i=bytes.length -1; i>=0; i--) {
		chars[i] = (char) bytes[i];
	}
	return new String(chars);
}
/** Inspects a record. use this method with careful, none of the params are checked for validity.
 * the cursor is not advanced, neither the current record position. this method must be used
 * only for a fast way of viewing the contents of a record,
 * like searching for a specific header or filling a grid of data.
 * <i>buf.length</i> bytes (at maximum) are readen from the record into <i>buf</i>.
 * Returns the number of bytes read (can be different of buf.length if buf.length is greater
 * than the record size) or -1 if an error prevented the read operation from occurring. added by guich*/
public int inspectRecord(byte buf[], int recPosition) {
	byte rec[] = (byte[]) _records.get(recPosition);
	int count;
	if (rec.length > buf.length) {
		count = buf.length;
	} else {
		count = rec.length;
	}
	waba.sys.Vm.copyArray(rec, 0, buf, 0, count);
	return count;
}
/**
 * Returns true if the catalog is open and false otherwise. This can
 * be used to check if opening or creating a catalog was successful.
 */

public boolean isOpen() {
	return _isOpen;
}
/**
 * Returns the complete list of existing catalogs. If no catalogs exist, this
 * method returns null.
 */

public static String[] listCatalogs() {
	File dir = new File(BASE_DIR, File.READ_ONLY);
	if (!dir.isDir()) {
		return null;
	}
	String[] files = dir.listDir();
	int j = 0;
	for (int i = 0; i < files.length; i++) {
		int len = files[i].length();
		if (len > 4 && waba.sys.Convert.toLowerCase(files[i].substring(len-4)).equals(".pdb")) {
			files[j] = files[i].substring(0,len-4);
			j++;
		}
	}
	if (j == 0) {
		return null;
	}
	if (j != files.length) {
		String[] valid = new String[j];
		Vm.copyArray(files, 0, valid, 0, j);
		return valid;
	}
	return files;
}
/**
 * Reads bytes from the current record into a byte array. Returns the
 * number of bytes actually read or -1 if an error prevented the
 * read operation from occurring. After the read is complete, the location of
 * the cursor in the current record (where read and write operations start from)
 * is advanced the number of bytes read.
 * @param buf the byte array to read data into
 * @param start the start position in the array
 * @param count the number of bytes to read
 */

public int readBytes(byte buf[], int start, int count) {
	return _readWriteBytes(buf, start, count, true);
}
/**
 * Resizes a record. This method changes the size (in bytes) of the current record.
 * The contents of the existing record are preserved if the new size is larger
 * than the existing size. If the new size is less than the existing size, the
 * contents of the record are also preserved but truncated to the new size.
 * Returns true if the operation is successful and false otherwise.
 * @param size the new size of the record
 */

public boolean resizeRecord(int size) {
	if (_recordPos == -1)
		return false;
	byte oldRec[] = (byte[]) _records.get(_recordPos);
	byte newRec[] = new byte[size];
	int copyLen;
	if (oldRec.length < newRec.length)
		copyLen = oldRec.length;
	else
		copyLen = newRec.length;
	waba.sys.Vm.copyArray(oldRec, 0, newRec, 0, copyLen);
	_records.set(_recordPos, newRec);
	//_cursor = 0; guich@120
	return true;
}
/**
 * Sets the current record position and locks the given record. The value
 * -1 can be passed to unset and unlock the current record. If the operation
 * is succesful, true is returned and the read/write cursor is set to
 * the beginning of the record. Otherwise, false is returned.
 */

public boolean setRecordPos(int pos) {
	if (pos < 0 || pos >= _records.getCount()) {
		_recordPos = -1;
		return false;
	}
	_recordPos = pos;
	_cursor = 0;
	return true;
}
/**
 * Advances the cursor in the current record a number of bytes. The cursor
 * defines where read and write operations start from in the record. Returns
 * the number of bytes actually skipped or -1 if an error occurs.
 * @param count the number of bytes to skip
 */

public int skipBytes(int count) {
	if (_recordPos == -1)
		return -1;
	byte rec[] = (byte[]) _records.get(_recordPos);
	if (_cursor + count > rec.length)
		return -1;
	_cursor += count;
	return count;
}
/** converts the records to a pdb file.  */
private boolean toPDB(int fileType) {
	File dir = new File(BASE_DIR, File.DONT_OPEN);
	if (!dir.exists()) {
		dir.createDir();
	}
	File file = new File(getFileName(), fileType);
	if (!file.isOpen()) {
		return false;
	}
	byte name[] = new byte[32];
	short attributes = (short) 0x8000;
	short version = 1;
	int creationDate = 0xb08823ad;
	int modificationDate = 0xb08823ad;
	int lastBackupDate = 0xb08823ad;
	int modificationNumber = 1;
	int appInfoID = 0;
	int sortInfoID = 0;
	byte type[] = getBytes(_type);
	byte creator[] = getBytes(_creator);
	int uniqueIDSeed = 0;

	// copies the db name to inside the array <name>
	byte[] bn = getBytes(_name);
	for (int i = 0; i < name.length; i++)
		name[i] = (i < bn.length) ? bn[i] : (byte)0;
	short numRecords = (short) _records.getCount();
	DataStream os = new DataStream(file);
	int offset = 80 + numRecords * 8;

	// DatabaseHdrType
	os.writeBytes(name, 0, name.length);
	os.writeShort(attributes);
	os.writeShort(version);
	os.writeInt(creationDate);
	os.writeInt(modificationDate);
	os.writeInt(lastBackupDate);
	os.writeInt(modificationNumber);
	os.writeInt(appInfoID);
	os.writeInt(sortInfoID);
	os.writeBytes(type, 0, type.length);
	os.writeBytes(creator, 0, creator.length);
	os.writeInt(uniqueIDSeed);

	// RecordListType
	int nextRecordListID = 0;
	os.writeInt(nextRecordListID);
	os.writeShort(numRecords);
	for (int i = 0; i < numRecords; i++) {
		os.writeInt(offset); // LocalChunkID
		os.writeByte(0); // attributes
		os.writeByte((byte) (i >> 16)); // uniqueID
		os.writeByte((byte) (i >> 8)); // uniqueID
		os.writeByte((byte) (i >> 0)); // uniqueID
		offset += ((byte[]) _records.get(i)).length;
	}
	os.writeShort(0); // pad

	for (int i = 0; i < numRecords; i++) {
		byte[] bytes = (byte[]) _records.get(i);
		os.writeBytes(bytes, 0, bytes.length);
	}
	os.close();
	return true;
}
/**
 * Writes to the current record. Returns the number of bytes written or -1
 * if an error prevented the write operation from occurring.
 * After the write is complete, the location of the cursor in the current record
 * (where read and write operations start from) is advanced the number of bytes
 * written.
 * @param buf the byte array to write data from
 * @param start the start position in the byte array
 * @param count the number of bytes to write
 */

public int writeBytes(byte buf[], int start, int count) {
	return _readWriteBytes(buf, start, count, false);
}
}
