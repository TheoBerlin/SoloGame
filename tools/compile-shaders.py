import os, sys

# Assuming that the script is being called from the project's root folder
shadersFolder = "assets\\Engine\\Shaders\\"

def findGLSLCPath():
    driveLetters    = ['C', 'D']
    vulkanVersion   = "1.2.135.0"

    for driveLetter in driveLetters:
        path = f"{driveLetter}:\\VulkanSDK\\{vulkanVersion}\\Bin\\glslc.exe"
        if os.path.exists(path):
            return path

    print(f"Failed to find glslc.exe in '<drive letter>:\\VulkanSDK\\{vulkanVersion}\\Bin\\glslc.exe'. Is the installed version not {vulkanVersion}?")
    sys.exit(1)

def listGLSLShaders():
    allShaders  = os.listdir(shadersFolder)
    glslShaders = []

    for shader in allShaders:
        if ".glsl" in shader:
            glslShaders.append(shader)

    return glslShaders

def compileShaders(glslcPath):
    shaders = listGLSLShaders()

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

        os.system(f"{glslcPath} -O -fshader-stage={stage} {shadersFolder}{shader} -o {shadersFolder}{shader.replace('.glsl', '.spv')}")

    print(f"Compiled {len(shaders)} shaders")

def main():
    glslcPath = findGLSLCPath()

    if not os.path.exists(shadersFolder):
        print(f"Could not find shaders folder: {shadersFolder}")
        return 1

    compileShaders(glslcPath)
    return 0

if __name__ == "__main__":
    main()
