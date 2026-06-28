#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <cppunzip.hpp>
#include <system_error>

#include "beatmap_parser.hh"

namespace fs = std::filesystem;

static void find_local_maps(void);
static bool extract_osz(const std::string& fp);
static fs::path get_data_dir(void);
static void parse_hitobjects(OsuBeatmap* bmp, const std::string& line);
static void parse_general(OsuBeatmap* bmp, const std::string& line);

struct { /* section dispatch table */
	std::string section;
	void        (*fn)(OsuBeatmap* bmp, const std::string& line);
} sections_dispatch[] = { /* TODO implement the rest */
	{"[General]",		parse_general},
	{"[Editor]", 		NULL},
	{"[Metadata]", 		NULL},
	{"[Difficulty]", 	NULL},
	{"[Events]", 		NULL},
	{"[TimingPoints]", 	NULL},
	{"[Colours]", 		NULL},
	{"[HitObjects]", 	parse_hitobjects},
};

static bool extract_osz(const std::string& fp)
{
	std::ifstream file(fp, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "failed to read " << fp << std::endl;
		return false;
	}

	fs::path targetdir = get_data_dir() / "maps";
	std::error_code ec;
	if (!fs::exists(fp, ec)) {
		std::cerr << "file does not exist: " << fp << std::endl;
		return false;
	}

	cppunzip::IStreamFile zipfile(file);
	cppunzip::UnZipper unzipper(zipfile);
	for (auto& entry : unzipper.listFiles()) {
		if (entry.isDir())
			continue;

		fs::path outfp = targetdir / entry.fileName();
		if (outfp.has_parent_path()) {
			fs::create_directories(outfp.parent_path(), ec);
			if (ec) {
				std::cerr << "failed to create directory " << outfp.parent_path().string() 
				          << ": " << ec.message() << std::endl;
				return false;
			}
		}

		/* decompress entry buffer from archive */
		std::vector<uint8_t> dcdata = entry.readContent();
		std::ofstream outstream(outfp, std::ios::binary);
		if (outstream.is_open()) {
			outstream.write((const char*)(dcdata.data()), dcdata.size());

			if (!outstream) {
				std::cerr << "write error while dumping: " << outfp.string() << std::endl;
				outstream.close();
				return false;
			}
			outstream.close();
		}
		else {
			std::cerr << "failed to open/write: " << outfp.string() << std::endl;
			return false;
		}
	}

	return true;
}

static void find_local_maps(void)
{
	fs::path map_dir = get_data_dir() / "maps";

	for (const auto& entry : fs::directory_iterator(map_dir)) {
		if (entry.is_directory()) {
			for (const auto& sub : fs::directory_iterator(entry.path())) {
				if (sub.path().extension() == ".osu")
					std::cout << "found map: " << sub.path().filename() << std::endl;
			}
		}
	}
}

static fs::path get_data_dir(void)
{
	fs::path path;
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	const char* LOCALAPPDATA = std::getenv("LOCALAPPDATA");
	if (LOCALAPPDATA) {
		path = fs::path(LOCALAPPDATA) / "rox";
	}
	else {
		const char* USERPROFILE = std::getenv("USERPROFILE");
		if (USERPROFILE)
			path = fs::path(USERPROFILE) / "AppData" / "Local" / "rox";
	}
#else /* UNIX Systems */
	const char* XDG_DATA_HOME = std::getenv("XDG_DATA_HOME");
	if (XDG_DATA_HOME) {
		path = fs::path(XDG_DATA_HOME) / "rox";
	}
	else {
		const char* HOME = std::getenv("HOME");
		if (HOME)
			path = fs::path(HOME) / ".local" / "share" / "rox";
		else /* fallback to local cache */
			path = fs::current_path() / "rox_cache";
	}
#endif
	std::error_code ec;
	if (!fs::exists(path, ec)) {
		if (ec) {
			std::cerr << "check dir error: " << ec.message() << std::endl;
			return fs::path();
		}

		fs::create_directories(path, ec);
		if (ec) {
			std::cerr << "filesystem setup error: " << ec.message() << std::endl;
			return fs::path();
		}
	}
	return path;
}

static void parse_hitobjects(OsuBeatmap* bmp, const std::string& line)
{
	std::stringstream ss(line);
	std::string s_x;
	std::string s_y;
	std::string s_time;
	std::string s_type;

	/* get first 4 fields (x, y, time, type) */
	if (std::getline(ss, s_x, ',') &&
	    std::getline(ss, s_y, ',') &&
	    std::getline(ss, s_time, ',') &&
	    std::getline(ss, s_type, ','))
	{
		OsuHitObject obj = {
			.x = std::stof(s_x), .y = std::stof(s_y),
			.time_ms = std::stof(s_time), .type = std::stoi(s_type),
		};
		bmp->objects.push_back(obj);
	}
}

static void parse_general(OsuBeatmap* bmp, const std::string& line)
{
	size_t colon_pos = line.find(':');
	if (colon_pos != std::string::npos) {
		std::string key = line.substr(0, colon_pos);
		if (key == "AudioFilename") {
			std::string val = line.substr(colon_pos + 1);

			/* strip leading spaces/tabs */
			size_t first_char = val.find_first_not_of(" \t");
			if (first_char != std::string::npos)
				bmp->audio_fp = val.substr(first_char);
			else
				bmp->audio_fp = val;
		}
	}
}

void load_osu_beatmap(OsuBeatmap* bmp, const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "failed to open beatmap: " << path << std::endl;
		return;
	}

	std::string line;
	std::string section = "";

	bmp->audio_fp.clear();
	bmp->objects.clear();

	while (std::getline(file, line)) {
		/* strip ending rc */
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		/* skip comments/empty line */
		if (line.empty() || line.rfind("//", 0) == 0)
			continue;

		if (line[0] == '[' && line.back() == ']') {
			section = line;
			continue;
		}

		for (auto& i : sections_dispatch) {
			if (section == i.section) {
				if (!i.fn) {
					std::cerr << "yet to implement " << i.section << " parser" << std::endl;
					break;
				}
				i.fn(bmp, line);
			}	
		}
	}
	file.close();
}

