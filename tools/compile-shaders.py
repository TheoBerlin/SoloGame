import os, sys

# Assuming that the script is being called from the project's root folder
SHADERS_FOLDER = "assets\\Engine\\Shaders\\"

def find_GLSLC_path():
    sdkPathVars         = ['VK_SDK_PATH', 'VULKAN_SDK']
    sdkPathVarExists    = False
    sdkPath             = None
    for sdkPathVar in sdkPathVars:
        sdkPath = os.environ.get(sdkPathVar)
        if sdkPath:
            glslcPath = f'{sdkPath}\\Bin\\glslc.exe'
            if os.path.exists(glslcPath):
                return glslcPath
            else:
                print(f'Bin subdirectory in Vulkan SDK [{sdkPath}] pointed at by environment variable {sdkPathVar} does not contain glslc.exe')

    if not sdkPathVarExists:
        print(f'No environment variables for path to Vulkan SDK are set: {sdkPathVars}')

    print(f'Could not find glslc.exe')
    sys.exit(1)

def list_GLSL_Shaders():
    allShaders  = os.listdir(SHADERS_FOLDER)
    glslShaders = []

    for shader in allShaders:
        if ".glsl" in shader:
            glslShaders.append(shader)

    return glslShaders

def compile_shaders(glslcPath):
    shaders = list_GLSL_Shaders()

    for shader in shaders:
        stage = ""
        if "_vs" in shader:
            stage = "vertex"
        elif "_hs" in shader:
            stage = "tesscontrol"
        elif "_ds" in shader:
            stage = "tesseval"
        elif "_gs" in shader:
            stage = "geometry"
        elif "_fs" in shader:
            stage = "fragment"
        elif "_cs" in shader:
            stage = "compute"
        else:
            print(f"Failed to recognize shader stage by file name: {shader}")
            continue

        os.system(f"{glslcPath} -O -fshader-stage={stage} {SHADERS_FOLDER}{shader} -o {SHADERS_FOLDER}{shader.replace('.glsl', '.spv')}")

    print(f"Compiled {len(shaders)} shaders")

def main():
    glslcPath = find_GLSLC_path()

    if not os.path.exists(SHADERS_FOLDER):
        print(f"Could not find shaders folder: {SHADERS_FOLDER}")
        return 1

    compile_shaders(glslcPath)
    return 0

if __name__ == "__main__":
    main()
