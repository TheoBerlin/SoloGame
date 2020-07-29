import re, sys, getopt

# Regexes that will catch messages to be suppressed
suppressedRegexes = [
    "Class '.*' has a constructor with 1 argument that is not explicit",
    "Consider using std::transform",
    "The function '.*' is never used",
    r"^\(information\)"
]

def isSuppressed(line):
    for suppressedRegex in suppressedRegexes:
        if re.search(suppressedRegex, line) is not None:
            return True
    return False

def printWarning(line):
    fileRegex       = r"\[([^:]+):([0-9]+)\]"
    anything        = ".*"
    categoryRegex   = r"(\([^)]+\))"
    messageRegex    = "(.*?)$"
    match           = re.search(rf"{fileRegex}(?:{anything}{fileRegex})?{anything}{categoryRegex}\s{messageRegex}", line)
    if match is None or len(match.groups()) != 6:
        print(f"Failed to regex search string: {line}")
        return

    searchResults   = match.groups()

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
    suppressedMessages = []
    warningCount = 0

    with open(fileName) as file:
        for line in file:
            if isSuppressed(line):
                suppressedMessages.append(line)
            else:
                printWarning(line)
                warningCount += 1

    print(f"Warnings: {warningCount}, suppressed messages: {len(suppressedMessages)}")
    print("Suppressed messages:")
    for suppressedMessage in suppressedMessages:
        print(suppressedMessage, end='')

    # Succeed if the lint report is empty, or only contains the information line
    if warningCount > 0:
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
