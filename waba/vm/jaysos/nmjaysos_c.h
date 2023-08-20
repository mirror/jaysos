/* $Id: nmport_c.h,v 1.4 2002/01/24 17:02:14 cgrigis Exp $ */

#ifndef __NMPORT_C_H__
#define __NMPORT_C_H__

/*
Copyright (C) 1998, 1999, 2000 Wabasoft

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.
*/

void WindowDestroy(WObject obj);
void MainWinDestroy(WObject obj);
void GraphicsDestroy(WObject obj);
void ImageDestroy(WObject obj);
#if WITH_CATALOG_CLASS
void CatalogDestroy(WObject obj);
#endif
void FileDestroy(WObject obj);
void SocketDestroy(WObject obj);
void SerialPortDestroy(WObject obj);

extern ClassHook classHooks[];

#define FUNC(f) extern Var f (Var stack[])

/* some generic functions */
FUNC(Return0Func);
FUNC(ReturnNeg1Func);

/* all the native functions */

FUNC(VmPrintLn);
FUNC(VmPrint);
FUNC(VmExec);
FUNC(VmGetTimeStamp);
FUNC(copyArray);
FUNC(VmExit);
FUNC(VmSleep);
FUNC(VmSetDeviceAutoOff);
FUNC(VmGetUserName);
FUNC(VmGetPlatform);
FUNC(VmIsColor);
FUNC(FileGetLength);
FUNC(FileCreateDir);
FUNC(FileReadBytes);
FUNC(FileRename);
FUNC(FileCreate);
FUNC(FileWriteBytes);
FUNC(FileListDir);
FUNC(FileSeekWaba);
FUNC(FileIsDir);
FUNC(FileCloseWaba);
FUNC(FileIsOpen);
FUNC(FileDeleteWaba);
FUNC(FileExists);
FUNC(ImageCreate);
FUNC(ImageFree);
FUNC(ImageSetPixels);
FUNC(ImageUseImagePalette);
FUNC(ImageLoad);
FUNC(SoundBeep);
FUNC(SoundTone);
FUNC(TimeCreate);
FUNC(SocketCreate);
FUNC(SocketRead);
FUNC(SocketWrite);
FUNC(SocketSetReadTimeout);
FUNC(SocketClose);
FUNC(SocketIsOpen);
FUNC(WindowCreate);
#if WITH_CATALOG_CLASS
FUNC(CatalogListCatalogs);
FUNC(CatalogAddRecord);
FUNC(CatalogSkipBytes);
FUNC(CatalogCreate);
FUNC(CatalogRead);
FUNC(CatalogDeleteRecord);
FUNC(CatalogSetRecordPos);
FUNC(CatalogGetRecordSize);
FUNC(CatalogResizeRecord);
FUNC(CatalogWrite);
FUNC(CatalogGetRecordCount);
FUNC(CatalogClose);
FUNC(CatalogIsOpen);
FUNC(CatalogDelete);
#endif //WITH_CATALOG_CLASS
FUNC(GraphicsCopyRect);
FUNC(GraphicsClearClip);
FUNC(GraphicsSetFont);
FUNC(GraphicsSetDrawOp);
FUNC(GraphicsSetClip);
FUNC(GraphicsSetBackColor);
FUNC(GraphicsSetColor);
FUNC(GraphicsGetClip);
FUNC(GraphicsFillRect);
FUNC(GraphicsDrawLine);
FUNC(GraphicsTranslate);
FUNC(GraphicsDrawDots);
FUNC(GraphicsDrawChars);
FUNC(GraphicsCreate);
FUNC(GraphicsDrawCursor);
FUNC(GraphicsSetClip);
FUNC(GraphicsFree);
FUNC(GraphicsSetForeColor);
FUNC(GraphicsSetTextColor);
FUNC(GraphicsFillPolygon);
FUNC(GraphicsDrawString);
FUNC(ConvertStringToInt);
FUNC(ConvertFloatToIntBitwise);
FUNC(ConvertCharToString);
FUNC(ConvertIntToFloatBitwise);
FUNC(ConvertFloatToString);
//#if WITH_64BITS
FUNC(ConvertDoubleToString);
//#endif
FUNC(ConvertIntToString);
//#if WITH_64BITS
FUNC(ConvertLongToString);
//#endif
FUNC(ConvertBooleanToString);
FUNC(SoundClipPlay);
FUNC(PalmOsPrefSetPalmOsPref);
FUNC(PalmOsPrefGetPalmOsPref);
FUNC(MainWinCreate);
FUNC(MainWinExit);
FUNC(MainWinSetTimerInterval);
FUNC(SerialPortReadCheck);
FUNC(SerialPortRead);
FUNC(SerialPortWrite);
FUNC(SerialPortSetReadTimeout);
FUNC(SerialPortClose);
FUNC(SerialPortSetFlowControl);
FUNC(SerialPortIsOpen);
FUNC(SerialPortCreate);
FUNC(FontMetricsGetStringWidth);
FUNC(FontMetricsGetCharWidth);
FUNC(FontMetricsCreate);
FUNC(FontMetricsGetCharArrayWidth);

#endif /* __NMPORT_C_H__ */
