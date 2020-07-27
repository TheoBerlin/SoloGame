import re, sys, getopt

def readLine(line):
    fileRegex       = r"\[([^:]+):([0-9]+)\]"
    anything        = ".*"
    categoryRegex   = r"(\([^)]+\))"
    messageRegex    = "(.*?)$"
    searchResults   = re.search(rf"{fileRegex}(?:{anything}{fileRegex})?{anything}{categoryRegex}\s{messageRegex}", line).groups()

    if len(searchResults) != 6:
        print(f"Failed to regex search string: {line}")
        return

    firstFileName   = searchResults[0]
    firstLineNr     = searchResults[1]
    lintCategory    = searchResults[4]
    message         = searchResults[5]
    outMessage = ""
    if searchResults[2] is not None and searchResults[3] is not None:
        # The line contains two files. Have the second one be printed in the output message
        secondFileName  = searchResults[2]
        secondLineNr    = searchResults[3]
        outMessage += f"[{secondFileName}:{secondLineNr}]: "

    outMessage += f"{lintCategory} {message}"
    print(f"::warning file={firstFileName},line={firstLineNr}::{outMessage}")

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
    if (lineCount == 0) or (containsInfoLine and lineCount == 1):
        return 0

    return 1

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
            sys.exit(1)
        elif opt in ("-i", "--ifile"):
            inputFile = arg

    sys.exit(readReport(inputFile))

if __name__ == "__main__":
    main(sys.argv[1:])
