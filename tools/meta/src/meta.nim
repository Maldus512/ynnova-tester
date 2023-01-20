import std/parsecsv
import std/parseopt
import std/options
import std/strformat
import strutils
import streams
import os


type Args = object
    csvPath: string
    outputPath: string


proc getCsvPath(): Option[Args] =
  var p = initOptParser(os.commandLineParams())

  var csvPath : Option[string] = none(string)
  var outputPath : Option[string] = none(string)

  while true:
    p.next()
    case p.kind
    of cmdArgument: 
        if csvPath.isNone:
            csvPath = some(p.key)
        else:
            outputPath = some(p.key)
    of cmdEnd: break
    else: continue

  return csvPath.map(
    func (csv:string): Option[Args] = 
        outputPath.map(
            func (output:string): Args = 
                Args(csvPath:csv, outputPath:output)
        )
  ).flatten()


proc readCsv(path: string): seq[tuple[code:int, description:string]] =
  var s = newFileStream(path, fmRead)
  if s == nil:
    quit("cannot open the file" & path)

  var content : seq[tuple[code:int, description:string]] = @[]
  var parser: CsvParser
  open(parser, s, path, separator=';')
  while readRow(parser):
    content.add((code:parseInt(parser.row[0]), description: parser.row[1]))
  close(parser)

  return content


proc main() =
  let args = getCsvPath()
  if args.isSome:
    let args = args.get()
    let content = readCsv(args.csvPath)

    var moduleName : string = args.csvPath.lastPathPart()
    moduleName.removeSuffix(".csv")

    var headerfile = fmt("#ifndef {moduleName.toUpperAscii()}_H_INCLUDED\n#define {moduleName.toUpperAscii()}_H_INCLUDED\n\n")
    headerfile = headerfile & fmt("const char *{moduleName}_to_string(int {moduleName});\n\n")
    headerfile = headerfile & "#endif"

    var tabs = 0
    var cfile = "#include <stdlib.h>\n"
    cfile = cfile & fmt("#include \"{moduleName}.h\"\n\n")
    cfile = cfile & fmt("const char *{moduleName}_to_string(int {moduleName}) {'{'}\n")
    tabs += 4
    cfile = cfile & fmt("{spaces(tabs)}switch ({moduleName}) {'{'}\n")
    tabs += 4

    for line in items(content):
        cfile = cfile & fmt("{spaces(tabs)}case {line.code}:\n{spaces(tabs+4)}return \"{line.description}\";\n")

    cfile = cfile & fmt("{spaces(tabs)}default:\n{spaces(tabs+4)}return NULL;\n")
    tabs -= 4;
    cfile = cfile & fmt("{spaces(tabs)}}}\n")
    tabs -= 4;
    cfile = cfile & fmt("{spaces(tabs)}}}\n")

    writeFile(fmt("{args.outputPath}/{moduleName}.h"), headerfile)
    writeFile(fmt("{args.outputPath}/{moduleName}.c"), cfile)

  else:
    echo "Please specify input csv and output folder"



when isMainModule:
    main()