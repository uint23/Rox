#include <iostream>
#include <fstream>
#include <sstream>

#include "beatmap_parser.hh"

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
					continue;
				}
				i.fn(bmp, line);
			}	
		}
	}
	file.close();
}

