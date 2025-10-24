
import
  std/[macros, strutils]

{. push compileTime .}

proc addColonPragma*(procDef: NimNode; name: string; value: NimNode) =
  procDef.addPragma(newColonExpr(ident(name), value))

proc addColonPragma*(procDef: NimNode; name, value: string; ) =
  addColonPragma(procDef, name, newLit(value))

proc nodeIsExportc*(n: NimNode): bool =
  # Tests if the given node is an identifier with value of exportc or exportcpp
  result = n.eqIdent("exportc") or n.eqIdent("exportcpp")

proc getIdent*(node: NimNode): string =
  # Extracts the string value of the ident assuming node is a postfix
  # expression, symbol or an identifier.
  case node.kind
  of nnkPostfix:
    if node[0].eqIdent("*"): 
      result = node[1].strVal
  of nnkSym, nnkIdent:
    result = node.strVal
  else:
    discard

proc getExportedName*(sym: NimNode): string =
  # Gets the value given to the type's exportc/exportcpp pragma. If no string
  # was given to the pragma, then the type name is returned. If no exportc/exportcpp
  # pragma was present then an empty string is returned.
  expectKind(sym, nnkSym)
  let impl = getImpl(sym)
  if impl != nil and impl.kind == nnkTypeDef:
    if impl[0].kind == nnkPragmaExpr:
      let typeName = getIdent(impl[0][0])
      for p in impl[0][1]:
        if nodeIsExportc(p):
          result = typeName
        elif p.kind == nnkExprColonExpr and nodeIsExportc(p[0]):
          result = p[1].strVal % [typeName]

proc getCallableName*(n: NimNode): string =
  # If the node matches a UFCS function call (n.kind is either nnkCommand,
  # nnkCall or nnkIdent), return the name of the function.
  # an empty string is returned if the node does not match.
  case n.kind
  of nnkCommand, nnkCall:
    return getIdent(n[0])
  of nnkIdent:
    return n.strVal
  else:
    return ""

{. pop .}
