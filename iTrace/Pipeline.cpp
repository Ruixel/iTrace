#include "Pipeline.h"
#include "ItemRenderer.h"
#include "BooleanCommands.h"
#include "CinematicCamera.h"
#include <iostream>


#ifdef _WIN32 or _WIN64

#include <Windows.h>

#endif
namespace iTrace {

	std::vector<std::string> Echo(std::vector<std::string> Input) {

		if (Input[0] == ">help") {

			return { "echo command. simply prints whatever comes after it (unless the first word is >help, which brings up this)" };

		}


		std::string Out = "";

		for (auto& in : Input) {
			Out += in + " ";
		}

		return { Out };

	}

	std::vector<std::string> Fill(std::vector<std::string> Input) {

		//Command: setblock 
		//Usage: 

		//setblock (.)x (.)y (.)z [type]
		//. = relative to the camera 
		//type is the NAME of the block

		//Other uses: 
		//setblock >help
		//prints information about the setblock command 

		//setblock >example
		//prints an example for a setblock command 

		if (Input.size() == 0) {
			return { "Setblock command failed, not enough data provided for command to execute" };
		}

		if (Input[0] == ">help") {

			return {
				"setblock (.)x (.)y (.z) type",
				". = relative to the camera",
				"type is the name of the block (use blocklist for list of blocks)",
				"Other uses: ",
				"setblock >example",
				"prints an example of a setblock command"
			};

		}
		else if (Input[0] == ">example") {

			return {
				"setblock . . . grass",
				"adds a grass block at the players position",
				"(players position is based on their feet, not their head)"
			};

		}

		if (Input.size() < 4) {
			return { "Setblock command failed, not enough data provided for command to execute" };
		}

		Vector3i BlockCoordinate = Vector3i(0);
		bool Relative[3] = { false, false, false };


		std::string CoordinateNames = "xyz";

		auto SafeParseInt = [](std::string Text, int Offset) {

			std::string NewString;
			if (Offset == 0)
				NewString = Text;
			else {
				for (int x = Offset; x < Text.size(); x++) {
					NewString += Text[x];

				}
			}

			bool IsNumber = true;

			for (auto& Character : NewString) {
				if (!std::isdigit(Character) && !(Character == '-' && NewString[0] == '-')) {
					IsNumber = false;
					break;
				}
			}

			if (!IsNumber) {
				return -2147483647;
			}
			else {
				return std::stoi(NewString);
			}


		};

		Vector3i CameraPosition = Vector3i(std::any_cast<Vector3f>(GetGlobalCommandPusher().GivenConstantData["camera_pos"]));
		CameraPosition.y -= 2;
		for (int Coordinate = 0; Coordinate < 3; Coordinate++) {

			if (Input[Coordinate][0] == '.') {
				Relative[Coordinate] = true;
				if (Input[Coordinate].size() == 1)
					BlockCoordinate[Coordinate] = CameraPosition[Coordinate];
				else {
					BlockCoordinate[Coordinate] = CameraPosition[Coordinate] + SafeParseInt(Input[Coordinate], 1);
				}


			}
			else {
				BlockCoordinate[Coordinate] = SafeParseInt(Input[Coordinate], 0);
			}

			if (BlockCoordinate[Coordinate] == -2147483647) {
				return { "failed to parse" + CoordinateNames[Coordinate] + std::string(" coordinate."), "input was: " + Input[Coordinate] };
			}

		}


		//hardcoded limits for the world right now! 

		if (BlockCoordinate.x >= 0 && BlockCoordinate.y >= 0 && BlockCoordinate.z >= 0 &&
			BlockCoordinate.z < CHUNK_SIZE && BlockCoordinate.y < CHUNK_SIZE && BlockCoordinate.z < CHUNK_SIZE) {

			int TypeIdx = -1;

			//find the appropriate type index! 

			for (int i = 0; i < Chunk::GetBlockSize(); i++) {

				auto& Block = Chunk::GetBlock(i);

				if (Block.Name == Input[3]) {

					TypeIdx = i;

				}


			}





			if (TypeIdx == -1) {
				return { "Could not find block named: " + Input[3]," (use blocklist for a list of all block types" };
			}

			GetGlobalCommandPusher().AddCommand(CommandPush{ "set_block", {BlockCoordinate, TypeIdx} });

			return { "Added block at: " + std::to_string(BlockCoordinate.x) + " " + std::to_string(BlockCoordinate.y) + " " + std::to_string(BlockCoordinate.z) };

		}
		else {
			return { "Cannot place block outside of world!" };
		}


		return {};



	}

