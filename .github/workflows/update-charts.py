import json, os, subprocess, sys, getopt

repoOwner   = 'TheoBerlin'
repo        = 'SoloGame'

benchmarkFileName = 'benchmark_results.json'

def print_help(helpString, args):
    print('Intended usage:')
    print(helpString)
    print(f'Used flags: {str(args)}')

def get_commit_info(commitID):
    URL = f'https://api.github.com/repos/{repoOwner}/{repo}/commits/{commitID}'

    import requests
    resp = requests.get(URL)
    if resp.status_code != 200:
        print(f'Error: {URL} returned {resp.status_code}')
        sys.exit(1)
    return resp.json()

def update_average_fps_chart(chartData, vkResults, dx11Results, commitID):
    commitInfo = get_commit_info(commitID)
    commitMsg = commitInfo['commit']['message']
    timestamp = commitInfo['commit']['author']['date']

    chartData['vulkan'].append(vkResults['AverageFPS'])
    chartData['directx11'].append(dx11Results['AverageFPS'])
    chartData['labels'].append(commitID[:7])
    chartData['tooltips'].append(f'{commitMsg}\n{timestamp}')

def update_charts(commitID, dataPath, vkResultsPath, dx11ResultsPath):
    print('Updating charts in docs/')

    with open(vkResultsPath, 'r') as benchmarkFile:
        vkResults = json.load(benchmarkFile)
        benchmarkFile.close()

    with open(dx11ResultsPath, 'r') as benchmarkFile:
        dx11Results = json.load(benchmarkFile)
        benchmarkFile.close()

    with open(f'{dataPath}/charts.json', 'r+') as chartsFile:
        chartsData = json.load(chartsFile)
        update_average_fps_chart(chartsData['AverageFPS'], vkResults, dx11Results, commitID)
        chartsFile.seek(0)
        json.dump(chartsData, chartsFile, indent=4)
        chartsFile.truncate()
        chartsFile.close()

def run_git_command(args):
    completedProcess = subprocess.run(args, capture_output=True)
    if completedProcess.returncode != 0:
        print(f"Failed:\n{completedProcess.stderr}")
        return 1

    print(" Success")
    return 0

def commit_changes(dataPath):
    args = ['git', 'add', f'{dataPath}/*']

    run_git_command(args)
    args = ['git', 'commit', '-m', '\"Update charts data\"']

    run_git_command(args)
    args = ['git', 'push', '-u', 'origin', 'master']

def main(argv):
    commitID = '8ed033477cd8e94bd9d10bfccc67efaf3b4db3f5'
    #commitID = os.environ.get('GITHUB_SHA')
    if not commitID:
        print('Failed to retireve commit ID through GITHUB_SHA environment variable')
        sys.exit(1)

    helpStr = '''usage: --data <path_to_pages_data> --vk <path_to_vk_results.json> --dx11 <path_to_dx11_results.json> --pat <PAT_TOKEN>\n
        data: relative path to _data directory inside the pages repo\n
        vk: path to .JSON file to retrieve Vulkan benchmarks results from\n
        dx11: path to .JSON file to retrieve DirectX 11 benchmarks results from
        pat: Personal Access Token with rights to write to the GitHub Pages repo'''
    try:
        opts, args = getopt.getopt(argv, 'h', ['help', 'data=', 'vk=', 'dx11=', 'pat='])
    except getopt.GetoptError:
        print_help(helpStr, args)
        sys.exit(1)

    dataPath        = None
    vkResultsPath   = None
    dx11ResultsPath = None
    pat_token       = None

    for opt, arg in opts:
        if opt in ['-h', '--help']:
            print_help(helpStr, args)
            sys.exit(1)
        if opt == '--data':
            dataPath = arg
        elif opt == '--vk':
            vkResultsPath = arg
        elif opt == '--dx11':
            dx11ResultsPath = arg
        elif opt == '--pat':
            pat_token = arg

    if not dataPath or not vkResultsPath or not dx11ResultsPath or not pat_token:
        print('Missing argument(s)')
        print_help(helpStr, args)
        sys.exit(1)

    if dataPath.endswith('/'):
        dataPath = dataPath[:-1]

    update_charts(commitID, dataPath, vkResultsPath, dx11ResultsPath)
    commit_changes(dataPath)

if __name__ == '__main__':
    main(sys.argv[1:])
