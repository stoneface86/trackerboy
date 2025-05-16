
import
  ./[document, interop],
  libtrackerboy/[data, io],
  std/[os, paths, streams]


type
  IoResult* {.exportc.} = enum
    ioSuccess
    ioNotRecognized
    ioRevisionTooNew
    ioRevisionTooOld
    ioCorrupted
    ioReadError
    ioWriteError
    ioFileError

  AutoBackupResult* {.exportc.} = enum
    backupSuccess
    backupFailed
    backupFailedLocationInUse
  
  DocumentFilePrivate = object
    path: Path
    autoBackup: bool
    lastBackupResult: AutoBackupResult

  DocumentFile* {.exportc.} = object
    p: DocumentFilePrivate

func toIoResult(fr: FormatResult): IoResult =
  case fr
  of frNone: ioSuccess
  of frInvalidSignature: ioNotRecognized
  of frInvalidRevision: ioRevisionTooNew
  of frCannotUpgrade: ioRevisionTooOld
  of frInvalidSize..frInvalidTerminator: ioCorrupted
  of frReadError: ioReadError
  of frWriteError: ioWriteError

static: 
  constvar(IoResult)
  constvar(AutoBackupResult)

proc safeClose(s: Stream) =
  try:
    s.close()
  except IoError, OsError:
    discard

proc initDocumentFile*(): DocumentFile {.front.} =
  discard

proc destructor*(d: var DocumentFile)
  {.front, autodestructor.} =
  `=destroy`(d)

proc setAutoBackup*(d: var DocumentFile; on: bool)
  {.front, automember.} =
  d.p.autoBackup = on

proc lastBackupResult*(d: DocumentFile): AutoBackupResult
  {.front, automember.} =
  result = d.p.lastBackupResult

proc open*(df: var DocumentFile; doc: var Document; filename: string): IoResult
  {.front, automember.} =
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

proc saveImpl(df: var DocumentFile; doc: Document; filename: string): IoResult 
  {. raises: [] .} =
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

proc save*(df: var DocumentFile; doc {.bycref.}: Document): IoResult
  {.front, automember.} =
  result = df.saveImpl(doc, df.p.path.string)

proc save*(df: var DocumentFile; doc {.bycref.}: Document; filename: string): IoResult
  {.front, automember.} =
  result = df.saveImpl(doc, filename)
  if result == ioSuccess:
    df.p.path = filename.Path

