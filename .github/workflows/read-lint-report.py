import csv, re, sys, getopt

# Regexes that will catch messages to be suppressed
suppressedRegexes = [
    "Class '.*' has a constructor with 1 argument that is not explicit",
    "Consider using std::transform",
    "The function '.*' is never used",
    "Class '.*' does not have a .* which is recommended since it has dynamic memory",
    "Member variable '.*' is in the wrong place in the initializer list",
    r"^\(information\)"
]

def isSuppressed(line):
    for suppressedRegex in suppressedRegexes:
        if re.search(suppressedRegex, line) is not None:
            return True
    return False

# Returns true if a warning was printed
def printWarning(line, filesToLint):
    fileRegex       = r"\[([^:]+):([0-9]+)\]"
    anything        = ".*"
    categoryRegex   = r"(\([^)]+\))"
    messageRegex    = "(.*?)$"
    match           = re.search(rf"{fileRegex}(?:{anything}{fileRegex})?{anything}{categoryRegex}\s{messageRegex}", line)
    if match is None or len(match.groups()) != 6:
        print(f"Failed to regex search string: {line}")
        return False

    searchResults   = match.groups()

    firstFileName   = searchResults[0]
    firstLineNr     = searchResults[1]
    lintCategory    = searchResults[4]
    message         = searchResults[5]
    secondFileName  = ""
    secondLineNr    = ""

    outMessage = ""
    if searchResults[2] is not None and searchResults[3] is not None:
        # The line contains two files. Have the second one be printed in the output message
        secondFileName  = searchResults[2]
        secondLineNr    = searchResults[3]
        outMessage += f"[{secondFileName}:{secondLineNr}]: "

    if firstFileName not in filesToLint and secondFileName not in filesToLint:
        return False

    outMessage += f"{lintCategory} {message}"
    print(f"::warning file={firstFileName},line={firstLineNr}::{outMessage}")
    return True

def readReport(fileName, filesToLint):
    suppressedMessages = []
    warningCount = 0

    with open(fileName) as file:
        for line in file:
            if isSuppressed(line):
                suppressedMessages.append(line)
            elif printWarning(line, filesToLint):
                warningCount += 1

    print(f"Warnings: {warningCount}, suppressed messages: {len(suppressedMessages)}")
    print("Suppressed messages:")
    for suppressedMessage in suppressedMessages:
        print(suppressedMessage, end='')

    # Succeed if the lint report is empty, or only contains the information line
    return 0 if warningCount == 0 else 1

def getCSVValues(file):
    values = []

    with open(file) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        line_count = 0
        for row in csv_reader:
            if line_count == 0:
                print(f'Column names are {", ".join(row)}')
                line_count += 1
            else:
                values += row
                line_count += 1
    return values

def getFilesToLint(modifiedFilesPath, addedFilesPath):
    modifiedFiles = getCSVValues(modifiedFilesPath) if modifiedFilesPath != "" else []
    addedFiles = getCSVValues(addedFilesPath) if addedFilesPath != "" else []
    return modifiedFiles + addedFiles

def main(argv):
    inputFile = ""
    modifiedFilesPath = ""
    addedFilesPath = ""
    helpStr = '''read-lint-report.py --report <path> --modified-files <path> --added-files <path>\n
        modified-files: path to csv file containing file paths of files modified in a merge request
        added-files: path to csv file containing file paths of files added in a merge request'''
    try:
        opts, args = getopt.getopt(argv,"h:",["report=modified-files=added-files="])
        (args) # Hack to remove lint error of args not being used
    except getopt.GetoptError:
        print("Intended usage:")
        print(helpStr)
        print("Used flags: " + str(argv))
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print(helpStr)
            sys.exit(1)
        elif opt == "report":
            inputFile = arg
        elif opt == "modified-files":
            modifiedFilesPath = arg
        elif opt == "added-files":
            addedFilesPath = arg

    filesToLint = getFilesToLint(modifiedFilesPath, addedFilesPath)
    sys.exit(readReport(inputFile, filesToLint))

if __name__ == "__main__":
    main(sys.argv[1:])
