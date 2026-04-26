#pragma once
#include <string>

namespace PasswordGenerator
{
	std::string normalizeService(const std::string &service);
	std::string normalizeUser(const std::string &user);
	std::string generateHash(const std::string &data);
	std::string generateHash(const std::string &data, size_t length);
	std::string generateServicePassword(const std::string &masterPw, const std::string &service, const std::string &user);
	std::string generateServicePassword(const std::string &masterPw, const std::string &service, const std::string &user, size_t length);
}
