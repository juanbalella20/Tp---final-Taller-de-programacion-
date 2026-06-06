#ifndef GAME_CONFIG_H_
#define GAME_CONFIG_H_

#include <string>

class GameConfig {
public:
    // Level
    double xp_base;
    double xp_exponent;
    int    xp_level_offset;
    double xp_kill_factor;
    double gold_base;
    double gold_exponent;
    double gold_excess_factor;
    int    newbie_max_level;
    int    level_diff_max;

    static GameConfig& instance();
    void load(const std::string& toml_path);

private:
    GameConfig() = default;
    GameConfig(const GameConfig&) = delete;
    GameConfig& operator=(const GameConfig&) = delete;
};

#endif