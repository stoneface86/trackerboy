
import
  ./[document, interop],
  libtrackerboy/[data, io],
  std/[os, paths, streams]


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
  
  DocumentFilePrivate = object
    path: Path
    autoBackup: bool
    lastBackupResult: BAutoBackupResult

  BDocumentFile* {.exportc.} = object
    p: DocumentFilePrivate

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

proc saveImpl(df: var BDocumentFile; doc: BDocument; filename: string): BIoResult =
  if filename == "":
    result = ioFileError
  else:
    # auto backup if configured
    if df.p.autoBackup and fileExists(filename):
      let backupPath = filename & ".bak"
      if dirExists(backupPath):
        df.p.lastBackupResult = backupFailedLocationInUse
      else:
        try:
          copyFile(filename, backupPath)
          df.p.lastBackupResult = backupSuccess
        except IoError, OsError:
          df.p.lastBackupResult = backupFailed
    # serialize the document's module to the given filename
    let fs = newFileStream(filename, fmWrite)
    if fs == nil:
      result = ioFileError
    else:
      result = toIoResult(serialize(doc.module, fs))
      fs.safeClose()

members(BDocumentFile):
  constructor:
    proc _(): _ =
      discard
  
  destructor

  proc setAutoBackup*(d: var _; on: bool) =
    d.p.autoBackup = on

  proc lastBackupResult*(d: _): BAutoBackupResult =
    result = d.p.lastBackupResult

  proc open*(df: var _; doc: var BDocument; filename: string): BIoResult =
    # deserialize the module stored in the given file, updating doc's module on success
    let fs = newFileStream(filename, fmRead)
    if fs == nil:
      result = ioFileError
    else:
      var module: Module
      result = toIoResult(module.deserialize(fs))
      if result == ioSuccess:
        doc.module() = module
        df.p.path = filename.Path
      fs.safeClose()

  proc save*(df: var _; doc {.bycref.}: BDocument): BIoResult =
    result = df.saveImpl(doc, df.p.path.string)

  proc save*(df: var _; doc {.bycref.}: BDocument; filename: string): BIoResult =
    result = df.saveImpl(doc, filename)
    if result == ioSuccess:
      df.p.path = filename.Path