	std::vector<std::string> SetBlock(std::vector<std::string> Input) {

		//Command: setblock 
		//Usage: 

		//setblock (.)x (.)y (.)z [type]
		//. = relative to the camera 
		//type is the NAME of the block

		//Other uses: 
		//setblock >help
		//prints information about the setblock command 

		//setblock >example
		//prints an example for a setblock command 

		if (Input.size() == 0) {
			return { "Setblock command failed, not enough data provided for command to execute" };
		}

		if (Input[0] == ">help") {

			return {
				"setblock (.)x (.)y (.z) type",
				". = relative to the camera",
				"type is the name of the block (use blocklist for list of blocks)",
				"Other uses: ",
				"setblock >example",
				"prints an example of a setblock command"
			};

		}
		else if (Input[0] == ">example") {

			return {
				"setblock . . . grass",
				"adds a grass block at the players position",
				"(players position is based on their feet, not their head)"
			};

		}

		if (Input.size() < 4) {
			return { "Setblock command failed, not enough data provided for command to execute" };
		}

		Vector3i BlockCoordinate = Vector3i(0);
		bool Relative[3] = { false, false, false };


		std::string CoordinateNames = "xyz";

		auto SafeParseInt = [](std::string Text, int Offset) {

			std::string NewString;
			if (Offset == 0)
				NewString = Text;
			else {
				for (int x = Offset; x < Text.size(); x++) {
					NewString += Text[x];

				}
			}

			bool IsNumber = true;

			for (auto& Character : NewString) {
				if (!std::isdigit(Character) && !(Character == '-' && NewString[0] == '-')) {
					IsNumber = false;
					break;
				}
			}

			if (!IsNumber) {
				return -2147483647;
			}
			else {
				return std::stoi(NewString);
			}


		};

		Vector3i CameraPosition = Vector3i(std::any_cast<Vector3f>(GetGlobalCommandPusher().GivenConstantData["camera_pos"]));
		CameraPosition.y -= 2;
		for (int Coordinate = 0; Coordinate < 3; Coordinate++) {

			if (Input[Coordinate][0] == '.') {
				Relative[Coordinate] = true;
				if (Input[Coordinate].size() == 1)
					BlockCoordinate[Coordinate] = CameraPosition[Coordinate];
				else {
					BlockCoordinate[Coordinate] = CameraPosition[Coordinate] + SafeParseInt(Input[Coordinate], 1);
				}


			}
			else {
				BlockCoordinate[Coordinate] = SafeParseInt(Input[Coordinate], 0);
			}

			if (BlockCoordinate[Coordinate] == -2147483647) {
				return { "failed to parse" + CoordinateNames[Coordinate] + std::string(" coordinate."), "input was: " + Input[Coordinate] };
			}

		}


		//hardcoded limits for the world right now! 

		if (BlockCoordinate.x >= 0 && BlockCoordinate.y >= 0 && BlockCoordinate.z >= 0 &&
			BlockCoordinate.z < CHUNK_SIZE && BlockCoordinate.y < CHUNK_SIZE && BlockCoordinate.z < CHUNK_SIZE) {

			int TypeIdx = -1;

			//find the appropriate type index! 

			for (int i = 0; i < Chunk::GetBlockSize(); i++) {

				auto& Block = Chunk::GetBlock(i);

				if (Block.Name == Input[3]) {

					TypeIdx = i;

				}


			}





			if (TypeIdx == -1) {
				return { "Could not find block named: " + Input[3]," (use blocklist for a list of all block types" };
			}

			GetGlobalCommandPusher().AddCommand(CommandPush{ "set_block", {BlockCoordinate, TypeIdx} });

			return { "Added block at: " + std::to_string(BlockCoordinate.x) + " " + std::to_string(BlockCoordinate.y) + " " + std::to_string(BlockCoordinate.z) };

		}
		else {
			return { "Cannot place block outside of world!" };
		}


		return {};



	}

