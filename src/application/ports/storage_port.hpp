/**
 * @file storage_port.hpp
 * @brief IStorage - Interface para persistência de dados
 * 
 * Clean Architecture: Application Layer - Output Port
 */

#ifndef APPLICATION_PORTS_STORAGE_PORT_HPP
#define APPLICATION_PORTS_STORAGE_PORT_HPP

#include <cstdint>
#include <string>

namespace Snake {
namespace Application {
namespace Ports {

/**
 * @brief Interface para persistência local
 * 
 * Abstrai:
 * - Arquivo (desktop)
 * - EEPROM (Arduino)
 * - LocalStorage (Web)
 */
class IStorage {
public:
    virtual ~IStorage() = default;
    
    /**
     * @brief Salva highscore local
     */
    virtual bool saveHighscore(uint32_t score) = 0;
    
    /**
     * @brief Carrega highscore local
     */
    virtual uint32_t loadHighscore() = 0;
    
    /**
     * @brief Salva dados genéricos
     */
    virtual bool saveData(const std::string& key, const std::string& value) = 0;
    
    /**
     * @brief Carrega dados genéricos
     */
    virtual std::string loadData(const std::string& key) = 0;
    
    /**
     * @brief Check if key exists
     */
    virtual bool hasKey(const std::string& key) = 0;
    
    /**
     * @brief Remove dados
     */
    virtual bool removeData(const std::string& key) = 0;
};

} // namespace Ports
} // namespace Application
} // namespace Snake

#endif // APPLICATION_PORTS_STORAGE_PORT_HPP
