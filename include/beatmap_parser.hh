#ifndef BEATMAP_PARSER_H
#define BEATMAP_PARSER_H

#include <string>
#include <vector>

typedef struct {
	// ...
} OsuHOType; /* hit object */

typedef struct {
	float x;
	float y;
	float time_ms;
	int   type; /* bitmask */
} OsuHitObject;

typedef struct {
	std::string               audio_fp;
	std::vector<OsuHitObject> objects;
} OsuBeatmap;

bool extract_install_osz(const std::string& fp);
std::vector<std::string> find_local_beatmaps(void);
void load_osu_beatmap(OsuBeatmap* bmp, const std::string& path);

#endif /* BEATMAP_PARSER_H */