	std::vector<std::string> Blocklist(std::vector<std::string> Input) {


		std::vector<Rendering::Chunk::BlockType> Blocks = {};



		for (int s = 1; s < Chunk::GetBlockSize(); s++) {
			Blocks.push_back(Chunk::GetBlock(s));
		}

		std::sort(Blocks.begin(), Blocks.end(), [](const Chunk::BlockType& a, const Chunk::BlockType& b) {

			return a.Name[0] < b.Name[0];

			});



		std::vector<std::string> OutPut = {};





		if (Input.size() > 0) {

			if (Input[0] == "-s") {
				//search function, not implemented ATM 
				return {};
			}

		}


		for (int i = 0; i < Blocks.size(); i += 8) {
			OutPut.push_back("");
			for (int j = 0; j < 8; j++) {
				if (i + j >= Blocks.size())
					break;
				OutPut.back() += Blocks[i + j].Name + (i + j == Blocks.size() - 1 ? "" : ", ");
			}
		}



		return OutPut;



	}

	SoundType ActiveSoundtype = SoundType::NONE;

	void Pipeline::PreparePipeline(Camera& Camera, Window& Window)
	{

#ifdef _WIN32 or _WIN64

		//HWND hwnd = Window.GetRawWindow()->getSystemHandle();
		//SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

#endif

		RequestBoolean("freefly", false);
		RequestBoolean("noclip", true);
		RequestBoolean("slowframes", false);
		RequestBoolean("newfiltering", true);
		RequestBoolean("bettershadows", true);
		RequestBoolean("spatialupscale", true);

		//GetGlobalCommandPusher().World = &World; 

		GetGlobalCommandPusher().GivenConstantData["camera_pos"] = Camera.Position;

		glClearColor(0.0, 0.0, 0.0, 0.0);
		Core::PrepareHaltonSequence();
		PreparePostProcess();

		World.PrepareWorldManger();
		RenderItemRequests();
		Deferred.PrepareDeferredRenderer(Window);
		Indirect.PrepareIndirectLightingHandler(Window);
		Compositor.PrepareCompositing();
		Glow.PreparePostProcess(Window, Camera);
		Sky.PrepareSkyRenderer(Window);
		Combiner.PrepareLightCombiner(Window);
		Text = TextSystem("Textures/Font.png");

		Commands.AddCommands(Command{ "echo", Echo });
		Commands.AddCommands(Command{ "blocklist", Blocklist });
		Commands.AddCommands(Command{ "setblock", SetBlock });
		Commands.AddCommands(Command{ "boolean", BooleanCommand });
		Commands.AddCommands(Command{ "keyframe", KeyFrameCommand });
		Commands.AddCommands(Command{ "animation", AnimationCommand });

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		Core::PrepareHaltonSequence();



		Inventory.PrepareInventory();

		Sounds.GenerateOpenALExtensions();
		Sounds.SetupEFX();
		Sounds.PrepareSoundBlockData();

		Sounds.LoadSound("Footsteps", "Sound/Footsteps/Grass/Step.wav");
		//Sounds.LoadSound("Music", "Sound/Creeper.wav");

		//Sounds.AddSoundInstance(SoundInstance(), "CreeperInstance", "Footsteps");
		FootSteps.PrepareFootStepManager(Sounds);
		FootSteps.SetActiveMaterial(SoundType::STONE, Sounds);

		SoundEffect Effect = SoundEffect("Forest", "Ambience", Vector3f(0.0), SoundEffectType::LOOP, true, true); 

		SoundEffects.AddSoundEffect(Effect, Sounds); 
		//SoundEffects.GetSoundEffect("Forest").SetVolume(100.0); 
	}

