#include "App.h"
#include "Util.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"
#include "spdlog/spdlog.h"
#include <filesystem>

using namespace std;
using namespace App;
using namespace Error;
using namespace Util;

using enum CommandLineArgumentType;

App::CommandLineSettings::CommandLineSettings()
{
	Filepath = "";
	ConfigPath = "config.cfg";
	LogLevel = 0;
}


CommandLineSettings ParseCommandLine(CommandLine CmdLine)
{
	CommandLineSettings Output;

	for (auto& Argument : CmdLine.Arguments)
		if (Argument.Type == ApplicationPath)
		{
			Output.Filepath = Argument.Data;
		}

		else if (Argument.Data == "-cfg")
		{
			if (Argument.Parameters.size() != 1)
			{
				//error - no parameters
				Output.Errors.AddError("-cfg: No Filepath specified");
				continue;
			}

			Output.ConfigPath = Argument.Parameters[0];
		}

		else if (Argument.Data == "-log")
		{
			if (Argument.Parameters.size() != 1)
			{
				//error - no parameters
				Output.Errors.AddError("-log: no log level specified (should be a number, 0 - 6)");
				continue;
			}

			auto IntBuff = stoi(Argument.Parameters[0]);
			if (IntBuff < 0 || IntBuff > 6)
			{
				//invalid log level bro
				Output.Errors.AddError("-log: Invalid log level, should be a number 0 to 6, got: " + Argument.Parameters[0]);
				continue;
			}

			Output.LogLevel = IntBuff;
		}

		else
		{
			//unknown parameter - error
			Output.Errors.AddError("Invalid argument: " + Argument.Data);
		}

	return Output;
}

App::ConfigParser::ConfigParser(std::string _Filepath)
{
	Filepath = _Filepath;
	
	if (!FileExists(Filepath))
	{
		Errors.AddError("Missing file @ " + Filepath);
		return;
	}
}

Config App::ConfigParser::Parse()
{
	if(Errors.HasErrored())
		return Config();

	Config Output;


	
	return Output;
}

App::Application::Application(int argc, char* argv[]) : CmdLine(argc, argv), IO(Data)
{
	spdlog::set_level(spdlog::level::level_enum::trace);
	srand(time(0));

	spdlog::info("Application launch");
	spdlog::trace("Running dir " + std::filesystem::current_path().string());

	spdlog::debug("Parsing command line");
	auto Settings = ParseCommandLine(CmdLine);
	if(Settings.Errors.HasErrored())
		spdlog::debug("Parsing command line");
	
	/*
	* Config parser - TODO
	spdlog::debug("Opening config " + Settings.ConfigPath);
	ConfigParser CfgParser(Settings.ConfigPath);
	if (CfgParser.Errors.HasErrored())
	{
		//error with config file, use exist defaults
		spdlog::warn("Errors in config @ " + Settings.ConfigPath);
	}
	*/
	Data.UpdateFromConfig(Cfg);
}

bool App::Application::Ended() const
{
	return Data.Halted;
}

bool SDLInit(AppData& Data, Config& Cfg)
{
	spdlog::info("SDL Init");

	spdlog::debug("SDL Init - SDL_Init");
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		spdlog::critical("SDL init error: {}", SDL_GetError());
		return false;
	}

	spdlog::debug("SDL Init - Creating main window {}, {}", Cfg.ScreenWidth, Cfg.ScreenHeight);
	Data.RenderData.MainWindow = SDL_CreateWindow("SportsRace", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Cfg.ScreenWidth, Cfg.ScreenHeight, SDL_WINDOW_SHOWN);
	if (Data.RenderData.MainWindow == nullptr)
	{
		spdlog::critical("SDL init error SDL_CreateWindow(): {}", SDL_GetError());
		return false;
	}
	Data.RenderData.MainSurface = SDL_GetWindowSurface(Data.RenderData.MainWindow);

	//Create renderer for window
	spdlog::debug("SDL Init - Creating renderer");
	if ((Data.RenderData.MainRenderer = SDL_CreateRenderer(Data.RenderData.MainWindow, -1, SDL_RENDERER_ACCELERATED)) == nullptr)
	{
		spdlog::critical("SDL init error SDL_CreateRenderer(): {}", SDL_GetError());
		return false;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(Data.RenderData.MainRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	spdlog::debug("SDL Init - Starting IMG loader ");
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		spdlog::critical("SDL init error IMG_Init(): {}", SDL_GetError());
		return false;
	}

	//init TTF
	spdlog::debug("SDL Init - Init TTF");
	if (TTF_Init() == -1)
	{
		spdlog::critical("SDL init error TTF_Init(): {}", SDL_GetError());
		return false;
	}

	spdlog::debug("SDL Init - Starting Audio {}hz, {}ch, ({})chunksize", 44100, 2, 2048);
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		spdlog::warn("SDL Init - Warning, could not start audio: {}", SDL_GetError());
	}

	return true;
}

