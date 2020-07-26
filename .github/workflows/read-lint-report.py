import sys, getopt

# checkGroup could be 'warning, inconclusive'. Remove the trailing 'inconclusive'
# checkGroup = checkGroup.split(',')[0]
# checkColor = colorMapping[checkGroup]
# checkGroup = re.search("\[(.*?)\]: \((.*?)\)", line).group(2)

def readReport(fileName):
    containsInfoLine = False

    with open(fileName) as file:
        lineCount = 0
        for line in file:
            lineCount += 1
            if line.startswith("(information)"):
                containsInfoLine = True
            print(f"::warning ::{line}")

    # Fail if the lint report is not empty
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
