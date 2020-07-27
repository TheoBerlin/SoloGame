import re, sys, getopt

def readLine(line):
    searchResults   = re.search(r"^\[([^:]+):([0-9]+)\]: \(([^)]+)\)\s(.*?)$", line)
    fileName        = searchResults.group(1)
    lineNr          = searchResults.group(2)
    lintCategory    = searchResults.group(3)
    message         = searchResults.group(4)

    outMessage = f"({lintCategory}) {message}"

    print(f"::warning file={fileName},line={lineNr}::{outMessage}")

def readReport(fileName):
    containsInfoLine = False

    lineCount = 0
    with open(fileName) as file:
        for line in file:
            lineCount += 1
            if line.startswith("(information)"):
                containsInfoLine = True
            else:
                readLine(line)

    # Succeed if the lint report is empty, or only contains the information line
    return (lineCount == 0) or (containsInfoLine and lineCount == 1)

def main(argv):
    inputFile = ""
    try:
        opts, args = getopt.getopt(argv,"hi:",["ifile="])
        (args) # Hack to remove lint error of args not being used
    except getopt.GetoptError:
        print("read-lint-report.py -i <inputfile>")
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print("read-lint-report.py -i <inputfile>")
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputFile = arg

    return readReport(inputFile)

if __name__ == "__main__":
    main(sys.argv[1:])
