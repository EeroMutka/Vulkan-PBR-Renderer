#include "common.h"
#include "render.h"
#include "asset_import.h"

#define FIRE_OS_WINDOW_IMPLEMENTATION
#include "src/fire/fire_os_window.h"
#include "src/utils/key_input/key_input_fire_os.h"

DS_Arena* TEMP; // Arena for per-frame, temporary allocations

int main() {
	// -- World state -----

	Camera camera = {};
	camera.pos = {0.f, 5.f, 5.f};
	
	HMM_Vec2 sun_angle = {56.5f, 97.f};

	// --------------------

	DS_Arena frame_temp_arena;
	DS_ArenaInit(&frame_temp_arena, 4096, DS_HEAP);
	TEMP = &frame_temp_arena;

	const uint32_t window_width = 1920;
	const uint32_t window_height = 1080;

	OS_Window window = OS_CreateWindow(window_width, window_height, "PBR Renderer");
	OS_SetWindowFullscreen(&window, true);

	GPU_Init(window.handle);

	Renderer renderer = {};
	InitRenderer(&renderer, window_width, window_height);
	
	RenderObject world = LoadMesh(&renderer, "../resources/SunTemple/SunTemple.fbx", {0.f, 25.f, 0.f});
	RenderObject skybox = LoadMesh(&renderer, "../resources/Skybox_200x200x200.fbx", {});
	GPU_Texture* tex_env_cube = MakeTextureFromHDRIFile("../resources/shipyard_cranes_track_cube.hdr");

	GPU_Graph* graphs[2];
	int graph_idx = 0;
	GPU_MakeSwapchainGraphs(2, &graphs[0]);
	
	Input::Frame inputs = {};
	
	while (!OS_WindowShouldClose(&window)) {
		DS_ArenaReset(&frame_temp_arena);

		Input::ResetFrame(&inputs, &frame_temp_arena);

		OS_Event event;
		while (OS_PollEvent(&window, &event, NULL, NULL)) {
			Input::OS_AddEvent(&inputs, event);
		}
		if (inputs.KeyIsDown(Input::Key::Escape)) break;

		if (inputs.KeyIsDown(Input::Key::_9)) sun_angle.X -= 0.5f;
		if (inputs.KeyIsDown(Input::Key::_0)) sun_angle.X += 0.5f;
		if (inputs.KeyIsDown(Input::Key::_8)) sun_angle.Y -= 0.5f;
		if (inputs.KeyIsDown(Input::Key::_7)) sun_angle.Y += 0.5f;
		
		HotreloadShaders(&renderer, tex_env_cube);
		
		float movement_speed = 0.05f;
		float mouse_speed = 0.001f;
		float FOV = 75.f;
		float z_near = 0.02f;
		float z_far = 10000.f;
		UpdateCamera(&camera, inputs, movement_speed, mouse_speed, FOV, (float)window_width / (float)window_height, z_near, z_far);
		
		graph_idx = (graph_idx + 1) % 2;
		GPU_Graph* graph = graphs[graph_idx];
		GPU_GraphWait(graph);

		GPU_Texture* backbuffer = GPU_GetBackbuffer(graph);
		if (backbuffer) {
			BuildRenderCommands(&renderer, graph, backbuffer, &world, &skybox, camera, sun_angle);
			GPU_GraphSubmit(graph);
		}
	}

	GPU_DestroyGraph(graphs[0]);
	GPU_DestroyGraph(graphs[1]);

	UnloadMesh(&world);
	UnloadMesh(&skybox);
	GPU_DestroyTexture(tex_env_cube);

	DeinitRenderer(&renderer);

	GPU_Deinit();

	return 0;
}