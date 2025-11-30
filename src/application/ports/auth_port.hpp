/**
 * @file auth_port.hpp
 * @brief IAuth - Interface para autenticação
 * 
 * Clean Architecture: Application Layer - Output Port
 */

#ifndef APPLICATION_PORTS_AUTH_PORT_HPP
#define APPLICATION_PORTS_AUTH_PORT_HPP

#include <string>

namespace Snake {
namespace Application {
namespace Ports {

/**
 * @brief Authenticated user information
 */
struct UserInfo {
    std::string odUserId;
    std::string displayName;
    std::string email;
    std::string photoUrl;
};

/**
 * @brief Resultado da autenticação
 */
struct AuthResult {
    bool success;
    std::string errorMessage;
    UserInfo user;
};

/**
 * @brief Interface para autenticação
 * 
 * Abstrai:
 * - Google OAuth Device Flow
 * - Firebase Auth
 * - etc.
 */
class IAuth {
public:
    virtual ~IAuth() = default;
    
    /**
     * @brief Check if authenticated
     */
    virtual bool isAuthenticated() const = 0;
    
    /**
     * @brief Start authentication flow
     * @return URL and code to display to user
     */
    virtual AuthResult startDeviceFlow() = 0;
    
    /**
     * @brief Aguarda conclusão da autenticação
     */
    virtual AuthResult waitForAuth() = 0;
    
    /**
     * @brief Logout
     */
    virtual void logout() = 0;
    
    /**
     * @brief Return user information
     */
    virtual UserInfo getCurrentUser() const = 0;
    
    /**
     * @brief Return access token for APIs
     */
    virtual std::string getAccessToken() const = 0;
    
    /**
     * @brief Renova token se necessário
     */
    virtual bool refreshTokenIfNeeded() = 0;
};

} // namespace Ports
} // namespace Application
} // namespace Snake

#endif // APPLICATION_PORTS_AUTH_PORT_HPP
