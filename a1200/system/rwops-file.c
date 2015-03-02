#include <proto/dos.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/rwops.h"

static int
FileRead(RwOpsT *stream asm("a0"),
         void *buffer asm("d2"), unsigned int size asm("d3"))
{
  int res = 0;

  if (stream->opened) {
    SetIoErr(0L);
    if ((res = Read(stream->u.file.fd, buffer, size)) == -1)
      PrintFault(IoErr(), "Read");
  }

  return res;
}

static int
FileWrite(RwOpsT *stream asm("a0"),
          const void *buffer asm("d2"), unsigned int size asm("d3"))
{
  int res = 0;
  
  if (stream->opened) {
    SetIoErr(0L);
    if ((res = Write(stream->u.file.fd, (void *)buffer, size)) == -1)
      PrintFault(IoErr(), "Write");
  }

  return res;
}

static int 
FileSeek(RwOpsT *stream asm("a0"),
         int offset asm("d2"), SeekModeT whence asm("d3"))
{
  if (stream->opened) {
    int mode = -2;
    int error;

    switch (whence) {
      case IO_SEEK_SET:
        mode = OFFSET_BEGINNING;
        break;
      case IO_SEEK_CUR:
        mode = OFFSET_CURRENT;
        break;
      case IO_SEEK_END:
        mode = OFFSET_END;
        break;
    }

    SetIoErr(0L);
    Seek(stream->u.file.fd, offset, mode);
    if ((error = IoErr())) {
      PrintFault(IoErr(), "Seek");
      return -1;
    }
  }

  return 0;
}

static int FileTell(RwOpsT *stream asm("a0")) {
  return Seek(stream->u.file.fd, 0, OFFSET_CURRENT);
}

static int FileClose(RwOpsT *stream asm("a0")) {
  stream->opened = false;

  return Close(stream->u.file.fd);
}

RwOpsT *RwOpsFromFile(const char *file, const char *mode) {
  RwOpsT *stream = NewRecord(RwOpsT);
  int dosmode = 0;

  if (*mode == 'r')
    dosmode = MODE_OLDFILE;
  else if (*mode == 'w')
    dosmode = MODE_NEWFILE;
  else if (*mode == 'a')
    dosmode = MODE_READWRITE;

  stream->type = IO_FILE;
  stream->u.file.fd = Open(file, dosmode);
  stream->opened = true;

  stream->read = FileRead;
  stream->write = FileWrite;
  stream->seek = FileSeek;
  stream->tell = FileTell;
  stream->close = FileClose;

  if (!stream->u.file.fd) {
    MemUnref(stream);
    stream = NULL;
  }


  return stream;
}