bool SDLClose(AppData& Data)
{
	spdlog::debug("SDL Close");

	//Destroy window and renderer
	SDL_DestroyRenderer(Data.RenderData.MainRenderer);
	SDL_DestroyWindow(Data.RenderData.MainWindow);
	Data.RenderData.MainRenderer = nullptr;
	Data.RenderData.MainWindow = nullptr;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	return true;
}

bool App::Application::Init()
{
	spdlog::info("Application init");

	//Init SDL
	if (!SDLInit(Data, Cfg))
		return false;

	srand(time(0));

	spdlog::debug("IO Init");
	IO.Init();

	spdlog::debug("Starting AppStateMachine: Pushing MainMenuScreen");

	return true;
}

void App::Application::Update()
{
	IO.Update();
}

App::AppIO::AppIO(AppData& _Data) : Data(_Data)
{

}

bool App::AppIO::Init()
{
	return Player.Init();
}

void App::AppIO::Update()
{
	Esc = false;
	MouseButtons = 0;

	//handle input
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
		switch (Event.type)
		{
		case SDL_KEYDOWN:
			// Handle any key presses here.
			if (Event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				Esc = true;
			break;

		case SDL_MOUSEBUTTONDOWN:
			// Handle mouse clicks here.
			MouseButtons = Event.button.button;
			break;

		case SDL_QUIT:
			Data.Halted = true;
			break;
		}

	//MouseButtons =
	SDL_GetMouseState(&MousePosition.x, &MousePosition.y);
}

App::AppData::AppData()
{
}

void App::AppData::UpdateFromConfig(Config Cfg)
{
	ScreenWidth = Cfg.ScreenWidth; ScreenHeight = Cfg.ScreenHeight;
	ShowFPS = Cfg.ShowFPS;
}


/*
Takes the standard C/C++ char* array, returns a vector of CommandLineArgument objects
Arguments have the following syntax
	-argument
	/argument

Any argument may have unlimited parameters
	-argument param1
	/argument param1 param2

	eg. "app.exe -filepath c:/path/file.csv"
	eg. "app.exe /move c:/src/file.csv c:/dst/file.csv"

Free floating parameters have no parameters
	argument1 argument2

	eg "app.exe hidelaucher showfps"
*/
App::CommandLine::CommandLine(int argc, char* argv[])
{
	CommandLineArgument arg;
	bool parsingParams = false;
	int index = 0;
	while (index < argc)
	{
		switch (argv[index][0])
		{
		case '//':
		case '-':	//command			
			if (parsingParams)
			{
				Arguments.push_back(arg);
				arg = CommandLineArgument();
			}

			if (argv[index][0] == '--')
				arg.Type = OptionalArgument;
			else
				arg.Type = Argument;

			arg.Data = string(argv[index]).substr(1, strlen(argv[index]) - 1);
			parsingParams = true;
			break;

			//parameter
		default:
			arg.Parameters.push_back(argv[index]);
			if (!parsingParams)
			{
				//the first item is always the exe location
				arg.Type = index == 0 ? ApplicationPath : FreeFloatingParameter;
				arg.Data = argv[index];
				Arguments.push_back(arg);
				arg = CommandLineArgument();
			}
			break;
		}
		index++;
	}

	if (parsingParams)
		Arguments.push_back(arg);
}
