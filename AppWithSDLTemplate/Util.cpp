#include "Util.h"
#include "Audio.h"
#include "SDL2/SDL.h"
#include "App.h"
#include <fstream>
#include <iostream>

using namespace std;
using namespace App;
using namespace Audio;

bool Util::FileExists(std::string Filepath)
{
	ifstream ifile;
	ifile.open(Filepath);
	if (ifile)
		return true;
	else
		return false;
}

std::string  Util::itos(int i)
{
	return "";
}

std::string  Util::uitos(unsigned int i)
{
	return "";
}

