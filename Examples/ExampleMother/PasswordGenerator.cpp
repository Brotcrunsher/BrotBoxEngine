#include "PasswordGenerator.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <sodium/crypto_pwhash_argon2id.h>

namespace
{
	std::string replaceAll(std::string str, const std::string &from, const std::string &to)
	{
		size_t pos = 0;
		while ((pos = str.find(from, pos)) != std::string::npos)
		{
			str.replace(pos, from.size(), to);
			pos += to.size();
		}
		return str;
	}
}

std::string PasswordGenerator::normalizeService(const std::string &service)
{
	std::string s = service;
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	s = replaceAll(s, " ", "");
	s = replaceAll(s, "http://", "");
	s = replaceAll(s, "https://", "");
	auto slashPos = s.find('/');
	if (slashPos != std::string::npos)
	{
		s = s.substr(0, slashPos);
	}
	size_t dotCount = 0;
	for (char c : s) { if (c == '.') dotCount++; }
	if (dotCount > 1)
	{
		std::vector<std::string> tokens;
		size_t start = 0;
		size_t end;
		while ((end = s.find('.', start)) != std::string::npos)
		{
			tokens.push_back(s.substr(start, end - start));
			start = end + 1;
		}
		tokens.push_back(s.substr(start));
		s = tokens[tokens.size() - 2] + "." + tokens[tokens.size() - 1];
	}
	return s;
}

std::string PasswordGenerator::normalizeUser(const std::string &user)
{
	std::string s = user;
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	s = replaceAll(s, " ", "");
	return s;
}

std::string PasswordGenerator::generateHash(const std::string &data)
{
	unsigned char hash[16] = {};
	const int err = crypto_pwhash_argon2id(hash, sizeof(hash), data.c_str(), data.size(), (const unsigned char *)"BrotbEnginePWGv1", 5, static_cast<size_t>(256 * 1024 * 1024), crypto_pwhash_argon2id_ALG_ARGON2ID13);
	if (err != 0)
	{
		std::cerr << "FATAL: crypto_pwhash_argon2id failed." << std::endl;
		std::abort();
	}
	const std::string lower = "abcdefghijklmnopqrstuvwxyz";
	const std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const std::string digits = "0123456789";
	const std::string special = "!@#%*-_=+,.?";
	const std::string all = lower + upper + digits + special;
	std::string retVal;
	for (size_t i = 0; i < sizeof(hash); i++)
	{
		const std::string *set = nullptr;
		// The first char is always lower, second always upper and so on. This is a very simple way to ensure
		// we fulfill the vast majority of pw requirements while not reducing the amount of entropy by a lot.
		if (i == 0)
			set = &lower;
		else if (i == 1)
			set = &upper;
		else if (i == 2)
			set = &digits;
		else if (i == 3)
			set = &special;
		else
			set = &all;

		retVal += (*set)[hash[i] % set->size()];
	}
	return retVal;
}

std::string PasswordGenerator::generateServicePassword(const std::string &masterPw, const std::string &service, const std::string &user)
{
	std::string normServ = normalizeService(service);
	std::string normUser = normalizeUser(user);
	std::string hashableString = masterPw + "|||" + normServ;
	if (!normUser.empty())
		hashableString += "|||" + normUser;
	return generateHash(hashableString);
}
