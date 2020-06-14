import os, sys

# Assuming that the script is being called from the project's root folder
shadersFolder = "GameProject\\Engine\\Rendering\\Shaders\\"

def findGLSLCPath():
    driveLetters    = ['C', 'D']
    vulkanVersion   = "1.2.135.0"

    for driveLetter in driveLetters:
        path = "{}:\\VulkanSDK\\{}\\Bin\\glslc.exe".format(driveLetter, vulkanVersion)
        if os.path.exists(path):
            return path

    print("Failed to find glslc.exe in '<drive letter>:\\VulkanSDK\\{}\\Bin\\glslc.exe'. Is the installed version not {}?".format(vulkanVersion, vulkanVersion))
    sys.exit(1)

def listGLSLShaders():
    allShaders  = os.listdir(shadersFolder)
    glslShaders = []

    for shader in allShaders:
        if ".hlsl" in shader:
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

        os.system("{} -O -fshader-stage={} {}{} -o {}".format(glslcPath, stage, shadersFolder, shader, shader.replace(".glsl",  ".spv")))

    print("Compiled {} shaders".format(len(shaders)))

def main():
    glslcPath = findGLSLCPath()

    if not os.path.exists(shadersFolder):
        print("Could not find shaders folder: {}".format(shadersFolder))
        return 1

    compileShaders(glslcPath)
    return 0

if __name__ == "__main__":
    main()
