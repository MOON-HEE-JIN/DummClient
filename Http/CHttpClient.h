#pragma once

#include <string>

class CHttpClient
{
public:
	CHttpClient();
	~CHttpClient();

	bool DummyLogin(const std::string& id, const std::string& password, long long& tokenValue) const;

private:
	bool m_bInitialized;
	static size_t WriteResponseCallback(char* contents, size_t size, size_t count, void* output) noexcept;
};
