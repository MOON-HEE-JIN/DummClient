#include "CHttpClient.h"
#include "../Log/CLog.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <memory>

CHttpClient::CHttpClient()
	: m_bInitialized(curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK)
{
}

CHttpClient::~CHttpClient()
{
	if (m_bInitialized)
		curl_global_cleanup();
}

size_t CHttpClient::WriteResponseCallback(char* contents, size_t size, size_t count, void* output) noexcept
{
	const size_t bytes = size * count;
	try
	{
		static_cast<std::string*>(output)->append(contents, bytes);
		return bytes;
	}
	catch (...)
	{
		return 0;
	}
}

bool CHttpClient::DummyLogin(const std::string& id, const std::string& password, long long& tokenValue) const
{
	using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
	using HeaderList = std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>;
	CurlHandle curl(m_bInitialized ? curl_easy_init() : nullptr, &curl_easy_cleanup);
	HeaderList headers(curl_slist_append(nullptr, "Content-Type: application/json"), &curl_slist_free_all);
	if (!curl || !headers)
	{
		g_LogDummy.ELog("DummyLogin[%s] failed to initialize HTTP client", id.c_str());
		return false;
	}

	const nlohmann::json request = { {"id", id}, {"password", password} };
	const std::string body = request.dump();
	std::string response;
	curl_easy_setopt(curl.get(), CURLOPT_URL, "https://localhost:5080/api/clientconnection/dummylogin");
	curl_easy_setopt(curl.get(), CURLOPT_CAINFO, R"(F:\certs\loginserver-new.crt)");
	curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
	curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, body.c_str());
	curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT_MS, 3000L);
	curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 5000L);
	curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteResponseCallback);
	curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);

	const CURLcode result = curl_easy_perform(curl.get());
	if (result != CURLE_OK)
	{
		g_LogDummy.ELog("DummyLogin[%s] failed: %s", id.c_str(), curl_easy_strerror(result));
		return false;
	}

	long status = 0;
	curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status);
	const auto json = nlohmann::json::parse(response, nullptr, false);
	if (status != 200 || !json.is_object() || !json.contains("success") || json["success"] != true
		|| !json.contains("tokenValue") || !json["tokenValue"].is_number_integer() || json["tokenValue"] <= 0)
	{
		g_LogDummy.ELog("DummyLogin[%s] failed (HTTP %ld): invalid login response", id.c_str(), status);
		return false;
	}
	tokenValue = json["tokenValue"].get<long long>();
	return true;
}
