#pragma once
#include <QString>
#include <optional>

class UserSession {
public:
    static UserSession& instance() {
        static UserSession inst;
        return inst;
    }

    void login(const QString& username) { currentUsername = username; }
    void logout() { currentUsername.reset(); }

    bool isLoggedIn() const { return currentUsername.has_value(); }
    QString username() const { return currentUsername.value_or(""); }

private:
    std::optional<QString> currentUsername;
    UserSession() = default;
    UserSession(const UserSession&) = delete;
    UserSession& operator=(const UserSession&) = delete;
};
