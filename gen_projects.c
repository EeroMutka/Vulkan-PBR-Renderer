#include "src/Fire/fire_build.h"
#define ArrCount(X) sizeof(X)/sizeof(X[0])

static void AddVulkanSDK(BUILD_Project* p, const char* vk_sdk) {
	BUILD_AddIncludeDir(p, BUILD_Concat2(vk_sdk, "/Include"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/vulkan-1.lib"));
	
	// If compiling with the GLSLang shader compiler, the following are required
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/GenericCodeGen.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/glslang.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/MachineIndependent.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/OGLCompiler.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/OSDependent.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/SPIRV.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/SPIRV-Tools.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/SPIRV-Tools-diff.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/SPIRV-Tools-link.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/SPIRV-Tools-lint.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/SPIRV-Tools-opt.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/SPIRV-Tools-reduce.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/SPIRV-Tools-shared.lib"));
	BUILD_AddLinkerInput(p, BUILD_Concat2(vk_sdk, "/Lib/glslang-default-resource-limits.lib"));
}

int main() {
	const char* vk_sdk = getenv("VULKAN_SDK");
	if (vk_sdk == NULL) {
		printf("ERROR: Vulkan SDK not found (\"VULKAN_SDK\" environment variable is missing).\n");
		return 0;
	}
	
	BUILD_ProjectOptions opts = {
		.debug_info = true,
		.enable_optimizations = false,
		.c_runtime_library_dll = true, // glslang.lib uses /MD
	};
	
	// ---------------------------------------------------------
	
	BUILD_Project triangle;
	BUILD_ProjectInit(&triangle, "triangle", &opts);
	BUILD_AddIncludeDir(&triangle, "../"); // repository root dir
	BUILD_AddSourceFile(&triangle, "../src/demo_triangle/triangle.cpp");
	AddVulkanSDK(&triangle, vk_sdk);
	
	// ---------------------------------------------------------
	
	BUILD_Project simple_pbr;
	BUILD_ProjectInit(&simple_pbr, "simple_pbr", &opts);
	BUILD_AddIncludeDir(&simple_pbr, "../"); // repository root dir
	BUILD_AddSourceFile(&simple_pbr, "../src/demo_simple_pbr/simple_pbr.cpp");
	AddVulkanSDK(&simple_pbr, vk_sdk);

	// ---------------------------------------------------------
	
	BUILD_Project gi_pbr;
	BUILD_ProjectInit(&gi_pbr, "global_illumination_pbr", &opts);
	BUILD_AddIncludeDir(&gi_pbr, "../"); // repository root dir
	BUILD_AddIncludeDir(&gi_pbr, "../third_party"); // required for including <assimp/..>
	BUILD_AddSourceFile(&gi_pbr, "../src/demo_global_illumination_pbr/main.cpp");
	BUILD_AddSourceFile(&gi_pbr, "../src/demo_global_illumination_pbr/common.cpp");
	BUILD_AddSourceFile(&gi_pbr, "../src/demo_global_illumination_pbr/os_utils.cpp");
	BUILD_AddSourceFile(&gi_pbr, "../src/demo_global_illumination_pbr/render.cpp");
	BUILD_AddSourceFile(&gi_pbr, "../src/demo_global_illumination_pbr/asset_import.cpp");
	AddVulkanSDK(&gi_pbr, vk_sdk);
	
	BUILD_AddSourceFile(&gi_pbr, "../src/gpu/gpu_vulkan.c");
	
	// include assimp
	BUILD_AddLinkerInput(&gi_pbr, "../third_party/assimp/lib/assimp-vc143-mt.lib");
	BUILD_AddExtraCompilerArg(&gi_pbr, "/wd\"4251\""); // disable a warning generated by assimp
	
	BUILD_AddLinkerInput(&gi_pbr, "User32.lib"); // required for vulkan usage (imp_GetWindowLongPtrW)
	
	// ---------------------------------------------------------
	
	BUILD_Project* projects[] = {&triangle, &simple_pbr, &gi_pbr};
	BUILD_CreateDirectory("build");
	
	// Copy Assimp dll to the build directory
	BUILD_CopyFile("third_party/assimp/lib/assimp-vc143-mt.dll", "build/assimp-vc143-mt.dll");
	
	if (BUILD_CreateVisualStudioSolution("build", ".", "demos.sln", projects, ArrCount(projects), BUILD_GetConsole())) {
		printf("Project files were generated successfully! See the \"build\" folder.\n");
	}
	else {
		printf("ERROR: failed to generate project files.\n");
	}
	
	return 0;
}
