#include "utility/Version.h"

#include <vector>
#include <sstream>

namespace {
	template <typename ContainerType>
	ContainerType split(const std::string& str, const std::string& delimiter)
	{
		size_t pos = 0;
		size_t oldPos = 0;
		ContainerType c;

		do
		{
			pos = str.find(delimiter, oldPos);
			c.push_back(str.substr(oldPos, pos - oldPos));
			oldPos = pos + delimiter.size();
		}
		while (pos != std::string::npos);

		return c;
	}
}


Version Version::s_version;

Version Version::fromString(const std::string& versionString)
{
	try
	{
		Version version;
		std::vector<std::string> parts = split<std::vector<std::string>>(versionString, ".");

		if (parts.size())
		{
			version.m_year = std::stoi(parts[0]);
		}

		if (parts.size() > 1)
		{
			version.m_minorNumber = std::stoi(parts[1]);
		}

		if (parts.size() > 2)
		{
			std::vector<std::string> hashParts = split<std::vector<std::string>>(parts[2], "-");
			if (hashParts.size())
			{
				version.m_commitNumber = std::stoi(hashParts[0]);
			}

			if (hashParts.size() > 1)
			{
				version.m_commitHash = hashParts[1];
			}
		}

		return version;
	}
	catch (std::invalid_argument e)
	{
		// LOG_ERROR("Version string is invalid: " + versionString);
	}

	return Version();
}

bool Version::operator<(const Version& other) const
{
	if (m_year != other.m_year)
	{
		return m_year < other.m_year;
	}
	else if (m_minorNumber != other.m_minorNumber)
	{
		return m_minorNumber < other.m_minorNumber;
	}
	else
	{
		return m_commitNumber < other.m_commitNumber;
	}
}

bool Version::operator>(const Version& other) const
{
	if (m_year != other.m_year)
	{
		return m_year > other.m_year;
	}
	else if (m_minorNumber != other.m_minorNumber)
	{
		return m_minorNumber > other.m_minorNumber;
	}
	else
	{
		return m_commitNumber > other.m_commitNumber;
	}
}

Version& Version::operator+=(const int& number)
{
	int minor = this->m_minorNumber - 1 + number;
	this->m_year += minor/4;
	this->m_minorNumber = (minor%4) + 1;
	return *this;
}

bool Version::isValid()
{
	if (m_minorNumber < 5 && m_minorNumber > 0
			&& m_year > 2016)
	{
		return true;
	}
	return false;
}

void Version::setApplicationVersion(const Version& version)
{
	s_version = version;
}

const Version& Version::getApplicationVersion()
{
	return s_version;
}

Version::Version(int year, int minor, int commit, const std::string& hash)
	: m_year(year)
	, m_minorNumber(minor)
	, m_commitNumber(commit)
	, m_commitHash(hash)
{
}

bool Version::isEmpty() const
{
	return m_year == 0 && m_minorNumber == 0 && m_commitNumber == 0;
}

std::string Version::toShortString() const
{
	std::stringstream ss;
	ss << m_year << '.' << m_minorNumber;
	return ss.str();
}

std::string Version::toString() const
{
	std::stringstream ss;
	ss << m_year << '.' << m_minorNumber << '-' << m_commitNumber << '-' << m_commitHash;
	return ss.str();
}

std::string Version::toDisplayString() const
{
	std::stringstream ss;
	ss << m_year << '.' << m_minorNumber << '.' << m_commitNumber;
	return ss.str();
}