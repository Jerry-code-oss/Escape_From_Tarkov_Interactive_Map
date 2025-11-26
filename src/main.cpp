#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace eft {
struct Config {
    std::filesystem::path imagePath;
    int x{0};
    int y{0};
};

class ConfigParser {
public:
    static Config load(const std::filesystem::path& filePath) {
        Config cfg;
        std::map<std::string, std::string> entries;

        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开配置文件: " + filePath.string());
        }

        std::string line;
        size_t lineNumber = 0;
        while (std::getline(file, line)) {
            ++lineNumber;
            // Skip empty lines and comments starting with '#'.
            if (line.empty() || line[0] == '#') {
                continue;
            }

            const auto equals = line.find('=');
            if (equals == std::string::npos) {
                std::ostringstream oss;
                oss << "配置文件第 " << lineNumber << " 行缺少 '=' 分隔符";
                throw std::runtime_error(oss.str());
            }

            auto key = line.substr(0, equals);
            auto value = line.substr(equals + 1);

            // Trim whitespace.
            trim(key);
            trim(value);

            entries[key] = value;
        }

        if (entries.count("image_path") == 0) {
            throw std::runtime_error("配置缺少 image_path 项");
        }
        if (entries.count("x") == 0 || entries.count("y") == 0) {
            throw std::runtime_error("配置缺少 x 或 y 坐标");
        }

        cfg.imagePath = entries.at("image_path");
        cfg.x = std::stoi(entries.at("x"));
        cfg.y = std::stoi(entries.at("y"));

        return cfg;
    }

private:
    static void trim(std::string& text) {
        const auto first = text.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            text.clear();
            return;
        }
        const auto last = text.find_last_not_of(" \t\r\n");
        text = text.substr(first, last - first + 1);
    }
};

class CoastlineMap {
public:
    CoastlineMap(int width, int height)
        : width_(width), height_(height) {
        generateCoastline();
    }

    bool inBounds(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    void render(int playerX, int playerY) const {
        for (int row = 0; row < height_; ++row) {
            for (int col = 0; col < width_; ++col) {
                if (col == playerX && row == playerY) {
                    std::cout << 'P';
                    continue;
                }

                std::cout << terrain_[row][col];
            }
            std::cout << '\n';
        }
    }

    int width() const { return width_; }
    int height() const { return height_; }

private:
    int width_;
    int height_;
    std::vector<std::vector<char>> terrain_;

    void generateCoastline() {
        terrain_.assign(height_, std::vector<char>(width_, '~')); // Water by default.

        // Create a simple wavy coastline using a sine-like pattern.
        for (int row = 0; row < height_; ++row) {
            int coastline = static_cast<int>(width_ * 0.3 + (std::sin(row / 3.0) * width_ * 0.05));
            for (int col = coastline; col < width_; ++col) {
                terrain_[row][col] = '#'; // Land
            }
        }

        // Add some points of interest for visualization.
        const std::vector<std::pair<int, int>> markers = {
            {width_ - 5, height_ / 4},
            {width_ - 8, height_ / 2},
            {width_ - 3, height_ * 3 / 4}
        };
        for (const auto& [col, row] : markers) {
            if (inBounds(col, row)) {
                terrain_[row][col] = '*';
            }
        }
    }
};

} // namespace eft

void printUsage(const std::string& programName) {
    std::cout << "用法: " << programName << " <配置文件路径>\n\n"
              << "配置文件格式 (键值对):\n"
              << "  image_path=/absolute/or/relative/path/to/shoreline.jpg\n"
              << "  x=玩家在地图上的列坐标 (0 起始)\n"
              << "  y=玩家在地图上的行坐标 (0 起始)\n\n"
              << "示例:\n"
              << "  image_path=assets/shoreline_reference.jpg\n"
              << "  x=12\n"
              << "  y=6\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    try {
        const auto config = eft::ConfigParser::load(argv[1]);

        // Validate image path exists.
        if (!std::filesystem::exists(config.imagePath)) {
            std::cerr << "警告: 无法找到配置的地图图片: " << config.imagePath << "\n";
        }

        eft::CoastlineMap map(40, 20);

        if (!map.inBounds(config.x, config.y)) {
            std::ostringstream oss;
            oss << "玩家坐标超出地图范围。有效范围: x ∈ [0, " << map.width() - 1
                << "], y ∈ [0, " << map.height() - 1 << "]";
            throw std::runtime_error(oss.str());
        }

        std::cout << "==== 逃离塔科夫 - 海岸线原型 ====" << '\n';
        std::cout << "地图图片路径: " << std::filesystem::absolute(config.imagePath) << '\n';
        std::cout << "玩家坐标: (" << config.x << ", " << config.y << ")" << '\n';
        std::cout << "---------------------------------" << '\n';

        map.render(config.x, config.y);

        std::cout << "---------------------------------" << '\n';
        std::cout << "P = 你的位置, '#' = 陆地, '~' = 水域, '*' = 地标" << '\n';
    } catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
