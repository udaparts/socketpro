#ifndef __ICOS_GEO_IP_DATABASE_H
#define __ICOS_GEO_IP_DATABASE_H

#include <string>
#include "icosdefines.h"

namespace iCOS 
{
	class CGeoSourceData
	{
		typedef struct tagGeoIpDb
		{
			GEOIPRecord *GEOIPDatabase;
			size_t Count;
		} GeoIpDb;

		typedef struct tagGeoLocation
		{
			GEOLocationRecord *GEOIPLocation;
			size_t Count;
		} GeoLocation;

	public:
		//point to source IpDb and location files
		CGeoSourceData(const std::string &pathToGeoIpDbFile, const std::string &pathToGeoLocationFile);
		virtual ~CGeoSourceData();

	public:
		bool IsLoaded() const;
		const GEOLocationRecord* GetLocations(size_t &count) const;
		const GEOIPRecord* GetIps(size_t &count) const;

	private:
		//disable copy constructor and assignment operator
		CGeoSourceData(const CGeoSourceData& geoIpDb);
		CGeoSourceData& operator=(const CGeoSourceData& geoIpDb);

		void LoadGeoIpDb();
		void LoadGeoLocation();

	private:
		GeoIpDb		m_GeoIpDb;
		GeoLocation	m_GeoLocation;
		std::string m_pathIpDb;
		std::string m_pathLocation;
	};

}

#endif