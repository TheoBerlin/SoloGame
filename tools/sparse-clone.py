import os, sys, getopt, subprocess

def run_git_command(args):
    completedProcess = subprocess.run(args, capture_output=True)
    if completedProcess.returncode != 0:
        print(f"Failed:\n{completedProcess.stderr}")
        return 1

    print(" Success")
    return 0

def sparse_clone(repoURL, localDir, branch, directories):
    os.makedirs(localDir)
    os.chdir(localDir)
    print(f"Created directory {localDir}")

    print(f"Cloning git repo...", end='', flush=True)
    if run_git_command(['git', 'clone', repoURL, '--no-checkout', '--depth', '1', '.']) != 0:
        return 1
    print(" Success")

    print(f"Initializing sparse checkout...", end='', flush=True)
    if run_git_command(['git', 'sparse-checkout', 'init']) != 0:
        return 1

    # Loops over remaining args
    print(f"Adding subdirectories/files... ", end='', flush=True)
    if run_git_command(['git', 'sparse-checkout', 'set', ' '.join(directories)]) != 0:
        return 1

    print(f"Checking out to {branch}... ", end='', flush=True)
    if run_git_command(['git', 'checkout', branch]) != 0:
        return 1

    return 0

def main(argv):
    helpStr = '''Usage: sparse-clone.py --branch <branch> <repoURL> <localDir> <directories/files...>\n
        example: python sparse-clone.py https://github.com/Peter/CoolLibrary vendor/CoolLibrary/ PeterLib/scripts/ PeterLib/singleHeader2.hpp\n
        -b or --branch: branch to fetch from. Optional, default: master'''

    branch = "master"

    try:
        opts, args = getopt.getopt(argv, "hb:", ["branch="])
    except getopt.GetoptError:
        print("Used flags: " + str(args))
        print(helpStr)
        sys.exit(2)
    print(opts)
    print(args)
    if "-h" in opts or "--help" in opts or len(args) < 3:
        print(helpStr)
        sys.exit(1)
    for opt, arg in opts:
        if opt in ["-b", "--branch"]:
            branch = arg

    return sparse_clone(args[0], args[1], branch, args[2:])

if __name__ == "__main__":
    main(sys.argv[1:])
