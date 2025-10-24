#
# C/C++ interop
#
# Generates a C++ header that includes the Nim compiler generated header.
# The header generator allows for the exporting of constants to either a
# C++ constexpr variable or a preprocessor define.
#
# Also included is a String wrapper type for nim strings, so that the frontend
# can easily access nim strings without needing a conversion to a `cstring`
#

import
  std/[macros, strformat],
  ./macroutils

{. push compileTime .}

proc rewriteThis(procDef: NimNode; typing: NimNode = nil; ): NimNode =
  # changes the name of the first parameter (C++ "this") to a gensym.
  # also if `typing` is not nil, then the parameter's typing is overwritten
  # as well
  let this = procDef.params[1]
  result = this[0]
  this[0] = genSym(nskParam, "p" & this[0].strVal)
  if typing != nil:
    this[1] = typing

proc paramVarToPtr(procDef: NimNode; thisType: NimNode) =
  # converts the first parameter of the given procDef to `ptr thisType`
  let
    ident = rewriteThis(procDef, newTree(nnkPtrTy, thisType))
    param = procDef.params[1][0]
    body = procDef.body
  procDef.body = quote do:
    template `ident`(): var `thisType` {.used.} = `param`[]
    block:
      `body`

proc constMemberFix(procDef, thisType: NimNode; ) {.compileTime.} =
  # allows const member functions to work by removing the const qualifier on
  # the C++ `this` variable. A view is then created from this pointer, to
  # ensure that the constness is still enforced by Nim
  let
    ident = rewriteThis(procDef, thisType)
    param = procDef.params[1][0]
    unsafeThis = genSym(nskLet, "unsafeThis")
    body = procDef.body
  procDef.body = quote do:
    let `unsafeThis` {.importc, noinit.}: ptr `thisType`
    {. emit: [
      `unsafeThis`,
      " = const_cast<decltype(",
      `unsafeThis`,
      ")>(this);" ].}
    let `ident`: lent `thisType` = `unsafeThis`[]
    block:
      `body`

template bycref*() {.pragma.}
  ## Custom pragma to annotate that a parameter should be passed by a C++
  ## const reference inside a [members] block
  ##

template bymove*() {.pragma.}
  ## Custom pragma to annotate that a parameter should be passed by a C++
  ## move reference inside a [members] block
  ##

type
  CxxPassStyle = enum
    cxxByVal        # T t
    cxxByConstRef   # T const& t
    cxxByRef        # T& t
    cxxByMoveRef    # T&& t

func memberParams(params: openArray[CxxPassStyle]; startingIndex: int): string =
  let last = params.len - 1
  for i, style in pairs(params):
    let n = $(i + startingIndex)
    result.add(&"'{n} ")
    case style
    of cxxByVal:
      discard
    of cxxByConstRef:
      result.add("const& ")
    of cxxByRef:
      discard
    of cxxByMoveRef:
      result.add("& ")
    result.add(&"#{n}")
    if i != last:
      result.add(", ")

func renderMember(params: openArray[CxxPassStyle]; isConst, isStatic: bool): string =
  if isStatic:
    result = "static "
  result.add("$1(")
  result.add(memberParams(params, 2))
  result.add(") ")
  if isConst:
    result.add("const ")
  result.add("-> '0")

func renderConstructor(params: openArray[CxxPassStyle]; class: string): string =
  result = &"{class}("
  result.add(memberParams(params, 1))
  result.add(")")

iterator paramIdents(procDef: NimNode): NimNode =
  let params = procDef.params
  for i in 1..<params.len:
    let p = params[i]
    for j in 0..<(p.len() - 2):
      yield p[j]

proc processMemberParams(procDef: NimNode): seq[CxxPassStyle] {.compileTime.} =
  for paramIdent in paramIdents(procDef):
    var style: CxxPassStyle
    if paramIdent.kind == nnkPragmaExpr:
      let pragmas = paramIdent[1]
      for i in 0..<pragmas.len:
        if pragmas[i].kind == nnkIdent:
          case pragmas[i].strVal
          of "bycref":
            style = cxxByConstRef
            pragmas[i] = ident("bycopy")
          of "bymove":
            style = cxxByMoveRef
            pragmas[i] = ident("byref")
          of "bycopy": style = cxxByVal
          of "byref": style = cxxByRef
          else: discard
    result.add(style)

proc replaceUnderscore(procDef: NimNode; replace: string) =
  proc impl(n: NimNode; replace: string): NimNode =
    if n.eqIdent("_"):
      return ident(replace)
    else:
      for i in 0..<len(n):
        n[i] = impl(n[i], replace)
      return n
  
  let params = procDef.params
  params[0] = impl(params[0], replace)
  for i in 1..<len(params):
    params[i] = impl(params[i], replace)
    
