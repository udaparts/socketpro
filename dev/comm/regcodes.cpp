static bool reg = false;
			if (!reg) {
				SPA::ServerSide::ServerCoreLoader.RegisterMe(SPA::Sqlite::sidSqlite, 0x6f00AA00BB0000CC);
				reg = true;
			}
			
static bool reg = false;
			if (!reg) {
				SPA::ServerSide::ServerCoreLoader.RegisterMe(SPA::Mysql::sidMysql, 0x7f00110022003300);
				reg = true;
			}
			
static bool reg = false;
			if (!reg) {
				SPA::ServerSide::ServerCoreLoader.RegisterMe(SPA::Queue::sidQueue, 0x5f00240039078012);
				reg = true;
			}

static bool reg = false;
			if (!reg) {
				SPA::ServerSide::ServerCoreLoader.RegisterMe(SPA::sidFile, 0x4f00990088000077);
				reg = true;
			}

b2 runtime-link=static address-model=32

b2 runtime-link=static address-model=64

b2 --toolset=msvc-10.0 define=BOOST_USE_WINAPI_VERSION=0x0501 runtime-link=static address-model=32

b2 --toolset=msvc-10.0 define=BOOST_USE_WINAPI_VERSION=0x0501 runtime-link=static address-model=64