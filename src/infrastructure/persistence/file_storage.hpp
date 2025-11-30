/**
 * @file file_storage.hpp
 * @brief FileStorage - Adaptador de persistência em arquivo
 * 
 * Clean Architecture: Infrastructure Layer - Adapter
 */

#ifndef INFRASTRUCTURE_PERSISTENCE_FILE_STORAGE_HPP
#define INFRASTRUCTURE_PERSISTENCE_FILE_STORAGE_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cstdlib>
#include "application/ports/storage_port.hpp"

namespace Snake {
namespace Infrastructure {
namespace Persistence {

/**
 * @brief Storage usando arquivos locais
 */
class FileStorage : public Application::Ports::IStorage {
public:
    FileStorage() {
        // Define caminho base
        const char* home = getenv("HOME");
        if (home) {
            basePath_ = std::string(home) + "/.tsnake/";
        } else {
            basePath_ = "./";
        }
        
        // Tenta criar diretório
        ensureDirectory();
    }
    
    explicit FileStorage(const std::string& basePath) : basePath_(basePath) {
        ensureDirectory();
    }
    
    bool saveHighscore(uint32_t score) override {
        return saveData("highscore", std::to_string(score));
    }
    
    uint32_t loadHighscore() override {
        std::string data = loadData("highscore");
        if (data.empty()) return 0;
        
        try {
            return static_cast<uint32_t>(std::stoul(data));
        } catch (...) {
            return 0;
        }
    }
    
    bool saveData(const std::string& key, const std::string& value) override {
        std::string path = basePath_ + sanitizeKey(key);
        std::ofstream file(path);
        if (!file.is_open()) return false;
        
        file << value;
        return file.good();
    }
    
    std::string loadData(const std::string& key) override {
        std::string path = basePath_ + sanitizeKey(key);
        std::ifstream file(path);
        if (!file.is_open()) return "";
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    bool hasKey(const std::string& key) override {
        std::string path = basePath_ + sanitizeKey(key);
        std::ifstream file(path);
        return file.good();
    }
    
    bool removeData(const std::string& key) override {
        std::string path = basePath_ + sanitizeKey(key);
        return std::remove(path.c_str()) == 0;
    }

private:
    void ensureDirectory() {
        // Cria diretório se não existir (Linux/macOS)
        std::string cmd = "mkdir -p " + basePath_ + " 2>/dev/null";
        (void)system(cmd.c_str());
    }
    
    std::string sanitizeKey(const std::string& key) {
        std::string result;
        for (char c : key) {
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || c == '-') {
                result += c;
            } else {
                result += '_';
            }
        }
        return result + ".dat";
    }
    
    std::string basePath_;
};

} // namespace Persistence
} // namespace Infrastructure
} // namespace Snake

#endif // INFRASTRUCTURE_PERSISTENCE_FILE_STORAGE_HPP
