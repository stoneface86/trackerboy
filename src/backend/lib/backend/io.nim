
import
  ./[document, interop],
  libtrackerboy/[data, io],
  std/[os, streams]


type
  BIoResult* {.exportc.} = enum
    ioSuccess
    ioNotRecognized
    ioRevisionTooNew
    ioRevisionTooOld
    ioCorrupted
    ioReadError
    ioWriteError
    ioFileError

  BAutoBackupResult* {.exportc.} = enum
    backupSuccess
    backupFailed
    backupFailedLocationInUse
  
  BSaveResult* {.exportc.} = object
    io: BIoResult
    backup: BAutoBackupResult

  BIo* {.exportc.} = object

func toIoResult(fr: FormatResult): BIoResult =
  case fr
  of frNone: ioSuccess
  of frInvalidSignature: ioNotRecognized
  of frInvalidRevision: ioRevisionTooNew
  of frCannotUpgrade: ioRevisionTooOld
  of frInvalidSize..frInvalidTerminator: ioCorrupted
  of frReadError: ioReadError
  of frWriteError: ioWriteError

header:
  addEnum(BIoResult)
  addEnum(BAutoBackupResult)

proc safeClose(s: Stream) =
  try:
    s.close()
  except IoError, OsError:
    discard

members(BIo):
  proc save*(_: static _; doc {.bycref.}: BDocument; filename: string;
             backup: bool
             ): BSaveResult =
    if filename == "":
      result.io = ioFileError
    else:
      # auto backup if configured
      if backup and fileExists(filename):
        let backupPath = filename & ".bak"
        if dirExists(backupPath):
          result.backup = backupFailedLocationInUse
        else:
          try:
            copyFile(filename, backupPath)
            result.backup = backupSuccess
          except IoError, OsError:
            result.backup = backupFailed
      # serialize the document's module to the given filename
      let fs = newFileStream(filename, fmWrite)
      if fs == nil:
        result.io = ioFileError
      else:
        result.io = toIoResult(serialize(doc.module, fs))
        fs.safeClose()
  
  proc load*(_: static _; doc: var BDocument; filename: string): BIoResult =
    # deserialize the module stored in the given file, updating doc's module on success
    let fs = newFileStream(filename, fmRead)
    if fs == nil:
      result = ioFileError
    else:
      var module: Module
      result = toIoResult(module.deserialize(fs))
      if result == ioSuccess:
        doc.module = move(module)
      fs.safeClose()
