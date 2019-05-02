#include "stdafx.h"
#include "GeoSourceData.h"
#include <fstream>

namespace iCOS {

	CGeoSourceData::CGeoSourceData(const std::string &pathToGeoIpDbFile, const std::string &pathToGeoLocationFile)
		: m_pathIpDb(pathToGeoIpDbFile), m_pathLocation(pathToGeoLocationFile)
	{
		::memset(&m_GeoIpDb, 0, sizeof(m_GeoIpDb));
		::memset(&m_GeoLocation, 0, sizeof(m_GeoLocation));

		LoadGeoIpDb();
		LoadGeoLocation();
	}

	CGeoSourceData::~CGeoSourceData(void)
	{
		if (m_GeoLocation.GEOIPLocation != nullptr)
			::free(m_GeoLocation.GEOIPLocation); 
		if (m_GeoIpDb.GEOIPDatabase != nullptr)
			::free(m_GeoIpDb.GEOIPDatabase);
	}

	bool CGeoSourceData::IsLoaded() const
	{
		return (m_GeoLocation.GEOIPLocation != nullptr && m_GeoIpDb.GEOIPDatabase != nullptr);
	}

	const GEOLocationRecord* CGeoSourceData::GetLocations(size_t &count) const 
	{
		count = m_GeoLocation.Count;
		return m_GeoLocation.GEOIPLocation;
	}

	const GEOIPRecord* CGeoSourceData::GetIps(size_t &count) const
	{
		count = m_GeoIpDb.Count;
		return m_GeoIpDb.GEOIPDatabase;
	}

	void CGeoSourceData::LoadGeoIpDb()
	{
		//Load source IpDb from a SQL database in the near future by MS OLEDB technology (http://www.udaparts.com/oledbpro.zip)?
		
		//I implement it as we load source ip data from a file
		std::ifstream file(m_pathIpDb, std::ios::in | std::ios::binary);
		if (file.is_open())
		{
			file.seekg(0, std::ifstream::end);
			size_t bytes = (size_t)file.tellg();
			assert(bytes % sizeof(GEOIPRecord) == 0);
			file.seekg(0, std::ifstream::beg);
			m_GeoIpDb.GEOIPDatabase = (GEOIPRecord*)::malloc(bytes + EXTRABYTES);
			do
			{
				if (m_GeoIpDb.GEOIPDatabase == nullptr)
					break;
				if (!file.read((char*)m_GeoIpDb.GEOIPDatabase, bytes))
					break;
#ifdef _DEBUG
				size_t read = (size_t)file.gcount();
				assert(read == bytes);
#endif
				m_GeoIpDb.Count = bytes/sizeof(GEOIPRecord);
				return;
			}while(false);
			if (m_GeoIpDb.GEOIPDatabase != nullptr)
			{
				::free(m_GeoIpDb.GEOIPDatabase);
				m_GeoIpDb.GEOIPDatabase = nullptr;
			}
		}
	}

	void CGeoSourceData::LoadGeoLocation()
	{
		//Load source locations from a SQL database in the near future by MS OLEDB technology (http://www.udaparts.com/oledbpro.zip)?

		//I implement it as we load source location data from a file
		std::ifstream file(m_pathLocation, std::ios::in | std::ios::binary);
		if (file.is_open())
		{
			file.seekg(0, std::ifstream::end);
			size_t bytes = (size_t)file.tellg();
			assert(bytes % sizeof(GEOLocationRecord) == 0);
			file.seekg(0, std::ifstream::beg);
			m_GeoLocation.GEOIPLocation = (GEOLocationRecord*)::malloc(bytes + EXTRABYTES);
			do
			{
				if (m_GeoLocation.GEOIPLocation == nullptr)
					break;
				if (!file.read((char*)m_GeoLocation.GEOIPLocation, bytes))
					break;
#ifdef _DEBUG
				size_t read = (size_t)file.gcount();
				assert(read == bytes);
#endif
				m_GeoLocation.Count = bytes/sizeof(GEOLocationRecord);
				return;
			}while(false);
			if (m_GeoLocation.GEOIPLocation != nullptr)
			{
				::free(m_GeoLocation.GEOIPLocation);
				m_GeoLocation.GEOIPLocation = nullptr;
			}
		}
	}
}