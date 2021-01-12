import json, os, subprocess, sys, getopt

BENCHMARK_FILE_NAME = 'benchmark_results.json'

PRESENTATION_MODE   = 'immediate'

def print_help(helpString, args):
    print('Intended usage:')
    print(helpString)
    print(f'Used flags: {str(args)}')

def remove_existing_benchmark_files(vkResultsPath, dx11ResultsPath):
    for fileName in [BENCHMARK_FILE_NAME, vkResultsPath, dx11ResultsPath]:
        if os.path.exists(fileName):
            os.remove(fileName)

def set_engine_config(API):
    with open('engine_config.json', 'r+') as cfgFile:
        config = json.load(cfgFile)
        config['API'] = API
        config['PresentationMode'] = PRESENTATION_MODE
        cfgFile.seek(0)
        json.dump(config, cfgFile, indent=4)
        cfgFile.truncate()
        cfgFile.close()

def run_benchmark(binPath, API):
    set_engine_config(API)
    print(f'Benchmarking using {API}... ', end='', flush=True)
    completedProcess = subprocess.run([binPath, '--benchmark'], capture_output=True)
    if completedProcess.returncode != 0:
        print(f'Failed:\n{completedProcess.stdout}\n\n{completedProcess.stderr}')
        sys.exit(1)

    print(' Success')

def main(argv):
    helpStr = '''usage: --bin <binpath> --vk <name_of_vk_results.json> --dx11 <name_of_dx11_results.json>\n
        bin: path to application binary to benchmark\n
        vk: name of .JSON file to create and store Vulkan benchmarks results in\n
        dx11: name of .JSON file to create and store DirectX 11 benchmarks results in'''
    try:
        opts, args = getopt.getopt(argv, 'h', ['help', 'bin=', 'vk=', 'dx11='])
    except getopt.GetoptError:
        print_help(helpStr, args)
        sys.exit(1)

    for opt, arg in opts:
        if opt in ['-h', '--help']:
            print_help(helpStr, args)
            sys.exit(1)
        if opt == '--bin':
            binPath = arg
        elif opt == '--vk':
            vkResultsPath = arg
        elif opt == '--dx11':
            dx11ResultsPath = arg

    if not binPath or not vkResultsPath or not dx11ResultsPath:
        print('Missing argument')
        print_help(helpStr, args)
        sys.exit(1)

    remove_existing_benchmark_files(vkResultsPath, dx11ResultsPath)

    run_benchmark(binPath, 'DirectX 11')
    os.rename(BENCHMARK_FILE_NAME, dx11ResultsPath)

    run_benchmark(binPath, 'Vulkan')
    os.rename(BENCHMARK_FILE_NAME, vkResultsPath)

if __name__ == '__main__':
    main(sys.argv[1:])