	void Pipeline::RunPipeline(Camera& Camera, Window& Window)
	{
		bool Running = true;
		sf::Event Event;
		sf::Clock GameClock;
		int Frame = 0;
		float T = 0.;

		bool Lines = false;
		bool ShowGUI = true;

		sf::Clock FrameRateCounter;

		int Frames = 0;

		float TimeOfDay = 0.0;

		bool Active = true;

		int Block = 0;

		int FramesPerSecond = 0;

		while (Running) {

			while (Window.GetRawWindow()->pollEvent(Event)) {
				switch (Event.type) {

				case sf::Event::MouseWheelMoved:
					Block += Event.mouseWheel.delta;



					if (Block < 0) {
						Block += Chunk::GetBlockSize();
					}
					else if (Block >= Chunk::GetBlockSize()) {
						Block -= Chunk::GetBlockSize();
					}

					Inventory.ActiveSlot = Block;

					break;

				case sf::Event::MouseButtonPressed:

					if (Event.mouseButton.button == sf::Mouse::Left) {
						World.CastBlock(Camera, Chunk::BLOCK_ACTION::BREAK, 128, 0);

						//Chunk::GetBlock(Camera, Chunk::BLOCK_ACTION::BREAK, 128, 0);
					}
					else if (Event.mouseButton.button == sf::Mouse::Right) {


						World.CastBlock(Camera, Chunk::BLOCK_ACTION::PLACE, 128, Block);

					}


					break;

				case sf::Event::KeyPressed:


					if (!Commands.PollCommands(Event)) {
						switch (Event.key.code) {
						case sf::Keyboard::Escape:
							World.Chunk->DumpToFile();
							return;
							break;

						case sf::Keyboard::P:
							Active = !Active;
							break;

						case sf::Keyboard::R:
							if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
								Indirect.ReloadIndirect(Window);
								Deferred.ReloadDeferred(Window);
								Combiner.ReloadLightCombiner(Window);
								Sky.ReloadSky();
								Glow.ReloadPostProcess(Window, Camera);
								Sounds.ReloadSounds();
							}
							break;
						case sf::Keyboard::F1:
							ShowGUI = !ShowGUI;
							Window.GetRawWindow()->setMouseCursorVisible(ShowGUI);
							break;

						case sf::Keyboard::Space:
							if (glm::abs(Camera.Acceleration.y) < 0.0001) {

								Camera.Velocity = Vector3f(0.0, 9.0, 0.0);


							}
							break;

						case sf::Keyboard::H:
							Sounds.AddSoundInstance(SoundInstance(Camera.Position, 1.0), "MusicInstance", "Music");

							break;

						}
					}
					break;

				case sf::Event::TextEntered:
					Commands.PollCommands(Event);
					break;


				}
			}


			bool UpdateWorld = false;

			while (!GetGlobalCommandPusher().Commands.empty()) {
				auto Command = GetGlobalCommandPusher().PopCommand();

				if (Command.CommandID == "set_block") {
					Vector3i Location = std::any_cast<Vector3i>(Command.CommandData[0]);
					int TypeIdx = std::any_cast<int>(Command.CommandData[1]);

					World.Chunk->SetBlock(Location.x, Location.y, Location.z, TypeIdx);
					UpdateWorld = true;
				}

			}

			if (UpdateWorld)
				World.Chunk->UpdateMeshData({ nullptr,nullptr ,nullptr ,nullptr });


			if (FrameRateCounter.getElapsedTime().asSeconds() >= 1.0) {

				std::cout << Frames << '\n';
				FramesPerSecond = Frames;
				FrameRateCounter.restart();
				Frames = 0;
			}

			//FOV management

			World.GenerateWorld(Camera);

			float ShadowMultiplier = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ? 750.0 : 200.0;

			if (!Commands.Active && Active) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
					Sky.Direction.x += .015 * Window.GetFrameTime() * ShadowMultiplier;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
					Sky.Direction.x -= .015 * Window.GetFrameTime() * ShadowMultiplier;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
					Sky.Direction.y += .015 * Window.GetFrameTime() * ShadowMultiplier;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
					Sky.Direction.y -= .015 * Window.GetFrameTime() * ShadowMultiplier;


				Sky.UpdateDirection();

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::O)) {

					Camera.fov += 45.0 * Window.GetFrameTime();
					Camera.fov = glm::clamp(Camera.fov, 10.0f, 90.0f);
					Camera.Project = glm::perspective(glm::radians(Camera.fov), float(Window.GetResolution().x) / float(Window.GetResolution().y), Camera.znear, Camera.zfar);

				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {

					Camera.fov -= 45.0 * Window.GetFrameTime();
					Camera.fov = glm::clamp(Camera.fov, 10.0f, 90.0f);
					Camera.Project = glm::perspective(glm::radians(Camera.fov), float(Window.GetResolution().x) / float(Window.GetResolution().y), Camera.znear, Camera.zfar);


				}

			}

			Frames++;
			Window.SetFrameTime(GameClock.getElapsedTime().asSeconds());
			GameClock.restart();
			T += Window.GetFrameTime();
			Frame++;
			Window.SetTimeOpened(T);
			Window.SetFrameCount(Frame);

			if (!Commands.Active)
				Camera.HandleInput(Window, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ? 10.f : 4.0f, 0.15f, Active, Active);

			GetGlobalCommandPusher().GivenConstantData["camera_pos"] = Camera.Position;
			GetGlobalCommandPusher().GivenConstantData["camera_rot"] = Camera.Rotation;
			GetGlobalCommandPusher().GivenConstantData["camera_fov"] = Camera.fov;


			if (!PollAnimation(Camera, Window)) {
				if (!GetBoolean("freefly")) {
					Camera.Acceleration = Vector3f(0.0, -25.0, 0.0);
					Camera.Velocity += Camera.Acceleration * glm::min(Window.GetFrameTime(), 0.1f);
					Camera.Position += Camera.Velocity * glm::min(Window.GetFrameTime(), 0.1f);
				}
				if (!GetBoolean("freefly") || !GetBoolean("noclip"))
					World.Chunk->ManageCollision(Camera.Position, Camera.Acceleration, Camera.Velocity);
			}

			Camera.PrevView = Camera.View;
			Camera.View = Core::ViewMatrix(Camera.Position, Camera.Rotation);
			//Camera.Project = Camera.RawProject;

			Sky.RenderSky(Window, Camera, World);

			glEnable(GL_DEPTH_TEST);


			Deferred.RenderDeferred(Window, Camera, World, Sky.Orientation);
			Indirect.RenderIndirectLighting(Window, Camera, Deferred, World, Sky);
			Combiner.CombineLighting(Window, Camera, Indirect, Deferred, Sky);
			Glow.RenderPostProcess(Window, Sky, Indirect, Deferred, Combiner);

			glBindFramebuffer(GL_FRAMEBUFFER, NULL);

			glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y);
			Compositor.DoCompositing(Camera, Deferred, Combiner, Glow);

			if (ShowGUI) {

				Text.PrepareTextDrawing();
				//Text.DisplayText("Framerate: " + std::to_string(FramesPerSecond), Window, 0.005, Vector2f(-.97, .95), Vector3f(1.0));
				//Text.DisplayText("X: " + std::to_string(Camera.Position.x) + " Y: " + std::to_string(Camera.Position.y) + " Z: " + std::to_string(Camera.Position.z), Window, 0.005, Vector2f(-.97, .9), Vector3f(1.0));
				//Text.DisplayText("Type: " + Chunk::GetBlock(Block).Name, Window, 0.005, Vector2f(-.97, .85), Vector3f(1.0));
				Commands.DrawCommandText(&Text, Window);

				Profiler::DrawProfiling(Window, Text);

				Text.EndTextDrawing();

				Inventory.DrawInventory(Window);

			}

			glFinish();

			if (GetBoolean("slowframes")) {
				Sleep(30);
			}


			Vector3i Pos = Camera.Position;
			Pos.y -= 2;

			auto BlockIdx = World.Chunk->GetBlock(Pos.x, Pos.y, Pos.z);
			auto& Block = Chunk::GetBlock(BlockIdx);

			bool Case = FootSteps.Poll(Camera, Sounds, Window);

			if (Case && Block.SoundMaterialType != ActiveSoundtype && Block.SoundMaterialType != SoundType::NONE) {
				FootSteps.SetActiveMaterial(Block.SoundMaterialType, Sounds);
			}

			Window.GetRawWindow()->display();

			Camera.PrevProject = Camera.Project;
			//Sounds.SetSoundInstanceOrigin("CreeperInstance", Camera.Position - Vector3f(0.0,1.0,0.0));
			FootSteps.Step();
			SoundEffects.PollSoundEffects(Sounds, Window, Camera); 
			Sounds.Update(Camera, Window, World);

		}





	}

}