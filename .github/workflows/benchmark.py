import json, os, subprocess, sys, getopt

repoOwner   = 'TheoBerlin'
repo        = 'SoloGame'

benchmarkFileName       = 'benchmark_results'
benchmarkFileNameVulkan = benchmarkFileName + '_vk'
benchmarkFileNameDX11   = benchmarkFileName + '_dx11'

def remove_existing_benchmark_files():
    for fileName in [benchmarkFileName, benchmarkFileNameVulkan, benchmarkFileNameDX11]:
        if os.path.exists(fileName + '.json'):
            os.remove(fileName + '.json')

def set_rendering_api(API):
    with open('engine_config.json', 'r+') as cfgFile:
        config = json.load(cfgFile)
        config['API'] = API
        cfgFile.seek(0)
        json.dump(config, cfgFile, indent=4)
        cfgFile.truncate()
        cfgFile.close()

def run_benchmark(binPath, API):
    set_rendering_api(API)
    print(f'Benchmarking using {API}... ', end='', flush=True)
    completedProcess = subprocess.run([binPath, '--benchmark'], capture_output=True)
    if completedProcess.returncode != 0:
        print(f'Failed:\n{completedProcess.stdout}\n\n{completedProcess.stderr}')
        sys.exit(1)

    print(' Success')

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

def update_charts(commitID):
    # The files benchmark_results_vk.json and ...dx.json have been created.
    # Use their data to update the charts in /docs
    print('Updating charts in docs/')

    with open(benchmarkFileNameVulkan + '.json', 'r') as benchmarkFile:
        vkResults = json.load(benchmarkFile)
        benchmarkFile.close()

    with open(benchmarkFileNameDX11 + '.json', 'r') as benchmarkFile:
        dx11Results = json.load(benchmarkFile)
        benchmarkFile.close()

    with open('docs/_data/charts.json', 'r+') as chartsFile:
        chartsData = json.load(chartsFile)
        update_average_fps_chart(chartsData['AverageFPS'], vkResults, dx11Results, commitID)
        chartsFile.seek(0)
        json.dump(chartsData, chartsFile, indent=4)
        chartsFile.truncate()
        chartsFile.close()

def main(argv):
    commitID = '8ed033477cd8e94bd9d10bfccc67efaf3b4db3f5'
    #commitID = os.environ.get('GITHUB_SHA')
    if not commitID:
        print('Failed to retireve commit ID through GITHUB_SHA environment variable')
        sys.exit(1)

    helpStr = '''usage: <binpath>\n
        binpath: path to application binary to benchmark'''
    try:
        opts, args = getopt.getopt(argv, 'h', ['help'])
    except getopt.GetoptError:
        print('Intended usage:')
        print(helpStr)
        print(f'Used flags: {str(args)}')
        sys.exit(2)
    if 'h' in opts or 'help' in opts or len(args) == 0:
            print(helpStr)
            sys.exit(1)

    binPath = args[0]

    remove_existing_benchmark_files()

    run_benchmark(binPath, 'DirectX 11')
    os.rename(benchmarkFileName + '.json', benchmarkFileNameDX11 + '.json')

    run_benchmark(binPath, 'Vulkan')
    os.rename(benchmarkFileName + '.json', benchmarkFileNameVulkan + '.json')

    update_charts(commitID)

if __name__ == '__main__':
    main(sys.argv[1:])
