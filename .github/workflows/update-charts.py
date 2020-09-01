import json, os, subprocess, sys, getopt

REPO_OWNER   = 'TheoBerlin'
REPO_NAME    = 'SoloGamePages'
REPO_DIR     = 'docs'

def print_help(helpString, args):
    print('Intended usage:')
    print(helpString)
    print(f'Used flags: {str(args)}')

def pull_pages_repo(repoURL):
    subprocess.run(f'git clone {repoURL} {REPO_DIR}', shell=True, check=True)

# Gets information regarding a commit in the game repository
def get_commit_info(commitID):
    # GITHUB_REPOSITORY: repoOwner/repoName
    gameRepoInfo = os.environ.get('GITHUB_REPOSITORY')
    if not gameRepoInfo:
        print('Missing environment variable: GITHUB_REPOSITORY')
        sys.exit(1)
    URL = f'https://api.github.com/repos/{gameRepoInfo}/commits/{commitID}'

    import requests
    resp = requests.get(URL)
    if resp.status_code != 200:
        print(f'Error: {URL} returned {resp.status_code}')
        sys.exit(1)
    return resp.json()

def update_commit_data(chartData, commitID):
    commitInfo = get_commit_info(commitID)
    commitMessage = commitInfo['commit']['message'].replace('\n\n', '\n')
    chartData['commits'][commitID] = {
        'message': commitMessage,
        'timestamp': commitInfo['commit']['author']['date']
    }

def update_average_fps_chart(chartData, vkResults, dx11Results, commitID):
    chartData['commitIDs'].append(commitID)
    chartData['vulkan'].append(vkResults['AverageFPS'])
    chartData['directx11'].append(dx11Results['AverageFPS'])

def update_peak_memory_usage_chart(chartData, vkResults, dx11Results, commitID):
    chartData['commitIDs'].append(commitID)
    chartData['vulkan'].append(vkResults['PeakMemoryUsage'])
    chartData['directx11'].append(dx11Results['PeakMemoryUsage'])

def update_charts(commitID, vkResultsPath, dx11ResultsPath, repoDir):
    print(f'Updating charts in {repoDir}/_data/')

    with open(vkResultsPath, 'r') as benchmarkFile:
        vkResults = json.load(benchmarkFile)
        benchmarkFile.close()

    with open(dx11ResultsPath, 'r') as benchmarkFile:
        dx11Results = json.load(benchmarkFile)
        benchmarkFile.close()

    with open(f'{repoDir}/_data/charts.json', 'r+') as chartsFile:
        chartsData = json.load(chartsFile)
        update_commit_data(chartsData, commitID)
        update_average_fps_chart(chartsData['AverageFPS'], vkResults, dx11Results, commitID)
        update_peak_memory_usage_chart(chartsData['PeakMemoryUsage'], vkResults, dx11Results, commitID)
        chartsFile.seek(0)
        json.dump(chartsData, chartsFile, indent=4)
        chartsFile.truncate()
        chartsFile.close()

def commit_changes(repoURL):
    subprocess.run('git add _data/*', shell=True, check=True)
    subprocess.run('git commit -m \"Update charts data\"', shell=True, check=True)
    subprocess.run(f'git remote set-url origin {repoURL}', shell=True, check=True)
    subprocess.run(f'git push {repoURL}', shell=True, check=True)

def main(argv):
    commitID = os.environ.get('GITHUB_SHA')
    if not commitID:
        print('Failed to retireve commit ID through GITHUB_SHA environment variable')
        sys.exit(1)

    helpStr = '''usage: --data <path_to_pages_data> --vk <path_to_vk_results.json> --dx11 <path_to_dx11_results.json>\n
        vk: path to .JSON file to retrieve Vulkan benchmarks results from\n
        dx11: path to .JSON file to retrieve DirectX 11 benchmarks results from'''
    try:
        opts, args = getopt.getopt(argv, 'h', ['help', 'vk=', 'dx11='])
    except getopt.GetoptError:
        print_help(helpStr, args)
        sys.exit(1)

    vkResultsPath   = None
    dx11ResultsPath = None
    pat             = os.environ.get('PAT') # Personal Access Token for executing authenticated git commands

    for opt, arg in opts:
        if opt in ['-h', '--help']:
            print_help(helpStr, args)
            sys.exit(1)
        elif opt == '--vk':
            vkResultsPath = arg
        elif opt == '--dx11':
            dx11ResultsPath = arg

    if not vkResultsPath or not dx11ResultsPath or not pat:
        print('Missing argument(s)')
        print_help(helpStr, args)
        sys.exit(1)

    repoURL = f'https://{pat}:x-oauth-basic@github.com/{REPO_OWNER}/{REPO_NAME}.git'
    pull_pages_repo(repoURL)
    update_charts(commitID[:7], vkResultsPath, dx11ResultsPath, REPO_DIR)
    os.chdir(REPO_DIR)
    commit_changes(repoURL)

if __name__ == '__main__':
    main(sys.argv[1:])