proc memberImpl(procDef, class: NimNode; exportedClass: string): NimNode =
  replaceUnderscore(procDef, class.strVal)
  let params = procDef.params
  type MemberKind = enum
    mkError
    mkVar
    mkConst
    mkStatic
  let kind = block:
    if params.len < 2:
      mkError
    else:
      let firstType = params[1][1]
      if firstType.kind == nnkCommand and firstType.len == 2 and
         firstType[0].eqIdent("static") and firstType[1].eqIdent(class):
        mkStatic
      elif firstType.kind == nnkVarTy and firstType[0].eqIdent(class):
        mkVar
      elif firstType.eqIdent(class):
        mkConst
      else:
        mkError
  case kind
  of mkVar:
    paramVarToPtr(procDef, class)
  of mkConst:
    constMemberFix(procDef, class)    
  of mkStatic:
    params[1][0] = ident("_")
    params[1][1] = ident(class.strVal)
    var paramsNoThis = params[0..0] & params[2..^1]
    result = newProc(
      ident(&"{class.strVal}_{procDef.name.strVal}"),
      paramsNoThis,
      newEmptyNode(),
      nnkProcDef,
      newTree(
        nnkPragma,
        newColonExpr(
          ident("importcpp"),
          newLit(&"{exportedClass}::{procDef.name.strVal}(@)")
        )
      )
    )
  of mkError:
    error(&"first parameter must be of type {class}, var {class} or static {class}", procDef)
  var styles = processMemberParams(procDef)
  styles.delete(0)
  procDef.addColonPragma("member", renderMember(styles, kind == mkConst, kind == mkStatic))

proc constructorImpl(procDef, class: NimNode; classExport: string) =
  if procDef.name.strVal == "_":
    procDef.name = ident("init" & class.strVal)
  replaceUnderscore(procDef, class.strVal)
  let returnType = procDef.params[0]
  if not returnType.eqIdent(class):
    error(&"constructor must return {class.strVal}", returnType)
  let styles = procDef.processMemberParams()
  procDef.addColonPragma("constructor", renderConstructor(styles, classExport))

proc destructorMember(class: string): string =
  result =  &"~{class}()"

proc destructorImpl(paramName: string; body, class: NimNode; classExport: string): NimNode =
  let 
    member = destructorMember(classExport)
    paramIdent = ident(paramName)
  result = quote do:
    proc destructor(`paramIdent`: var `class`) {.member: `member`.} =
      `body`
  paramVarToPtr(result, class)

proc destructorImpl(class: NimNode; classExport: string): NimNode =
  let 
    destroy = ident("=destroy")
    member = destructorMember(classExport)
  result = quote do:
    proc destructor*(o: ptr `class`) {.member: `member`.} =
      `destroy`(o[])

{. pop .}

macro members*(class: typed; body) =
  ## Declare and add member functions to the given object type `class`. Each
  ## statement in `body` must be a procedure declaration or a command block
  ## (constructor or destructor). `class` must have an `exportc` or `exportcpp`
  ## set.
  ## 
  ## To add a member function, write a procedure declaration with the first
  ## parameter being of type `class` or use `_` as a placeholder. The exact
  ## typing of the first parameter determines the type of member function added:
  ## - if the type is `class` then the proc will become a const member function
  ## - if the type is `var class` then the proc will become a non-const member function
  ## - if the type is `static class` then the proc will become a static member function
  ## 
  ## To add one or more constructors, add a `constructor` block with procedure
  ## definitions. Each constructor proc must return `class` or `_`. If the
  ## procedure name is `_`, it will be renamed to `init class`, ie, a constructor
  ## for class `FooBar` will use `initFooBar` as the proc name.
  ## 
  ## To specify a destructor, add a destructor command with an identifier to refer
  ## to the object, followed by the destructor code block. If you omit the identifier
  ## and code block then a default destructor using nim's `=destroy` hook will
  ## be generated for you. The name of the destructor proc is `destructor` in
  ## both cases.
  ## 
  ##
  result = newStmtList()
  let exportedClass = getExportedName(class)
  if exportedClass == "":
    error("Type must have an exportc or exportcpp pragma", class)
  var destructorDeclared = false
  expectKind(body, nnkStmtList)
  for node in body:
    let command = getCallableName(node)
    if command == "":
      expectKind(node, nnkProcDef)
      let extra = memberImpl(node, class, exportedClass)
      result.add node
      if extra != nil:
        result.add extra
    elif command.eqIdent("constructor"):
      expectLen(node, 2)
      expectKind(node[1], nnkStmtList)
      for p in node[1]:
        expectKind(p, nnkProcDef)
        constructorImpl(p, class, exportedClass)
        result.add p
    elif command.eqIdent("destructor"):
      if destructorDeclared:
        error("can only declare 1 destructor")
      if len(node) == 0:
        result.add destructorImpl(class, exportedClass)
      else:
        expectLen(node, 3)
        expectKind(node[1], nnkIdent)
        expectKind(node[2], nnkStmtList)
        result.add destructorImpl(node[1].strVal, node[2], class, exportedClass)
      destructorDeclared = true
    else:
      error("command must either be constructor or destructor", node)

